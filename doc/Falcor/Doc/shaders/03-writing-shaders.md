# 编写Shader (Writing Shaders)

本文档详细介绍如何在Falcor中编写Shader代码。Falcor使用Slang着色语言，它是HLSL的超集。

## Slang语言简介

Slang是一种现代的着色语言，基于HLSL并添加了许多增强功能：
- **模块系统**: 更好的代码组织
- **泛型**: 支持模板编程
- **接口**: 面向对象的特性
- **自动微分**: 用于机器学习应用

对于DX12开发，Slang代码与HLSL高度兼容。

## Shader文件命名约定

```
<PassName>.<ShaderType>.slang
```

常见的ShaderType：
- `.cs.slang` - Compute Shader
- `.ps.slang` - Pixel Shader
- `.vs.slang` - Vertex Shader
- `.gs.slang` - Geometry Shader
- `.rt.slang` - Ray Tracing Shaders

示例：
- `FilmGrain.cs.slang` - FilmGrain的Compute Shader
- `GBuffer.ps.slang` - GBuffer的Pixel Shader

## Compute Shader基础结构

### 最小化示例

```hlsl
// MyShader.cs.slang

// 定义线程组大小
[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    // Shader代码
}
```

### 完整示例 - FilmGrain

```hlsl
// FilmGrain.cs.slang

// 1. 常量缓冲区 (Constant Buffer)
cbuffer CB
{
    float frameTime;     // 帧时间，用于动画
    float grainAmount;   // 颗粒强度
    uint enabled;        // 启用标志
}

// 2. 资源绑定
Texture2D gSrc;                    // 输入纹理（只读）
RWTexture2D<float4> gDst;          // 输出纹理（读写）

// 3. 常量定义
static const float PI = 3.14159265359;

// 4. 计算着色器入口点
[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    // 4.1 获取像素坐标
    uint2 pixel = dispatchThreadId.xy;

    // 4.2 获取纹理尺寸
    uint2 dimensions;
    gDst.GetDimensions(dimensions.x, dimensions.y);

    // 4.3 边界检查
    if (any(pixel >= dimensions))
        return;

    // 4.4 读取源颜色
    float4 color = gSrc[pixel];

    // 4.5 主要效果逻辑
    if (enabled == 1)
    {
        // 生成随机噪声
        float randomIntensity = frac(
            10000.0 * sin(
                (float(pixel.x) + float(pixel.y) * frameTime) * PI
            )
        );

        // 应用颗粒效果
        float amount = grainAmount * randomIntensity;
        color.rgb += amount;
    }

    // 4.6 写入输出
    gDst[pixel] = color;
}
```

## Shader组件详解

### 1. Constant Buffer (常量缓冲区)

```hlsl
cbuffer CB
{
    float myFloat;
    float3 myVector;
    float4x4 myMatrix;
    uint myInteger;
}
```

**对齐规则 (重要)**：
- `float` / `int` / `uint`: 4字节对齐
- `float2`: 8字节对齐
- `float3`: 16字节对齐（注意：占用16字节！）
- `float4`: 16字节对齐

**错误示例**：
```hlsl
cbuffer CB
{
    float a;      // 偏移 0
    float3 b;     // 偏移 16（不是4！）
    float c;      // 偏移 32
}
```

**正确做法**：
```hlsl
cbuffer CB
{
    float a;
    float c;
    float padding1;
    float padding2;
    float3 b;
}
```

或使用 `float4`：
```hlsl
cbuffer CB
{
    float4 data1;  // x=a, y=c, z和w未使用
    float3 b;
}
```

### 2. 纹理资源

#### 输入纹理（只读）

```hlsl
Texture2D<float4> gInput;           // 2D纹理
Texture2D<float> gDepth;            // 单通道纹理
Texture2DArray<float4> gArray;      // 纹理数组
TextureCube<float4> gCubemap;       // 立方体贴图
```

#### 输出纹理（读写）

```hlsl
RWTexture2D<float4> gOutput;        // UAV纹理
RWTexture2D<uint> gCounters;        // 整数纹理
```

#### 采样器

```hlsl
SamplerState gSampler;              // 采样器状态

// 使用采样器
float4 color = gInput.Sample(gSampler, uv);

// 直接加载（无过滤）
float4 color = gInput[pixel];
```

### 3. 结构化缓冲区

```hlsl
// 只读
StructuredBuffer<MyStruct> gInputBuffer;

// 读写
RWStructuredBuffer<MyStruct> gOutputBuffer;

// 定义结构体
struct MyStruct
{
    float3 position;
    float3 normal;
    float2 texCoord;
};

// 访问
MyStruct data = gInputBuffer[index];
gOutputBuffer[index] = data;
```

### 4. 线程组配置

```hlsl
[numthreads(X, Y, Z)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    // ...
}
```

**常用配置**：
- **通用计算**: `[numthreads(16, 16, 1)]` - 256个线程
- **1D处理**: `[numthreads(256, 1, 1)]`
- **小型任务**: `[numthreads(8, 8, 1)]` - 64个线程
- **大型任务**: `[numthreads(32, 32, 1)]` - 1024个线程（最大）

**最大限制**：
- 每个线程组最多 1024 个线程（X * Y * Z ≤ 1024）
- 每个维度最大 1024

### 5. 系统语义 (System Semantics)

```hlsl
[numthreads(16, 16, 1)]
void main(
    uint3 dispatchThreadId : SV_DispatchThreadID,    // 全局线程ID
    uint3 groupThreadId : SV_GroupThreadID,          // 组内线程ID
    uint3 groupId : SV_GroupID,                      // 组ID
    uint groupIndex : SV_GroupIndex                  // 组内扁平索引
)
{
    // dispatchThreadId = groupId * [16, 16, 1] + groupThreadId
    // groupIndex = groupThreadId.z * 16 * 16 + groupThreadId.y * 16 + groupThreadId.x
}
```

## 常用Shader模式

### 模式1：图像处理 (像FilmGrain)

```hlsl
Texture2D gInput;
RWTexture2D<float4> gOutput;

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint2 pixel = dispatchThreadId.xy;
    uint2 dimensions;
    gOutput.GetDimensions(dimensions.x, dimensions.y);

    if (any(pixel >= dimensions))
        return;

    // 读取输入
    float4 color = gInput[pixel];

    // 处理
    color.rgb = processColor(color.rgb);

    // 写入输出
    gOutput[pixel] = color;
}
```

### 模式2：采样器模式（模糊、缩放等）

```hlsl
Texture2D gInput;
RWTexture2D<float4> gOutput;
SamplerState gSampler;

cbuffer CB
{
    float2 texelSize;  // 1.0 / 纹理尺寸
}

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint2 pixel = dispatchThreadId.xy;
    uint2 dimensions;
    gOutput.GetDimensions(dimensions.x, dimensions.y);

    if (any(pixel >= dimensions))
        return;

    // 计算UV坐标
    float2 uv = (float2(pixel) + 0.5) / float2(dimensions);

    // 采样周围像素（例如：简单模糊）
    float4 color = float4(0, 0, 0, 0);
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            float2 offset = float2(x, y) * texelSize;
            color += gInput.Sample(gSampler, uv + offset);
        }
    }
    color /= 9.0;  // 平均

    gOutput[pixel] = color;
}
```

### 模式3：并行归约 (Parallel Reduction)

```hlsl
RWStructuredBuffer<float> gData;
groupshared float sData[256];  // 共享内存

[numthreads(256, 1, 1)]
void main(
    uint3 groupThreadId : SV_GroupThreadID,
    uint3 groupId : SV_GroupID
)
{
    uint tid = groupThreadId.x;
    uint gid = groupId.x * 256 + tid;

    // 加载到共享内存
    sData[tid] = gData[gid];
    GroupMemoryBarrierWithGroupSync();

    // 归约
    for (uint s = 128; s > 0; s >>= 1)
    {
        if (tid < s)
        {
            sData[tid] += sData[tid + s];
        }
        GroupMemoryBarrierWithGroupSync();
    }

    // 写回结果
    if (tid == 0)
    {
        gData[groupId.x] = sData[0];
    }
}
```

## 内置函数

### 数学函数

```hlsl
// 基础数学
float x = abs(value);
float x = sqrt(value);
float x = pow(base, exponent);
float x = exp(value);
float x = log(value);

// 三角函数
float x = sin(angle);
float x = cos(angle);
float x = tan(angle);

// 插值
float x = lerp(a, b, t);           // 线性插值
float x = smoothstep(min, max, x);  // 平滑插值

// 范围
float x = clamp(value, min, max);
float x = saturate(value);          // clamp(value, 0, 1)
float x = min(a, b);
float x = max(a, b);

// 向量
float x = dot(v1, v2);             // 点积
float3 x = cross(v1, v2);          // 叉积
float x = length(v);               // 长度
float3 x = normalize(v);           // 归一化
float x = distance(p1, p2);        // 距离
```

### 纹理函数

```hlsl
// 加载（无过滤）
float4 color = gTexture[pixel];
float4 color = gTexture.Load(int3(pixel, mipLevel));

// 采样（带过滤）
float4 color = gTexture.Sample(gSampler, uv);
float4 color = gTexture.SampleLevel(gSampler, uv, mipLevel);

// 尺寸
uint width, height;
gTexture.GetDimensions(width, height);

uint width, height, mipLevels;
gTexture.GetDimensions(0, width, height, mipLevels);
```

### 原子操作

```hlsl
RWTexture2D<uint> gCounter;

uint originalValue;
InterlockedAdd(gCounter[pixel], 1, originalValue);
InterlockedMin(gCounter[pixel], value, originalValue);
InterlockedMax(gCounter[pixel], value, originalValue);
InterlockedCompareExchange(gCounter[pixel], compare, value, originalValue);
```

## 调试技巧

### 1. 颜色可视化

```hlsl
// 可视化标量值
float value = computeValue();
gOutput[pixel] = float4(value, value, value, 1.0);

// 可视化向量
float3 normal = computeNormal();
gOutput[pixel] = float4(normal * 0.5 + 0.5, 1.0);  // 映射[-1,1]到[0,1]

// 可视化UV
gOutput[pixel] = float4(uv, 0, 1);

// 热力图
float t = saturate(value / maxValue);
float3 heatmap = lerp(float3(0, 0, 1), float3(1, 0, 0), t);  // 蓝到红
gOutput[pixel] = float4(heatmap, 1);
```

### 2. 条件输出

```hlsl
// 高亮特定区域
if (pixel.x < 100 && pixel.y < 100)
{
    gOutput[pixel] = float4(1, 0, 0, 1);  // 红色
}
else
{
    gOutput[pixel] = normalOutput;
}

// 调试特定像素
if (all(pixel == uint2(320, 240)))  // 屏幕中心
{
    // 可以在这里设置断点（使用PIX或NSight）
    gOutput[pixel] = float4(1, 1, 0, 1);  // 黄色标记
}
```

### 3. NaN/Inf检测

```hlsl
float4 color = computeColor();

// 检测无效值
bool hasNaN = any(isnan(color));
bool hasInf = any(isinf(color));

if (hasNaN || hasInf)
{
    gOutput[pixel] = float4(1, 0, 1, 1);  // 品红色表示错误
}
else
{
    gOutput[pixel] = color;
}
```

## 性能优化

### 1. 避免分支

**慢**：
```hlsl
if (condition)
    result = computeA();
else
    result = computeB();
```

**快**：
```hlsl
result = condition ? computeA() : computeB();
// 或
result = lerp(computeB(), computeA(), float(condition));
```

### 2. 使用共享内存

访问共享内存比全局内存快很多：

```hlsl
groupshared float4 sCache[16][16];

[numthreads(16, 16, 1)]
void main(uint3 groupThreadId : SV_GroupThreadID, ...)
{
    // 加载到共享内存
    sCache[groupThreadId.y][groupThreadId.x] = gInput[...];
    GroupMemoryBarrierWithGroupSync();

    // 从共享内存读取
    float4 value = sCache[groupThreadId.y][groupThreadId.x];
}
```

### 3. 向量化操作

**慢**：
```hlsl
color.r = processChannel(color.r);
color.g = processChannel(color.g);
color.b = processChannel(color.b);
```

**快**：
```hlsl
color.rgb = processChannel(color.rgb);  // 如果函数支持向量
```

## 下一步

- [04-C++与Shader集成](./04-cpp-shader-integration.md): 学习如何从C++调用Shader
- [05-GPU单元测试](./05-gpu-unit-testing.md): 学习如何测试Shader代码
- [10-调试技巧](./10-debugging-tips.md): 深入的Shader调试方法

## 参考资源

- [Slang官方文档](https://shader-slang.com/slang/user-guide/)
- [HLSL参考](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl)
- [Compute Shader详解](https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-compute-shader)
