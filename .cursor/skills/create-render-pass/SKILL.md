---
name: falcor-create-render-pass
description: Create a new Falcor render pass from the template. Use when the user asks to create a render pass, add a new pass, scaffold a RenderPass, or make a new Falcor pass.
---

# Create Render Pass

## Quick Start

```bash
python tools/make_new_render_pass.py <PassName>
```

Example: `python tools/make_new_render_pass.py MyPostFX` → creates `Source/RenderPasses/MyPostFX/`

Run from the project root (e.g., `F:/Falcor`).

## What Gets Created

| File | Purpose |
|------|---------|
| `Source/RenderPasses/<PassName>/CMakeLists.txt` | Build config |
| `Source/RenderPasses/<PassName>/<PassName>.cpp` | Implementation (reflect, execute, renderUI stubs) |
| `Source/RenderPasses/<PassName>/<PassName>.h` | Header with RenderPass subclass |

The script also adds `add_subdirectory(<PassName>)` to `Source/RenderPasses/CMakeLists.txt` (sorted alphabetically).

## Naming

- Use PascalCase: `MyPostFX`, `GaussianBlur`, `MyPathTracer`
- Name must not already exist in `Source/RenderPasses/`

## Adding Shaders

The template has no shader files. To add Slang shaders:

1. Create `.cs.slang`, `.ps.slang`, or `.rt.slang` in the pass directory
2. Update `CMakeLists.txt`:

```cmake
target_sources(MyPostFX PRIVATE
    MyPostFX.cpp
    MyPostFX.h
    MyPostFX.cs.slang   # add shader(s)
)
```

3. If the pass needs shaders copied to the build output, add:

```cmake
target_copy_shaders(MyPostFX RenderPasses/MyPostFX)
```

Reference: `SimplePostFX`, `ToneMapper`, `PathTracer` for examples.

## Key Implementation Hooks

- `reflect()`: declare inputs/outputs via `reflector.addInput()`, `reflector.addOutput()`
- `execute()`: run the pass; access resources via `renderData.getTexture("name")`
- `renderUI()`: add GUI controls
- `FALCOR_PLUGIN_CLASS(ClassName, "DisplayName", "Description")` in header

## After Creation

1. Edit `.cpp` and `.h` to implement logic
2. Build: `cmake --build build/windows-vs2022 --config Debug --target <PassName>`
3. Use in Mogwai or RenderGraph scripts
