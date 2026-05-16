#pragma once

#include <filesystem>
#include <optional>
#include <vector>

namespace fflag::cli {

struct Args {
    std::optional<std::filesystem::path> dump_file;
    std::filesystem::path                diff_file = "stable.txt";
    std::optional<std::filesystem::path> stable_output;
    std::vector<std::filesystem::path>   stable_inputs;
};

Args parse(int argc, char** argv);

}
