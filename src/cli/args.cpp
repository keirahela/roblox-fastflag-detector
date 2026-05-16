#include "cli/args.hpp"

#include <string>

namespace fflag::cli {

namespace {

bool has_next(int i, int argc) { return i + 1 < argc; }

}


Args parse(int argc, char** argv) {
    Args a;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--all" && has_next(i, argc)) {
            a.dump_file = argv[++i];
        }
        else if (arg == "--diff" && has_next(i, argc)) {
            a.diff_file = argv[++i];
        }
        else if (arg == "--stable" && has_next(i, argc)) {
            a.stable_output = argv[++i];
            // Consume baseline paths until the next flag (anything starting with '-').
            while (has_next(i, argc) && argv[i + 1][0] != '-') {
                a.stable_inputs.emplace_back(argv[++i]);
            }
        }
    }
    return a;
}

}
