#include "commands/stable.hpp"

#include "flags/baseline.hpp"
#include "process/flag_reader.hpp"

#include <cstdint>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

namespace fflag::commands {

int run_stable(const std::filesystem::path& output_path,
               const std::vector<std::filesystem::path>& inputs) {
    if (inputs.size() < 2) {
        std::fprintf(stderr, "--stable needs at least 2 input baselines.\n");
        return 1;
    }

    auto seed = flags::read_baseline(inputs[0]);
    if (!seed || seed->values.empty()) {
        std::fprintf(stderr, "Couldn't read %s\n", inputs[0].string().c_str());
        return 1;
    }
    std::printf("Seed: %s  (%zu flags, version %s)\n",
                inputs[0].string().c_str(),
                seed->values.size(), seed->version.c_str());

    for (std::size_t i = 1; i < inputs.size(); ++i) {
        auto next = flags::read_baseline(inputs[i]);
        if (!next) {
            std::fprintf(stderr, "Couldn't read %s\n", inputs[i].string().c_str());
            return 1;
        }
        if (next->version != seed->version) {
            std::fprintf(stderr, "WARN: %s is for %s, seed is %s -- skipping\n",
                         inputs[i].string().c_str(),
                         next->version.c_str(), seed->version.c_str());
            continue;
        }

        const auto before = seed->values.size();
        for (auto it = seed->values.begin(); it != seed->values.end(); ) {
            const auto found = next->values.find(it->first);
            if (found == next->values.end() || found->second != it->second)
                it = seed->values.erase(it);
            else
                ++it;
        }

        std::printf("After %s: %zu stable (dropped %zu)\n",
                    inputs[i].string().c_str(),
                    seed->values.size(), before - seed->values.size());
    }

    std::vector<process::DumpedFlag> out;
    out.reserve(seed->values.size());
    for (const auto& [name, raw] : seed->values) out.push_back({name, raw});

    if (!flags::write_baseline(output_path, seed->version, out)) {
        std::fprintf(stderr, "Failed to write %s\n", output_path.string().c_str());
        return 1;
    }
    std::printf("Wrote %zu stable flags to %s\n",
                out.size(), output_path.string().c_str());
    return 0;
}

}
