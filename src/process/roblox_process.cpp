#include "process/roblox_process.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>

#include <cstring>

namespace fflag::process {

namespace {

bool read_remote(HANDLE proc, std::uint64_t addr, void* out, SIZE_T n) {
    SIZE_T got = 0;
    return ReadProcessMemory(proc, reinterpret_cast<LPCVOID>(addr), out, n, &got) &&
           got == n;
}


// SizeOfImage lives at e_lfanew + 0x50 in the PE optional header.
bool read_image_size(HANDLE proc, std::uint64_t base, std::uint32_t& out) {
    std::uint8_t pe[0x1000];
    if (!read_remote(proc, base, pe, sizeof(pe))) return false;

    std::uint32_t e_lfanew = 0;
    std::memcpy(&e_lfanew, pe + 0x3C, sizeof(e_lfanew));
    if (e_lfanew == 0 || e_lfanew + 0x54u > sizeof(pe)) return false;

    std::memcpy(&out, pe + e_lfanew + 0x50, sizeof(out));
    return true;
}


// First module from EnumProcessModules is the .exe by Windows convention.
bool main_module_psapi(HANDLE proc, std::uint64_t& base, std::string& exe) {
    HMODULE m = nullptr;
    DWORD needed = 0;
    if (!K32EnumProcessModules(proc, &m, sizeof(m), &needed) || needed < sizeof(HMODULE))
        return false;

    base = reinterpret_cast<std::uint64_t>(m);

    char path[MAX_PATH] = {};
    if (K32GetModuleFileNameExA(proc, m, path, MAX_PATH) > 0) exe = path;
    return true;
}


bool main_module_toolhelp(DWORD pid, std::uint64_t& base, std::string& exe) {
    util::ProcessHandle snap(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid));
    if (!snap) return false;

    MODULEENTRY32 me{};
    me.dwSize = sizeof(me);
    if (!Module32First(snap.get(), &me)) return false;

    base = reinterpret_cast<std::uint64_t>(me.modBaseAddr);
    exe  = me.szExePath;
    return true;
}

}


std::optional<DWORD> find_roblox_pid() {
    util::ProcessHandle snap(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
    if (!snap) return std::nullopt;

    PROCESSENTRY32 pe{};
    pe.dwSize = sizeof(pe);
    if (!Process32First(snap.get(), &pe)) return std::nullopt;

    do {
        if (_stricmp(pe.szExeFile, kRobloxExe) == 0) return pe.th32ProcessID;
    } while (Process32Next(snap.get(), &pe));

    return std::nullopt;
}


util::ProcessHandle open_for_read(DWORD pid) {
    return util::ProcessHandle(
        OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid));
}


std::optional<ModuleInfo> query_main_module(HANDLE proc, DWORD pid) {
    ModuleInfo info;

    if (!main_module_psapi(proc, info.base, info.exe_path) &&
        !main_module_toolhelp(pid, info.base, info.exe_path)) {
        return std::nullopt;
    }
    if (info.base == 0) return std::nullopt;
    if (!read_image_size(proc, info.base, info.image_size)) return std::nullopt;

    return info;
}


std::string version_from_path(std::string_view path) {
    const auto start = path.find("version-");
    if (start == std::string_view::npos) return {};

    const auto end = path.find_first_of("\\/", start);
    return std::string(end == std::string_view::npos
                       ? path.substr(start)
                       : path.substr(start, end - start));
}

}
