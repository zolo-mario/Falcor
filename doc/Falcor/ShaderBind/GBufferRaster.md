# GBufferRaster RenderPass Shader 绑定分析

## 1. Pass 基本信息

### 描述
GBufferRaster 是一个光栅化 G-Buffer 生成 RenderPass，通过光栅化将场景几何体渲染到多个纹理中，包括位置、法线、切线、纹理坐标、运动向量、材质数据等。支持 alpha test、着色法线调整和多种剔除模式。使用 Pixel Shader Barycentrics 和 Rasterizer Ordered Views (ROVs) 进行精确的像素着色。

### 入口点函数
- **DepthPass** - `[vsMain]` Vertex Shader + `[psMain]` Pixel Shader (DepthPass.3d.slang)：预深度 pass，执行 alpha test 并填充深度缓冲区
- **GBufferPass** - `[vsMain]` Vertex Shader + `[psMain]` Pixel Shader (GBufferRaster.3d.slang)：主 G-Buffer pass，渲染所有通道数据

## 2. 资源绑定清单

### 2.1 DepthPass 资源绑定

| C++ 绑定代码 | Shader 声明 | 资源类型 | 绑定方式 | 备注 |
|-------------|-------------|---------|---------|------|
| (Scene auto) | (Scene resources) | CBV/SRV | 自动 | 场景几何体和材质资源 |
| (FBO auto) | (Render targets) | RTV | 自动 | 不需要渲染目标（仅深度） |
| (FBO auto) | (Depth target) | DSV | 自动 | 深度缓冲区绑定到 FBO |

### 2.2 GBufferPass 资源绑定

| C++ 绑定代码 | Shader 声明 | 资源类型 | 绑定方式 | 备注 |
|-------------|-------------|---------|---------|------|
| (Scene auto) | (Scene resources) | CBV/SRV | 自动 | 场景几何体和材质资源 |
| `var["PerFrameCB"]["gFrameDim"]` | `uint2 gFrameDim;` | CBV | 手动 | 帧分辨率（宽高） |
| `var["gPosW"]` (RT) | `float4 posW : SV_TARGET0;` | RTV | 自动 (FBO) | 世界空间位置（可选） |
| `var["gNormW"]` (RT) | `float4 normW : SV_TARGET1;` | RTV | 自动 (FBO) | 世界空间着色法线（可选） |
| `var["gTangentW"]` (RT) | `float4 tangentW : SV_TARGET2;` | RTV | 自动 (FBO) | 世界空间切线（可选） |
| `var["gFaceNormalW"]` (RT) | `float4 faceNormalW : SV_TARGET3;` | RTV | 自动 (FBO) | 世界空间面法线（可选） |
| `var["gTexC"]` (RT) | `float2 texC : SV_TARGET4;` | RTV | 自动 (FBO) | 纹理坐标（可选） |
| `var["gTexGrads"]` (RT) | `float4 texGrads : SV_TARGET5;` | RTV | 自动 (FBO) | 纹理梯度（可选） |
| `var["gMotionVector"]` (RT) | `float2 mvec : SV_TARGET6;` | RTV | 自动 (FBO) | 运动向量（可选） |
| `var["gMaterialData"]` (RT) | `uint4 mtlData : SV_TARGET7;` | RTV | 自动 (FBO) | 材质数据（可选） |
| `var["gVBuffer"]` (UAV) | `RasterizerOrderedTexture2D<PackedHitInfo> gVBuffer;` | UAV | 手动 | 可见性缓冲区（可选，ROV） |
| `var["gGuideNormalW"]` (UAV) | `RasterizerOrderedTexture2D<float4> gGuideNormalW;` | UAV | 手动 | 引导法线（可选，ROV） |
| `var["gDiffOpacity"]` (UAV) | `RasterizerOrderedTexture2D<float4> gDiffOpacity;` | UAV | 手动 | 漫反射不透明度（可选，ROV） |
| `var["gSpecRough"]` (UAV) | `RasterizerOrderedTexture2D<float4> gSpecRough;` | UAV | 手动 | 镜面反射和粗糙度（可选，ROV） |
| `var["gEmissive"]` (UAV) | `RasterizerOrderedTexture2D<float4> gEmissive;` | UAV | 手动 | 自发光颜色（可选，ROV） |
| `var["gViewW"]` (UAV) | `RasterizerOrderedTexture2D<float4> gViewW;` | UAV | 手动 | 世界空间视图方向（可选，ROV） |
| `var["gPosNormalFwidth"]` (UAV) | `RasterizerOrderedTexture2D<float2> gPosNormalFwidth;` | UAV | 手动 | 位置和法线过滤宽度（可选，ROV） |
| `var["gLinearZAndDeriv"]` (UAV) | `RasterizerOrderedTexture2D<float2> gLinearZAndDeriv;` | UAV | 手动 | 线性 Z 和导数（可选，ROV） |
| `var["gMask"]` (UAV) | `RasterizerOrderedTexture2D<float> gMask;` | UAV | 手动 | 掩码（可选，ROV） |

## 3. Constant Buffer 结构映射

### PerFrameCB 结构

```cpp
// C++ 端：通过 var["PerFrameCB"]["gFrameDim"] = mFrameDim 设置
// Shader 端定义：
cbuffer PerFrameCB
{
    uint2 gFrameDim;  // offset: 0, size: 8 bytes
}
// 总大小: 8 bytes（可能填充到 16 字节）
```

### 对齐分析

| 成员 | 偏移 | C++ 类型 | HLSL 类型 | 对齐 |
|------|------|----------|-----------|------|
| gFrameDim | 0 | uint2 | uint2 | 8 bytes |

### GBuffer 通道列表（渲染目标）

| 索引 | Channel Name | Shader Variable | 格式 | 可选 | 描述 |
|------|-------------|-----------------|------|------|------|
| 0 | posW | gPosW | RGBA32Float | 是 | 世界空间位置 |
| 1 | normW | gNormW | RGBA32Float | 是 | 世界空间着色法线 |
| 2 | tangentW | gTangentW | RGBA32Float | 是 | 世界空间切线（含符号） |
| 3 | faceNormalW | gFaceNormalW | RGBA32Float | 是 | 世界空间面法线 |
| 4 | texC | gTexC | RG32Float | 是 | 纹理坐标 |
| 5 | texGrads | gTexGrads | RGBA16Float | 是 | 纹理梯度 (ddx, ddy) |
| 6 | mvec | gMotionVector | RG32Float | 是 | 运动向量 |
| 7 | mtlData | gMaterialData | RGBA32Uint | 是 | 材质数据（ID, header.x, header.y, lobes） |

### GBufferRaster 额外通道（UAV）

| Channel Name | Shader Variable | 格式 | 可选 | 描述 |
|-------------|-----------------|------|------|------|
| vbuffer | gVBuffer | HitInfo | 是 | 可见性缓冲区（实例ID、图元索引、重心坐标） |
| guideNormalW | gGuideNormalW | RGBA32Float | 是 | 引导法线 |
| diffuseOpacity | gDiffOpacity | RGBA32Float | 是 | 漫反射反射率和不透明度 |
| specRough | gSpecRough | RGBA32Float | 是 | 镜面反射率和粗糙度 |
| emissive | gEmissive | RGBA32Float | 是 | 自发光颜色 |
| viewW | gViewW | RGBA32Float | 是 | 世界空间视图方向 |
| pnFwidth | gPosNormalFwidth | RG32Float | 是 | 位置和法线过滤宽度 |
| linearZ | gLinearZAndDeriv | RG32Float | 是 | 线性 Z 和导数 |
| mask | gMask | R32Float | 是 | 掩码 |

## 4. 纹理/缓冲区生命周期

### 4.1 RenderGraph 管理的资源

| 资源 | 用途 | 生命周期 |
|------|------|---------|
| `depth` | 深度缓冲区 | 由 RenderGraph 创建（reflect() 时声明） |
| `posW` / `normW` / `tangentW` / `faceNormalW` / `texC` / `texGrads` / `mvec` / `mtlData` | G-Buffer 渲染目标 | 由 RenderGraph 创建（reflect() 时声明） |
| `vbuffer` / `guideNormalW` / `diffuseOpacity` / `specRough` / `emissive` / `viewW` / `pnFwidth` / `linearZ` / `mask` | G-Buffer UAV 输出 | 由 RenderGraph 创建（reflect() 时声明） |

### 4.2 Pass 内部管理的资源

| 资源 | 类型 | 创建时机 | 销毁时机 | 说明 |
|------|------|---------|---------|------|
| `mpFbo` | ref<Fbo> | 构造函数 | 随 `GBufferRaster` 析构 | 用于绑定渲染目标 |
| `mDepthPass.pState` | ref<GraphicsState> | 构造函数 | 随 `GBufferRaster` 析构 | 深度 pass 图形状态 |
| `mDepthPass.pProgram` | ref<Program> | execute() -> recreatePrograms() | 随 `GBufferRaster` 析构 | 深度 pass 程序 |
| `mDepthPass.pVars` | ref<ProgramVars> | execute() | 随 `GBufferRaster` 析构 | 深度 pass 程序变量 |
| `mGBufferPass.pState` | ref<GraphicsState> | 构造函数 | 随 `GBufferRaster` 析构 | GBuffer pass 图形状态 |
| `mGBufferPass.pProgram` | ref<Program> | execute() -> recreatePrograms() | 随 `GBufferRaster` 析构 | GBuffer pass 程序 |
| `mGBufferPass.pVars` | ref<ProgramVars> | execute() | 随 `GBufferRaster` 析构 | GBuffer pass 程序变量 |

### 4.3 资源创建逻辑

**FBO 创建**：
- 在构造函数中创建
- execute() 中动态绑定渲染目标和深度缓冲区

**程序创建/重建**：
- 深度 pass：使用 `DepthPass.3d.slang`，vsEntry="vsMain"，psEntry="psMain"
- GBuffer pass：使用 `GBufferRaster.3d.slang`，vsEntry="vsMain"，psEntry="psMain"
- 重建触发条件：场景更新标志 `IScene::UpdateFlags::RecompileNeeded`

**Shader Defines**：
- `USE_ALPHA_TEST`: 启用 alpha test
- `ADJUST_SHADING_NORMALS`: 调整着色法线
- `is_valid_<name>`: 标识哪些可选通道有效（动态生成）

## 5. 执行流程

### 5.1 execute() 函数资源绑定时序

```mermaid
sequenceDiagram
    participant RC as RenderContext
    participant GBR as GBufferRaster
    participant DP as DepthPass
    participant GP as GBufferPass
    participant FBO as Fbo
    participant RG as RenderGraph

    RG->>GBR: renderData (depth + all channels)
    GBR->>GBR: updateFrameDim()

    GBR->>RC: clearDsv(depth, 1.f, 0)

    GBR->>FBO: attachColorTarget(posW, 0)
    GBR->>FBO: attachColorTarget(normW, 1)
    ...
    GBR->>FBO: attachColorTarget(mtlData, 7)

    GBR->>RC: clearFbo(FBO, float4(0), 1.f, 0, Color)
    GBR->>RC: clearUAV(extra_channels, 0/float4(0))

    alt Scene exists
        GBR->>GBR: check scene recompile needed

        GBR->>DP: setProgram(defines: USE_ALPHA_TEST)
        GBR->>DP: setProgramVars()
        GBR->>DP: attachDepthStencilTarget(depth)
        DP->>RC: rasterize() - Depth pass
        DP->>RG: fill depth buffer

        GBR->>GP: setProgram(defines: ADJUST_SHADING_NORMALS, USE_ALPHA_TEST, is_valid_*)
        GBR->>GP: setProgramVars()
        GBR->>GP: bind UAVs (gVBuffer, gGuideNormalW, etc.)
        GBR->>GP: var["PerFrameCB"]["gFrameDim"] = frameDim
        GBR->>GP: setFbo(Fbo)
        GP->>RC: rasterize() - GBuffer pass
        GP->>RG: write render targets and UAVs
    end
```

### 5.2 执行流程说明

**1. 清除阶段**：
- 清除深度缓冲区（值 1.0）
- 清除所有渲染目标（值 0）
- 清除所有 UAV（整型格式清为 0，浮点格式清为 0）

**2. 深度 Pass**：
- 创建/获取深度 pass 程序
- 设置 `USE_ALPHA_TEST` define
- 绑定深度缓冲区到 FBO
- 光栅化场景（仅深度和 alpha test）
- 使用 `DepthStencilState`：`DepthFunc::Equal`，`DepthWriteMask::False`（实际上在深度 pass 中应该是 `Less` 或 `LessEqual`，但代码显示 GBuffer pass 使用 Equal）

**3. GBuffer Pass**：
- 创建/获取 GBuffer pass 程序
- 设置 `ADJUST_SHADING_NORMALS` 和 `USE_ALPHA_TEST` defines
- 为每个可选通道生成 `is_valid_<name>` defines
- 绑定所有 UAV（可选通道）
- 设置 `PerFrameCB["gFrameDim"]`
- 光栅化场景，写入所有渲染目标和 UAV

### 5.3 Shader Defines 动态生成

**getValidResourceDefines() 函数**：
```cpp
for (const auto& desc : channels)
{
    if (desc.optional && !desc.texname.empty())
    {
        defines.add("is_valid_" + desc.texname, renderData[desc.name] != nullptr ? "1" : "0");
    }
}
```

**Shader 端使用**：
```hlsl
#define is_valid(name) (is_valid_##name != 0)

if (is_valid(gPosW))
    psOut.posW = gbuf.posW;
```

### 5.4 内存屏障/同步点

- **显式同步点**：
  - 深度 pass 和 GBuffer pass 之间：隐式同步（depth buffer 已填充）
  - ROV 写入：Rasterizer Ordered Views 保证像素着色器按光栅化顺序执行
- **数据流**：
  ```
  Scene → DepthPass → depth buffer
                    ↓
  Scene → GBufferPass → render targets (8 RT) + UAVs (9 optional)
  ```

### 5.5 Rasterizer Ordered Views (ROVs)

**作用**：
- 保证像素着色器写入 UAV 的顺序与光栅化顺序一致
- 允许在像素着色器中进行原子的像素级操作
- 用于 GBuffer 中需要精确像素对应关系的通道

**声明方式**：
```hlsl
RasterizerOrderedTexture2D<PackedHitInfo> gVBuffer;
RasterizerOrderedTexture2D<float4> gGuideNormalW;
```

### 5.6 绑定频率总结

| 资源类型 | 绑定频率 | 示例 |
|---------|---------|------|
| FBO (渲染目标) | 每次 execute | mpFbo attach 8 RTs + depth |
| FBO (深度) | 每次 pass | depth pass 和 gbuffer pass |
| CBV | 每次 execute | gFrameDim |
| UAV (可选通道) | 每次 execute | gVBuffer, gGuideNormalW, etc. |
| Shader Defines | 程序重建时 | USE_ALPHA_TEST, ADJUST_SHADING_NORMALS, is_valid_* |

## 6. 特殊机制说明

### 6.1 双 Pass 与 FBO

深度 Pass 仅绑定 DSV，GBuffer Pass 绑定 8 RTV + 9 UAV（可选）。两 pass 均使用 Scene 自动绑定，共享 `DepthFunc::Equal`。

### 6.2 可选通道机制

**设计目标**：
- 允许下游 pass 选择需要的 G-Buffer 通道
- 减少内存占用和带宽消耗

**实现方式**：
1. 在 `reflect()` 中标记为 `Optional`
2. 在 shader 中使用 `is_valid_<name>` defines
3. 仅当资源可用时才写入对应通道

**示例**：
```hlsl
if (is_valid(gPosW))
    psOut.posW = gbuf.posW;
if (is_valid(gVBuffer))
    gVBuffer[ipos] = triangleHit.pack();
```

gVBuffer 使用 `SV_PrimitiveID` 和 `SV_Barycentrics` 打包 TriangleHit。

### 6.3 Alpha Test Define

**深度 Pass**：
```hlsl
#if USE_ALPHA_TEST
    let lod = ImplicitLodTextureSampler();
    if (alphaTest(vsOut, triangleIndex, lod))
        discard;
#endif
```

**GBuffer Pass**：
```hlsl
#if USE_ALPHA_TEST
    if (gScene.materials.alphaTest(v, vsOut.materialID, lod))
        discard;
#endif
```

### 6.4 着色法线 Define

**Purpose**：
- 调整着色法线以改善光照效果
- 防止在低角度光照下的伪影

**Shader Define**：
```cpp
mGBufferPass.pProgram->addDefine("ADJUST_SHADING_NORMALS", mAdjustShadingNormals ? "1" : "0");
```

**使用方式**：
```hlsl
uint hints = 0;
#if ADJUST_SHADING_NORMALS
    hints |= (uint)MaterialInstanceHints::AdjustShadingNormal;
#endif
**Defines**：`USE_ALPHA_TEST`（深度+GBuffer 双 pass）、`ADJUST_SHADING_NORMALS`（`addDefine(..., mAdjustShadingNormals ? "1" : "0")`）。剔除模式在 `mpScene->rasterize()` 时传入。`updateFrameDim()` 从 depth 纹理取分辨率。
