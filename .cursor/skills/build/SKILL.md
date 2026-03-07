---
name: build
description: 构建 Falcor 渲染框架及其插件。在用户要求构建、编译、重新构建 Falcor、构建 Niagara、构建指定 render pass 或 plugin，或执行构建相关命令时使用。
---

# Falcor 构建

## 快速开始

**推荐：本地 VS2022 一键构建**（仓库根目录）：
```bash
.\build_vs2022.ps1
.\build_vs2022.ps1 -Config Release
.\build_vs2022.ps1 -Target Karma
.\build_vs2022.ps1 -Target Niagara
```

**构建全部**（VS2026，Debug）：
```bash
cmake --build build/windows-vs2026 --config Debug
```

**构建全部**（VS2022，已完成 configure 后）：
```bash
cmake --build build/windows-vs2022 --config Debug
```

**构建指定 target**（如 Niagara、GBuffer、PathTracer）：
```bash
cmake --build build/windows-vs2026 --config Debug --target <TARGET_NAME>
```

**一键**（配置 + 构建，默认 VS2026）：
```bash
.\build.ps1
.\build.ps1 -Target Karma
.\build.ps1 -Preset windows-vs2022 -Target Niagara
.\build.ps1 -Config Release
```

或使用版本专属薄壳（等价于 `build.ps1 -Preset windows-vs2022|vs2026`）：
```bash
.\build_vs2022.ps1
.\build_vs2026.ps1
```

## 参数

| 参数 | 取值 | 默认 |
|------|------|------|
| `-Preset` | `windows-vs2022`、`windows-vs2026` | `windows-vs2026` |
| `-Config` | `Debug`、`Release` | `Debug` |
| `-Target` | Target 名称（省略 = 构建全部） | — |

## 常用 Target

| 类别 | 示例 |
|----------|----------|
| RenderPasses | GBuffer, PathTracer, RTXDIPass, TAA, ToneMapper, AccumulatePass, SVGFPass, DLSSPass, DebugPasses, Utils |
| Apps | Karma, Mogwai, Niagara, FalcorTest, ImageCompare, RenderGraphEditor |

Target 名称 = `Source/RenderPasses` 中的文件夹名（如 `GBuffer`、`PathTracer`）。

## 首次 / 重新配置

若构建目录不存在或 CMake 缓存过期，运行 setup 脚本（会自动配置）：
```bash
.\tools\dev\setup.ps1                        # VS2026（默认）
.\tools\dev\setup.ps1 -Preset windows-vs2022 # VS2022
```

或使用兼容壳（PowerShell 下请带 `.\` 前缀）：
```bash
.\setup_vs2026.bat
.\setup_vs2022.bat
```

也可单独重新运行 CMake 配置：
```bash
cmake --preset windows-vs2026 "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"
```

## 构建输出路径

| Preset | Config | 路径 |
|--------|--------|------|
| windows-vs2026 | Debug | `build/windows-vs2026/bin/Debug/` |
| windows-vs2026 | Release | `build/windows-vs2026/bin/Release/` |
| windows-vs2022 | Debug | `build/windows-vs2022/bin/Debug/` |
| windows-vs2022 | Release | `build/windows-vs2022/bin/Release/` |

## 提示

- 在仓库根目录运行（如 `c:/Falcor`）。
- 若 `cmake` 不在 PATH 中，使用 packman 版：`tools/.packman/cmake/bin/cmake.exe`（VS2022）或 `tools/cmake-4.2.3/bin/cmake.exe`（VS2026）。
- 依赖：确保 packman 包已安装（需要时运行 `setup_vs2026.bat` 或 `.\tools\dev\setup.ps1`）。
- 在 PowerShell 里调用批处理文件必须写成 `.\xxx.bat`，否则可能出现“找不到命令”。
- 构建后：通过 run skill 运行测试或 Mogwai。
- FalcorTest 包含 SlangUserGuide shader 示例和测试（`Source/Tools/FalcorTest/SlangUserGuide/`）。
