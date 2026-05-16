#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winhttp.h>

namespace fflag::util {

// Win32 hands out two flavors of "not a handle": NULL (OpenProcess, etc.) and
// INVALID_HANDLE_VALUE (CreateToolhelp32Snapshot, CreateFile). Treat both as empty.
class ProcessHandle {
public:
    ProcessHandle() = default;
    explicit ProcessHandle(HANDLE h) : h_(h) {}

    ~ProcessHandle() { reset(); }

    ProcessHandle(const ProcessHandle&) = delete;
    ProcessHandle& operator=(const ProcessHandle&) = delete;

    ProcessHandle(ProcessHandle&& o) noexcept : h_(o.h_) { o.h_ = nullptr; }

    ProcessHandle& operator=(ProcessHandle&& o) noexcept {
        if (this != &o) {
            reset();
            h_ = o.h_;
            o.h_ = nullptr;
        }
        return *this;
    }

    HANDLE get() const { return h_; }
    bool   valid() const { return h_ != nullptr && h_ != INVALID_HANDLE_VALUE; }
    explicit operator bool() const { return valid(); }

    void reset(HANDLE h = nullptr) {
        if (valid()) CloseHandle(h_);
        h_ = h;
    }

private:
    HANDLE h_ = nullptr;
};


class HttpHandle {
public:
    HttpHandle() = default;
    explicit HttpHandle(HINTERNET h) : h_(h) {}

    ~HttpHandle() { reset(); }

    HttpHandle(const HttpHandle&) = delete;
    HttpHandle& operator=(const HttpHandle&) = delete;

    HttpHandle(HttpHandle&& o) noexcept : h_(o.h_) { o.h_ = nullptr; }

    HttpHandle& operator=(HttpHandle&& o) noexcept {
        if (this != &o) {
            reset();
            h_ = o.h_;
            o.h_ = nullptr;
        }
        return *this;
    }

    HINTERNET get() const { return h_; }
    explicit operator bool() const { return h_ != nullptr; }

    void reset(HINTERNET h = nullptr) {
        if (h_) WinHttpCloseHandle(h_);
        h_ = h;
    }

private:
    HINTERNET h_ = nullptr;
};

}
