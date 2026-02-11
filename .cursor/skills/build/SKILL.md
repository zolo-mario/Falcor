---
name: falcor-build
description: Build the Falcor rendering framework and its plugins. Use when the user asks to build, compile, rebuild Falcor, build a specific render pass or plugin, or run build-related commands.
---

# Falcor Build

## Quick Start

Build everything:
```bash
cmake --build build/windows-vs2022 --config Debug -DCMAKE_MESSAGE_LOG_LEVEL=ERROR
```

Build a specific target (e.g., GBuffer, PathTracer):
```bash
cmake --build build/windows-vs2022 --config Debug --target <TARGET_NAME>
```

## Configuration

| Option | Values | Default |
|--------|--------|---------|
| `--config` | `Debug`, `Release` | Debug |
| `--target` | Target name (omit = build all) | — |

## Common Targets

- **RenderPasses**: GBuffer, PathTracer, RTXDIPass, TAA, ToneMapper, AccumulatePass, SVGFPass, DLSSPass, DebugPasses, Utils
- **Apps**: Mogwai, FalcorTest, ImageCompare, RenderGraphEditor

Target name = folder name in Source/RenderPasses (e.g., `GBuffer`, `PathTracer`).

## First-Time / Reconfigure

If the build directory does not exist or CMake cache is outdated:
```bash
cmake --preset windows-vs2022 -DCMAKE_POLICY_VERSION_MINIMUM=3.5
```

## Run Tests (Example)

VBufferMeshletRaster headless test:
```bash
./build/windows-vs2022/bin/Debug/Mogwai.exe --script "F:/Falcor/Source/RenderPasses/GBuffer/VBuffer/VBufferMeshletRasterTest.py" --scene "F:/Falcor/media/test_scenes/bunny.pyscene" --headless
```

Adjust the script path and `--config` (Debug/Release) as needed.

## Tips

- Use `-DCMAKE_MESSAGE_LOG_LEVEL=ERROR` to reduce CMake output noise.
- On Windows, run from the repo root (e.g., `F:/Falcor`).
- Dependencies are managed via packman; ensure `tools/packman` packages are installed before building.
