# mpScene->rasterize() 流程深入分析

## 1. 入口与重载

```cpp
// Scene.h:899
void rasterize(RenderContext*, GraphicsState*, ProgramVars*, RasterizerState::CullMode cullMode = Back);

// Scene.h:909
void rasterize(RenderContext*, GraphicsState*, ProgramVars*,
    const ref<RasterizerState>& pRasterizerStateCW,
    const ref<RasterizerState>& pRasterizerStateCCW);
```

**调用链**（以 VBufferRaster 为例）：
```
VBufferRaster::execute()
  → mpScene->rasterize(pRenderContext, mRaster.pState.get(), mRaster.pVars.get(), cullMode)
  → Scene::rasterize(..., cullMode)
  → Scene::rasterize(..., mFrontClockwiseRS[cullMode], mFrontCounterClockwiseRS[cullMode])
```

---

## 2. 核心实现 (Scene.cpp:339-375)

```cpp
void Scene::rasterize(..., RasterizerState::CullMode cullMode)
{
    rasterize(..., mFrontClockwiseRS[cullMode], mFrontCounterClockwiseRS[cullMode]);
}

void Scene::rasterize(..., pRasterizerStateCW, pRasterizerStateCCW)
{
    FALCOR_PROFILE(pRenderContext, "rasterizeScene");

    // 1. 绑定 Scene 参数块到着色器
    pVars->setParameterBlock(kParameterBlockName, mpSceneBlock);

    auto pCurrentRS = pState->getRasterizerState();
    bool isIndexed = hasIndexBuffer();

    // 2. 遍历所有 DrawArgs，逐个提交绘制
    for (const auto& draw : mDrawArgs)
    {
        FALCOR_ASSERT(draw.count > 0);

        // 2a. 设置 VAO（按索引格式选择 16/32 位）
        pState->setVao(draw.ibFormat == ResourceFormat::R16Uint ? mpMeshVao16Bit : mpMeshVao);

        // 2b. 按三角形绕序设置光栅化状态
        if (draw.ccw) pState->setRasterizerState(pRasterizerStateCCW);
        else          pState->setRasterizerState(pRasterizerStateCW);

        // 2c. 间接绘制
        if (isIndexed)
            pRenderContext->drawIndexedIndirect(pState, pVars, draw.count, draw.pBuffer.get(), 0, nullptr, 0);
        else
            pRenderContext->drawIndirect(pState, pVars, draw.count, draw.pBuffer.get(), 0, nullptr, 0);
    }

    // 3. 恢复原始光栅化状态
    pState->setRasterizerState(pCurrentRS);
}
```

---

## 3. mDrawArgs 的构建 (createDrawList)

**调用时机**：`Scene::finalize()` 中，在 `updateGeometryInstances(true)` 之后。

### 3.1 DrawArgs 结构

```cpp
// Scene.h:1203
struct DrawArgs
{
    ref<Buffer> pBuffer;       // 间接绘制参数缓冲
    uint32_t count;            // 绘制命令数量
    bool ccw;                  // 是否为逆时针绕序
    ResourceFormat ibFormat;   // R16Uint 或 R32Uint
};
```

### 3.2 构建逻辑 (Scene.cpp:2665-2748)

按 **索引格式** 和 **绕序** 分成 4 组：

| 组 | 索引格式 | 绕序 | 用途 |
|----|----------|------|------|
| 0 | 16-bit | CW  | 小网格、顺时针 |
| 1 | 32-bit | CW  | 大网格、顺时针 |
| 2 | 16-bit | CCW | 小网格、逆时针 |
| 3 | 32-bit | CCW | 大网格、逆时针 |

**有索引缓冲时**（`DrawIndexedArguments`）：
```cpp
for (const auto& instance : mGeometryInstanceData)
{
    if (instance.getType() != GeometryType::TriangleMesh) continue;

    const auto& mesh = mMeshDesc[instance.geometryID];
    bool use16Bit = mesh.use16BitIndices();

    DrawIndexedArguments draw;
    draw.IndexCountPerInstance = mesh.indexCount;
    draw.InstanceCount = 1;
    draw.StartIndexLocation = mesh.ibOffset * (use16Bit ? 2 : 1);
    draw.BaseVertexLocation = mesh.vbOffset;
    draw.StartInstanceLocation = instanceID++;

    // 按世界空间绕序分组
    (instance.isWorldFrontFaceCW()) ? drawClockwiseMeshes[i].push_back(draw)
                                    : drawCounterClockwiseMeshes[i].push_back(draw);
}
```

**无索引缓冲时**（`DrawArguments`）：
```cpp
draw.VertexCountPerInstance = mesh.vertexCount;
draw.InstanceCount = 1;
draw.StartVertexLocation = mesh.vbOffset;
draw.StartInstanceLocation = instanceID++;
```

---

## 4. VAO 与顶点布局

### 4.1 顶点缓冲 (createMeshVao)

| Slot | 缓冲 | 用途 |
|------|------|------|
| kStaticDataBufferIndex (0) | mMeshStaticData | 顶点位置、法线、切线、UV |
| kDrawIdBufferIndex (1) | pDrawIDBuffer | 实例 DrawID [0, 1, 2, ...] |

### 4.2 顶点布局

```cpp
// 静态数据 (Per-Vertex)
VERTEX_POSITION, VERTEX_PACKED_NORMAL_TANGENT_CURVE_RADIUS, VERTEX_TEXCOORD

// 实例数据 (Per-Instance, 1 元素)
INSTANCE_DRAW_ID (DRAW_ID) → 映射到 Shader 的 instanceID
```

### 4.3 Shader 输入 (Raster.slang)

```hlsl
struct VSIn
{
    float3 pos, packedNormalTangentCurveRadius;
    float2 texC;
    uint instanceID : DRAW_ID;   // 来自 DrawID 缓冲，即 GeometryInstanceID.index
    uint vertexID   : SV_VertexID;
};
```

`instanceID` 来自 `StartInstanceLocation` 对 DrawID 缓冲的偏移：每个间接绘制命令的 `StartInstanceLocation = instanceID`，GPU 从 `DrawIDBuffer[instanceID]` 读取，得到正确的实例索引。

---

## 5. 光栅化状态 (mFrontClockwiseRS / mFrontCounterClockwiseRS)

在 `Scene` 构造函数中创建 6 种状态：

```cpp
// Scene.cpp:192-197
mFrontClockwiseRS[None]  = RasterizerState::create(Desc().setFrontCounterCW(false).setCullMode(None));
mFrontClockwiseRS[Back]  = RasterizerState::create(Desc().setFrontCounterCW(false).setCullMode(Back));
mFrontClockwiseRS[Front] = RasterizerState::create(Desc().setFrontCounterCW(false).setCullMode(Front));
mFrontCounterClockwiseRS[None]  = ...
mFrontCounterClockwiseRS[Back]  = ...
mFrontCounterClockwiseRS[Front] = ...
```

- **CW**：物体空间顺时针为正面
- **CCW**：物体空间逆时针为正面  
根据 `instance.isWorldFrontFaceCW()` 选择 CW 或 CCW 状态，保证世界空间正面剔除正确。

---

## 6. RenderContext 绘制路径

### 6.1 drawIndexedIndirect

```cpp
// RenderContext.cpp:483
void RenderContext::drawIndexedIndirect(pState, pVars, maxCommandCount, pArgBuffer, ...)
{
    resourceBarrier(pArgBuffer, Resource::State::IndirectArg);
    auto encoder = drawCallCommon(pState, pVars);
    encoder->drawIndexedIndirect(maxCommandCount, pArgBuffer->getGfxBufferResource(), ...);
}
```

### 6.2 drawCallCommon 主要步骤

1. `pVars->prepareDescriptorSets(this)`：准备描述符
2. `ensureFboAttachmentResourceStates`：FBO 附件状态
3. VAO 顶点/索引缓冲 `resourceBarrier(..., VertexBuffer/IndexBuffer)`
4. 获取/创建 `RenderCommandEncoder`
5. `encoder->bindPipelineWithRootObject(pGso, pVars->getShaderObject())`
6. 绑定 VAO 的顶点缓冲、索引缓冲、图元拓扑、视口、裁剪

---

## 7. 数据流概览

```
┌─────────────────────────────────────────────────────────────────────────┐
│                        Scene::rasterize()                                │
├─────────────────────────────────────────────────────────────────────────┤
│  1. pVars->setParameterBlock("gScene", mpSceneBlock)                      │
│  2. for each draw in mDrawArgs:                                          │
│       ├─ setVao(mpMeshVao / mpMeshVao16Bit)                              │
│       ├─ setRasterizerState(CW or CCW)                                    │
│       └─ drawIndexedIndirect(draw.count, draw.pBuffer)                    │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  drawIndexedIndirect 内部                                                │
│  - 每个 DrawIndexedArguments:                                            │
│      IndexCountPerInstance, InstanceCount=1, StartIndexLocation,         │
│      BaseVertexLocation, StartInstanceLocation(=instanceID)               │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  Vertex Shader 输入                                                       │
│  - 顶点缓冲: pos, normal, tangent, texC (按 BaseVertexLocation 偏移)     │
│  - 索引缓冲: 按 StartIndexLocation 取三角形索引                           │
│  - 实例缓冲: DrawID[StartInstanceLocation] = instanceID → 传入 vsIn       │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  VS 使用 instanceID 查询场景                                             │
│  - gScene.getWorldMatrix(instanceID)                                     │
│  - gScene.getMaterialID(instanceID)                                      │
│  - gScene.getGeometryInstance(instanceID)                                │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## 8. 关键点

1. **仅 TriangleMesh**：`createDrawList` 只处理 `GeometryType::TriangleMesh`，不包含 Displaced/Curve/SDFGrid。
2. **间接绘制**：每个 mesh 实例对应一条间接绘制命令，`InstanceCount=1`，`StartInstanceLocation` 用于实例索引。
3. **DrawID 缓冲**：`[0, drawCount)` 作为实例 ID，通过 `StartInstanceLocation` 索引到 DrawID 缓冲，传入 VS 的 `instanceID`。
4. **绕序**：按 `isWorldFrontFaceCW()` 分 CW/CCW，保证正确剔除。
5. **索引格式**：16/32 位索引分别用 `mpMeshVao16Bit` / `mpMeshVao`。
6. **createDrawList 时机**：仅在 `finalize()` 中调用，不随场景更新。
