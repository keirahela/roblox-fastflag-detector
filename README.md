# roblox-fastflag-detector

A Windows tool that reads FastFlag values out of a running RobloxPlayerBeta.exe and compares them to a baseline. Any flag that doesn't match gets printed. Anything a normal client wouldn't change gets tagged `[UNAUTHORIZED]`.

Read-only. It doesn't write to Roblox and doesn't inject anything.

## Build

Windows + MSVC. WinHTTP, psapi, and tlhelp32 ship with Windows so there are no extra deps.

```bat
:: from "x64 Native Tools Command Prompt for VS"
build.bat            :: release -> .\build\reader.exe
build.bat debug      :: debug   -> .\build-debug\reader.exe
```

The CRT is statically linked, so the .exe runs on any Windows machine without the VC++ Redistributable. There's also a CMakeLists.txt if you'd rather use CMake.

## Usage

```
reader.exe                              Diff vs ./stable.txt
reader.exe --diff BASELINE              Diff vs BASELINE
reader.exe --all FILE                   Dump every flag to FILE
reader.exe --stable OUT B1 B2 [B3...]   Intersect baselines into OUT (offline)
```

Workflow:

1. On a clean Roblox install, dump the flags a few times: `reader.exe --all baseline.txt`. Do it across different sessions, ideally different machines.
2. Intersect the dumps: `reader.exe --stable stable.txt baseline1.txt baseline2.txt`. The output only keeps flags that agreed everywhere.
3. To audit a client, just run `reader.exe`. It defaults to `--diff stable.txt`. Exit code 0 means nothing unauthorized, 2 means something was found, 1 is an error.

## False positives: telemetry and A/B rollout

A vanilla Roblox client will not produce a clean diff. Roblox flips a bunch of FFlags from the server based on cohort: telemetry sampling rates, A/B tests, gradual feature rollouts. Two clean machines on the same Roblox build will disagree within minutes of each other. A single machine will disagree with itself across sessions.

If you compare two raw `--all` dumps you'll see dozens of differences with no modification involved. That's the whole reason the allowlist exists.

The allowlist is `kAllowlist` in [src/flags/types.hpp](src/flags/types.hpp). Anything in it gets tagged `[ALLOWED]` instead of `[UNAUTHORIZED]`. Roughly it covers user-tunable graphics and debug toggles (texture quality override, MSAA samples, grass distance, Alt+Enter fullscreen behavior, stuff a player can legitimately change locally) and Roblox-side rollout/telemetry that only the server flips (SlimRollout, ChannelName, EnableInputRecorder4, sampling rates, video capture limits).

It's a signal/noise tradeoff. If clean clients start flipping a new telemetry flag, add it. If a flag in the list could actually be abused, take it out. There's no canonical answer.

`stable.txt` also decays. It's an intersection of dumps from one point in time, and Roblox keeps shipping new flags and retiring old ones, so the baseline drifts. Regenerate it every so often.

One more thing: if the installed Roblox build doesn't match the offset DB, the reader prints `*** VERSION MISMATCH ***` on startup. Always re-test after a Roblox update. Running a diff against a stale offset DB just produces garbage everywhere and it's not worth debugging.

## Offset source

The detector needs the offset of each FFlag inside the current Roblox build. It pulls a JSON file from a third-party host:

```
https://imtheo.lol/Offsets/FFlags.json
```

Cached in `%LOCALAPPDATA%\fflag-reader\`. If you want to host the JSON yourself or pin to a specific build, edit `kHost` and `kPath` in [src/flags/offsets.cpp](src/flags/offsets.cpp).

## Layout

```
src/
  main.cpp              entry + attach/dispatch glue
  cli/                  argv parsing
  commands/             diff / dump / stable subcommands
  flags/                Offsets, Baseline, FlagType, allowlist
  net/                  WinHTTP wrapper
  parse/                tiny JSON helpers (no third-party deps)
  process/              process / module discovery, bulk RPM reader
  util/                 file I/O + RAII handle wrappers
```

## Disclaimer

This tool reads memory from another process. That might be against Roblox's Terms of Service, but Hyperion / Byfron doesn't enforce these areas, so it won't get you banned. Provided as-is under the MIT license.

## License

[MIT](LICENSE) © 2026 keirahela
