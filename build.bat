@echo off

setlocal

where cl >nul 2>nul
if errorlevel 1 (
    echo ERROR: cl.exe not on PATH.
    echo Open "x64 Native Tools Command Prompt for VS" and re-run build.bat.
    exit /b 1
)

set CONFIG=%~1
if "%CONFIG%"=="" set CONFIG=release

if /i "%CONFIG%"=="release" (
    set CFG_FLAGS=/O2 /MT /DNDEBUG /GL
    set LINK_FLAGS=/LTCG
    set OUTDIR=build
) else if /i "%CONFIG%"=="debug" (
    set CFG_FLAGS=/Od /MTd /Zi /RTC1
    set LINK_FLAGS=
    set OUTDIR=build-debug
) else (
    echo ERROR: unknown config "%CONFIG%". Use "release" or "debug".
    exit /b 1
)

if not exist %OUTDIR% mkdir %OUTDIR%

cl /nologo /std:c++17 /W4 /permissive- /utf-8 /EHsc ^
   %CFG_FLAGS% ^
   /DWIN32_LEAN_AND_MEAN /DNOMINMAX /D_CRT_SECURE_NO_WARNINGS ^
   /Isrc ^
   /Fo%OUTDIR%\ /Fe%OUTDIR%\reader.exe /Fd%OUTDIR%\reader.pdb ^
   src\main.cpp ^
   src\cli\args.cpp ^
   src\commands\diff.cpp ^
   src\commands\dump.cpp ^
   src\commands\stable.cpp ^
   src\flags\baseline.cpp ^
   src\flags\offsets.cpp ^
   src\flags\types.cpp ^
   src\net\http_client.cpp ^
   src\parse\json.cpp ^
   src\process\flag_reader.cpp ^
   src\process\roblox_process.cpp ^
   src\util\file_io.cpp ^
   /link %LINK_FLAGS% winhttp.lib psapi.lib

if errorlevel 1 (
    echo.
    echo BUILD FAILED ^(%CONFIG%^)
    exit /b 1
)

echo.
echo Built: %OUTDIR%\reader.exe   ^(%CONFIG%^)
endlocal
