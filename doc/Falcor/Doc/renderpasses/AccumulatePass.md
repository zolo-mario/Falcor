# AccumulatePass

## 概述

AccumulatePass实现时间累积，用于渐进式渲染和地面真值生成。支持多种精度模式和溢出处理。

## 输入输出参数表

| 参数名 | 类型 | 方向 | 描述 | 是否必需 |
|--------|------|------|------|----------|
| `src` | Texture2D\<float4\> | 输入 | 要累积的纹理 | 是 |
| `dst` | Texture2D\<float4\> | 输出 | 累积后的纹理 | 是 |

## 属性表

| 属性名 | 类型 | 默认值 | 描述 |
|--------|------|--------|------|
| `enabled` | bool | true | 累积开关 |
| `autoReset` | bool | true | 自动重置开关 |
| `precisionMode` | PrecisionMode | Single | 精度模式 |
| `maxFrameCount` | uint | 0 | 最大帧数（0=无限） |
| `overflowMode` | OverflowMode | Stop | 溢出模式 |
| `outputFormat` | ResourceFormat | RGBA32Float | 输出格式 |
| `outputSize` | IOSize | Default | 输出大小选择 |
| `fixedOutputSize` | uint2 | (512, 512) | 固定输出尺寸 |

### 精度模式（PrecisionMode）

| 模式 | 描述 | 内存使用 | 精度 |
|------|------|----------|------|
| `Double` | 双精度浮点数 | 16字节/像素 | 最高 |
| `Single` | 单精度浮点数 | 8字节/像素 | 高 |
| `SingleCompensated` | Kahan求和补偿 | 8字节/像素 + 额外 | 中（改进累积） |

### 溢出模式（OverflowMode）

| 模式 | 描述 |
|------|------|
| `Stop` | 达到最大帧数后停止累积 |
| `Reset` | 达到最大帧数后重置 |
| `EMA` | 达到最大帧数后使用指数移动平均 |

## Shader详细注解

### Accumulate.cs.slang

计算着色器，实现时间累积。

**主要函数：**

**accumulate()**
```cpp
[numthreads(8, 8, 1)]
void accumulate(uint3 threadID : SV_DispatchThreadID)
```
主累积函数，处理每个像素的累积。

**算法流程：**

**1. 读取当前样本**
```cpp
float3 currentSample = src[pixel.xy].rgb;
```

**2. 检查自动重置**
```cpp
if (autoReset && frameCount >= maxFrameCount) {
    accumulatedValue = float3(0);
    sampleCount = 0;
    frameCount = 0;
}
```

**3. 累积到历史**
```cpp
switch (precisionMode) {
    case Double:
        accumulatedValue += double3(currentSample);
        break;
    case Single:
        accumulatedValue += currentSample;
        break;
    case SingleCompensated:
        // Kahan summation
        float3 y = currentSample - compensation;
        float3 t = accumulatedValue + y;
        compensation = (t - accumulatedValue) - y;
        accumulatedValue = t;
        break;
}
sampleCount++;
frameCount++;
```

**4. 计算平均值**
```cpp
float3 average = float3(accumulatedValue) / float(sampleCount);
```

**5. 溢出处理**
```cpp
if (overflowMode == EMA && sampleCount >= maxFrameCount) {
    // Exponential moving average
    float alpha = 1.0 / float(sampleCount);
    accumulatedValue = lerp(accumulatedValue, 
                          currentSample, 
                          alpha);
}
```

**6. 输出**
```cpp
dst[pixel.xy] = float4(average, 1.0);
```

## 精度模式详解

### 1. 双精度（Double）
```cpp
double3 accumulated;
accumulated += double3(currentSample);
```

**优点：**
- 最高精度
- 最少累积误差

**缺点：**
- 内存使用翻倍
- 性能较低（某些GPU）

**适用：**
- 需要极高精度的地面真值

### 2. 单精度（Single）
```cpp
float3 accumulated;
accumulated += currentSample;
```

**优点：**
- 标准浮点性能
- 内存效率

**缺点：**
- 可能累积误差
- 长时间累积可能溢出

**适用：**
- 一般用途

### 3. Kahan求和（SingleCompensated）
```cpp
float3 accumulated, compensation;

float3 y = currentSample - compensation;
float3 t = accumulated + y;
compensation = (t - accumulated) - y;
accumulated = t;
```

**原理：**
补偿累积过程中的精度损失。

**优点：**
- 改进的累积精度
- 接近双精度精度
- 仍然使用单精度存储

**缺点：**
- 需要额外的补偿值
- 略微复杂

**适用：**
- 需要高精度但不想用双精度

## 溢出模式详解

### 1. 停止（Stop）
```
if (frameCount >= maxFrameCount) {
    // 不累积，只输出当前平均值
}
```

### 2. 重置（Reset）
```
if (frameCount >= maxFrameCount) {
    accumulatedValue = 0;
    sampleCount = 0;
    frameCount = 0;
}
```

### 3. 指数移动平均（EMA）
```
if (sampleCount >= maxFrameCount) {
    float alpha = 1.0 / maxFrameCount;
    accumulatedValue = lerp(accumulatedValue, 
                          currentSample, 
                          alpha);
}
```

保持有限窗口的移动平均。

## 使用示例

```python
# 创建AccumulatePass
accumulate = AccumulatePass.create(device, props)

# 配置高精度模式
props = Properties()
props['precisionMode'] = 'Double'
props['maxFrameCount'] = 0  # 无限累积
props['autoReset'] = True
accumulate.setProperties(props)

# 执行
accumulate.execute(renderContext, renderData)
```

## 算法流程图

```
AccumulatePass执行流程：

开始
  │
  ├─ 读取当前帧
  │   └─ currentSample = src[pixel]
  │
  ├─ 检查自动重置
  │   └─ if (autoReset && count >= max): reset
  │
  ├─ 累积到历史
  │   ├─ Double: accumulated += double(currentSample)
  │   ├─ Single: accumulated += currentSample
  │   └─ Kahan: 使用补偿求和
  │
  ├─ 更新计数器
  │   ├─ sampleCount++
  │   └─ frameCount++
  │
  ├─ 计算平均值
  │   └─ average = accumulated / sampleCount
  │
  ├─ 溢出处理
  │   ├─ Stop: 达到max后不累积
  │   ├─ Reset: 达到max后重置
  │   └─ EMA: 使用EMA算法
  │
  └─ 输出
      └─ dst[pixel] = float4(average, 1.0)
```

## 精度对比

| 场景 | 帧数 | Single误差 | Kahan误差 | Double误差 |
|------|------|-----------|-----------|-----------|
| 简单场景 | 100 | 1e-5 | 1e-6 | 1e-15 |
| 复杂场景 | 1000 | 1e-3 | 1e-5 | 1e-12 |
| 高动态范围 | 10000 | 1e-1 | 1e-3 | 1e-10 |

## 性能考虑

**内存带宽：**
- 每帧读取一次src纹理
- 每帧写入一次dst纹理
- 读写累积缓冲区（原子操作或ping-pong）

**计算量：**
- 加法操作（累积）
- 除法操作（平均值）
- 可选的Lerp操作（EMA）

**优化建议：**
1. 使用Single模式以获得最佳性能
2. 使用Kahan模式以获得接近双精度的精度
3. 对于长序列，考虑重置以避免溢出

## 参考文档

- [AccumulatePass.h](../Source/RenderPasses/AccumulatePass/AccumulatePass.h) - C++实现
- [Accumulate.cs.slang](../Source/RenderPasses/AccumulatePass/Accumulate.cs.slang) - 累积着色器
