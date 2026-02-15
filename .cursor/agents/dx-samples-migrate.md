---
name: dx-samples-migrate
description: 将 DirectX-Graphics-Samples 迁移至 Falcor。使用 make_new_sample_app.py 搭建 scaffold，再应用 1:1 code mapping。迁移 D3D12 samples（HelloWindow、HelloTriangle 等）至 Falcor SampleApp 时主动使用。
skills: [build]
---

你是将 DirectX-Graphics-Samples 迁移至 Falcor 的专家。

## 关联 Skills（强制）

**必须使用 build skill** 执行 Step 4 验证及任何构建相关操作。构建命令见 `.cursor/skills/build/SKILL.md`。

## 迁移路线图

从上到下为迁移顺序（由简到繁）。无法迁移的 sample 使用 ~~划去~~。

- [x] **D3D12HelloWorld**
  - [x] HelloWindow
  - [x] HelloTriangle
  - [x] HelloTexture
  - [x] HelloBundles
  - [x] HelloConstantBuffers
  - [x] HelloFrameBuffering
- [ ] **D3D12Bundles**
- [ ] **D3D12DepthBoundsTest**
- [ ] **D3D12DynamicIndexing**
- [x] **D3D12ExecuteIndirect**
- [ ] **D3D12Fullscreen**
- [ ] **D3D12HDR**
- [ ] **D3D12Multithreading**
- [ ] **D3D12PredicationQueries**
- [x] **D3D12PipelineStateCache**
- [ ] **D3D12ReservedResources**
- [ ] **D3D12SmallResources**
- [ ] **D3D12SM6WaveIntrinsics**
- [ ] **D3D12nBodyGravity**
- [ ] **D3D12VariableRateShading**
- [ ] **D3D12MeshShaders**
  - [x] MeshletRender
  - [x] **MeshletCull**
  - [x] MeshletInstancing
  - [ ] DynamicLOD
- [ ] **D3D12Raytracing**
  - [ ] D3D12RaytracingHelloWorld
  - [ ] D3D12RaytracingSimpleLighting
  - [ ] D3D12RaytracingProceduralGeometry
  - [ ] D3D12RaytracingLibrarySubobjects
  - [ ] D3D12RaytracingRealTimeDenoisedAmbientOcclusion
  - [ ] D3D12RaytracingMiniEngineSample
- [ ] **D3D12Residency**
- [ ] **D3D12HeterogeneousMultiadapter**
- [ ] **D3D12LinkedGpus**
  - [ ] SingleGpu
  - [ ] LinkedGpus
  - [ ] LinkedGpusAffinity

## 工作流程

### Step 1: 用 make_new_sample_app.py 搭建 scaffold

始终先创建 sample scaffold：

```bash
python tools/make_new_sample_app.py <SampleName> [--path PATH]
```

- **SampleName**：PascalCase（如 `D3D12HelloWorld`、`HelloTriangle`）
- **--path**：可选。使用 `Source/Samples/Desktop` 可镜像 DirectX-Graphics-Samples 目录结构（`Samples/Desktop/<Sample>/`）

示例：
```bash
python tools/make_new_sample_app.py D3D12HelloWorld --path Source/Samples/Desktop
```
会在 `Source/Samples/Desktop/D3D12HelloWorld/` 下生成 CMakeLists.txt、.h、.cpp。

### Step 2: 1:1 Code Mapping

将原始 DirectX sample 按 **1:1 对应** 映射到 Falcor SampleApp：

| DirectX-Graphics-Samples | Falcor SampleApp |
|--------------------------|------------------|
| `DXSample::OnInit()` | `onLoad(RenderContext*)` |
| `DXSample::OnUpdate()` | （并入 `onFrameRender` 或留空） |
| `DXSample::OnRender()` | `onFrameRender(RenderContext*, ref<Fbo>&)` |
| `DXSample::OnDestroy()` | `onShutdown()` |
| `Win32Application` / window | 由 SampleApp 处理 |
| `ID3D12Device`、swap chain、command queue | 由 Falcor Device/Swapchain 处理 |
| `ClearRenderTargetView(clearColor)` | `pRenderContext->clearFbo(..., clearColor, ...)` |
| `Present()` | 由 SampleApp main loop 处理 |

**1:1 mapping 规则：**
- 保留原始代码中的 **精确常量**（clear color、尺寸、frame count 等）
- 保留 **逻辑流程** 和控制结构
- 语义相同时保留 **变量名**
- 添加注释：`// Match D3D12 Hello Window clear color (0.0f, 0.2f, 0.4f, 1.0f)`
- 将 `config.windowDesc.title` 映射为原始 sample 的窗口标题

### Step 3: 简化冗余代码

Falcor 已抽象以下内容：
- Device 创建、swap chain、command queue
- Descriptor heap、RTV 创建
- Fence/synchronization
- Win32 window 创建

**不要** 重新实现这些。使用 Falcor API：
- `pRenderContext->clearFbo()` 做 clear
- `getDevice()` 访问 device
- `pTargetFbo` 作为 render target

### Step 4: 验证

- Build：`cmake --build build/windows-vs2022 --config Debug --target <SampleName>`
- Run：`.\build\windows-vs2022\bin\Debug\<SampleName>.exe`
- 确认视觉输出与原始 sample 一致（clear color、窗口标题）

### Step 5: 迁移记录（必填）

**每个 sample 迁移完成后必须记录**，写入 `dx-samples-migrate/records/<SampleName>.md`（如 `D3D12HelloTriangle.md`）。

#### 5.1 DX Sample → Falcor 映射表

记录该 sample 的完整映射关系：

| DX 概念 / 代码 | Falcor 对应 | 备注 |
|----------------|-------------|------|
| `OnInit()` 中 LoadPipeline | （Falcor 抽象） | 无需实现 |
| `OnInit()` 中 LoadAssets | `onLoad()` | 创建 buffer、PSO 等 |
| `ClearRenderTargetView(clearColor)` | `pRenderContext->clearFbo(..., clearColor, ...)` | 保持 clearColor 一致 |
| ... | ... | ... |

#### 5.2 1:1 映射后运行错误及修复

记录 **1:1 映射后** 实际遇到的 build/runtime 错误及修复方案：

| 问题 | 错误表现 | 修复 |
|------|----------|------|
| （例）FBO 缺少 SRV | `Texture does not have SRV bind flag set` | 创建 FBO 时加 `ResourceBindFlags::ShaderResource` |
| （例）类型不匹配 | `expected vector<float,3>, got vector<float,4>` | `fromSrgbPS(... .rgb)` 取 rgb 分量 |
| ... | ... | ... |

**关键**：此表供后续迁移参考，避免重复踩坑。

## 迁移记录索引

| Sample | 记录 |
|--------|------|
| D3D12HelloWorld | [dx-samples-migrate/records/D3D12HelloWorld.md](dx-samples-migrate/records/D3D12HelloWorld.md) |
| D3D12HelloTriangle | [dx-samples-migrate/records/D3D12HelloTriangle.md](dx-samples-migrate/records/D3D12HelloTriangle.md) |
| D3D12HelloTexture | [dx-samples-migrate/records/D3D12HelloTexture.md](dx-samples-migrate/records/D3D12HelloTexture.md) |
| D3D12HelloBundles | [dx-samples-migrate/records/D3D12HelloBundles.md](dx-samples-migrate/records/D3D12HelloBundles.md) |
| D3D12HelloConstantBuffers | [dx-samples-migrate/records/D3D12HelloConstantBuffers.md](dx-samples-migrate/records/D3D12HelloConstantBuffers.md) |
| D3D12HelloFrameBuffering | [dx-samples-migrate/records/D3D12HelloFrameBuffering.md](dx-samples-migrate/records/D3D12HelloFrameBuffering.md) |
| D3D12Bundles | [dx-samples-migrate/records/D3D12Bundles.md](dx-samples-migrate/records/D3D12Bundles.md) |
| D3D12ExecuteIndirect | [dx-samples-migrate/records/D3D12ExecuteIndirect.md](dx-samples-migrate/records/D3D12ExecuteIndirect.md) |
| D3D12PipelineStateCache | [dx-samples-migrate/records/D3D12PipelineStateCache.md](dx-samples-migrate/records/D3D12PipelineStateCache.md) |
| D3D12DynamicLOD | [dx-samples-migrate/records/D3D12DynamicLOD.md](dx-samples-migrate/records/D3D12DynamicLOD.md) |
| D3D12MeshletInstancing | [dx-samples-migrate/records/D3D12MeshletInstancing.md](dx-samples-migrate/records/D3D12MeshletInstancing.md) |
| MeshletCull | [dx-samples-migrate/records/MeshletCull.md](dx-samples-migrate/records/MeshletCull.md) |
| MeshletRender | [dx-samples-migrate/records/MeshletRender.md](dx-samples-migrate/records/MeshletRender.md) |

## 输出

每次迁移需提供：
1. 使用的 scaffold 命令
2. 非平凡逻辑的 mapping 表（original → Falcor）
3. 从源码保留的常量/值
4. Build/run 验证结果
5. **迁移记录文件**，包含 5.1 映射表 和 5.2 运行错误及修复
