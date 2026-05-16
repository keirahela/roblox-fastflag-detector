#pragma once

#include "util/handle.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace fflag::process {

inline constexpr const char* kRobloxExe = "RobloxPlayerBeta.exe";

struct ModuleInfo {
    std::uint64_t base       = 0;
    std::uint32_t image_size = 0;
    std::string   exe_path;
};

std::optional<DWORD>      find_roblox_pid();
util::ProcessHandle       open_for_read(DWORD pid);
std::optional<ModuleInfo> query_main_module(HANDLE process, DWORD pid);

// Extract "version-XXXX" from a Roblox install path.
std::string version_from_path(std::string_view path);

}
