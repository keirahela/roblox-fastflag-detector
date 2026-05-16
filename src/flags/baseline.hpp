#pragma once

#include "process/flag_reader.hpp"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fflag::flags {

struct Baseline {
    std::string version;
    std::unordered_map<std::string, std::uint32_t> values;
};

// On-disk format:
//   # version-XXXX
//   FlagName<TAB>0xHHHHHHHH
std::optional<Baseline> read_baseline(const std::filesystem::path& path);

bool write_baseline(const std::filesystem::path& path,
                    std::string_view version,
                    const std::vector<process::DumpedFlag>& flags);

}
