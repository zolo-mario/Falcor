---
name: dx-samples-migrate
description: 将 DirectX-Graphics-Samples 迁移至 Falcor。使用 make_new_sample_app.py 搭建 scaffold，再应用 1:1 code mapping。迁移 D3D12 samples（HelloWindow、HelloTriangle 等）至 Falcor SampleBase 插件，通过 Karma 运行。
skills: [build, run]
---

你是将 DirectX-Graphics-Samples 迁移至 Falcor 的专家。

## 关联 Skills（强制）

**必须使用 build skill** 执行 Step 4 构建；**run skill** 用于运行 Karma（含 `--sample`、`--headless`）。构建/运行命令见 `.cursor/skills/build/SKILL.md`、`.cursor/skills/run/SKILL.md`。

## 迁移路线图

从上到下为迁移顺序（由简到繁）。无法迁移的 sample 使用 ~~划去~~。

**不支持特性备注：**
- **Bundle**：Slang/Falcor 不支持 D3D12 Bundle，相关 sample 禁用。
- **HelloFrameBuffering**：与 HelloTriangle 渲染相同，Falcor 已抽象 frame buffering，移除冗余。
- **D3D12Fullscreen**：Falcor Window 无运行时全屏切换 API，放弃移植。

- [x] **D3D12HelloWorld**
  - [x] HelloWindow
  - [x] HelloTriangle
  - [x] HelloTexture
  - ~~HelloBundles~~（Bundle 不支持，禁用）
  - [x] HelloConstantBuffers
  - ~~HelloFrameBuffering~~（与 HelloTriangle 渲染相同，Falcor 已抽象 frame buffering，移除冗余）
- ~~**D3D12Bundles**~~（Bundle 不支持，禁用）
- [ ] **D3D12DepthBoundsTest**
- ~~**D3D12DynamicIndexing**~~（Bundle 不支持，禁用）
- [x] **D3D12ExecuteIndirect**
- ~~**D3D12Fullscreen**~~（放弃移植）
- [x] **D3D12HDR** //show nothing
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
  - [x] D3D12RaytracingHelloWorld
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

## Plan 阶段（前置确认）

**若 Slang/Falcor 不支持某 DX12 特性**，必须在 Plan 阶段主动说明，并**等待用户确认**后才开始移植。不得在未确认的情况下直接开始迁移。

示例：若目标 sample 依赖 Bundle、Reserved Resources 等 Slang 暂不支持的特性，先列出依赖与限制，待用户确认后再执行 Step 1。

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
会在 `Source/Samples/Desktop/D3D12HelloWorld/` 下生成 CMakeLists.txt、.h、.cpp。模板使用 `add_plugin`、继承 `SampleBase`，Sample 作为插件由 Karma 加载。

### Step 2: 1:1 Code Mapping

将原始 DirectX sample 按 **1:1 对应** 映射到 Falcor SampleBase（Karma 插件）：

| DirectX-Graphics-Samples | Falcor SampleBase |
|--------------------------|------------------|
| `DXSample::OnInit()` | `onLoad(RenderContext*)` |
| `DXSample::OnUpdate()` | （并入 `onFrameRender` 或留空） |
| `DXSample::OnRender()` | `onFrameRender(RenderContext*, ref<Fbo>&)` |
| `DXSample::OnDestroy()` | `onShutdown()` |
| `Win32Application` / window | 由 Karma（SampleApp）处理 |
| `ID3D12Device`、swap chain、command queue | 由 Falcor Device/Swapchain 处理 |
| `ClearRenderTargetView(clearColor)` | `pRenderContext->clearFbo(..., clearColor, ...)` |
| `Present()` | 由 Karma main loop 处理 |

**1:1 mapping 规则：**
- 保留原始代码中的 **精确常量**（clear color、尺寸、frame count 等）
- 保留 **逻辑流程** 和控制结构
- 语义相同时保留 **变量名**
- 添加注释：`// Match D3D12 Hello Window clear color (0.0f, 0.2f, 0.4f, 1.0f)`
- Sample 作为插件运行于 Karma，无独立窗口标题

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

### 可复用 Falcor 资产

当 sample 需要 3D 模型或相机时，**优先复用 Falcor 内置资产**，避免移植外部模型或自定义相机逻辑：

| 需求 | Falcor 对应 | 用法 |
|------|-------------|------|
| **场景/模型** | `test_scenes/bunny.pyscene` | `SceneBuilder(getDevice(), "test_scenes/bunny.pyscene", Settings(), ...).getScene()`，含 bunny_dense.obj |
| **相机** | Scene 内建 Camera + OrbiterCameraController | `mpScene->setCameraController(Scene::CameraControllerType::Orbiter)`，`mpScene->setCameraSpeed(25.f)`，转发 `onKeyEvent`/`onMouseEvent` 到 `mpScene`，`mpScene->update()` 每帧 |
| **Meshlet 数据** | SceneMeshletData | `mpScene->getMeshletData(pRenderContext)`，配合 `import Scene.Scene` 的 mesh shader |

示例：MeshletRender 迁移时用 bunny + Falcor 相机替代 Dragon.bin + 自定义 SimpleCamera，见 [MeshletRender.md](dx-samples-migrate/records/MeshletRender.md)。

### Step 4: 验证

- Build：`cmake --build build/windows-vs2022 --config Debug --target Karma`（会构建所有 Sample 插件）
- Run：
  - 交互：`.\build\windows-vs2022\bin\Debug\Karma.exe`，在树形 UI 中选择对应 Sample
  - 直接加载：`Karma.exe --sample Samples/Desktop/<SampleName>`（如 `--sample Samples/Desktop/D3D12ExecuteIndirect`；Hello* 为 `Samples/Desktop/D3D12HelloWorld/HelloTriangle`，Meshlet* 为 `Samples/Desktop/D3D12MeshShaders/MeshletCull`）
  - Headless：`Karma.exe --sample <path> --headless`（无窗口，用于自动化）
- 确认视觉输出与原始 sample 一致（clear color 等）

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
| ~~D3D12HelloBundles~~（禁用） | [dx-samples-migrate/records/D3D12HelloBundles.md](dx-samples-migrate/records/D3D12HelloBundles.md) |
| D3D12HelloConstantBuffers | [dx-samples-migrate/records/D3D12HelloConstantBuffers.md](dx-samples-migrate/records/D3D12HelloConstantBuffers.md) |
| ~~D3D12HelloFrameBuffering~~（禁用） | [dx-samples-migrate/records/D3D12HelloFrameBuffering.md](dx-samples-migrate/records/D3D12HelloFrameBuffering.md) |
| ~~D3D12Bundles~~（禁用） | [dx-samples-migrate/records/D3D12Bundles.md](dx-samples-migrate/records/D3D12Bundles.md) |
| D3D12ExecuteIndirect | [dx-samples-migrate/records/D3D12ExecuteIndirect.md](dx-samples-migrate/records/D3D12ExecuteIndirect.md) |
| D3D12HDR | [dx-samples-migrate/records/D3D12HDR.md](dx-samples-migrate/records/D3D12HDR.md) |
| D3D12PipelineStateCache | [dx-samples-migrate/records/D3D12PipelineStateCache.md](dx-samples-migrate/records/D3D12PipelineStateCache.md) |
| D3D12DynamicLOD | [dx-samples-migrate/records/D3D12DynamicLOD.md](dx-samples-migrate/records/D3D12DynamicLOD.md) |
| D3D12MeshletInstancing | [dx-samples-migrate/records/D3D12MeshletInstancing.md](dx-samples-migrate/records/D3D12MeshletInstancing.md) |
| MeshletCull | [dx-samples-migrate/records/MeshletCull.md](dx-samples-migrate/records/MeshletCull.md) |
| MeshletRender | [dx-samples-migrate/records/MeshletRender.md](dx-samples-migrate/records/MeshletRender.md) |
| D3D12RaytracingHelloWorld | [dx-samples-migrate/records/D3D12RaytracingHelloWorld.md](dx-samples-migrate/records/D3D12RaytracingHelloWorld.md) |

## 输出

每次迁移需提供：
1. 使用的 scaffold 命令
2. 非平凡逻辑的 mapping 表（original → Falcor）
3. 从源码保留的常量/值
4. Build/run 验证结果
5. **迁移记录文件**，包含 5.1 映射表 和 5.2 运行错误及修复
