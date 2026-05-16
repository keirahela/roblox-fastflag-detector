#pragma once

#include <optional>
#include <string>

namespace fflag::net {

// Synchronous HTTPS GET. Returns body on 200, nullopt on anything else.
std::optional<std::string> https_get(const wchar_t* host, const wchar_t* path);

}
