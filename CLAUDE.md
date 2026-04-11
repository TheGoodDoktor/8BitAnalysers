# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

8-Bit Analysers is a suite of GUI tools for analysing and annotating games on 8-bit systems (ZX Spectrum, C64, CPC, BBC Micro, Arcade Z80, Tube Elite). Each analyser is a separate CMake project sharing a large common library. This repo is on the `ArcadeZ80` branch, which adds arcade Z80 game analysis support.

## Build Commands

Each analyser lives under `Source/<AnalyserName>/`. Build steps are identical across all of them:

```bash
cd Source/ArcadeZ80   # or ZXSpectrum, C64, CPC, BBC, TubeElite
mkdir build && cd build
cmake ..
# Linux/macOS:
make
# macOS Xcode project:
cmake -G Xcode ..
# Windows: opens Visual Studio solution generated in build/
```

Binaries are output to `build/bin/`. The VS debugger working directory is set to `../../Data/ArcadeZ80` in the CMake project, so on Windows run the executable from `Data/ArcadeZ80/`.

**First run:** copy `Data/<AnalyserName>/imgui.ini` to your working directory. Edit the generated `globalconfig.json` to set `WorkspaceRoot`, `SnapshotFolder`, and `PokesFolder` paths.

**Clone with submodules:**
```bash
git clone --recursive https://github.com/TheGoodDoktor/8BitAnalysers
```

## Tests

Tests are disabled by default (controlled by `with_tests` in CMake). When enabled, they use Google Test:

```bash
# Enable in CMakeLists.txt: set with_tests true
# Then build and run:
ctest --test-dir build
# or run directly:
./build/bin/SpectrumAnalyserTest
```

Test source is in `Source/Shared/CodeAnalyser/Tests/` and `Source/ZXSpectrum/Tests/`.

## Architecture

### Multi-Analyser Structure

Each analyser app (`Source/<Name>/`) is independently buildable and links against the shared library. The shared library (`Source/Shared/`) is included via `CMakeShared.txt` and compiled directly into each app (no separate shared `.so`/`.dll`).

### Key Subsystems in `Source/Shared/`

- **`CodeAnalyser/`** — Core analysis engine. `FCodeAnalyser` owns analysis state, manages memory pages (`FCodeAnalysisPage`), and drives the disassembly pipeline. CPU-specific disassemblers live in `CodeAnalyser/Z80/` and `CodeAnalyser/6502/`. The `Commands/` subdirectory implements undo/redo via a command pattern.

- **`Misc/EmuBase.h`** — `FEmuBase` is the base class all platform emulators derive from (e.g. `FArcadeZ80` in `Source/ArcadeZ80/`). It owns the `FCodeAnalyser` instance and wires up the main loop.

- **`MCPServer/`** — Model Context Protocol server that exposes analyser state to LLMs. `MCPServer.cpp` manages the server lifecycle; `MCPTools.cpp` defines callable tools; `MCPTransport.cpp` handles the communication layer.

- **`LuaScripting/`** — Lua integration for user automation. `LuaConsole` provides an interactive console; `LuaCoreAPI` and `LuaSys` expose analyser internals to Lua scripts.

- **`CodeAnalyser/UI/`** — All ImGui rendering for the analyser. CPU-specific UI is in `UI/Z80/` and `UI/6502/`.

- **`ImGuiSupport/`** — Platform backends for ImGui: `GLFW/` (all platforms), `Windows/` (DX11 option, currently unused — GLFW is the active backend on Windows too).

- **`Util/`** — Platform-specific file/memory/graphics utilities. `Windows/`, `Linux/`, and `Mac/` subdirs are selected at compile time by CMake.

### Vendor Dependencies (`Source/Vendor/`)

All managed as git submodules. Key ones: `chips` (CPU emulation cores for Z80/6502), `imgui`, `glfw`, `lua`, `nlohmann/json`, `implot`, `magic_enum`, `ImGuiColorTextEdit`.

### Coding Conventions

- `#pragma once` for all headers
- Class names prefixed with `F` (structs/classes): `FCodeAnalyser`, `FAddressRef`, `FArcadeZ80`
- Interface/abstract base classes prefixed with `I`: `ICPUInterface`
- Pointer members prefixed with `p`: `pEmulator`
- Bool members prefixed with `b`: `bRunMCPServer`
- Enum class values are `PascalCase`; enum types prefixed with `E`: `EEventType`
