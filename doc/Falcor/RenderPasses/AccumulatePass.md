---
modified: 2026-02-01T12:29:45+08:00
created: 2026-01-13T08:39:58+08:00
tags:
  - 2026/01/13
---
## 第一性原理拆解：AccumulatePass 的本质

**核心问题**：蒙特卡洛路径追踪每帧产生的图像含有大量随机噪声，如何利用时间维度的信息来降低噪声？

**数学本质**：
$$
\bar{L}_N = \frac{1}{N} \sum_{i=1}^{N} L_i
$$

其中 $$\bar{L}_N$$ 是累积 N 帧后的估计值，$$L_i$$ 是第 i 帧的采样结果。根据大数定律，随着 N 增大，方差以 $$O(1/N)$$ 的速率收敛。

---

## 技术架构分析

### 精度模式对比

| 模式 | 存储格式 | 精度 | 显存开销 | 适用场景 |
|------|---------|------|---------|---------|
| **Single** | `RGBA32Float` ×1 | 23位尾数 | 低 | 短期累积（<1000帧） |
| **Double** | `RGBA32Uint` ×2 | 模拟64位 | 中 | 超长累积，避免精度丢失 |
| **SingleCompensated** | `RGBA32Float` ×2 | Kahan求和 | 中 | 平衡精度与性能 |

**关键实现细节**（源码 L227-259）：
- `SingleCompensated` 模式启用了 `FloatingPointModePrecise` 编译标志，确保 Kahan 补偿求和算法不被编译器优化破坏。

### 自动重置逻辑（源码 L129-156）

```
场景变化检测 → 排除 Camera Jitter/History → 其余变化触发 reset()
```

**设计意图**：TAA（时域抗锯齿）依赖 Jitter，不应触发重置；而几何/材质/光照变化会使历史帧失效。

---

## 批判性评估

### 优势
1. **模块化设计**：通过 `RenderPassReflection` 实现 I/O 解耦，可灵活接入任意渲染图。
2. **精度可控**：三种模式覆盖从实时预览到离线渲染的需求。
3. **溢出策略完备**：`EMA`（指数移动平均）模式支持无限累积而不溢出。

### 劣势/局限
1. **无运动补偿**：累积假设静态场景，动态物体会产生鬼影（Ghosting）。
2. **显存占用线性增长**：`Double` 和 `SingleCompensated` 需要额外 buffer。
3. **重置粒度粗糙**：源码注释 `TODO: Add UI options to select which changes should trigger reset` 表明当前缺乏细粒度控制。

---

## 工程实践建议

| 场景 | 推荐配置 | 理由 |
|------|---------|------|
| 实时预览 | `Single` + `maxFrameCount=64` + `OverflowMode::EMA` | 快速收敛，防止卡顿 |
| 离线渲染 | `Double` + `maxFrameCount=0`（无限） | 最高精度，避免浮点累积误差 |
| 可微渲染 | `Single`（如 WARDiffPathTracer 示例） | 梯度计算对精度要求相对宽松 |

---

## 置信度评级

- **技术描述准确性**：**高**（基于源码直接分析）
- **性能数据**：**低**（未提供 benchmark，需实测）
- **适用性建议**：**中**（依赖具体硬件和场景复杂度）

## AccumulatePass 用例汇总

根据提供的源码引用，整理出以下用例：

| 用例来源 | Pass 名称 | 配置参数 | 用途说明 |
|---------|----------|---------|---------|
| `WARDiffPathTracer.py` (L15-16) | `AccumulatePassPrimal` | `enabled: True`, `precisionMode: Single` | 可微渲染中累积 **Primal（前向）** 路径追踪结果 |
| `WARDiffPathTracer.py` (L18-19) | `AccumulatePassDiff` | `enabled: True`, `precisionMode: Single` | 可微渲染中累积 **Differential（梯度）** 数据 |
| `PathTracerAdaptive.py` (L9-10) | `AccumulatePass` | `enabled: True`, `precisionMode: Single` | 自适应路径追踪测试中累积采样结果 |

---

### 配置参数速查

| 参数 | 类型 | 默认值 | 可选值 |
|------|------|-------|-------|
| `enabled` | bool | `True` | `True` / `False` |
| `precisionMode` | enum | - | `Single` / `Double` / `SingleCompensated` |
| `outputFormat` | ResourceFormat | `RGBA32Float` | 任意有效纹理格式 |
| `outputSize` | enum | - | 输出尺寸选择模式 |
| `fixedOutputSize` | uint2 | - | 固定输出分辨率 |
| `autoReset` | bool | `True` | 场景变化时自动重置 |
| `maxFrameCount` | uint | `0` | 最大累积帧数（0=无限） |
| `overflowMode` | enum | `Stop` | `Stop` / `Reset` / `EMA` |

---

### 典型创建模板

```python
# 基础用法
AccumulatePass = createPass("AccumulatePass", {
    'enabled': True,
    'precisionMode': 'Single'
})

# 完整配置
AccumulatePass = createPass("AccumulatePass", {
    'enabled': True,
    'precisionMode': 'Double',
    'autoReset': True,
    'maxFrameCount': 1024,
    'overflowMode': 'EMA'
})
```

## 第一性原理拆解：Shader 架构

### 核心问题
如何在 GPU Compute Shader 中高效实现多精度的时域累积，同时处理浮点精度损失问题？

### 数据流架构

```
┌─────────────┐     ┌──────────────────────┐     ┌──────────────┐
│ gCurFrame   │────▶│  Accumulation Kernel │────▶│ gOutputFrame │
│ (Input)     │     │  (3种模式)            │     │ (Output)     │
└─────────────┘     └──────────┬───────────┘     └──────────────┘
                               │
                    ┌──────────▼───────────┐
                    │  Intermediate Buffer  │
                    │  (模式相关)           │
                    └──────────────────────┘
```

---

## 三种累积模式深度分析

### 1. Single Precision（单精度标准求和）

**数学公式**：

| 模式 | 公式 |
|------|------|
| 标准累积 | $$\text{sum}_{n+1} = \text{sum}_n + x_n, \quad \bar{x} = \frac{\text{sum}_{n+1}}{n+1}$$ |
| EMA 模式 | $$\bar{x}_{n+1} = \text{lerp}(\bar{x}_n, x_n, \frac{1}{n+1})$$ |

**代码逻辑**：

```hlsl
if (gMovingAverageMode)
{
    // EMA: 存储的是均值，直接混合
    output = lerp(gLastFrameSum[pixelPos], curColor, curWeight);
    gLastFrameSum[pixelPos] = output;  // 存均值
}
else
{
    // 标准: 存储的是累加和
    float4 sum = gLastFrameSum[pixelPos] + curColor;
    output = sum * curWeight;
    gLastFrameSum[pixelPos] = sum;  // 存总和
}
```

**关键差异**：

| 属性 | 标准模式 | EMA 模式 |
|------|---------|---------|
| 存储内容 | 原始累加和 | 当前均值 |
| 数值范围 | 随帧数线性增长 | 始终在输入范围内 |
| 精度风险 | 累加和溢出 | 无溢出风险 |
| 适用场景 | 有限帧累积 | 无限帧累积 |

---

### 2. Single Compensated（Kahan 补偿求和）

**数学原理**：Kahan Summation Algorithm

标准浮点加法存在精度损失：
$$
\text{fl}(a + b) = (a + b)(1 + \delta), \quad |\delta| \leq \epsilon_{\text{machine}}
$$

Kahan 算法通过追踪误差项 $$c$$ 进行补偿：

```
y = x - c          // 用误差修正当前值
sumNext = sum + y  // 执行加法
c = (sumNext - sum) - y  // 计算新的误差项
```

**代码逐行解析**：

```hlsl
float4 sum = gLastFrameSum[pixelPos];   // 当前累加和
float4 c = gLastFrameCorr[pixelPos];    // 累积误差补偿项

float4 y = curColor - c;                // 关键：用误差修正输入值
float4 sumNext = sum + y;               // 执行加法

// 计算本次加法产生的舍入误差
// (sumNext - sum) 理论上应该等于 y，但由于精度损失会有偏差
gLastFrameCorr[pixelPos] = (sumNext - sum) - y;
```

**为什么必须启用 `FloatingPointModePrecise`**：

| 无精确模式 | 有精确模式 |
|-----------|-----------|
| 编译器可能优化 `(sumNext - sum) - y` 为 `0` | 保持运算顺序，正确计算误差 |
| Kahan 算法失效 | 算法正常工作 |

---

### 3. Double Precision（双精度求和）

**核心挑战**：GPU 纹理格式不支持 `double`，需要用两个 `uint4` 模拟。

**位级存储方案**：

```
double (64-bit) = [Hi 32-bit][Lo 32-bit]
                      │           │
                      ▼           ▼
              gLastFrameSumHi  gLastFrameSumLo
```

**代码关键操作**：

```hlsl
// 从两个 uint 重建 double
double sum = asdouble(sumLo[i], sumHi[i]);

// 执行双精度运算
sum += (double)curColor[i];

// 拆分回两个 uint 存储
asuint(sum, sumLo[i], sumHi[i]);

// 输出时转回 float
output[i] = (float)(sum * curWeight);
```

---

## 输入格式处理

通过预处理宏实现类型泛化：

```hlsl
#if _INPUT_FORMAT == INPUT_FORMAT_FLOAT
typedef float4 InputDataType;
#elif _INPUT_FORMAT == INPUT_FORMAT_UINT
typedef uint4 InputDataType;
#elif _INPUT_FORMAT == INPUT_FORMAT_SINT
typedef int4 InputDataType;
#endif
```

**设计意图**：同一份 shader 代码支持不同输入格式，避免代码重复。

---

## 线程调度分析

```hlsl
[numthreads(16, 16, 1)]
```

| 参数 | 值 | 说明 |
|------|-----|------|
| Thread Group Size | 16×16 = 256 | 标准 tile 大小，利于缓存 |
| Dispatch 计算 | `ceil(width/16) × ceil(height/16)` | 覆盖全分辨率 |
| 边界检查 | `if (any(dispatchThreadId.xy >= gResolution)) return;` | 处理非 16 整除的分辨率 |

---

## 批判性评估

### 优势
1. **精度覆盖完整**：从 23-bit（single）到 52-bit（double）尾数精度可选
2. **无分支冗余**：`gAccumulate` 和 `gMovingAverageMode` 在运行时确定，编译器可优化
3. **显存访问合并**：16×16 tile 保证相邻线程访问相邻像素

### 劣势/风险
1. **Double 模式性能损失**：
   - 每像素需要 4 次 `asdouble` + 4 次 `asuint` 位操作
   - 显存带宽翻倍（2×uint4 vs 1×float4）
   
2. **Compensated 模式依赖编译器**：
   - 若 `FloatingPointModePrecise` 未正确传递，算法静默失效
   - **置信度：高风险点**

3. **缺少 HDR 溢出保护**：
   - 标准 Single 模式在 HDR 场景下累加和可能超出 `float` 范围

---

## 性能与精度权衡总结

| 模式 | 精度 (有效位) | 显存带宽 | ALU 开销 | 推荐帧数上限 |
|------|-------------|---------|---------|-------------|
| Single | ~23 bit | 1× | 低 | ~1,000 |
| SingleCompensated | ~46 bit | 2× | 中 | ~100,000 |
| Double | ~52 bit | 2× | 高 | 无限制 |

**置信度评级**：
- 算法正确性：**高**（标准数值算法）
- 性能数据：**低**（需 GPU Profiler 实测）
- 边界条件处理：**中**（HDR 溢出未处理）