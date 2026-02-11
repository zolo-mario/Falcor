---
modified: 2026-02-03T10:30:58+08:00
created: 2026-02-02T23:46:14+08:00
tags:
  - 2026/02/02
---
# VBufferRT RenderPass Shader 绑定分析

## 1. Pass 基本信息

### 描述
VBufferRT 是一个光线追踪可见性缓冲区（V-Buffer）生成 Pass。支持两种执行模式：传统光线追踪（DXR）和计算着色器（使用 TraceRayInline）。输出可见性缓冲区和多个可选通道（深度、运动向量、视图方向、时间、掩码）。

### 入口点函数

**Ray Tracing 模式**：
- **rayGen** - `[raygeneration]` Shader：光线生成着色器
- **miss** - `[miss]` Shader：未命中着色器
- **anyHit** - `[anyhit]` Shader：任意命中着色器（alpha test）
- **closestHit** - `[closesthit]` Shader：最近命中着色器（三角形）
- **displacedTriangleMeshClosestHit** - `[closesthit]` Shader：置换网格最近命中
- **displacedTriangleMeshIntersection** - `[intersection]` Shader：置换网格相交
- **curveClosestHit** - `[closesthit]` Shader：曲线最近命中
- **curveIntersection** - `[intersection]` Shader：曲线相交
- **sdfGridClosestHit** - `[closesthit]` Shader：SDF 网格最近命中
- **sdfGridIntersection** - `[intersection]` Shader：SDF 网格相交

**Compute 模式**：
- **main** - `[main]` Compute Shader：[numthreads(16, 16, 1)] 计算着色器，使用 TraceRayInline

## 2. 资源绑定清单

### 2.1 Constant Buffer 绑定

| C++ 绑定代码 | Shader 声明 | 资源类型 | 绑定方式 | 备注 |
|-------------|-------------|---------|---------|------|
| `var["gVBufferRT"]["frameDim"]` | `uint2 frameDim;` | CBV | 手动 | 帧分辨率（宽高） |
| `var["gVBufferRT"]["frameCount"]` | `uint frameCount;` | CBV | 手动 | 帧计数（用于采样） |

### 2.2 Shader Resource View (SRV) 绑定

| C++ 绑定代码 | Shader 声明 | 资源类型 | 绑定方式 | 备注 |
|-------------|-------------|---------|---------|------|
| (Scene auto) | (Scene resources) | CBV/SRV/Accel | 自动 | 场景几何体、材质和加速结构 |

### 2.3 Unordered Access View (UAV) 绑定

| C++ 绑定代码 | Shader 声明 | 资源类型 | 绑定方式 | 备注 |
|-------------|-------------|---------|---------|------|
| `var["gVBuffer"]` | `RWTexture2D<PackedHitInfo> gVBuffer;` | UAV | 手动 | 可见性缓冲区（输出） |
| `var["gDepth"]` (可选) | `RWTexture2D<float> gDepth;` | UAV | 手动 | 深度缓冲区 NDC（可选） |
| `var["gMotionVector"]` (可选) | `RWTexture2D<float2> gMotionVector;` | UAV | 手动 | 屏幕空间运动向量（可选） |
| `var["gViewW"]` (可选) | `RWTexture2D<float4> gViewW;` | UAV | 手动 | 世界空间视图方向（可选） |
| `var["gTime"]` (可选) | `RWTexture2D<uint> gTime;` | UAV | 手动 | 每像素执行时间（可选） |
| `var["gMask"]` (可选) | `RWTexture2D<float> gMask;` | UAV | 手动 | 掩码（可选） |

## 3. Constant Buffer 结构映射

### gVBufferRT 结构

```cpp
// C++ 端：通过 var["gVBufferRT"][key] 逐个设置
// Shader 端定义：
struct VBufferRT
{
    uint2 frameDim;     // offset: 0, size: 8 bytes
    uint frameCount;     // offset: 8, size: 4 bytes
}
// 总大小: 12 bytes（可能填充到 16 字节）
```

### 对齐分析

| 成员         | 偏移  | C++ 类型 | HLSL 类型 | 对齐      |
| ---------- | --- | ------ | ------- | ------- |
| frameDim   | 0   | uint2  | uint2   | 8 bytes |
| frameCount | 8   | uint   | uint    | 4 bytes |

## 4. 纹理/缓冲区生命周期

### 4.1 RenderGraph 管理的资源

所有 6 个输出通道（vbuffer + 5 个可选通道）均由 RenderGraph 管理。

### 4.2 Pass 内部管理的资源

| 资源 | 类型 | 创建时机 | 销毁时机 | 说明 |
|------|------|---------|---------|------|
| `mRaytrace.pProgram` | ref<Program> | executeRaytrace() | 随 `VBufferRT` 析构 | 光线追踪程序 |
| `mRaytrace.pVars` | ref<RtProgramVars> | executeRaytrace() | 随 `VBufferRT` 析构 | 光线追踪程序变量 |
| `mRaytrace.pBindingTable` | ref<RtBindingTable> | executeRaytrace() | 随 `VBufferRT` 析构 | 绑定表 |
| `mpComputePass` | ref<ComputePass> | executeCompute() | 随 `VBufferRT` 析构 | 计算着色器 pass |
| `mpSampleGenerator` | ref<SampleGenerator> | 构造函数 | 随 `VBufferRT` 析构 | 采样生成器 |

## 5. 执行流程

### 5.1 execute() 函数资源绑定时序

```mermaid
sequenceDiagram
    participant RC as RenderContext
    participant VBRT as VBufferRT
    participant RT as RayTracing Mode
    participant CP as Compute Mode
    participant RG as RenderGraph

    RG->>VBRT: renderData[vbuffer + optional channels]

    alt 无场景
        VBRT->>RC: clearUAV(gVBuffer, uint4(0))
        VBRT->>RC: clearRenderPassChannels(extra)
    else 场景存在
        VBRT->>VBRT: 检查场景更新

        VBRT->>VBRT: 配置 DOF
        alt mUseDOF && 相机光圈 > 0
            VBRT->>RG: kRenderPassPRNGDimension = 2
        else
            VBRT->>RG: kRenderPassPRNGDimension = 0
        end

        alt mUseTraceRayInline
            VBRT->>CP: create ComputePass (如果不存在)
            CP->>CP: bindSceneForRaytracing()
            CP->>CP: bindSampleGenerator()
            CP->>CP: bindShaderData(all UAVs)
            CP->>RC: execute(threadCount = frameDim)
        else
            VBRT->>RT: create Program & RtProgramVars (如果不存在)
            RT->>RT: create BindingTable
            RT->>RT: setRayGen("rayGen")
            RT->>RT: setMiss(0, "miss")
            RT->>RT: setHitGroups(TriangleMesh, DisplacedMesh, Curve, SDFGrid)
            RT->>RT: bindSampleGenerator()
            RT->>RT: bindShaderData(all UAVs)
            RT->>RC: raytrace(dispatchDim = frameDim)
        end
    end
```

### 5.2 Ray Tracing 执行流程

**RayGen Shader**：
1. 生成光线（pinhole 或 thinlens with DOF）
2. 追踪光线
3. 如果命中：写入 hit info
4. 如果未命中：清零所有通道

**Hit Groups**：
- **TriangleMesh**: `anyHit` (alpha test) → `closestHit` (写入 hit data)
- **DisplacedTriangleMesh**: `intersection` (相交测试) → `closestHit` (写入 hit data)
- **Curve**: `intersection` (相交测试) → `closestHit` (写入 hit data)
- **SDFGrid**: `intersection` (相交测试) → `closestHit` (写入 hit data)

**Miss Shader**：
- 清零 VBuffer
- 清零所有可选通道（或设置默认值）

### 5.3 Compute 模式执行流程

**Main Shader**（每像素一个线程）：
1. 检查边界
2. 生成光线（pinhole 或 thinlens with DOF）
3. 使用 `SceneRayQuery.traceRay()` 追踪光线
4. 如果命中：写入 hit info
5. 如果未命中：清零所有通道
6. 写入时间（如果启用）

### 5.4 Shader Defines

```cpp
defines.add("COMPUTE_DEPTH_OF_FIELD", mComputeDOF ? "1" : "0");
defines.add("USE_ALPHA_TEST", mUseAlphaTest ? "1" : "0");
defines.add("RAY_FLAGS", std::to_string((uint32_t)rayFlags));
defines.add(getValidResourceDefines(kVBufferExtraChannels, renderData));
```

### 5.5 绑定频率总结

| 资源类型 | 绑定频率 | 示例 |
|---------|---------|------|
| CBV | 每次 execute | frameDim, frameCount |
| UAV (所有通道) | 每次 execute | gVBuffer, gDepth, gMotionVector, etc. |
| Shader Defines | 程序重建/每帧 | COMPUTE_DEPTH_OF_FIELD, RAY_FLAGS, is_valid_* |

## 6. 特殊机制说明

### 6.1 双执行模式与可选通道

RT 模式使用 Scene + Binding Table；Compute 模式使用 `bindShaderDataForRaytracing`。5 个额外通道（depth, mvec, viewW, time, mask）通过 `is_valid_*` defines 控制。DOF 启用时 `COMPUTE_DEPTH_OF_FIELD` 和 PRNG 维度影响 SampleGenerator 绑定。

### 6.2 输出通道与 Binding

| Channel | 格式 | 可选 |
|---------|------|------|
| vbuffer | HitInfo | 否 |
| depth / mvec / viewW / time / mask | 见绑定表 | 是 |

光线剔除通过 `RayFlags` 传入。Payload 4 bytes，递归深度 1。
