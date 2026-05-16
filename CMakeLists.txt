cmake_minimum_required(VERSION 3.20)
project(roblox-fastflag-detector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
    message(FATAL_ERROR "roblox-fastflag-detector targets Windows only (WinHTTP / psapi / tlhelp32).")
endif()

add_executable(reader
    src/main.cpp
    src/cli/args.cpp
    src/commands/diff.cpp
    src/commands/dump.cpp
    src/commands/stable.cpp
    src/flags/baseline.cpp
    src/flags/offsets.cpp
    src/flags/types.cpp
    src/net/http_client.cpp
    src/parse/json.cpp
    src/process/flag_reader.cpp
    src/process/roblox_process.cpp
    src/util/file_io.cpp
)

target_include_directories(reader PRIVATE src)
target_link_libraries(reader PRIVATE winhttp psapi)

target_compile_definitions(reader PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

if(MSVC)
    target_compile_options(reader PRIVATE
        /W4 /permissive- /utf-8 /Zc:__cplusplus
    )
    target_compile_definitions(reader PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(reader PRIVATE -Wall -Wextra -Wpedantic)
endif()
