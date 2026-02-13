---
name: slang-user-guide
description: Slang 入门教程与 FalcorTest 示例。用于在 SlangUserGuide 中添加新 Slang 示例、创建对应 GPU 测试、运行教程测试。位置：Source/Tools/FalcorTest/SlangUserGuide/。
---

# Slang User Guide

## 目录结构

```
Source/Tools/FalcorTest/SlangUserGuide/
├── 01-get-started/
│   ├── hello-world.slang
│   └── HelloWorldTests.cpp
├── 02-conventional-features/
│   ├── conventional-features.slang   # 标量、向量、结构体、枚举、函数等
│   └── ConventionalFeaturesTests.cpp
└── (未来章节...)
```

## GPU 测试框架参考（UnitTest.cpp）

**规范示例**：`Source/Tools/FalcorTest/Testing/UnitTest.cpp` 中的 `GPU_TEST(TestGPUTest)` 及 `Testing/UnitTest.cs.slang`。

### C++ 测试模板

```cpp
#include "Testing/UnitTest.h"

namespace Falcor
{
GPU_TEST(MyTestName)
{
    ctx.createProgram("path/to/shader.slang", "main");  // 入口默认 "main"，可指定如 "computeMain"
    ctx.allocateStructuredBuffer("result", nElements);  // 可选: pInitData, initDataSize
    ctx["TestCB"]["field"] = value;                     // 设置 cbuffer 字段（若有）
    ctx.runProgram(width, height, depth);               // 或 runProgram() 默认 (1,1,1)

    std::vector<T> result = ctx.readBuffer<T>("result");
    EXPECT_EQ(result[i], expected);
}
} // namespace Falcor
```

### Shader 模板（UnitTest.cs.slang）

```slang
RWStructuredBuffer<float> result;

cbuffer TestCB
{
    int nValues;
    float scale;
};

void main()
{
    for (int i = 0; i < nValues; ++i)
        result[i] = scale * i;
}
```

### GPUUnitTestContext API（UnitTest.h）

| 方法 | 说明 |
|------|------|
| `createProgram(path, entry="main", defines, flags, shaderModel)` | 创建计算程序 |
| `allocateStructuredBuffer(name, nElements, pInitData?, initDataSize?)` | 分配 StructuredBuffer |
| `ctx["bufferName"]` / `ctx["cbName"]["field"]` | 绑定资源或设置 cbuffer |
| `runProgram(w, h, d)` | 调度 (w×h×d) 线程 |
| `readBuffer<T>(bufferName)` | 读回 buffer 内容 |
| `EXPECT_EQ`, `EXPECT_NE`, `EXPECT`, `ASSERT_EQ` 等 | 断言宏 |

## 添加新 Slang 示例

1. **创建 .slang 文件**：在 `SlangUserGuide/<章节>/` 下创建
2. **更新 CMakeLists.txt**：添加 `.slang` 和 `*Tests.cpp` 到 `target_sources`
3. **创建 GPU 测试**：按上述模板使用 `GPU_TEST`、`createProgram`、`allocateStructuredBuffer`、`runProgram`、`readBuffer`

## 运行 SlangUserGuide 测试

```bash
tests/run_unit_tests.bat --test-suite HelloWorldTests
```

套件名 = 源文件名（不含扩展名），如 `HelloWorldTests.cpp` → `HelloWorldTests`。

## 参考示例

- **Testing/UnitTest.cpp + UnitTest.cs.slang**：cbuffer + RWStructuredBuffer，`ctx["TestCB"]["field"]` 传参
- **01-get-started/hello-world.slang**：StructuredBuffer + RWStructuredBuffer，`buffer0[i] + buffer1[i] -> result[i]`
- **02-conventional-features/conventional-features.slang**：标量、向量、数组、结构体、枚举、函数（[Conventional Language Features](https://shader-slang.org/slang/user-guide/conventional-features.html)）
