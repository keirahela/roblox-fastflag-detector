#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace fflag::flags {

struct Offsets {
    std::string   version;
    std::uint64_t flag_list_ptr = 0;
    std::uint64_t to_flag       = 0;
    std::uint64_t to_value      = 0;
    std::unordered_map<std::string, std::uint64_t> flags;
};

std::optional<Offsets> parse_offsets_json(std::string_view body);

// Cache-first, network, then cache-fallback. Writes through when version changes.
std::optional<Offsets> refresh_offsets();

}
