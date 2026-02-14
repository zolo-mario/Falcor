# VBufferMeshletRaster RenderPass Shader 绑定分析

## 1. Pass 基本信息

### 描述
VBufferMeshletRaster 使用 **Mesh Shader**（SM 6.5）光栅化生成 V-Buffer。通过 meshlet 几何体进行 mesh + pixel shader 渲染，输出 PackedHitInfo（VBuffer 格式）。**支持多实例**：SceneMeshletData 为每个几何实例展开 meshlet，每个 GpuMeshlet 携带 instanceID 用于获取世界矩阵与顶点偏移。无剔除。需要 Shader Model 6.5 和 Pixel Shader Barycentrics 支持。

### 入口点函数

| 入口点 | 类型 | 说明 |
|--------|------|------|
| **meshMain** | Mesh Shader | `[numthreads(128,1,1)]`，输出顶点/三角形/图元数据 |
| **psMain** | Pixel Shader | 输出 PackedHitInfo 到 SV_TARGET0 |

---

## 2. 资源绑定清单

### 2.1 Constant Buffer (CBV)

| C++ 绑定代码 | Shader 声明 | 资源类型 | 绑定方式 | 备注 |
|-------------|-------------|---------|---------|------|
| `var["CB"]["gFrameDim"]` | `uint2 gFrameDim;` | CBV | 手动 | 帧分辨率（当前 shader 未使用） |
| `var["CB"]["gMeshletCount"]` | `uint gMeshletCount;` | CBV | 手动 | Meshlet 总数 |

### 2.2 Shader Resource View (SRV)

| C++ 绑定代码 | Shader 声明 | 资源类型 | 绑定方式 | 备注 |
|-------------|-------------|---------|---------|------|
| `var["gMeshlets"]` | `StructuredBuffer<GpuMeshlet> gMeshlets;` | SRV | 手动 | Meshlet 元数据（来自 SceneMeshletData） |
| `var["gMeshletVertices"]` | `StructuredBuffer<uint> gMeshletVertices;` | SRV | 手动 | Meshlet 顶点索引（全局顶点 ID） |
| `var["gMeshletTriangles"]` | `StructuredBuffer<uint> gMeshletTriangles;` | SRV | 手动 | Meshlet 三角形索引（3 个 uint 每三角形，局部 0–63） |

### 2.3 场景自动绑定

| C++ 绑定代码 | Shader 声明 | 资源类型 | 绑定方式 | 备注 |
|-------------|-------------|---------|---------|------|
| `mpScene->bindShaderDataForRaytracing(pRenderContext, var["gScene"])` | `ParameterBlock<Scene> gScene` | CBV/SRV/Accel | 自动 | 场景几何、材质、相机、顶点数据 |

### 2.4 Render Target / Depth (FBO)

| C++ 绑定代码 | Shader 声明 | 资源类型 | 绑定方式 | 备注 |
|-------------|-------------|---------|---------|------|
| `mpFbo->attachColorTarget(pVBuffer, 0)` | `PackedHitInfo : SV_TARGET0` | RTV | FBO attach | VBuffer 主输出 |
| `mpFbo->attachDepthStencilTarget(pDepth)` | `SV_Depth`（隐式） | DSV | FBO attach | 深度缓冲 D32Float |

### 2.5 可选 Extra Channels（⚠️ 当前未实现）

| reflect 声明 | Shader 端 | 状态 | 备注 |
|-------------|-----------|------|------|
| `viewW` → `gViewW` | 无 | ❌ 未绑定 | 未添加 `is_valid_gViewW`，未绑定纹理 |
| `depth` → `gDepth` | 无 | ❌ 未绑定 | 未添加 `is_valid_gDepth`，未绑定纹理 |
| `mvec` → `gMotionVector` | 无 | ❌ 未绑定 | 未添加 `is_valid_gMotionVector`，未绑定纹理 |

**说明**：`reflect()` 中通过 `addRenderPassOutputs(kVBufferExtraChannels)` 声明了这些可选输出，`execute()` 中会 `clearRenderPassChannels` 清空，但 **未** 调用 `getValidResourceDefines()` 生成宏，**未** 在循环中绑定到 `var[channel.texname]`，且 **shader 中无对应声明与写入**。与 VBufferRaster / VBufferRT 的实现不一致。

---

## 3. Constant Buffer 结构映射

### CB 结构（MeshletRaster.ms.slang）

```hlsl
cbuffer CB
{
    uint2 gFrameDim;      // offset: 0,  size: 8 bytes
    uint gMeshletCount;   // offset: 8,  size: 4 bytes
    // padding to 16 bytes
}
```

### 对齐分析

| 成员 | 偏移 | C++ 类型 | HLSL 类型 | 对齐 |
|------|------|----------|-----------|------|
| gFrameDim | 0 | uint2 | uint2 | 8 bytes |
| gMeshletCount | 8 | uint | uint | 4 bytes |
| _pad | 12 | - | (implicit) | 4 bytes → 16 |

---

## 4. GpuMeshlet 结构映射

Shader 端 `GpuMeshlet` 必须与 C++ `GpuMeshletDesc` 布局一致：

| 成员 | Shader 类型 | C++ 类型 | 偏移 | 说明 |
|------|-------------|----------|------|------|
| vertexOffset | uint | uint32_t | 0 | 顶点缓冲区偏移 |
| triangleOffset | uint | uint32_t | 4 | 三角形缓冲区偏移 |
| vertexCount | uint | uint32_t | 8 | 顶点数 |
| triangleCount | uint | uint32_t | 12 | 三角形数 |
| boundCenter | float3 | float3 | 16 | 包围球中心（需 16B 对齐） |
| boundRadius | float | float | 28 | 包围球半径 |
| instanceID | uint | uint32_t | 32 | 几何实例 ID |
| primitiveOffset | uint | uint32_t | 36 | 图元偏移 |
| meshID | uint | uint32_t | 40 | 网格 ID |
| _pad0 | uint | uint32_t | 44 | 填充 |

**注意**：`float3 boundCenter` 在 HLSL 中占 12 字节，后接 `float boundRadius` 共 16 字节，满足 16 字节对齐。

---

## 5. 纹理/缓冲区生命周期

### 5.1 RenderGraph 管理的资源

| 资源 | 用途 | 生命周期 |
|------|------|---------|
| `vbuffer` | VBuffer 主输出（PackedHitInfo） | RenderGraph 创建 |
| `depthStencil` | 深度缓冲 | RenderGraph 创建 |
| `viewW`, `depth`, `mvec` | 可选 extra channels | RenderGraph 创建（当前未写入） |

### 5.2 SceneMeshletData 管理的资源

| 资源 | 类型 | 创建时机 | 说明 |
|------|------|---------|------|
| `getMeshletBuffer()` | StructuredBuffer<GpuMeshletDesc> | `SceneMeshletData::build()` | Meshlet 元数据 |
| `getMeshletVerticesBuffer()` | StructuredBuffer<uint> | `SceneMeshletData::build()` | 顶点索引 |
| `getMeshletTrianglesBuffer()` | StructuredBuffer<uint> | `SceneMeshletData::build()` | 三角形索引（meshopt uint8→uint32 转换） |

### 5.3 Pass 内部资源

| 资源 | 创建时机 | 销毁时机 |
|------|---------|---------|
| `mRaster.pProgram` | setScene()（有 meshlet 时） | 随 Pass 析构 |
| `mRaster.pVars` | setScene() | 随 Pass 析构 |
| `mRaster.pState` | 构造函数 | 随 Pass 析构 |
| `mpFbo` | 构造函数 | 随 Pass 析构 |

---

## 6. 执行流程

### 6.1 绑定顺序（execute 内）

```
1. CB 绑定：var["CB"]["gFrameDim"], var["CB"]["gMeshletCount"]
2. SRV 绑定：gMeshlets, gMeshletVertices, gMeshletTriangles
3. 场景绑定：mpScene->bindShaderDataForRaytracing(var["gScene"])
4. FBO：attachColorTarget(vbuffer, 0), attachDepthStencilTarget(depth)
5. drawMeshTasks(mMeshletCount, 1, 1)
```

符合推荐顺序：**CBV → SRV → 自动绑定(Scene) → FBO**。

### 6.2 Mesh Shader 执行流程

1. `meshMain` 按 `SV_GroupID.x` 获取 meshletID
2. 从 `gMeshlets[meshletID]` 读取元数据
3. 通过 `gScene.getWorldMatrix()`、`getGeometryInstance()` 获取变换与顶点偏移
4. 从 `gMeshletVertices`、`gMeshletTriangles` 输出顶点与三角形
5. `SetMeshOutputCounts(numVertices, numPrimitives)` 设置输出数量
6. 顶点/三角形/图元数据写入 `OutputVertices`、`OutputIndices`、`OutputPrimitives`

### 6.3 Pixel Shader 执行流程

1. `psMain` 接收 `MeshVertexOut`、`MeshPrimitiveOut`、`SV_Barycentrics`
2. 构造 `TriangleHit`（instanceID、primitiveIndex、barycentrics）
3. `triangleHit.pack()` 得到 `PackedHitInfo` 写入 `SV_TARGET0`

---

## 7. 问题与建议

### 7.1 Extra Channels 未实现

- **现象**：`reflect()` 声明了 `viewW`、`depth`、`mvec`，但 shader 无对应变量，C++ 未绑定。
- **建议**：若需与 VBufferRaster 对齐，可参考 VBufferRaster 增加 `getValidResourceDefines()`、循环绑定 `var[channel.texname]`，并在 shader 中声明并写入；否则可从 `kVBufferExtraChannels` 中移除以保持接口一致。

### 7.2 gFrameDim 未使用

- **现象**：`gFrameDim` 已绑定到 CB，但 mesh/ps shader 中未使用。
- **建议**：若为预留字段可保留；否则可从 cbuffer 中移除以节省 CB 空间。

### 7.3 绑定正确性

- CB、gMeshlets、gMeshletVertices、gMeshletTriangles、gScene 的绑定与 shader 声明一致。
- GpuMeshlet 与 GpuMeshletDesc 布局匹配。
- FBO 的 RTV/DSV 与 shader 输出一致。
