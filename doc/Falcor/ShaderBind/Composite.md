# Composite RenderPass Shader 绑定分析

## 1. Pass 基本信息

### 描述
Composite 是一个简单的合成 RenderPass，将两个缓冲区（输入 A 和 B）按指定模式混合到输出 C。每个输入可以独立缩放。支持混合模式：加法和乘法。如果输出缓冲区是整数格式，浮点值会使用四舍五入到最近偶数转换为整数。

### 入口点函数
- **main** - `[numthreads(16, 16, 1)]` Compute Shader：执行混合操作

## 2. 资源绑定清单

### 2.1 Constant Buffer 绑定

| C++ 绑定代码 | Shader 声明 | 资源类型 | 绑定方式 | 备注 |
|-------------|-------------|---------|---------|------|
| `var["CB"]["frameDim"]` | `uint2 frameDim;` | CBV | 手动 | 帧分辨率（宽高） |
| `var["CB"]["scaleA"]` | `float scaleA;` | CBV | 手动 | 输入 A 的缩放因子 |
| `var["CB"]["scaleB"]` | `float scaleB;` | CBV | 手动 | 输入 B 的缩放因子 |

### 2.2 Shader Resource View (SRV) 绑定

| C++ 绑定代码 | Shader 声明 | 资源类型 | 绑定方式 | 备注 |
|-------------|-------------|---------|---------|------|
| `var["A"]` | `Texture2D<float4> A;` | SRV | 手动 | 输入 A 纹理（可选） |
| `var["B"]` | `Texture2D<float4> B;` | SRV | 手动 | 输入 B 纹理（可选） |

### 2.3 Unordered Access View (UAV) 绑定

| C++ 绑定代码 | Shader 声明 | 资源类型 | 绑定方式 | 备注 |
|-------------|-------------|---------|---------|------|
| `var["output"]` | `RWTexture2D<float4/uint4/int4> output;` | UAV | 手动 | 输出纹理（类型取决于格式） |

## 3. Constant Buffer 结构映射

### CB 结构

```cpp
// C++ 端：通过 var["CB"][key] 逐个设置
// Shader 端定义：
cbuffer CB
{
    uint2 frameDim; // offset: 0, size: 8 bytes
    float scaleA;    // offset: 8, size: 4 bytes
    float scaleB;    // offset: 12, size: 4 bytes
}
// 总大小: 16 bytes（可能填充到 16 字节的倍数）
```

### 对齐分析

| 成员 | 偏移 | C++ 类型 | HLSL 类型 | 对齐 |
|------|------|----------|-----------|------|
| frameDim | 0 | uint2 | uint2 | 8 bytes |
| scaleA | 8 | float | float | 4 bytes |
| scaleB | 12 | float | float | 4 bytes |

## 4. 纹理/缓冲区生命周期

### 4.1 RenderGraph 管理的资源

| 资源 | 用途 | 生命周期 |
|------|------|---------|
| `A` | 输入 A 纹理 | 由 RenderGraph 在 execute() 前提供（可选） |
| `B` | 输入 B 纹理 | 由 RenderGraph 在 execute() 前提供（可选） |
| `out` | 输出纹理 | 由 RenderGraph 创建（reflect() 时声明） |

### 4.2 Pass 内部管理的资源

| 资源 | 类型 | 创建时机 | 销毁时机 | 说明 |
|------|------|---------|---------|------|
| `mCompositePass` | ref<ComputePass> | 构造函数 | 随 `Composite` 析构 | 计算着色器 pass |

## 5. 执行流程

### 5.1 execute() 函数资源绑定时序

```mermaid
sequenceDiagram
    participant RC as RenderContext
    participant CP as CompositePass
    participant RG as RenderGraph

    RG->>CP: renderData[kOutput]
    CP->>CP: 更新 mOutputFormat

    CP->>CP: mCompositePass->addDefines(getDefines())
    alt Defines 变化
        CP->>CP: mCompositePass->setVars(nullptr)
    end

    CP->>CP: var["CB"]["frameDim"] = mFrameDim
    CP->>CP: var["CB"]["scaleA"] = mScaleA
    CP->>CP: var["CB"]["scaleB"] = mScaleB
    CP->>CP: var["A"] = renderData.getTexture(kInputA) // 可以为 nullptr
    CP->>CP: var["B"] = renderData.getTexture(kInputB) // 可以为 nullptr
    CP->>CP: var["output"] = pOutput

    CP->>RC: execute(pRenderContext, mFrameDim.x, mFrameDim.y)
```

### 5.2 混合算法

**Add 模式**：
```hlsl
result = (scaleA * A[pixel]) + (scaleB * B[pixel]);
```

**Multiply 模式**：
```hlsl
result = (scaleA * A[pixel]) * (scaleB * B[pixel]);
```

### 5.3 输出格式转换

**Shader Defines**：
- `_OUTPUT_FORMAT_FLOAT` - 浮点输出（默认）
- `_OUTPUT_FORMAT_UINT` - 无符号整数输出
- `_OUTPUT_FORMAT_SINT` - 有符号整数输出

**整数转换**：
```hlsl
#if OUTPUT_FORMAT != OUTPUT_FORMAT_FLOAT
    result = round(result); // 四舍五入到最近偶数
#endif
output[pixel] = result;
```

### 5.4 Shader Defines 动态生成

**getDefines() 函数**：
```cpp
// 混合模式
uint32_t compositeMode = 0;
switch (mMode)
{
case Mode::Add:
    compositeMode = COMPOSITE_MODE_ADD;
    break;
case Mode::Multiply:
    compositeMode = COMPOSITE_MODE_MULTIPLY;
    break;
}

// 输出格式
uint32_t outputFormat = 0;
switch (getFormatType(mOutputFormat))
{
case FormatType::Uint:
    outputFormat = OUTPUT_FORMAT_UINT;
    break;
case FormatType::Sint:
    outputFormat = OUTPUT_FORMAT_SINT;
    break;
default:
    outputFormat = OUTPUT_FORMAT_FLOAT;
    break;
}

DefineList defines;
defines.add("COMPOSITE_MODE", std::to_string(compositeMode));
defines.add("OUTPUT_FORMAT", std::to_string(outputFormat));
```

### 5.5 绑定频率总结

| 资源类型 | 绑定频率 | 示例 |
|---------|---------|------|
| CBV | 每次 execute | frameDim, scaleA, scaleB |
| SRV (A/B) | 每次 execute | 输入 A 和 B 纹理（可选） |
| UAV (output) | 每次 execute | 输出纹理 |

## 6. 特殊机制说明

### 6.1 可选输入

输入 A 和 B 都是可选的：
```cpp
reflector.addInput(kInputA, "Input A")
    .bindFlags(ResourceBindFlags::ShaderResource)
    .flags(RenderPassReflection::Field::Flags::Optional);
reflector.addInput(kInputB, "Input B")
    .bindFlags(ResourceBindFlags::ShaderResource)
    .flags(RenderPassReflection::Field::Flags::Optional);
```

在 shader 中，如果输入为 nullptr，采样结果为未定义（通常为 0）。

### 6.2 空指针处理

```cpp
var["A"] = renderData.getTexture(kInputA); // Can be nullptr
var["B"] = renderData.getTexture(kInputB); // Can be nullptr
```

如果某个输入为 nullptr，shader 中的 `A[pixel]` 或 `B[pixel]` 可能返回未定义的值。建议确保输入连接正确。

### 6.3 整数输出

**格式转换**：
- 浮点运算结果使用 `round()` 转换为整数
- 使用四舍五入到最近偶数（银行家舍入）

**适用场景**：
- 将浮点结果输出到整数格式（如 RGBA8Unorm）
- 避免截断误差

### 6.4 输出格式支持

通过 `mOutputFormat` 成员控制：
- 默认：`RGBA32Float`
- 可在构造函数或通过属性设置

### 6.5 缩放因子

**独立缩放**：
- `scaleA` - 输入 A 的缩放因子
- `scaleB` - 输入 B 的缩放因子

**默认值**：1.0（无缩放）

**UI 控制**：
```cpp
widget.var("Scale A", mScaleA);
widget.var("Scale B", mScaleB);
```

### 6.6 线程组大小

```hlsl
[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadId: SV_DispatchThreadID)
```

Dispatch 调用：
```cpp
mCompositePass->execute(pRenderContext, mFrameDim.x, mFrameDim.y);
```

自动计算线程组数量。

### 6.7 边界检查

```hlsl
const uint2 pixel = dispatchThreadId.xy;
if (any(pixel >= frameDim))
    return;
```

确保线程不超出图像边界。

### 6.8 输入 A/B 可以为 nullptr

如果输入未连接：
- `renderData.getTexture(kInputA)` 返回 nullptr
- Shader 中 `Texture2D<float4> A` 绑定到无效资源
- 采样行为未定义

**建议**：在 UI 中显示警告或确保输入已连接。

### 6.9 复合模式对比

| 模式 | 算法 | 典型用途 |
|------|------|---------|
| Add | `scaleA * A + scaleB * B` | 光照叠加、混合两张图像 |
| Multiply | `scaleA * A * scaleB * B` | 遮罩应用、调制效果 |

### 6.10 性能考虑

- 使用计算着色器进行逐像素操作
- 线程组大小 16x16 适合现代 GPU
- 可选输入减少内存带宽（如果不需要某个输入）
