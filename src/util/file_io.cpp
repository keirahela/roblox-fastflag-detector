#include "util/file_io.hpp"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <system_error>

namespace fflag::util {

std::filesystem::path cache_dir() {
    const char* la = std::getenv("LOCALAPPDATA");
    std::filesystem::path dir = (la && *la) ? la : ".";
    dir /= "fflag-reader";

    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    return dir;
}


std::optional<std::string> read_file(const std::filesystem::path& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return std::nullopt;

    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}


bool write_file(const std::filesystem::path& path, std::string_view data) {
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;

    f.write(data.data(), static_cast<std::streamsize>(data.size()));
    return f.good();
}

}
