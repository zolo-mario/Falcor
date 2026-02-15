---
name: build
description: 构建 Falcor 渲染框架及其插件。在用户要求构建、编译、重新构建 Falcor、构建 Niagara、构建指定 render pass 或 plugin，或执行构建相关命令时使用。
---

# Falcor 构建

## 快速开始

**构建全部**（在仓库根目录）：
```bash
cmake --build build/windows-vs2022 --config Debug
```

**构建指定 target**（如 Niagara、GBuffer、PathTracer）：
```bash
cmake --build build/windows-vs2022 --config Debug --target <TARGET_NAME>
```

**一键**（配置 + 构建）：
```bash
build_vs2022.bat
```

## 配置

| 选项 | 取值 | 默认 |
|------|------|------|
| `--config` | `Debug`、`Release` | Debug |
| `--target` | Target 名称（省略 = 构建全部） | — |

## 常用 Target

| 类别 | 示例 |
|----------|----------|
| RenderPasses | GBuffer, PathTracer, RTXDIPass, TAA, ToneMapper, AccumulatePass, SVGFPass, DLSSPass, DebugPasses, Utils |
| Apps | Mogwai, Niagara, FalcorTest, ImageCompare, RenderGraphEditor |

Target 名称 = `Source/RenderPasses` 中的文件夹名（如 `GBuffer`、`PathTracer`）。

## 首次 / 重新配置

若构建目录不存在或 CMake 缓存过期：
```bash
cmake --preset windows-vs2022 -DCMAKE_POLICY_VERSION_MINIMUM=3.5
```

## 构建输出路径

| 配置 | 路径 |
|--------|------|
| Debug | `build/windows-vs2022/bin/Debug/` |
| Release | `build/windows-vs2022/bin/Release/` |

## 提示

- 在仓库根目录运行（如 `c:/Falcor` 或 `F:/Falcor`）。
- 若 `cmake` 不在 PATH 中，使用 packman 的：`tools/.packman/cmake/bin/cmake.exe`。
- 依赖：确保 `tools/packman` 包已安装（需要时运行 `setup.bat`）。
- 构建后：通过 run skill 运行测试或 Mogwai。
- FalcorTest 包含 SlangUserGuide shader 示例和测试（`Source/Tools/FalcorTest/SlangUserGuide/`）。
