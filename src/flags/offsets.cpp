#include "flags/offsets.hpp"

#include "net/http_client.hpp"
#include "parse/json.hpp"
#include "util/file_io.hpp"

namespace fflag::flags {

namespace {

constexpr const wchar_t* kHost = L"imtheo.lol";
constexpr const wchar_t* kPath = L"/Offsets/FFlags.json";

// Anything smaller is treated as garbage / truncated.
constexpr std::size_t kMinBodyBytes = 1024;

constexpr const char* kJsonFile = "FFlags.json";
constexpr const char* kVerFile  = "version.txt";

}


std::optional<Offsets> parse_offsets_json(std::string_view body) {
    namespace J = parse::json;

    Offsets out;
    if (auto v = J::string_field(body, "Roblox Version")) out.version = std::move(*v);

    if (auto list = J::object_field(body, "FFlagList")) {
        if (auto v = J::number_field(*list, "Pointer")) out.flag_list_ptr = *v;
        if (auto v = J::number_field(*list, "ToFlag"))  out.to_flag       = *v;
        if (auto v = J::number_field(*list, "ToValue")) out.to_value      = *v;
    }

    auto flags_obj = J::object_field(body, "FFlags");
    if (!flags_obj) return std::nullopt;

    out.flags = J::parse_flat_map(*flags_obj);
    if (out.flags.empty()) return std::nullopt;
    return out;
}


std::optional<Offsets> refresh_offsets() {
    const auto dir       = util::cache_dir();
    const auto json_path = dir / kJsonFile;
    const auto ver_path  = dir / kVerFile;

    std::string cached_version;
    if (auto v = util::read_file(ver_path)) cached_version = std::move(*v);

    std::string body;
    if (auto cached = util::read_file(json_path); cached && cached->size() >= kMinBodyBytes)
        body = std::move(*cached);

    if (body.size() < kMinBodyBytes) {
        if (auto fetched = net::https_get(kHost, kPath)) body = std::move(*fetched);
    }

    if (body.size() < kMinBodyBytes) {
        if (auto cached = util::read_file(json_path)) body = std::move(*cached);
    }

    if (body.size() < kMinBodyBytes) return std::nullopt;

    auto parsed = parse_offsets_json(body);
    if (!parsed) return std::nullopt;

    if (parsed->version != cached_version) {
        // Best-effort cache writes; on failure we'll just re-fetch next time.
        (void)util::write_file(json_path, body);
        (void)util::write_file(ver_path, parsed->version);
    }
    return parsed;
}

}
