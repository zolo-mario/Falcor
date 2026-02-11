# ToneMapper

## 概述

ToneMapper实现了色调映射，包括自动曝光和白平衡功能。支持多种色调映射算子，用于将高动态范围（HDR）图像转换为显示范围。

## 输入输出参数表

| 参数名 | 类型 | 方向 | 描述 | 是否必需 |
|--------|------|------|------|----------|
| `src` | Texture2D\<float4\> | 输入 | HDR颜色缓冲区 | 是 |
| `dst` | Texture2D\<float4\> | 输出 | LDR颜色缓冲区（clamped到[0,1]） | 是 |
| `exposure` | Buffer | 输入 | 曝光值（可选） | 否 |
| `outputLuminance` | Texture2D\<float\> | 输出 | 对数亮度（可选） | 否 |
| `luminanceLod` | Buffer | 输出 | 平均亮度MIP级别（可选） | 否 |

## 属性表

| 属性名 | 类型 | 默认值 | 描述 |
|--------|------|--------|------|
| `mode` | ExposureMode | AperturePriority | 曝光模式 |
| `exposureCompensation` | float | 0.0 | 曝光补偿（F-stops） |
| `autoExposure` | bool | true | 自动曝光开关 |
| `exposureValue` | float | 0.0 | 曝光值（EV） |
| `filmSpeed` | float | 100.0 | 胶片速度（ISO） |
| `fNumber` | float | 1.0 | 光圈值（f-stop） |
| `shutterSpeed` | float | 1.0 | 快门速度 |
| `whiteBalance` | bool | true | 白平衡开关 |
| `whitePoint` | float | 6500.0 | 白点（Kelvin） |
| `operator` | ToneMapOp | Aces | 色调映射算子 |
| `clampOutput` | bool | true | 钳制输出开关 |

### 曝光模式（ExposureMode）
- `AperturePriority` - 光圈优先
- `ShutterPriority` - 快门优先

### 色调映射算子（ToneMapOp）
- `Aces` - Academy Color Encoding System
- `Clamp` - 简单钳制
- `Linear` - 线性映射
- `LinearSRGB` - 线性到sRGB
- `Reinhard` - Reinhard映射
- `ModifiedReinhard` - 改进的Reinhard
- `Uc2Hable` - Uncharted 2 Hable

## Shader详细注解

### ToneMapping.ps.slang

色调映射像素着色器。

**主要函数：**

**toneMap()**
```cpp
float3 toneMap(float3 color)
```
应用选定的色调映射算子。

支持的操作符：
- Aces：使用ACES色调映射曲线
- Reinhard：经典的Reinhard色调映射
- ModifiedReinhard：改进的Reinhard，支持白点
- Uc2Hable：Uncharted 2色调映射

**applyExposure()**
```cpp
float3 applyExposure(float3 color)
```
根据曝光设置调整亮度。

**computeExposure()**
```cpp
float computeExposure(float averageLuminance)
```
基于平均亮度和曝光参数计算曝光值。

**autoWhiteBalance()**
```cpp
float3x3 autoWhiteBalance(float3 pixelColor)
```
自动白平衡调整（如果启用）。

### Luminance.ps.slang

计算图像亮度的像素着色器。

**主要函数：**

**computeLuminance()**
```cpp
float computeLuminance(float3 color)
```
计算像素的对数亮度。

**logLuminance()**
```cpp
float logLuminance(float luminance)
```
计算对数亮度值：log2(luminance)。

**输出：**
- 对数亮度纹理
- 平均亮度MIP级别

## 色调映射算子详解

### 1. ACES（Academy Color Encoding System）
```
float3 aces(float3 x)
{
    // RRT + ODT
    float3 a = x * (x + 0.02457864) - 0.000090537;
    float3 b = x * (x + 0.4329510) + 0.238082;
    float3 c = x * (x + 0.8726951) + 0.301883;
    return saturate(a / b - c);
}
```

### 2. Reinhard
```
float3 reinhard(float3 x)
{
    float luminance = computeLuminance(x);
    return x / (1.0 + luminance);
}
```

### 3. Modified Reinhard
```
float3 modifiedReinhard(float3 x, float maxWhite)
{
    float luminance = computeLuminance(x);
    float numerator = luminance * (1.0 + luminance / (maxWhite * maxWhite));
    float denominator = 1.0 + luminance;
    float scale = numerator / denominator;
    return x * scale;
}
```

### 4. Uncharted 2 (Uc2Hable)
```
float3 uc2Hable(float3 x)
{
    float a = 0.15;
    float b = 0.50;
    float c = 0.10;
    float d = 0.20;
    float e = 0.02;
    float f = 0.30;

    float3 curr = ((x * (a * x + c * b) + d * e) / 
                 (x * (a * x + b) + d * f)) - e / f;

    float white = ((1.0 * (a * 1.0 + c * b) + d * e) / 
                 (1.0 * (a * 1.0 + b) + d * f)) - e / f;

    return curr / white;
}
```

## 自动曝光算法

**曝光方程：**
```
exposure = log2(frameTargetLuminance / currentLuminance) + exposureCompensation
```

其中：
- `frameTargetLuminance` - 目标帧亮度（中间灰）
- `currentLuminance` - 当前平均亮度
- `exposureCompensation` - 用户调整（F-stops）

**曝光控制：**
- AperturePriority模式：固定f值，调整快门速度
- ShutterPriority模式：固定快门速度，调整f值

## 白平衡

**色温调整：**
使用Kelvin色温调整RGB响应。

**范围：** 1000K（暖色）到10000K（冷色）

**算法：** 基于CIE颜色匹配函数的白平衡矩阵。

## 使用示例

```python
# 创建ToneMapper
tone_mapper = ToneMapper.create(device, props)

# 配置色调映射
props = Properties()
props['operator'] = 'Aces'
props['exposureCompensation'] = 0.5
props['autoExposure'] = True
tone_mapper.setProperties(props)
```

## 算法流程

```
ToneMapper执行流程：

开始
  │
  ├─ 计算亮度
  │   ├─ 计算每个像素的亮度
  │   ├─ 生成亮度MIP链
  │   └─ 读取平均亮度
  │
  ├─ 计算曝光
  │   ├─ 如果自动曝光启用：
  │   │   └─ exposure = log2(target / avgLuminance) + compensation
  │   ├─ 否则使用用户曝光值
  │   └─ 应用曝光补偿
  │
  ├─ 白平衡（如果启用）
  │   └─ 应用色温矩阵
  │
  ├─ 色调映射
  │   ├─ 对每个像素：
  │   │   ├─ 应用曝光
  │   │   ├─ 应用选定的色调映射算子
  │   │   └─ 如果启用则钳制到[0,1]
  │   └─ 输出到目标缓冲区
  │
  └─ 结束
```

## 参考文档

- [ToneMapper.h](../Source/RenderPasses/ToneMapper/ToneMapper.h) - C++实现
- [ToneMapping.ps.slang](../Source/RenderPasses/ToneMapper/ToneMapping.ps.slang) - 色调映射着色器
- [Luminance.ps.slang](../Source/RenderPasses/ToneMapper/Luminance.ps.slang) - 亮度计算着色器
- [ToneMapperParams.slang](../Source/RenderPasses/ToneMapper/ToneMapperParams.slang) - 参数定义
