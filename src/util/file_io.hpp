#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace fflag::util {

std::filesystem::path cache_dir();

std::optional<std::string> read_file(const std::filesystem::path& path);

bool write_file(const std::filesystem::path& path, std::string_view data);

}
