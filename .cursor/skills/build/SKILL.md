---
name: build
description: Build the Falcor rendering framework and its plugins. Use when the user asks to build, compile, rebuild Falcor, build a specific render pass or plugin, or run build-related commands.
---

# Falcor Build

## Quick Start

**Build all** (from repo root):
```bash
cmake --build build/windows-vs2022 --config Debug
```

**Build specific target** (e.g., GBuffer, PathTracer):
```bash
cmake --build build/windows-vs2022 --config Debug --target <TARGET_NAME>
```

**One-liner** (configure + build):
```bash
build_vs2022.bat
```

## Configuration

| Option | Values | Default |
|--------|--------|---------|
| `--config` | `Debug`, `Release` | Debug |
| `--target` | Target name (omit = build all) | — |

## Common Targets

| Category | Examples |
|----------|----------|
| RenderPasses | GBuffer, PathTracer, RTXDIPass, TAA, ToneMapper, AccumulatePass, SVGFPass, DLSSPass, DebugPasses, Utils |
| Apps | Mogwai, FalcorTest, ImageCompare, RenderGraphEditor |

Target name = folder name in Source/RenderPasses (e.g., `GBuffer`, `PathTracer`).

## First-Time / Reconfigure

If the build directory does not exist or CMake cache is outdated:
```bash
cmake --preset windows-vs2022 -DCMAKE_POLICY_VERSION_MINIMUM=3.5
```

## Build Output Paths

| Config | Path |
|--------|------|
| Debug | `build/windows-vs2022/bin/Debug/` |
| Release | `build/windows-vs2022/bin/Release/` |

## Tips

- Run from repo root (e.g., `c:/Falcor` or `F:/Falcor`).
- If `cmake` is not in PATH, use packman's: `tools/.packman/cmake/bin/cmake.exe`.
- Dependencies: ensure `tools/packman` packages are installed (run `setup.bat` if needed).
- After building: run tests or Mogwai via the run skill.
- FalcorTest includes SlangUserGuide shader examples and tests (`Source/Tools/FalcorTest/SlangUserGuide/`).
