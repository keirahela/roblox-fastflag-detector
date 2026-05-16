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
#include <filesystem>

namespace fflag::commands {

// Exit code: 0 = clean, 2 = unauthorized diffs, 1 = read error.
int run_diff(HANDLE process, std::uint64_t base,
             const flags::Offsets& offsets,
             const std::filesystem::path& baseline_path);

}
