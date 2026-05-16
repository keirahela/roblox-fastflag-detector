#pragma once

#include "flags/offsets.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstdint>
#include <string>
#include <vector>

namespace fflag::process {

struct DumpedFlag {
    std::string   name;
    std::uint32_t raw;
};

struct ReadStats {
    int skipped = 0;
};

std::vector<DumpedFlag> read_all_flags(HANDLE process,
                                       std::uint64_t base,
                                       const flags::Offsets& offsets,
                                       ReadStats& stats);

}
