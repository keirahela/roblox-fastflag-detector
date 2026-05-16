#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

// Tiny hand-rolled JSON helpers, tuned for the imtheo offset schema. Not
// general-purpose: no escapes, no nested quotes, no fancy types.
namespace fflag::parse::json {

std::optional<std::string>   string_field(std::string_view src, std::string_view key);
std::optional<std::uint64_t> number_field(std::string_view src, std::string_view key);
std::optional<std::string>   object_field(std::string_view src, std::string_view key);

std::unordered_map<std::string, std::uint64_t> parse_flat_map(std::string_view obj);

}
