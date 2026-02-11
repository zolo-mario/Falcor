# TAA (Temporal Anti-Aliasing)

## 概述

TAA实现时间抗锯齿，通过混合当前帧和历史帧来减少锯齿和闪烁。

## 输入输出参数表

| 参数名 | 类型 | 方向 | 描述 | 是否必需 |
|--------|------|------|------|----------|
| `src` | Texture2D\<float4\> | 输入 | 当前帧颜色 | 是 |
| `mvec` | Texture2D\<float2\> | 输入 | 运动向量 | 是 |
| `depth` | Texture2D\<float\> | 输入 | 深度缓冲区 | 是 |
| `dst` | Texture2D\<float4\> | 输出 | 抗锯齿后的颜色 | 是 |

## 属性表

| 属性名 | 类型 | 默认值 | 描述 |
|--------|------|--------|------|
| `alpha` | float | 0.1 | 混合因子（0=完全历史，1=完全当前） |
| `colorBoxSigma` | float | 1.0 | 颜色盒方差阈值 |
| `antiFlicker` | bool | true | 抗闪烁开关 |

## Shader详细注解

### TAA.ps.slang

全屏像素着色器，实现TAA算法。

**主要函数：**

**main()**
```cpp
float4 main(uint2 pixel)
```
主入口点，实现以下步骤：

**1. 像素重定位（基于运动向量）**
```cpp
float2 motion = mvec[pixel];
float2 prevPos = pixel + motion;
```

**2. 历史样本获取**
```cpp
float3 historyColor = prevTexture.Sample(prevSampler, prevPos * texDimInv).rgb;
```

**3. 颜色盒滤波（抗闪烁）**
```cpp
float3 minColor, maxColor;
float3 meanColor;
computeColorBox(src, pixel, windowSize, minColor, maxColor, meanColor);
```

**4. 被排斥的历史样本检测**
```cpp
float historyWeight = 1.0;
if (antiFlicker && isOutsideColorBox(historyColor, minColor, maxColor, sigma)) {
    historyWeight = 0.0;
}
```

**5. 混合当前和历史**
```cpp
float3 finalColor = lerp(historyColor * historyWeight, 
                      currentColor, 
                      alpha);
```

**6. 响应式混合（基于深度不连续性）**
```cpp
float disocclusion = detectDisocclusion(depth, pixel, prevPos);
alpha = lerp(alpha, 1.0, disocclusion);
```

**7. 钳制和输出**
```cpp
finalColor = clamp(finalColor, minColor, maxColor);
return float4(finalColor, 1.0);
```

## 算法详解

### 1. 运动向量重定位

使用运动向量将当前像素映射到历史帧位置：
```
prevPos = currentPos + motionVector
```

**注意：** 运动向量通常表示从前一帧到当前帧的位移。

### 2. 颜色盒滤波

在局部邻域内计算颜色统计：
```
minColor = min(neighbors)
maxColor = max(neighbors)
meanColor = mean(neighbors)
```

用于检测异常历史样本。

### 3. 被排斥历史检测

使用颜色盒方差检测历史样本是否有效：
```
if |historyColor - meanColor| > colorBoxSigma * (maxColor - minColor):
    historyWeight = 0.0
```

这防止在颜色剧烈变化时使用无效历史。

### 4. 深度不连续性检测

基于深度变化检测遮挡：
```
if abs(currentDepth - historyDepth) > threshold:
    alpha = 1.0  // 完全使用当前帧
```

### 5. 自适应混合

基于历史有效性调整混合：
```
finalColor = lerp(historyColor, currentColor, adaptiveAlpha)
```

其中：
```
adaptiveAlpha = baseAlpha * historyWeight + 
              (1.0 - historyWeight) * disocclusion
```

## 使用示例

```python
# 创建TAA
taa = TAA.create(device, props)

# 配置
props = Properties()
props['alpha'] = 0.1
props['colorBoxSigma'] = 1.0
props['antiFlicker'] = True
taa.setProperties(props)
```

## 算法流程图

```
TAA执行流程：

开始
  │
  ├─ 读取输入
  │   ├─ 当前帧颜色
  │   ├─ 运动向量
  │   └─ 深度缓冲区
  │
  ├─ 像素重定位
  │   └─ prevPos = pixel + motionVector
  │
  ├─ 获取历史样本
  │   └─ historyColor = prevTexture.Sample(prevPos)
  │
  ├─ 颜色盒滤波
  │   ├─ 计算局部最小颜色
  │   ├─ 计算局部最大颜色
  │   └─ 计算局部平均颜色
  │
  ├─ 被排斥历史检测
  │   └─ 如果历史在颜色盒外：weight = 0
  │
  ├─ 深度不连续性检测
  │   └─ 如果深度变化大：使用当前帧
  │
  ├─ 自适应混合
  │   └─ final = lerp(history, current, adaptiveAlpha)
  │
  ├─ 钳制
  │   └─ clamp to color box
  │
  └─ 输出
      └─ 写入目标缓冲区
```

## 参数调优指南

**Alpha（混合因子）：**
- 较低值（0.05-0.1）：更强的时间积累，更好的降噪
- 较高值（0.15-0.25）：更好的运动清晰度，更多噪点
- 推荐值：0.1

**ColorBoxSigma（方差阈值）：**
- 较低值（0.5-1.0）：更严格的排斥，更多噪点
- 较高值（1.5-2.0）：更宽松的排斥，更多拖影
- 推荐值：1.0

**AntiFlicker（抗闪烁）：**
- 启用：减少高频闪烁，但可能有更多拖影
- 禁用：更好的运动清晰度，但可能有闪烁
- 推荐值：true

## 常见问题

**1. 拖影（Ghosting）**
- 原因：alpha太低或历史排斥太弱
- 解决：增加alpha或降低colorBoxSigma

**2. 闪烁（Flickering）**
- 原因：alpha太高或颜色盒检测太弱
- 解决：降低alpha或启用antiFlicker

**3. 模糊（Blurriness）**
- 原因：alpha太低或过多使用历史
- 解决：增加alpha

## 参考文档

- [TAA.h](../Source/RenderPasses/TAA/TAA.h) - C++实现
