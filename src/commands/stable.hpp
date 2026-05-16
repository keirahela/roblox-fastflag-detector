#pragma once

#include <filesystem>
#include <vector>

namespace fflag::commands {

// Intersect N baselines, write the result to output_path. Offline.
int run_stable(const std::filesystem::path& output_path,
               const std::vector<std::filesystem::path>& inputs);

}
