#include "commands/dump.hpp"

#include "flags/baseline.hpp"
#include "process/flag_reader.hpp"

#include <cstdio>

namespace fflag::commands {

int run_dump(HANDLE proc, std::uint64_t base,
             const flags::Offsets& offsets,
             const std::filesystem::path& output) {
    process::ReadStats stats;
    const auto flags = process::read_all_flags(proc, base, offsets, stats);

    if (!flags::write_baseline(output, offsets.version, flags)) {
        std::fprintf(stderr, "Failed to write %s\n", output.string().c_str());
        return 1;
    }

    std::printf("Wrote %zu flags to %s   (%d outside readable region)\n",
                flags.size(), output.string().c_str(), stats.skipped);
    return 0;
}

}
