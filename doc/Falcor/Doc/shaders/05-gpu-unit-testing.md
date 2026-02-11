# GPU单元测试 (GPU Unit Testing)

本文档介绍如何为Shader编写GPU单元测试。GPU单元测试用于验证Shader的计算逻辑是否正确。

## 概述

Falcor的GPU单元测试系统允许你：
- 在GPU上运行小型Shader程序
- 设置输入数据
- 验证输出结果
- 自动化测试流程

测试位置：`Source/Tools/FalcorTest/Tests/`

## 基本结构

### 最简单的GPU测试

```cpp
// Source/Tools/FalcorTest/Tests/MyShaderTests.cpp

#include "Testing/UnitTest.h"

GPU_TEST(SimpleTest)
{
    // 1. 创建并编译shader程序
    ctx.createProgram("Tests/MyTest.cs.slang", "main");

    // 2. 分配输出缓冲区
    ctx.allocateStructuredBuffer("result", 1);

    // 3. 设置输入参数
    ctx["CB"]["value"] = 5.0f;

    // 4. 运行程序
    ctx.runProgram();

    // 5. 读取结果
    const float* result = ctx.mapBuffer<const float>("result");

    // 6. 验证结果
    EXPECT_EQ(result[0], 25.0f);

    // 7. 解除映射
    ctx.unmapBuffer("result");
}
```

### 对应的Shader文件

```hlsl
// Source/Tools/FalcorTest/Tests/MyTest.cs.slang

RWStructuredBuffer<float> result;

cbuffer CB
{
    float value;
};

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    result[0] = value * value;
}
```

## 详细示例：测试FilmGrain的噪声函数

### 创建测试文件

在 `Source/Tools/FalcorTest/Tests/` 目录下创建 `FilmGrainTests.cpp`：

```cpp
#include "Testing/UnitTest.h"

namespace
{
// 测试噪声函数的基本功能
GPU_TEST(FilmGrainNoise)
{
    // 加载shader
    ctx.createProgram("Tests/FilmGrainNoise.cs.slang", "testNoise");

    // 分配输出缓冲区 - 存储100个噪声值
    const uint32_t kNumSamples = 100;
    ctx.allocateStructuredBuffer("result", kNumSamples);

    // 设置测试参数
    ctx["CB"]["frameTime"] = 1.0f;
    ctx["CB"]["numSamples"] = kNumSamples;

    // 运行shader
    ctx.runProgram(kNumSamples, 1, 1);

    // 读取结果
    const float* result = ctx.mapBuffer<const float>("result");

    // 验证：所有值应该在[0, 1]范围内
    for (uint32_t i = 0; i < kNumSamples; i++)
    {
        EXPECT_GE(result[i], 0.0f) << "Sample " << i;
        EXPECT_LE(result[i], 1.0f) << "Sample " << i;
    }

    // 验证：值应该有足够的变化（不是全部相同）
    float minValue = result[0];
    float maxValue = result[0];
    for (uint32_t i = 1; i < kNumSamples; i++)
    {
        minValue = std::min(minValue, result[i]);
        maxValue = std::max(maxValue, result[i]);
    }

    EXPECT_GT(maxValue - minValue, 0.5f) << "Noise variation too low";

    ctx.unmapBuffer("result");
}

// 测试Film Grain效果的像素处理
GPU_TEST(FilmGrainPixelProcessing)
{
    ctx.createProgram("Tests/FilmGrainPixel.cs.slang", "testPixel");

    // 创建输入纹理（2x2像素）
    const uint32_t kWidth = 2;
    const uint32_t kHeight = 2;

    // 分配纹理
    ctx.allocateTexture2D("input", kWidth, kHeight, ResourceFormat::RGBA32Float);
    ctx.allocateTexture2D("output", kWidth, kHeight, ResourceFormat::RGBA32Float);

    // 设置输入数据
    float4 inputData[4] = {
        float4(1.0f, 0.0f, 0.0f, 1.0f),  // 红色
        float4(0.0f, 1.0f, 0.0f, 1.0f),  // 绿色
        float4(0.0f, 0.0f, 1.0f, 1.0f),  // 蓝色
        float4(1.0f, 1.0f, 1.0f, 1.0f),  // 白色
    };
    ctx.setTexture2D("input", inputData);

    // 设置参数
    ctx["CB"]["grainAmount"] = 0.0f;  // 无颗粒，应该直接复制
    ctx["CB"]["enabled"] = 0u;

    // 运行
    ctx.runProgram(kWidth, kHeight, 1);

    // 读取输出
    const float4* output = ctx.mapTexture2D<const float4>("output");

    // 验证：无颗粒时输出应该等于输入
    for (uint32_t i = 0; i < 4; i++)
    {
        EXPECT_EQ(output[i].r, inputData[i].r);
        EXPECT_EQ(output[i].g, inputData[i].g);
        EXPECT_EQ(output[i].b, inputData[i].b);
        EXPECT_EQ(output[i].a, inputData[i].a);
    }

    ctx.unmapTexture2D("output");
}
}
```

### 测试Shader实现

```hlsl
// Source/Tools/FalcorTest/Tests/FilmGrainNoise.cs.slang

RWStructuredBuffer<float> result;

cbuffer CB
{
    float frameTime;
    uint numSamples;
};

static const float PI = 3.14159265359;

// 从FilmGrain提取的噪声函数
float computeNoise(uint2 pixel, float time)
{
    return frac(
        10000.0 * sin(
            (float(pixel.x) + float(pixel.y) * time) * PI
        )
    );
}

[numthreads(256, 1, 1)]
void testNoise(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint idx = dispatchThreadId.x;
    if (idx >= numSamples)
        return;

    // 生成不同的像素坐标来测试
    uint2 pixel = uint2(idx % 10, idx / 10);

    result[idx] = computeNoise(pixel, frameTime);
}
```

```hlsl
// Source/Tools/FalcorTest/Tests/FilmGrainPixel.cs.slang

Texture2D<float4> input;
RWTexture2D<float4> output;

cbuffer CB
{
    float grainAmount;
    uint enabled;
};

static const float PI = 3.14159265359;

[numthreads(8, 8, 1)]
void testPixel(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint2 pixel = dispatchThreadId.xy;
    uint2 dimensions;
    output.GetDimensions(dimensions.x, dimensions.y);

    if (any(pixel >= dimensions))
        return;

    float4 color = input[pixel];

    if (enabled == 1)
    {
        float randomIntensity = frac(
            10000.0 * sin(
                (float(pixel.x) + float(pixel.y) * 1.0) * PI
            )
        );
        float amount = grainAmount * randomIntensity;
        color.rgb += amount;
    }

    output[pixel] = color;
}
```

## GPUUnitTestContext API

### 程序管理

```cpp
// 创建并编译shader程序
ctx.createProgram("path/to/shader.slang", "entryPoint");

// 创建带定义的程序
ctx.createProgram("shader.slang", "main", Program::DefineList{{"USE_FEATURE", "1"}});

// 运行程序
ctx.runProgram();                        // 默认1x1x1
ctx.runProgram(width, height, depth);    // 指定dispatch大小
```

### 缓冲区操作

```cpp
// 分配结构化缓冲区
ctx.allocateStructuredBuffer("bufferName", elementCount);
ctx.allocateStructuredBuffer("bufferName", elementCount, elementSize);

// 映射缓冲区读取数据
const MyStruct* data = ctx.mapBuffer<const MyStruct>("bufferName");
// ... 使用data ...
ctx.unmapBuffer("bufferName");

// 映射缓冲区写入数据
MyStruct* data = ctx.mapBuffer<MyStruct>("bufferName");
// ... 修改data ...
ctx.unmapBuffer("bufferName");
```

### 纹理操作

```cpp
// 分配2D纹理
ctx.allocateTexture2D("texName", width, height, ResourceFormat::RGBA32Float);

// 设置纹理数据
float4 data[width * height];
// ... 填充data ...
ctx.setTexture2D("texName", data);

// 读取纹理数据
const float4* output = ctx.mapTexture2D<const float4>("texName");
// ... 使用output ...
ctx.unmapTexture2D("texName");
```

### 设置参数

```cpp
// Constant Buffer参数
ctx["CB"]["floatParam"] = 1.0f;
ctx["CB"]["intParam"] = 42;
ctx["CB"]["vectorParam"] = float3(1, 2, 3);

// 直接访问根变量
ctx["globalTexture"] = pTexture;
```

## 断言宏

### 基本断言

```cpp
EXPECT(condition);                      // 期望为真
EXPECT_EQ(actual, expected);            // 期望相等
EXPECT_NE(actual, expected);            // 期望不等
EXPECT_LT(a, b);                        // a < b
EXPECT_LE(a, b);                        // a <= b
EXPECT_GT(a, b);                        // a > b
EXPECT_GE(a, b);                        // a >= b
```

### 浮点数比较

```cpp
// 使用epsilon比较
const float epsilon = 1e-5f;
EXPECT(std::abs(actual - expected) < epsilon);

// 或使用相对误差
EXPECT_EQ(actual, expected);  // 内部有合理的tolerance
```

### 添加额外信息

```cpp
EXPECT_EQ(result, expected) << "Pixel (" << x << ", " << y << ")";
EXPECT_GT(value, 0.0f) << "Value must be positive, got: " << value;
```

## 运行测试

### 方法1：使用批处理脚本

```bash
# 运行所有GPU测试
tests\run_unit_tests.bat

# 运行特定的测试（使用过滤器）
tests\run_unit_tests.bat -f FilmGrain

# 只运行GPU测试（跳过CPU测试）
tests\run_unit_tests.bat -c gpu
```

### 方法2：直接运行可执行文件

```bash
# 运行所有测试
build\windows-vs2022\bin\Release\FalcorTest.exe

# 运行特定测试
build\windows-vs2022\bin\Release\FalcorTest.exe -f FilmGrain

# 使用DX12
build\windows-vs2022\bin\Release\FalcorTest.exe -d d3d12
```

### 方法3：Visual Studio调试

1. 设置 `FalcorTest` 为启动项目
2. 右键 → Properties → Debugging → Command Arguments
3. 添加参数：`-f MyTest`
4. 按F5调试

## 测试组织

### 多个相关测试

```cpp
// 使用命名空间组织
namespace FilmGrainTests
{
GPU_TEST(NoiseFunction)
{
    // 测试噪声函数
}

GPU_TEST(GrainAmount)
{
    // 测试颗粒强度
}

GPU_TEST(EnableDisable)
{
    // 测试开关功能
}
}
```

### 跳过测试

```cpp
// 临时跳过测试
GPU_TEST(BrokenTest, "Skipped due to issue #123")
{
    // 这个测试会被跳过
}
```

## 实用测试模式

### 模式1：参数化测试

```cpp
GPU_TEST(FilmGrainParameterized)
{
    ctx.createProgram("Tests/FilmGrain.cs.slang", "main");
    ctx.allocateStructuredBuffer("result", 1);

    // 测试不同的参数值
    float testValues[] = {0.0f, 0.01f, 0.05f, 0.1f};

    for (float grainAmount : testValues)
    {
        ctx["CB"]["grainAmount"] = grainAmount;
        ctx.runProgram();

        const float* result = ctx.mapBuffer<const float>("result");
        // 验证结果...
        ctx.unmapBuffer("result");
    }
}
```

### 模式2：边界测试

```cpp
GPU_TEST(FilmGrainBoundaryConditions)
{
    ctx.createProgram("Tests/FilmGrain.cs.slang", "main");
    ctx.allocateTexture2D("output", 1920, 1080, ResourceFormat::RGBA32Float);

    // 测试边界像素
    ctx["CB"]["pixelX"] = 0u;      // 左边界
    ctx["CB"]["pixelY"] = 0u;      // 上边界
    ctx.runProgram();

    ctx["CB"]["pixelX"] = 1919u;   // 右边界
    ctx["CB"]["pixelY"] = 1079u;   // 下边界
    ctx.runProgram();

    // 验证没有崩溃或错误值...
}
```

### 模式3：性能测试

```cpp
GPU_TEST(FilmGrainPerformance)
{
    ctx.createProgram("Tests/FilmGrain.cs.slang", "main");
    ctx.allocateTexture2D("input", 1920, 1080, ResourceFormat::RGBA32Float);
    ctx.allocateTexture2D("output", 1920, 1080, ResourceFormat::RGBA32Float);

    // 预热
    ctx.runProgram(1920, 1080, 1);

    // 计时
    auto start = std::chrono::high_resolution_clock::now();

    const int iterations = 100;
    for (int i = 0; i < iterations; i++)
    {
        ctx.runProgram(1920, 1080, 1);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    float avgTime = duration.count() / (float)iterations / 1000.0f;  // ms

    // 这不是严格的断言，只是性能参考
    logInfo("Average execution time: {:.3f} ms", avgTime);

    // 可选：设置性能预期
    EXPECT_LT(avgTime, 1.0f) << "Performance regression detected";
}
```

## 常见问题

### Q: 测试通过但结果明显错误？
A: 检查：
1. epsilon值是否合适
2. 是否正确映射和解除映射缓冲区
3. dispatch大小是否正确

### Q: 如何调试失败的GPU测试？
A:
1. 使用Visual Studio调试FalcorTest
2. 在C++测试代码中设置断点
3. 使用PIX或NSight Graphics捕获GPU调用
4. 添加输出日志到shader

### Q: 测试很慢怎么办？
A:
1. 减小测试数据大小
2. 只在需要时运行完整测试
3. 使用过滤器运行特定测试

## 下一步

- [06-图像测试](./06-image-testing.md): 学习端到端的图像测试
- [07-运行单独测试](./07-running-tests.md): 学习测试运行器的高级用法
- [10-调试技巧](./10-debugging-tips.md): 深入的调试技巧

## 参考

完整的单元测试文档：`docs/development/unit-testing.md`
