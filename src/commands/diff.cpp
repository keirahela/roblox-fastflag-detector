#include "commands/diff.hpp"

#include "flags/baseline.hpp"
#include "flags/types.hpp"
#include "process/flag_reader.hpp"

#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fflag::commands {

namespace {

std::unordered_set<std::string> allowed_bare_names() {
    std::unordered_set<std::string> out;
    out.reserve(flags::kAllowlist.size());

    for (std::string_view entry : flags::kAllowlist) {
        // Accept both "FFlagFoo" and bare "Foo" forms.
        if (auto s = flags::strip_prefix(entry)) {
            out.insert(std::move(s->bare));
        } else {
            out.insert(std::string(entry));
        }
    }
    return out;
}


void warn_version_skew(std::string_view baseline_ver, std::string_view db_ver) {
    if (baseline_ver == db_ver) return;

    std::fprintf(stderr,
        "WARN: baseline is for %.*s, current offset db is %.*s.\n"
        "      Diff may include false positives.\n\n",
        int(baseline_ver.size()), baseline_ver.data(),
        int(db_ver.size()),       db_ver.data());
}


std::unordered_map<std::string, std::uint32_t> index_by_name(
    const std::vector<process::DumpedFlag>& v) {
    std::unordered_map<std::string, std::uint32_t> out;
    out.reserve(v.size());
    for (const auto& f : v) out.emplace(f.name, f.raw);
    return out;
}

}


int run_diff(HANDLE proc, std::uint64_t base,
             const flags::Offsets& offsets,
             const std::filesystem::path& baseline_path) {
    auto baseline = flags::read_baseline(baseline_path);
    if (!baseline || baseline->values.empty()) {
        std::fprintf(stderr, "Could not read baseline file: %s\n",
                     baseline_path.string().c_str());
        return 1;
    }
    warn_version_skew(baseline->version, offsets.version);

    process::ReadStats stats;
    const auto current = process::read_all_flags(proc, base, offsets, stats);
    const auto current_map = index_by_name(current);
    const auto allowed = allowed_bare_names();

    int diffs = 0;
    int unauthorized = 0;

    for (const auto& [name, baseline_raw] : baseline->values) {
        const auto it = current_map.find(name);
        if (it == current_map.end() || it->second == baseline_raw) continue;

        const bool ok = allowed.count(name) > 0;
        std::printf("%-50s %s  =>  %s   %s\n",
                    name.c_str(),
                    flags::render_raw(baseline_raw).c_str(),
                    flags::render_raw(it->second).c_str(),
                    ok ? "[ALLOWED]" : "[UNAUTHORIZED]");

        ++diffs;
        if (!ok) ++unauthorized;
    }

    std::printf("\n%d flag(s) differ from baseline (%d unauthorized, %d allowed)\n",
                diffs, unauthorized, diffs - unauthorized);
    return unauthorized > 0 ? 2 : 0;
}

}
