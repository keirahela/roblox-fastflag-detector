#include "net/http_client.hpp"

#include "util/handle.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winhttp.h>

#include <vector>

namespace fflag::net {

namespace {

constexpr wchar_t kUA[] = L"FFlagReader/1.0 (Windows)";

bool status_ok(HINTERNET req) {
    DWORD status = 0;
    DWORD size   = sizeof(status);

    if (!WinHttpQueryHeaders(req,
                             WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                             WINHTTP_HEADER_NAME_BY_INDEX,
                             &status, &size, WINHTTP_NO_HEADER_INDEX))
        return false;

    return status == 200;
}

std::optional<std::string> drain(HINTERNET req) {
    std::string body;

    for (;;) {
        DWORD avail = 0;
        if (!WinHttpQueryDataAvailable(req, &avail)) return std::nullopt;
        if (avail == 0) return body;

        std::vector<char> buf(avail);
        DWORD got = 0;
        if (!WinHttpReadData(req, buf.data(), avail, &got)) return std::nullopt;
        body.append(buf.data(), got);
    }
}

}


std::optional<std::string> https_get(const wchar_t* host, const wchar_t* path) {
    util::HttpHandle session(WinHttpOpen(kUA,
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0));
    if (!session) return std::nullopt;

    util::HttpHandle conn(WinHttpConnect(session.get(), host,
                                         INTERNET_DEFAULT_HTTPS_PORT, 0));
    if (!conn) return std::nullopt;

    util::HttpHandle req(WinHttpOpenRequest(conn.get(),
        L"GET", path, nullptr,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE));
    if (!req) return std::nullopt;

    if (!WinHttpSendRequest(req.get(),
                            WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
        return std::nullopt;

    if (!WinHttpReceiveResponse(req.get(), nullptr)) return std::nullopt;
    if (!status_ok(req.get()))                       return std::nullopt;

    return drain(req.get());
}

}
