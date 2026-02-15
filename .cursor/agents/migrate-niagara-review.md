# Niagara 迁移代码审查报告

对照 [migrate-niagara-renderer.md](.cursor/agents/migrate-niagara-renderer.md) 计划，对当前实现进行逐项审查。

---

## 0. 1:1 移植后出现的问题（已修复）

以下为 1:1 移植后实际遇到的**构建/运行时错误**及修复方案，供后续迁移参考。

| 问题 | 错误表现 | 修复 |
|------|----------|------|
| **Camera API 缺失** | `error C2039: 'getOrientation'/'getFovY': is not a member of 'Falcor::Camera'` | 从 `getViewMatrix()` 逆矩阵 `math::decompose` 提取 orientation；用 `focalLengthToFovY(getFocalLength(), getFrameHeight())` 计算 fovY |
| **PS 纹理采样类型** | `error 30019: expected vector<float,3>, got vector<float,4>`；`*=` 无匹配重载 | `fromSrgbPS(Sample(...).rgb)`；`albedo *= float4(fromSrgbPS(...), 1.0)` |
| **D3D12 根签名** | `Cannot append range with implicit lower bound after an unbounded range` | `Texture2D gTextures[64]` 固定大小；`mpTextures.resize(64, pWhiteTex)` |
| **HLSL 8 位类型** | `error: unknown type name 'int8_t'/'uint8_t'`（DXC 不支持） | `NiagaraMeshlet` 中 `int8_t`→`int16_t`，`uint8_t`→`uint16_t`；C++ 同步修改 |
| **SetMeshOutputCounts** | `SetMeshOutputCounts cannot be called multiple times` | 合并分支，先计算 vertexCount/triangleCount，再统一调用一次 |
| **FBO SRV 标志** | `Texture does not have SRV bind flag set`（blit 时） | 创建 FBO 时加 `ResourceBindFlags::ShaderResource` |
| **顶点位置未反量化** | 顶点呈碎片化、星形/齿轮状错误几何（PIX 捕获） | `vx,vy,vz` 为 meshopt_quantizeHalf 的 uint16，shader 中需 `dequantizeHalf` 转回 float；texcoord 同理 |

**关键点**：Vulkan/GLSL 与 D3D12/HLSL 在 unbounded 描述符、8 位类型、mesh shader 调用约定上存在差异，1:1 移植后需针对性适配。顶点位置/texcoord 为 meshopt fp16 量化格式，shader 中必须反量化。

---

## 1. 架构与数据流 ✅

| 计划要求 | 当前实现 | 状态 |
|---------|---------|------|
| meshlet → mesh shader → PS 输出 meshlet ID | 已实现 | ✅ |
| 无剔除（无 drawcull/clustercull/task） | 已实现 | ✅ |
| clusterIndices CPU 预计算 | 已实现 | ✅ |
| FBO: G-Buffer(2) + MeshletID(R32Uint) + Depth | 已实现 | ✅ |
| drawMeshTasks(totalMeshletCount, 1, 1) | 已实现 | ✅ |

---

## 2. GPU 缓冲区上传 ✅

| 计划 | 实现 | 状态 |
|-----|------|------|
| vb | mpVb, NiagaraVertex[] | ✅ |
| ib | mpIb, uint32[] | ✅ |
| mlb | mpMlb, NiagaraMeshlet[] | ⚠️ 见下 |
| mdb | mpMdb, meshletdata | ✅ |
| db | mpDb, NiagaraMeshDraw[] | ✅ |
| mb | mpMb, NiagaraMesh[] | ✅ |
| mtb | mpMtb, NiagaraMaterial[] | ✅ |
| cib | mpCib, clusterIndices | ⚠️ 见下 |
| dcb | mpDcb, MeshTaskCommand[] | ✅ |
| textures | mpTextures | ✅ |

---

## 3. 发现的问题（含已修复项）

### 🔴 P0：Camera API 缺失 ✅ 已修复

**位置**：`NiagaraScene.cpp` convertFalcorSceneToNiagaraScene

**问题**：Falcor `Camera` 无 `getOrientation()`/`getFovY()`，构建报错 `is not a member of 'Falcor::Camera'`。

**修复**：`orientation` 从 `math::decompose(math::inverse(getViewMatrix()), ...)` 提取；`fovY = focalLengthToFovY(getFocalLength(), getFrameHeight())`；include `Utils/Math/FalcorMath.h`。

---

### 🔴 P0：clusterIndices / commandId 索引错误 ✅ 已修复

**计划**：`commandId = drawIndex * 64`（每 draw 一个 command）

**问题**：计划中的 `drawIndex * 64` 与扁平化 1D 调度不匹配。`gTaskCommands` 按 draw 顺序存储，每 draw 一条记录，索引应为 `0, 1, 2, ...`。使用 `commandId = drawId * 64` 时，draw 1 会访问 `gTaskCommands[64]`，越界，导致 mesh shader 读取错误的 meshlet 数据（PIX 捕获显示 fragmented/noisy 输出）。

**修复**：
```cpp
uint32_t commandId = (uint32_t)(taskCommands.size() - 1);
```
（在每次 push cmd 后，commandId 为该 command 在数组中的下标）

---

### 🔴 P0：SetMeshOutputCounts 多次调用 ✅ 已修复

**位置**：`NiagaraMeshlet.ms.slang`

**问题**：Mesh shader 中 `SetMeshOutputCounts` 只能调用一次，当前存在两处（`ci == 0xFFFFFFFF` 分支与正常路径）。D3D12 报错：`SetMeshOutputCounts cannot be called multiple times`。

**修复**：合并为单一调用，先根据 `ci` 计算 `vertexCount`/`triangleCount`（无效时为 0），再统一调用一次。

---

### 🔴 P0：FBO 纹理缺少 ShaderResource 标志 ✅ 已修复

**位置**：`Niagara.cpp` onResize

**问题**：blit 需要源纹理可读（SRV），当前仅设置 `RenderTarget`。运行时报错：`Texture does not have SRV bind flag set`。

**修复**：
```cpp
ResourceBindFlags::RenderTarget | ResourceBindFlags::ShaderResource
```

---

### 🟡 P1：HLSL 与 NiagaraMeshlet 的 int8_t/uint8_t ✅ 已修复

**问题**：DXC 不支持 `int8_t`/`uint8_t`，编译报错 `unknown type name 'int8_t'`。HLSL 无原生 8 位标量类型。

**修复**：`NiagaraMeshlet` 中 `int8_t`→`int16_t`，`uint8_t`→`uint16_t`；C++ `niagaraScene.h` 与 `NiagaraScene.cpp` 同步修改；`shortRefs` 存为 `0/1`。

---

### 🟡 P1：纹理数组 unbounded 与 D3D12 根签名 ✅ 已修复

**位置**：`NiagaraMeshlet.ms.slang`

**问题**：`Texture2D gTextures[]` 为 unbounded 数组，D3D12 根签名报错：`Cannot append range with implicit lower bound after an unbounded range`（unbounded 后不能有其它描述符）。

**修复**：改为固定大小 `Texture2D gTextures[64]`；`mpTextures.resize(64, pWhiteTex)`；绑定循环 `for (i < kMaxTextures)`。

---

### 🟡 P1：PS 纹理采样类型 ✅ 已修复

**位置**：`NiagaraMeshlet.ms.slang` psMain

**问题**：`fromSrgbPS(float3)` 收到 `float4`；`albedo *=` 等操作类型不匹配。编译报错：`expected vector<float,3>, got vector<float,4>`。

**修复**：
```hlsl
albedo *= float4(fromSrgbPS(gTextures[...].Sample(...).rgb), 1.0);
specgloss *= float4(fromSrgbPS(...), 1.0);
emissive *= fromSrgbPS(... .rgb);
```

---

### 🟢 P2：Blit 前资源状态

**建议**：在 blit 前显式过渡 FBO 颜色纹理：
```cpp
pRenderContext->resourceBarrier(mpFbo->getColorTexture(0).get(), Resource::State::ShaderResource);
```

---

### 🟢 P2：D3D12 调试层崩溃 (0x87A)

**建议**：若 Debug 构建下出现 0x87A 异常，可临时禁用调试层：
```cpp
config.deviceDesc.enableDebugLayer = false;
```

---

## 4. 计划符合性检查

| 计划项 | 状态 |
|-------|------|
| 1:1 命名（drawId, meshletId, dataOffset, baseVertex, clusterIndices） | ✅ |
| 基于 meshlet.mesh.glsl (TASK=false) | ✅ |
| 使用 groupID.x 作为 cluster 索引 | ✅ |
| Mesh 输出 meshletId : MESHLET_ID | ✅ |
| PS 输出 G-Buffer + MeshletID R32Uint | ✅ |
| 无 VAO（mesh pipeline） | ✅ |
| Shader Model 6.5 | ✅ |
| 资源绑定表（gTaskCommands, gDraws, gMeshlets 等） | ✅ |

---

## 5. 未迁移组件（符合计划）

- drawcull.comp, tasksubmit.comp, clustercull.comp
- depthreduce.comp, meshlet.task.glsl
- mesh.vert.glsl, final.comp, shadow 相关

---

## 6. 建议修复优先级

1. **P0（必须）**：clusterIndices commandId、Camera API、SetMeshOutputCounts 单次调用、FBO ShaderResource
2. **P1（建议）**：int8/uint8→int16/uint16、固定纹理数组、PS 采样类型
3. **P2（可选）**：resourceBarrier、enableDebugLayer

**注**：本节「0. 1:1 移植后出现的问题」汇总了实际构建/运行错误，迁移时可直接对照排查。

---

## 7. 计划文档修正建议

计划中 clusterIndices 的 `commandId = drawIndex * 64` 在扁平化 1D 调度下应改为：**commandId = taskCommands 数组下标**（即每 draw 一条 command 时，commandId = drawId；若跳过 lodCount==0 的 draw，则用 `taskCommands.size()-1` 作为当前 command 的索引）。
