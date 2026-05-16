#include "flags/baseline.hpp"

#include <algorithm>
#include <cstdio>
#include <fstream>

namespace fflag::flags {

namespace {

std::optional<std::uint32_t> parse_hex_u32(std::string_view s) {
    if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s.remove_prefix(2);
    if (s.empty()) return std::nullopt;

    std::uint32_t v = 0;
    for (char c : s) {
        int d;
        if      (c >= '0' && c <= '9') d = c - '0';
        else if (c >= 'a' && c <= 'f') d = 10 + (c - 'a');
        else if (c >= 'A' && c <= 'F') d = 10 + (c - 'A');
        else break;
        v = (v << 4) | static_cast<std::uint32_t>(d);
    }
    return v;
}

}


std::optional<Baseline> read_baseline(const std::filesystem::path& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return std::nullopt;

    Baseline out;
    std::string line;

    while (std::getline(f, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        if (line[0] == '#') {
            const auto v = line.find("version-");
            if (v != std::string::npos) out.version = line.substr(v);
            continue;
        }

        const auto tab = line.find('\t');
        if (tab == std::string::npos) continue;

        if (auto raw = parse_hex_u32(std::string_view(line).substr(tab + 1))) {
            out.values.emplace(line.substr(0, tab), *raw);
        }
    }
    return out;
}


bool write_baseline(const std::filesystem::path& path,
                    std::string_view version,
                    const std::vector<process::DumpedFlag>& flags) {
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;

    f << "# " << version << "\n";

    // Sort by name for stable diffs.
    std::vector<std::size_t> order(flags.size());
    for (std::size_t i = 0; i < order.size(); ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
        return flags[a].name < flags[b].name;
    });

    char hex[16];
    for (std::size_t i : order) {
        std::snprintf(hex, sizeof(hex), "0x%08X", flags[i].raw);
        f << flags[i].name << '\t' << hex << '\n';
    }
    return f.good();
}

}
