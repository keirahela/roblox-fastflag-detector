#include "parse/json.hpp"

#include <string>
#include <tuple>

namespace fflag::parse::json {

namespace {

inline bool is_ws(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

inline bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

const char* skip_ws(const char* p, const char* end) {
    while (p < end && is_ws(*p)) ++p;
    return p;
}

// Reads decimal digits and advances p. Returns nullopt if there's no digit at *p.
std::optional<std::uint64_t> read_u64(const char*& p, const char* end) {
    if (p >= end || !is_digit(*p)) return std::nullopt;

    std::uint64_t v = 0;
    while (p < end && is_digit(*p)) {
        v = v * 10 + std::uint64_t(*p - '0');
        ++p;
    }
    return v;
}

// Find `"key":` and return a pointer just past the colon (whitespace skipped).
const char* seek_value(std::string_view src, std::string_view key) {
    const std::string needle = "\"" + std::string(key) + "\"";
    const auto k = src.find(needle);
    if (k == std::string_view::npos) return nullptr;

    const char* p   = src.data() + k + needle.size();
    const char* end = src.data() + src.size();

    while (p < end && *p != ':') ++p;
    if (p >= end) return nullptr;
    return skip_ws(p + 1, end);
}

}


std::optional<std::string> string_field(std::string_view src, std::string_view key) {
    const char* p = seek_value(src, key);
    if (!p) return std::nullopt;

    const char* end = src.data() + src.size();
    if (p >= end || *p != '"') return std::nullopt;

    const char* start = ++p;
    while (p < end && *p != '"') ++p;
    if (p >= end) return std::nullopt;

    return std::string(start, p);
}


std::optional<std::uint64_t> number_field(std::string_view src, std::string_view key) {
    const char* p = seek_value(src, key);
    if (!p) return std::nullopt;
    return read_u64(p, src.data() + src.size());
}


std::optional<std::string> object_field(std::string_view src, std::string_view key) {
    const char* p = seek_value(src, key);
    if (!p) return std::nullopt;

    const char* end = src.data() + src.size();
    if (p >= end || *p != '{') return std::nullopt;

    const char* start = p;
    int depth = 1;
    ++p;
    while (p < end && depth > 0) {
        if      (*p == '{') ++depth;
        else if (*p == '}') --depth;
        ++p;
    }
    if (depth != 0) return std::nullopt;

    return std::string(start, p);
}


// One forward pass: locate quote, locate quote, locate colon, read number.
// Avoids re-scanning the buffer and uses piecewise emplace so the key string
// is built in place inside the map node (one alloc instead of alloc+move).
std::unordered_map<std::string, std::uint64_t> parse_flat_map(std::string_view obj) {
    std::unordered_map<std::string, std::uint64_t> out;
    out.reserve(obj.size() / 48);  // FFlag lines average ~48 bytes

    const char* p   = obj.data();
    const char* end = p + obj.size();

    while (p < end) {
        while (p < end && *p != '"') ++p;
        if (p == end) break;
        const char* name_begin = ++p;

        while (p < end && *p != '"') ++p;
        if (p == end) break;
        const char* name_end = p++;

        while (p < end && *p != ':') ++p;
        if (p == end) break;
        p = skip_ws(p + 1, end);

        if (auto v = read_u64(p, end)) {
            out.emplace(std::piecewise_construct,
                        std::forward_as_tuple(name_begin, name_end),
                        std::forward_as_tuple(*v));
        }
    }
    return out;
}

}
