---
name: GraphicsGems Samples Migration
overview: 将 `media/Graphics Gems Series Source Code`（Gems I-V，600+ 个公共领域 C 源文件）迁移为 5 个 Falcor `add_plugin()` 样本，每卷一个插件，通过 SampleBase + ImGui 展示算法列表与运行结果。
todos:
  - id: create-dir-structure
    content: 创建 Source/Samples/GraphicsGems/ 目录结构（GG1~GG5 子目录）并复制 media/ 中的核心算法 .c/.h 文件到各自 src/ 子目录
    status: pending
  - id: umbrella-cmake
    content: 创建 Source/Samples/GraphicsGems/CMakeLists.txt（添加 GG1~GG5 子目录）并更新 Source/Samples/CMakeLists.txt
    status: pending
  - id: gg1-plugin
    content: 实现 GG1 插件：CMakeLists.txt、GraphicsGems1.h、GraphicsGems1.cpp（SampleBase + ImGui 算法列表）
    status: pending
  - id: gg2-plugin
    content: 实现 GG2 插件：CMakeLists.txt、GraphicsGems2.h、GraphicsGems2.cpp
    status: pending
  - id: gg3-plugin
    content: 实现 GG3 插件：CMakeLists.txt、GraphicsGems3.h、GraphicsGems3.cpp
    status: pending
  - id: gg4-plugin
    content: 实现 GG4 插件：CMakeLists.txt、GraphicsGems4.h、GraphicsGems4.cpp
    status: pending
  - id: gg5-plugin
    content: 实现 GG5 插件：CMakeLists.txt、GraphicsGems5.h、GraphicsGems5.cpp（含 Sampling patterns 数据文件路径）
    status: pending
  - id: fix-compile-errors
    content: 处理 MSVC C 编译兼容性问题（K&R 语法、缺失头文件、符号冲突等）
    status: pending
isProject: false
---

# Graphics Gems Series → Falcor Samples 迁移

## 目标目录结构

```
Source/Samples/GraphicsGems/
├── CMakeLists.txt          ← add_subdirectory(GG1..5)
├── GG1/
│   ├── CMakeLists.txt
│   ├── GraphicsGems1.h     ← SampleBase 插件头
│   ├── GraphicsGems1.cpp   ← SampleBase 插件实现
│   └── src/                ← 从 media/ 复制的 .c / .h 文件
├── GG2/ ... GG5/           ← 同上
```

## 关键文件与参考

- 现有样本模板: `[Source/Samples/ShaderToy/ShaderToy.h](Source/Samples/ShaderToy/ShaderToy.h)` / `[ShaderToy.cpp](Source/Samples/ShaderToy/ShaderToy.cpp)`
- 入口点: `[Source/Samples/CMakeLists.txt](Source/Samples/CMakeLists.txt)`（需增加 `add_subdirectory(GraphicsGems)`）
- 源数据: `media/Graphics Gems Series Source Code/Graphics Gems 1~5`

## 每个卷的 CMakeLists.txt 模式

```cmake
add_plugin(GraphicsGems1)

target_sources(GraphicsGems1 PRIVATE
    GraphicsGems1.h
    GraphicsGems1.cpp
    # 核心算法 .c 文件（不含有 main() 的演示文件）
    src/RayBox.c
    src/BoundSphere.c
    src/GGVecLib.c
    # ...其余算法文件
)

# 以 C 模式编译 .c 文件，避免 MSVC C++ 严格模式问题
set_source_files_properties(... PROPERTIES LANGUAGE C)

target_source_group(GraphicsGems1 "Samples/GraphicsGems")
```

## 插件 C++ 包装器模式

```cpp
// GraphicsGems1.h
class GraphicsGems1 : public SampleBase {
    FALCOR_PLUGIN_CLASS(GraphicsGems1, "GraphicsGems1",
                        SampleBase::PluginInfo{"Samples/GraphicsGems/GG1"});
public:
    explicit GraphicsGems1(SampleApp* pHost);
    static SampleBase* create(SampleApp* pHost);
    void onLoad(RenderContext* pRenderContext) override;
    void onGuiRender(Gui* pGui) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
};
```

`onGuiRender` 展示 ImGui 面板：每个算法一行，包含名称、来源页码、"运行"按钮和文本输出。

## C 源文件适配策略

- **仅复制核心算法文件**（不复制含 `main()` 的测试/演示文件）
- 测试/演示子目录（如 `Sturm/`, `AALines/`, `PolyScan/`）中的 `main.c` 跳过
- 各卷自带的 `GraphicsGems.h` 保留在各自 `src/` 目录，避免冲突
- `extern "C" { ... }` 在 .cpp 中包裹所有 C 函数声明
- `set_source_files_properties(...  PROPERTIES LANGUAGE C)` 强制 MSVC 用 C 模式编译
- 保留原始代码内容（含原版 magic numbers 与宏），不修改算法本身

## Graphics Gems V - Sampling patterns

该子目录仅含 `.data` 文件（无源码），作为数据资产随 GG5 插件分发，不参与编译。

## 需要修改的现有文件

- `[Source/Samples/CMakeLists.txt](Source/Samples/CMakeLists.txt)`: 增加 `add_subdirectory(GraphicsGems)`

