---
name: slang-user-guide
description: Slang 入门教程与 FalcorTest 示例。用于在 SlangUserGuide 中添加新 Slang 示例、创建对应 GPU 测试、运行教程测试。位置：Source/Tools/FalcorTest/SlangUserGuide/。
---

# Slang User Guide

## 目录结构

```
Source/Tools/FalcorTest/SlangUserGuide/
├── 01-get-started/
│   └── hello-world.slang      # Slang 计算着色器示例
├── HelloWorldTests.cpp        # 对应 GPU 单元测试
└── (未来章节...)
```

## 添加新 Slang 示例

1. **创建 .slang 文件**：在 `SlangUserGuide/<章节>/` 下创建，如 `02-xxx/example.slang`
2. **更新 CMakeLists.txt**：在 `Source/Tools/FalcorTest/CMakeLists.txt` 的 `target_sources` 中添加：
   ```cmake
   SlangUserGuide/<章节>/<文件名>.slang
   ```
3. **创建 GPU 测试**：在 `SlangUserGuide/` 下创建或扩展 `*Tests.cpp`，使用 `GPU_TEST(name)`、`ctx.createProgram()`、`ctx.allocateStructuredBuffer()`、`ctx.runProgram()`、`ctx.readBuffer()`
4. **更新 CMakeLists.txt**：添加测试 cpp 到 `target_sources`

## 运行 SlangUserGuide 测试

```bash
tests/run_unit_tests.bat --test-suite HelloWorldTests
```

套件名 = 源文件名（不含扩展名），如 `HelloWorldTests.cpp` → `HelloWorldTests`。

## 参考示例

- **hello-world.slang**：StructuredBuffer + RWStructuredBuffer，`buffer0[i] + buffer1[i] -> result[i]`
- **HelloWorldTests.cpp**：`createProgram("SlangUserGuide/01-get-started/hello-world.slang", "computeMain")`，`allocateStructuredBuffer` 三个 buffer，`runProgram(nElements, 1, 1)`，`readBuffer<float>("result")` 验证
