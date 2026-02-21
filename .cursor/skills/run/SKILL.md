---
name: falcor-run
description: 运行 Falcor 应用、测试和脚本。在用户要求运行 Karma、Mogwai、Niagara、构建、执行测试、图像测试、单元测试（含 SlangUserGuide、MemoryOrder 多线程测试）、Python 测试、headless 脚本、balls.py 等 Falcor Python 脚本（使用 packman Python）或启动 Falcor 应用时使用。
---

# Falcor 运行

## 快速参考

| 操作 | 命令 |
|------|------|
| **构建** | `build_vs2022.bat` 或 `.\build_vs2022.ps1` |
| 构建指定 target | `.\build_vs2022.ps1 --target Karma` |
| 单元测试 (C++) | `tests/run_unit_tests.bat` 或 `.\tests\run_unit_tests.bat` |
| MemoryOrder 单元测试 | `.\tests\run_unit_tests.bat --test-case MemoryOrder` |
| 图像测试 | `tests/run_image_tests.bat` |
| Python 测试 | `tests/run_python_tests.bat`（需 conda） |
| 查看图像测试结果 | `tests/view_image_tests.bat` |
| Mogwai headless 脚本 | `./build/windows-vs2022/bin/Debug/Mogwai.exe --script <SCRIPT> --scene <SCENE> --headless` |
| Mogwai 交互模式 | `./build/windows-vs2022/bin/Debug/Mogwai.exe` |
| Niagara | `./build/windows-vs2022/bin/Debug/Niagara.exe` |
| Karma（Sample 浏览器） | `./build/windows-vs2022/bin/Debug/Karma.exe` |
| Karma 直接加载 sample | `Karma.exe --sample Samples/Desktop/D3D12ExecuteIndirect` |
| Karma 带 sample 参数 | `Karma.exe --sample D3D12HDR --arg display-curve=sRGB --arg reference-white=80` |
| Karma headless | `Karma.exe --sample <path> --headless` |
| Packman Python (balls.py 等) | `tools/.packman/python/python.exe`，需设置 `PYTHONPATH` 和 `PATH` |

在项目根目录（如 `F:/Falcor`）运行 `.bat`/`.ps1` 脚本。

## Karma（Sample 浏览器）

Samples 作为插件（DLL）注册，通过 Karma 树形 UI 选择运行：

```bash
./build/windows-vs2022/bin/Debug/Karma.exe
```

**启动参数**（`Karma.exe --help`）：
- `-s, --sample <name>`：启动时直接加载 sample（path 如 `Samples/Desktop/D3D12ExecuteIndirect` 或 type 如 `D3D12ExecuteIndirect`）
- `-a, --arg <key=value>`：Sample 参数（可重复），通过 `setProperties` 注入，与 ImGui 控件共享同一成员变量
- `--headless`：无窗口模式（用于自动化测试等）

```bash
# 直接加载 D3D12ExecuteIndirect
Karma.exe --sample Samples/Desktop/D3D12ExecuteIndirect

# 带 D3D12HDR 参数（display-curve、reference-white）
Karma.exe --sample D3D12HDR --arg display-curve=ST2084 --arg reference-white=120

# headless 运行指定 sample
Karma.exe --sample D3D12ExecuteIndirect --headless
```

- Samples 位于 `bin/Debug/plugins/`（如 `HelloDXR.dll`）
- Sample 参数：参考 RenderPass 的 `Properties` / `setProperties` / `getProperties`；各 Sample 按需实现。Desktop 中已支持 `--arg` 的：D3D12HDR（`display-curve`, `reference-white`）、D3D12ExecuteIndirect（`enable-culling`）、MeshletCull（`draw-meshlets`）、D3D12MeshletInstancing（`instance-level`, `draw-meshlets`, `debug-instance-color`）
- 新 Sample 使用 `make_new_sample_app.py` 生成，继承 `SampleBase`，通过 `add_plugin` 构建

## Mogwai

**交互模式**（打开 GUI）：
```bash
./build/windows-vs2022/bin/Debug/Mogwai.exe
```

**Headless**（脚本渲染，无 GUI）：
```bash
./build/windows-vs2022/bin/Debug/Mogwai.exe --script "F:/Falcor/Source/RenderPasses/GBuffer/VBuffer/VBufferMeshletRasterTest.py" --scene "F:/Falcor/media/test_scenes/bunny.pyscene" --headless
```

若以 Release 构建，将 `Debug` 替换为 `Release`。

## Niagara

**运行**（SampleApp，加载默认场景 `Arcade/Arcade.pyscene`）：
```bash
./build/windows-vs2022/bin/Debug/Niagara.exe
```

若默认场景路径无法解析，将 `FALCOR_MEDIA_FOLDERS` 设为 media 目录（如 `F:/Falcor/media`）。

**构建**（全部或指定 target）：
```bash
.\build_vs2022.ps1
.\build_vs2022.ps1 --target Niagara
.\build_vs2022.ps1 --target Karma
```
或使用 `build_vs2022.bat`（批处理）。

## 测试套件

**单元测试**（FalcorTest.exe）：
```bash
tests/run_unit_tests.bat
```
选项：`--config Debug|Release`、`--environment <json>`、`--list-configs`、`--test-suite <regex>`、`--test-case <regex>`、`--tags <tags>`

**运行指定测试套件**（如 SlangUserGuide）：
```bash
tests/run_unit_tests.bat --test-suite HelloWorldTests
```

**运行 MemoryOrder 多线程/内存序测试**（`Source/Tools/FalcorTest/Multithread/MemoryOrderTests.cpp`）：
```bash
.\tests\run_unit_tests.bat --test-case MemoryOrder
```

**图像测试**（基于 Mogwai）：
```bash
tests/run_image_tests.bat
```
选项：`--gen-refs`、`--parallel N`、`--config`、`--environment`

**Python 测试**（需 conda + CONDA_PYTHON_EXE）：
```bash
tests/run_python_tests.bat
```

**查看图像测试结果**（Web 查看器）：
```bash
tests/view_image_tests.bat
```

## 构建输出路径

| 配置 | 路径 |
|--------|------|
| Debug | `build/windows-vs2022/bin/Debug/` |
| Release | `build/windows-vs2022/bin/Release/` |

可执行文件：`Karma.exe`、`Mogwai.exe`、`Niagara.exe`、`FalcorTest.exe`、`ImageCompare.exe`、`RenderGraphEditor.exe`

## Packman Python（Falcor Python 脚本）

如 `scripts/python/balls/balls.py` 等脚本使用构建输出中的 `falcor` 模块。使用 packman Python 运行：

```bash
# 在项目根目录；需已构建 Falcor（build/windows-vs2022/bin/Debug/python/falcor 存在）
set PYTHONPATH=build\windows-vs2022\bin\Debug\python;%PYTHONPATH%
set PATH=build\windows-vs2022\bin\Debug;build\windows-vs2022\bin\Debug\plugins;%PATH%
tools\.packman\python\python.exe scripts\python\balls\balls.py
```

PowerShell:
```powershell
$env:PYTHONPATH = "F:\Falcor\build\windows-vs2022\bin\Debug\python;$env:PYTHONPATH"
$env:PATH = "F:\Falcor\build\windows-vs2022\bin\Debug;F:\Falcor\build\windows-vs2022\bin\Debug\plugins;$env:PATH"
.\tools\.packman\python\python.exe scripts\python\balls\balls.py
```

备选：`scripts/python/balls/run_balls.bat`（若可用则使用 `uv run`）。

## 提示

- 在仓库根目录运行 `.bat`/`.ps1` 文件；它们会自动解析 `tools/.packman/python` 和 `tests/testing/`。
- **构建**：`build_vs2022.ps1` 使用 packman CMake；传入 `--target <name>` 指定 target。
- 若 packman Python 缺失，先运行 `setup.bat`。
- 图像测试默认使用 `tests/environment/default.json`，除非被覆盖。
- **SlangUserGuide** 测试位于 `Source/Tools/FalcorTest/SlangUserGuide/`；使用 `--test-suite HelloWorldTests` 运行（套件名 = 源文件名不含扩展名）。
