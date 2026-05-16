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

int run_dump(HANDLE process, std::uint64_t base,
             const flags::Offsets& offsets,
             const std::filesystem::path& output);

}
