# Shader开发指南 (Shader Development Guide)

本指南详细介绍在Falcor中开发Shader的完整流程，包括创建RenderPass、编写Shader、与C++集成、测试以及调试。

This guide provides a comprehensive workflow for developing shaders in Falcor, including creating RenderPasses, writing shaders, C++ integration, testing, and debugging.

## 目录 (Table of Contents)

1. [快速开始 (Quick Start)](./01-quick-start.md)
2. [创建RenderPass (Creating a RenderPass)](./02-creating-renderpass.md)
3. [编写Shader (Writing Shaders)](./03-writing-shaders.md)
4. [C++与Shader集成 (C++ and Shader Integration)](./04-cpp-shader-integration.md)
5. [GPU单元测试 (GPU Unit Testing)](./05-gpu-unit-testing.md)
6. [图像测试 (Image Testing)](./06-image-testing.md)
7. [运行单独测试 (Running Individual Tests)](./07-running-tests.md)
8. [Mogwai热更新Shader (Hot Reloading Shaders in Mogwai)](./08-hot-reload-mogwai.md)
9. [完整示例：FilmGrain (Complete Example: FilmGrain)](./09-filmgrain-example.md)
10. [调试技巧 (Debugging Tips)](./10-debugging-tips.md)

## 平台要求 (Platform Requirements)

本指南针对以下平台配置：
- **IDE**: Visual Studio 2022
- **Graphics API**: DirectX 12
- **OS**: Windows

## 示例：FilmGrain RenderPass

整个文档将以 `FilmGrain` RenderPass 作为实际案例，展示完整的开发流程。

The entire documentation uses the `FilmGrain` RenderPass as a practical example to demonstrate the complete development workflow.

## 快速参考 (Quick Reference)

### 创建新的RenderPass
```bash
# 使用批处理脚本创建新的RenderPass
tools\make_new_render_pass.bat YourPassName
```

### 运行测试
```bash
# 运行图像测试
tests\run_image_tests.bat

# 运行GPU单元测试
tests\run_unit_tests.bat

# 运行特定的图像测试
tests\run_image_tests.bat --filter FilmGrain
```

### Mogwai热更新
在Mogwai中按 `F5` 即可重新加载shader文件。

Press `F5` in Mogwai to hot reload shader files.

## 文件组织 (File Organization)

典型的RenderPass包含以下文件：

```
Source/RenderPasses/YourPass/
├── CMakeLists.txt           # CMake构建配置
├── YourPass.h               # C++头文件
├── YourPass.cpp             # C++实现
└── YourPass.cs.slang        # Compute Shader (或其他shader类型)
```

## 相关资源 (Related Resources)

- [Falcor单元测试文档](../development/unit-testing.md)
- [Falcor Python文档](../falcor-in-python.md)
- [RenderPass开发文档](../renderpasses/README.md)
