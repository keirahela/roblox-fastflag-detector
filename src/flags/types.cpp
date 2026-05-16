#include "flags/types.hpp"

#include <cstdio>

namespace fflag::flags {

std::optional<Stripped> strip_prefix(std::string_view name) {
    for (const auto& e : kPrefixes) {
        if (name.size() > e.prefix.size() &&
            name.compare(0, e.prefix.size(), e.prefix) == 0) {
            return Stripped{std::string(name.substr(e.prefix.size())), e.type};
        }
    }
    return std::nullopt;
}


std::string render_raw(std::uint32_t raw) {
    if ((raw & 0xFFFFFF00u) == 0) {
        if (raw == 0) return "false";
        if (raw == 1) return "true";
    }

    char buf[16];
    std::snprintf(buf, sizeof(buf), "%d", static_cast<std::int32_t>(raw));
    return buf;
}

}
