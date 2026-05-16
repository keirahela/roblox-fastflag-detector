#include "cli/args.hpp"
#include "commands/diff.hpp"
#include "commands/dump.hpp"
#include "commands/stable.hpp"
#include "flags/offsets.hpp"
#include "process/roblox_process.hpp"

#include <cstdio>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace {

int run_attached(const fflag::cli::Args& args,
                 const fflag::flags::Offsets& offsets) {
    const auto pid = fflag::process::find_roblox_pid();
    if (!pid) {
        std::fprintf(stderr, "%s not running.\n", fflag::process::kRobloxExe);
        return 1;
    }

    auto proc = fflag::process::open_for_read(*pid);
    if (!proc) {
        std::fprintf(stderr, "OpenProcess failed: %lu\n", GetLastError());
        return 1;
    }

    const auto mod = fflag::process::query_main_module(proc.get(), *pid);
    if (!mod) {
        std::fprintf(stderr, "Couldn't locate Roblox module.\n");
        return 1;
    }

    const auto installed = fflag::process::version_from_path(mod->exe_path);
    const bool match     = !installed.empty() && installed == offsets.version;

    std::printf("Roblox PID %lu   base 0x%llX   installed %s   %s\n\n",
                *pid,
                static_cast<unsigned long long>(mod->base),
                installed.empty() ? "?" : installed.c_str(),
                match ? "(matches offset db)" : "*** VERSION MISMATCH ***");

    if (!match) {
        std::fprintf(stderr,
            "WARN: offset db and Roblox build differ; reads will be unreliable.\n\n");
    }

    if (args.dump_file) {
        return fflag::commands::run_dump(proc.get(), mod->base, offsets, *args.dump_file);
    }
    return fflag::commands::run_diff(proc.get(), mod->base, offsets, args.diff_file);
}

}


int main(int argc, char** argv) {
    const auto args = fflag::cli::parse(argc, argv);

    // --stable is offline. No Roblox, no network.
    if (args.stable_output) {
        return fflag::commands::run_stable(*args.stable_output, args.stable_inputs);
    }

    const auto offsets = fflag::flags::refresh_offsets();
    if (!offsets) {
        std::fprintf(stderr, "Failed to load offsets.\n");
        return 1;
    }
    std::printf("Offset db: %s   (%zu flags)\n",
                offsets->version.c_str(), offsets->flags.size());

    return run_attached(args, *offsets);
}
