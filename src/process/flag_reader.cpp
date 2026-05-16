#include "process/flag_reader.hpp"

#include <algorithm>
#include <cstring>
#include <limits>

namespace fflag::process {

namespace {

struct Region {
    std::uint64_t             base = 0;
    std::vector<std::uint8_t> bytes;

    const std::uint8_t* slice(std::uint64_t addr, std::size_t n) const {
        if (addr < base) return nullptr;
        const auto off = addr - base;
        if (off + n > bytes.size()) return nullptr;
        return bytes.data() + static_cast<std::size_t>(off);
    }
};


// Single ReadProcessMemory over [min_off, max_off + 64). If that fails, chunked
// reads claw back what we can — any unreadable page stays zeroed.
bool bulk_read(HANDLE proc, std::uint64_t base,
               const flags::Offsets& offsets, Region& region) {
    if (offsets.flags.empty()) return false;

    auto          min_off = std::numeric_limits<std::uint64_t>::max();
    std::uint64_t max_off = 0;
    for (const auto& kv : offsets.flags) {
        min_off = std::min(min_off, kv.second);
        max_off = std::max(max_off, kv.second);
    }

    region.base = base + min_off;
    const std::size_t span = static_cast<std::size_t>(max_off - min_off) + 64;
    region.bytes.assign(span, 0);

    SIZE_T got = 0;
    if (ReadProcessMemory(proc, reinterpret_cast<LPCVOID>(region.base),
                          region.bytes.data(), span, &got) && got == span)
        return true;

    constexpr std::size_t kChunk = 1u << 20;
    for (std::size_t pos = 0; pos < span; pos += kChunk) {
        const std::size_t n = std::min(kChunk, span - pos);
        SIZE_T g = 0;
        ReadProcessMemory(proc, reinterpret_cast<LPCVOID>(region.base + pos),
                          region.bytes.data() + pos, n, &g);
    }
    return true;
}

}


std::vector<DumpedFlag> read_all_flags(HANDLE proc, std::uint64_t base,
                                       const flags::Offsets& offsets,
                                       ReadStats& stats) {
    std::vector<DumpedFlag> out;
    Region region;
    if (!bulk_read(proc, base, offsets, region)) return out;

    out.reserve(offsets.flags.size());
    for (const auto& [name, off] : offsets.flags) {
        const auto* cell = region.slice(base + off, 4);
        if (!cell) { ++stats.skipped; continue; }

        std::uint32_t raw = 0;
        std::memcpy(&raw, cell, sizeof(raw));
        out.push_back({name, raw});
    }
    return out;
}

}
