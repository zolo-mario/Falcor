---
modified: 2026-02-02T15:21:30+08:00
created: 2026-01-28T10:41:54+08:00
tags:
  - 2026/01/28
---
# README
### 1. Falcor 简介
Falcor 是一个旨在提高图形学研究和原型开发效率的实时渲染框架。它支持 DirectX 12 和 Vulkan。

**主要特性：**
*   **抽象化操作**：简化了着色器编译、模型加载和场景渲染等通用操作。
*   **光线追踪**：内置光线追踪支持。
*   **Python 脚本**：支持通过 Python 进行控制。
*   **渲染图系统**：用于构建模块化的渲染器。
*   **通用技术**：包含后处理特效等常用渲染技术。
*   **路径追踪**：包含无偏（Unbiased）路径追踪器。
*   **RTX 集成**：集成了 DLSS、RTXDI 和 NRD 等 NVIDIA RTX SDK。

---

### 2. 环境要求 (Prerequisites)
**基础要求：**
*   **系统**：Windows 10 版本 20H2 或更新。
*   **开发工具**：Visual Studio 2022。
*   **SDK**：Windows 10 SDK (10.0.19041.0)。
*   **硬件**：支持 DirectX Raytracing (DXR) 的 GPU（如 NVIDIA Titan V 或 GeForce RTX 系列）。
*   **驱动**：NVIDIA 驱动 466.11 或更新。

**可选安装：**
*   **Windows 10 Graphics Tools**：用于开启 DX12 调试层（可通过系统“可选功能”添加或下载离线包）。
*   NVAPI, CUDA, OptiX（详见下文）。

---

### 3. 构建指南 (Building Falcor)
Falcor 使用 **CMake** 构建系统。

#### Visual Studio 2022
1.  克隆仓库后运行 `setup_vs2022.bat`。
2.  解决方案将生成在 `build/windows-vs2022`，二进制文件在 `bin` 子目录中。

#### Visual Studio Code
1.  克隆仓库后运行 `setup.bat`（这会配置 `.vscode` 环境）。
2.  安装推荐的插件（尤其是 **CMake Tools**）。
3.  按 `Ctrl+Shift+P` 选择 `CMake: Select Configure Preset`，选择 **Windows Ninja/MSVC**。
4.  按 F7 构建。
*   **警告**：切勿从 Git Bash 启动 VS Code，这会导致环境变量路径格式错误。

#### Linux (Ubuntu 22.04) - 实验性支持
1.  克隆仓库后运行 `setup.sh`。
2.  安装依赖库：`sudo apt install xorg-dev libgtk-3-dev`。
3.  构建方式同 VS Code，Preset 选择 **Linux/GCC**。

---

### 4. 外部 SDK 集成指南
部分功能需要手动下载 SDK 并放入指定目录（`external/packman/`）：

| 功能组件 | 版本要求 | 安装路径/操作 |
| :--- | :--- | :--- |
| **DX12 Agility SDK** | 内置 | 在主 `.cpp` 中定义宏即可启用（默认在 Mogwai 等示例中开启）。 |
| **NVAPI** | R535 | 解压至 `external/packman/nvapi`。 |
| **NSight Aftermath** | 2023.1 | 解压至 `external/packman/aftermath`。 |
| **CUDA** | 11.6.2+ | 下载并安装 CUDA Toolkit，重新配置构建。 |
| **OptiX** | 7.3 | 解压至 `external/packman/optix` (需配合 CUDA 使用)。 |

**随附的 RTX SDK：**
Falcor 已包含 DLSS, RTXDI, NRD 的集成，但请注意它们的许可证可能与 Falcor 不同。

# Falcor 入门指南

这是一份关于 **Falcor 入门指南** 的中文重述。

### 1. 项目结构 (Project Layout)
Falcor 的源代码目录结构如下：

| 目录 | 说明 |
| :--- | :--- |
| **Source/Falcor** | **核心框架**。包含 Falcor 的核心功能，编译为共享库 (Shared Library)。 |
| **Source/Samples** | **示例程序**。直接调用 Falcor API 的独立应用程序，展示基础功能和抽象层的使用。 |
| **Source/Mogwai** | **Mogwai 宿主程序**。这是使用“渲染图 (Render Graphs)”的主要应用程序，包含实用的工具集。示例渲染图脚本位于 `Source/Mogwai/Data/`。 |
| **Source/RenderPasses** | **渲染通道组件**。包含构建渲染图所需的各种组件（共享库）。构建 Mogwai 时会自动编译这些依赖库。 |

---

### 2. 工作流 (Workflows)
使用 Falcor 主要有两种工作流：

#### A. 渲染图工作流 (Render Graphs) - **推荐**
这是原型设计或实现渲染技术的首选方式。流程是创建 Render Pass，构建 Render Graph，最后在 Mogwai 中运行。

**如何运行示例渲染图：**
1.  构建 Falcor。
2.  运行 `Mogwai` 应用程序。
3.  **加载脚本**：按 `Ctrl+O` 或菜单 `File -> Load Script`，选择 `.py` 文件（如 `Source/Mogwai/Data/ForwardRenderer.py`）。
4.  **加载场景**：按 `Ctrl+Shift+O` 或菜单 `File -> Load Scene`，选择场景文件（如 `media/Arcade/Arcade.pyscene`）。
    *   *提示*：也支持直接将文件拖拽进窗口加载。

**如何创建新的 Render Pass 库：**
运行命令：`tools/make_new_render_pass.bat <Name>`

#### B. 独立程序工作流 (Sample Applications)
适用于希望直接基于 Falcor 编写独立应用程序的场景。
*   **核心类**：`Renderer` 类是启动器，用户需继承该类并覆盖受保护的回调方法。
*   **参考示例**：`ModelViewer` 是很好的参考起点。

**如何创建新的示例程序：**
运行命令：`tools/make_new_sample_app.bat <Name>`

---

### 3. 着色器与数据文件管理
Falcor 通过相对路径在多个工作目录中搜索文件。
*   **数据文件 (Data files)**：指非着色器资源，如纹理和模型。

#### 搜索逻辑 (Visual Studio 环境)
1.  **数据文件**搜索顺序：
    *   项目文件夹内的 `Data` 子目录。
    *   可执行文件目录下的 `Data` 子目录。
    *   环境变量 `FALCOR_MEDIA_FOLDERS` 指定的目录（分号分隔）。
    *   通过代码 `addDataDirectory()` 手动添加的目录。
2.  **着色器文件**：相对于项目文件夹搜索。

#### 部署与发布
构建项目时，`Data` 文件夹和着色器文件会自动**复制**到可执行文件目录下的 `Data` 和 `Shaders` 文件夹中（保持层级结构）。
*   这意味着构建输出目录是**自包含 (Self-contained)** 的，便于分享。
*   当直接运行可执行文件时，Falcor 会优先在这些部署好的文件夹中搜索。

#### 最佳实践
*   **目录结构**：在你的**项目文件 (.vcxproj)** 旁创建一个 `Data` 目录存放资源。
*   **文件后缀**：
    *   着色器代码：`.hlsl` 或 `.slang`。
    *   仅主机端 (Host-only) 头文件：`.h`。
    *   主机与着色器共享 (Shared) 头文件：`.slang` 或 `.slangh`。
*   **API 调用**：使用 `findFileInDataDirectories()` 查找数据，使用 `findFileInShaderDirectories()` 查找着色器。

# Python

这是一份关于 **Falcor Python 扩展**使用的文档重述。

### 1. 简介 (Introduction)
Falcor 长期支持通过 Python 进行内部脚本控制（如创建渲染图或 `.pyscene` 场景构建）。
近期版本新增了 **Falcor Python 扩展**，允许用户直接从 Python 环境中调用 Falcor。

*   **核心优势**：结合 Python 生态系统，支持与 **NumPy** 和 **PyTorch** 等框架协同工作。
*   **当前状态**：API 覆盖范围有限，但已足以支持初步的机器学习应用开发。

---

### 2. Python 环境搭建 (Python Environment)
推荐使用 **Conda** 管理环境（支持 Windows/Linux）。

#### 步骤指南：
1.  **安装 Miniconda**：下载最新的 Python 3.10 版本。
    *   *重要提示*：Falcor 构建时预置了特定版本的 Python 二进制文件。请务必保持版本一致。若需使用其他版本，需在 CMake 中设置 `FALCOR_USE_SYSTEM_PYTHON` 为 `ON` 并重新编译。
2.  **创建环境**：使用仓库根目录下的配置文件创建基础环境：
    ```bash
    conda env create -f environment.yml
    ```
3.  **激活环境**：
    ```bash
    conda activate falcor-pytorch
    ```
4.  **配置路径**：
    进入 Falcor 的二进制输出目录（例如 `build/windows-ninja-msvc/bin/Release`），运行脚本以设置 Python 路径：
    *   Windows (CMD): `setpath.bat`
    *   Windows (PowerShell): `setpath.ps1`
    *   Linux: `setpath.sh`

**验证安装**：在 Python 解释器中输入 `import falcor`，若显示加载插件信息则表示成功。

---

### 3. IDE 支持
Falcor 构建系统会自动生成 **Python 接口存根文件 (Stub files)**，包含类型信息，可极大提升开发体验（如代码补全）。

*   **推荐工具**：Visual Studio Code (VS Code)。
*   **启动方式**：建议在已激活环境的 Conda 终端内输入 `code` 启动 VS Code。这样编辑器会自动继承正确的 Python 和系统路径配置。

---

### 4. 示例 (Examples)
文档提供了以下示例演示基础用法：

*   **scripts/python/balls/balls.py**
    *   演示如何从 Python 调用计算着色器 (Compute Shaders) 并将移动的 2D 圆形渲染到屏幕上。
*   **scripts/python/gaussian2d/gaussian2d.py**
    *   演示 **可微 Slang (Differentiable Slang)** 的用法。
    *   利用 Slang 的自动微分功能实现反向传播，将 2D 高斯函数拟合到图像上。
    *   展示了 PyTorch 与 CUDA 的互操作性（在 PyTorch 中运行优化循环）。

---

### 5. Agility SDK 限制
Falcor 使用 Microsoft Agility SDK 获取最新的 D3D12 功能。在 Python 扩展中使用时需注意以下限制：

1.  **开发者模式**：必须在 Windows 设置中开启“开发者模式 (Developer Mode)”。
2.  **驱动器位置**：Python 解释器可执行文件必须与 Falcor Python 扩展位于**同一个物理驱动器**上。

# 教程

这是一份关于 **Mogwai 使用**及 **Falcor 开发教程**（渲染通道、渲染图、着色器）的文档重述。

### 1. Mogwai 基础使用 (Mogwai Usage)

Mogwai 是 Falcor 的主要渲染应用程序，用于加载渲染图和场景。

#### 构建与运行
*   **构建**：在 Visual Studio 中选择 `Build -> Build Solution`。
*   **运行**：
    *   **VS 内**：`Ctrl+F5`。
    *   **命令行**：`Mogwai.exe [OPTIONS]`。
    *   **常用参数**：
        *   `--headless`: 无窗口模式。
        *   `-s, --script`: 加载 Python 脚本。
        *   `-S, --scene`: 加载场景文件（如 `.pyscene`）。
        *   `--list-gpus` / `--gpu`: 列出或指定 GPU。

#### 加载资源
*   **脚本 (.py)**：包含全局设置和渲染图配置。通过 `Ctrl+O` 或拖拽加载。
*   **场景**：由脚本指定或手动通过 `Ctrl+Shift+O` 加载。

#### 界面与控制 (UI & Controls)
*   **主要界面**：包括菜单栏、渲染图控制面板、时间轴、性能分析器（Profiler）和控制台。
*   **常用快捷键**：
    *   `F2`: 显/隐 GUI。
    *   `F12`: 截图；`Shift+F12`: 录制视频。
    *   `V`: 切换垂直同步。
    *   `F5`: 重载着色器。
    *   `Space`: 暂停/恢复时间。
    *   **相机控制**：`WASD` 移动，鼠标左键旋转，`Shift` 加速，`Ctrl` 减速。

---

### 2. 实现渲染通道 (Implementing a Render Pass)

渲染通道（Render Pass）是渲染图的基本组成单元。所有通道位于 `Source/RenderPasses` 并被编译为共享库。

#### 开发流程
1.  **创建**：运行 `tools/make_new_render_pass.bat <Name>` 生成模板。
2.  **核心函数实现**：
    *   **`create()`**：工厂方法，实例化类。
    *   **`reflect()`**：定义输入/输出/内部资源。使用 `RenderPassReflection` 添加 `Input`、`Output` 或 `Internal` 资源。
    *   **`execute()`**：执行渲染逻辑（如调用 `pRenderContext->blit()`）。
3.  **注册**：在源码中使用 `registerPlugin` 函数和 `FALCOR_PLUGIN_CLASS` 宏注册插件。

---

### 3. 创建与编辑渲染图 (Creating and Editing Render Graphs)

可以通过图形化编辑器或 Python 脚本两种方式构建渲染图。

#### 方法 A：渲染图编辑器 (GUI)
*   **操作**：将渲染通道拖入画布，连接节点。
*   **连接类型**：
    *   **数据依赖 (Data dependency)**：连接输入/输出引脚（实线），传递资源。
    *   **执行依赖 (Execution dependency)**：连接无标签引脚（虚线/特殊连线），控制执行顺序。
*   **输出**：必须标记至少一个 Output（勾选 `Graph Output`）。
*   **实时编辑**：在 Mogwai 运行时点击 `Edit` 可实时修改图结构和参数。

#### 方法 B：Python 脚本
使用 Falcor 的脚本 API 构建：
*   `RenderGraph("Name")`: 创建图。
*   `createPass("PassName", {dict})`: 创建通道并初始化参数。
*   `g.addPass(PassObj, "Name")`: 将通道加入图。
*   `g.addEdge("Src.out", "Dst.in")`: 创建连接。
*   `g.markOutput("Pass.res")`: 标记最终输出。

---

### 4. 编写着色器 (Writing Shaders)

本节以创建一个 **WireframePass**（线框渲染通道）为例，展示如何结合 C++ 和 Slang 着色器。

#### 1. 编写着色器 (Slang)
*   定义 `cbuffer` 存储常量（如颜色）。
*   编写顶点着色器（VS）和像素着色器（PS）。
*   Slang 会自动分配资源绑定位置，通常无需手动指定 `register`。

#### 2. C++ 类实现
需要在类中管理以下关键对象：
*   **GraphicsProgram**：加载着色器文件（`.slang`）。
*   **RasterizerState**：设置光栅化状态（如 `FillMode::Wireframe`, `CullMode::None`）。
*   **GraphicsState**：绑定 Program 和 RasterizerState。
*   **Scene**：处理场景数据。

#### 3. 关键逻辑
*   **`setScene()`**：
    *   获取场景指针。
    *   **重要**：调用 `mpProgram->addDefines(mpScene->getSceneDefines())`。这是为了让着色器正确理解场景资源的绑定数量和结构。
    *   创建 `GraphicsVars` 用于后续传参。
*   **`execute()`**：
    1.  **FBO 管理**：为输出纹理创建 FBO 并清屏。
    2.  **绑定 FBO**：`mpGraphicsState->setFbo(pTargetFbo)`。
    3.  **传参**：通过 `mpVars["CB"]["name"] = val` 传递着色器变量。
    4.  **渲染**：调用 `mpScene->rasterize(...)` 执行绘制。

#### 4. 构建与运行
*   在 `CMakeLists.txt` 中配置 `add_renderpass` 和 `target_sources`。
*   编写 Python 脚本加载该 DLL 并构建渲染图，运行即可看到线框渲染效果。

![[Falcor-1769568549158.png|800]]

![[58e52f8e846c9128710f2d8f9a556247_MD5.webp|800]]

# 开发

这是一份关于 **Falcor 开发指南** 的文档重述，涵盖了构建系统、错误处理、单元测试和代码规范四个主要部分。

### 1. CMake 构建系统 (CMake)
Falcor 使用 CMake 作为构建系统。

*   **添加源文件**：直接在 `CMakeLists.txt` 中的 `target_sources` 列表添加文件。着色器文件会被自动识别并复制到输出目录（前提是调用了 `target_copy_shaders`）。
*   **创建新项目**：
    *   新示例程序：运行 `tools/make_new_sample_app.bat <Name>`。
    *   新渲染通道：运行 `tools/make_new_render_pass.bat <Name>`。
*   **Falcor 专用 CMake 函数**：
    *   `target_copy_shaders(<target> <output_dir>)`: 复制着色器。
    *   `target_copy_data_folder(<target>)`: 复制数据文件夹。
    *   `add_falcor_executable(<target>)`: 创建应用程序并链接 Falcor 库。
    *   `add_renderpass(<target>)`: 创建渲染通道库。

---

### 2. 错误处理 (Error Handling)
Falcor 将错误分为三类并采用不同的处理机制：

1.  **逻辑错误 (Logic Errors)**：程序员的失误（本不该发生的情况）。
    *   **处理机制**：**断言 (Assertions)**。
    *   使用 `FALCOR_ASSERT()` 或 `FALCOR_ASSERT_EQ` 等宏。仅在 Debug 模式下生效。
2.  **运行时错误 (Runtime Errors)**：意外的运行时状况（如错误输入、文件损坏）。
    *   **处理机制**：**异常 (Exceptions)**。
    *   使用 `FALCOR_THROW(fmt, ...)` 抛出异常，或使用 `FALCOR_CHECK(cond, fmt, ...)` 检查条件。建议在公共 API 边界使用。
3.  **可恢复错误**：常见的失败检查。
    *   **处理机制**：**返回值** 或 错误码。

**错误报告与日志：**
*   **报告函数**：
    *   `reportError()`: 严重错误，用户可选择继续。
    *   `reportErrorAndAllowRetry()`: 允许用户重试（常用于着色器编译失败）。
    *   `reportFatalError()`: 致命错误，程序必须终止。
*   **日志系统**：
    *   **等级**：`Debug`, `Info`, `Warning`, `Error`, `Fatal`。
    *   **输出**：默认输出到控制台、VS 调试窗口和文件（文件名包含进程ID）。

---

### 3. 单元测试 (Unit Testing)
Falcor 拥有自定义的测试系统，核心程序为 `FalcorTest.exe`。

*   **运行测试**：
    *   脚本：`tests/run_unit_tests.bat`。
    *   直接运行：`FalcorTest.exe [OPTIONS]` (支持 `-c` 指定 CPU/GPU 类别，`-f` 过滤测试名)。
*   **添加测试**：
    *   **CPU 测试**：使用 `CPU_TEST(Name) { ... }` 宏。使用 `EXPECT_EQ` 等宏验证结果。
    *   **GPU 测试**：使用 `GPU_TEST(Name) { ... }` 宏。
        *   提供 `ctx` (GPUUnitTestContext) 对象，可用于创建程序 (`ctx.createProgram`)、分配缓冲 (`ctx.allocateStructuredBuffer`) 和运行着色器。
*   **图像测试**：
    *   通过 `run_image_tests.py` 运行，基于 Mogwai 生成图像并与参考图像对比。
    *   支持并行执行和 XML 报告生成。

---

### 4. 代码规范 (Coding Conventions)
项目采用现代 C++17 风格。

**文件与命名：**
*   **文件结构**：类/文件名使用 **PascalCase**。C++头文件用 `.h`，源文件用 `.cpp`；Slang 模块用 `.slang`，头文件用 `.slangh`。
*   **命名规则**：
    *   类/结构体/枚举/类型定义：`PascalCase`
    *   函数/公共成员变量：`camelCase`
    *   私有/保护成员变量：`mCamelCase`
    *   全局变量：`gCamelCase`
    *   宏：`UPPER_SNAKE_CASE`
    *   **特例**：接口方法名必须以动词开头（如 `isEnabled()` 而非 `enabled()`）。

**C++ 语言规范：**
*   使用 `using` 代替 `typedef`。
*   使用 `enum class`。
*   使用 `nullptr`。
*   **智能指针**：优先使用 `std::shared_ptr` / `std::unique_ptr`。类内部通常定义 `using SharedPtr = ...` 别名。
*   **头文件**：使用 `#pragma once`。

**代码格式：**
*   **缩进**：4 个空格，不使用 Tab。
*   **大括号**：所有大括号（包括 `if` 后）另起一行。
*   **注释**：
    *   推荐使用新式 Doxygen 风格：`/** ... */`，参数使用 `@param`。
    *   单行简短注释使用 `///` 或 `///<` (用于成员变量)。

**Slang 规范：**
*   入口点文件后缀：`.rt.slang` (光追), `.cs.slang` (计算), `.3d.slang` (图形)。
*   浮点数使用 `1.f` 格式以匹配 C++ 行为。

# 路径追踪

这是一份关于 **Falcor Path Tracer (路径追踪器)** 的文档重述。

### 1. 快速开始 (Getting Started)

要运行路径追踪器，请遵循以下步骤：
1.  构建 Falcor。
2.  启动 Mogwai 应用程序。
3.  **加载渲染图脚本**：
    *   标准路径追踪：`Source/Mogwai/Data/PathTracer.py`
    *   实时降噪版：`Source/Mogwai/Data/PathTracerNRD.py`
    *   *操作方式*：`Ctrl+O`、拖放文件或启动命令行参数 `--script`。
4.  **加载场景**：
    *   示例场景：`media/Arcade/Arcade.pyscene`
    *   支持 Assimp 格式、USD 以及 pbrt-v4。
    *   *操作方式*：`Ctrl+Shift+O`、拖放文件或启动命令行参数 `--scene`。

---

### 2. 概述 (Overview)

`PathTracer` 是一个基于 **DXR 1.1** 实现的无偏路径追踪器。

*   **执行流程**：全屏计算通道生成路径 -> RayGen 着色器循环处理路径段 -> Hit/Miss 着色器处理着色 -> 使用 Ray Queries (`TraceRayInline`) 进行可见性测试。
*   **采样**：通常每像素采样数 (spp) 设为 1，通过后续的 `AccumulatePass` 实现渐进式渲染。
*   **光照计算**：
    *   在每个路径顶点，通过光线追踪进行可见性测试。
    *   支持三种光源采样策略：I. 解析光源，II. 环境贴图，III. 网格光源。后两者支持多重重要性采样 (MIS)。
    *   *提示*：可开启 RTXDI 使用 ReSTIR 采样直接光照。
*   **反弹次数配置**：
    *   通过 `maxDiffuseBounces` (漫反射)、`maxSpecularBounces` (镜面反射)、`maxTransmissionBounces` (透射) 控制。
    *   通常透射次数需设置较高（如 10）以穿透多层介质，而漫反射/镜面反射设置较低（如 3）以优化性能。

---

### 3. 嵌套电介质 (Nested Dielectric Materials)

用于渲染重叠的透明材质（如玻璃杯中的液体）。

*   **要求**：网格必须重叠（避免气隙），且材质需设为双面 (Double-sided)。
*   **优先级系统**：Falcor 使用基于栈的方法解决优先级。
    *   通过 `.pyscene` 文件中的 `nestedPriority` 属性配置。
    *   数值越大，优先级越高。
    *   **注意**：默认值 `0` 被保留表示**最高**优先级。

---

### 4. 输入与输出 (Inputs & Outputs)

**输入 (Inputs):**
*   `vbuffer` (必须): 轻量级缓冲区，编码了命中的网格/图元索引和重心坐标。
*   `mvec` (可选): 屏幕空间运动矢量，用于 RTXDI 时域重采样。
*   `viewW` (可选): 世界空间视图方向，用于景深 (DOF) 正确渲染。
*   `sampleCount` (可选): 用于自适应采样的每像素样本数。

**输出 (Outputs):**
均为可选，仅在连接后续 Pass 时计算。
*   `color`: 最终辐射亮度 (Radiance)。
*   `albedo` / `specularAlbedo` / `indirectAlbedo`: 各类反照率信息。
*   `normal`: 世界空间法线。
*   `rayCount` / `pathLength`: 调试用，统计光线数和路径长度。
*   `nrd*`: NRD 降噪器所需的额外数据。

---

### 5. 采样策略 (Sampling Strategies)

*   **BSDF 采样 (MIS)**:
    *   使用 StandardMaterial（Disney 各向同性漫反射 + GGX 镜面/透射）。
    *   随机选择漫反射、镜面反射或透射。
*   **环境贴图采样 (MIS)**:
    *   启动时计算分层重要性贴图 (Mipmap)。
    *   PDF 与入射辐射度成正比。
*   **自发光网格采样 (MIS)**:
    *   构建光照 BVH，支持剔除零辐射三角形。
    *   支持分层重要性采样。
*   **解析光源采样**:
    *   支持点光、方向光、面光源（四边形/圆盘/球体）。
    *   等概率选择光源。

---

### 6. 验证与调试工具

1.  **MinimalPathTracer**:
    *   一个独立的、朴素的路径追踪 Pass。
    *   不使用 MIS，用于生成 Ground Truth (真值) 进行验证，但收敛较慢。
2.  **ErrorMeasurePass**:
    *   计算源图像与参考图像之间的误差。
3.  **着色器调试 (Shader Print/Assert)**:
    *   支持在 HLSL/Slang 中使用 `print()` 和 `assert()`。
    *   在 UI 中开启 "Pixel Debug" 并点击像素即可查看输出。
    *   建议冻结随机种子以避免数值闪烁。



























