# MeshletCull 迁移记录

## 与 DirectX-Graphics-Samples 原版对比

| 维度 | DirectX-Graphics-Samples 原版 | Falcor 迁移版 |
|------|-------------------------------|---------------|
| **应用框架** | 独立 Win32 exe，继承 `DXSample` | Karma 插件，继承 `SampleBase` |
| **入口** | `Main.cpp` → `Win32Application::Run` | `registerPlugin` → Karma 加载 |
| **生命周期** | `OnInit` / `OnUpdate` / `OnRender` / `OnDestroy` | `onLoad` / `onFrameRender` / `onShutdown` |
| **模型** | Dragon_LOD0.bin（MeshletModel） | **test_scenes/bunny.pyscene**（SceneMeshletData） |
| **管线** | AS + MS + PS（视锥 + 法线锥剔除） | AS + MS + PS（**仅视锥剔除**，SceneMeshletData 无 normal cone） |
| **相机** | SimpleCamera | Scene 内建 Camera + OrbiterCameraController |
| **UI** | Direct2D | ImGui（`onGuiRender`） |

### Bunny 版演示

**MeshletCull 使用 bunny 模型演示 AS 剔除机制**：在 dispatch mesh shader 之前，Amplification Shader 对每个 meshlet 做视锥剔除（boundCenter + boundRadius），仅对可见 meshlet 调用 DispatchMesh，从而减少 MS 调度。

## Scaffold 命令

```bash
python tools/make_new_sample_app.py MeshletCull --path Source/Samples/Desktop
```

## DX Sample → Falcor 映射表

| DX 概念 / 代码 | Falcor 对应 | 备注 |
|----------------|-------------|------|
| 独立 SampleApp 可执行文件 | SampleBase 插件 (add_plugin) | 由 Karma 加载，树路径 Samples/Desktop/MeshletCull |
| `LoadPipeline()` | （Falcor 抽象） | Device、swap chain、RTV/DSV heap 由 Karma 管理 |
| `LoadAssets()` → AS+MS+PS PSO | `ProgramDesc().amplificationEntry().meshEntry().psEntry()` | MeshletCullBunny.slang |
| `LoadAssets()` → Model | `SceneBuilder` + `Scene::getMeshletData` | 加载 test_scenes/bunny.pyscene |
| `D3DX12_MESH_SHADER_PIPELINE_STATE_DESC` | `Program::create` + `GraphicsState::setVao(nullptr)` | Mesh pipeline 无 VAO |
| `DispatchMesh(meshletCount/32, 1, 1)` | `drawMeshTasks(asGroupCount, 1, 1)` | 调度 AS 组，AS 内部 DispatchMesh 调度 MS |
| `m_rootSignature` (from AS) | Slang 自动生成 | 无需显式 root signature |
| `m_constantBuffer` | `mpConstantsBuffer` (StructuredBuffer\<CBData\>) | viewProj, planes, meshletCount |
| `SimpleCamera` | `mpScene->setCameraController(Orbiter)` | Scene 内建相机 |
| `FrustumVisualizer`, `CullDataVisualizer` | （未移植） | 调试可视化，可后续添加 |
| `Pick()` 射线拾取 | （未移植） | meshlet 高亮/选择，可后续添加 |
| `ClearRenderTargetView` + `ClearDepthStencilView` | `pRenderContext->clearFbo(..., clearColor, 1.0f, ...)` | 保持 clearColor 一致 |

## 保留的常量

| 常量 | 原始值 | Falcor 中 |
|------|--------|-----------|
| clearColor | `{ 0.0f, 0.2f, 0.4f, 1.0f }` | `float4(0.0f, 0.2f, 0.4f, 1.0f)` |
| window title | `L"D3D12 Meshlet Cull"` | `"D3D12 Meshlet Cull"` |
| 相机位置 | `{ 0, 15, 40 }` | `float3(0.f, 15.f, 40.f)` |
| AS_GROUP_SIZE | 32 | 32 |
| CULL_FLAG, MESHLET_FLAG | 0x1, 0x2 | 0x1, 0x2 |

## Shader 映射 (HLSL → Slang)

| 原始 | Falcor (MeshletCullBunny.slang) |
|------|--------|
| MeshletAS.hlsl `ampMain` | `ampMain`（视锥剔除，用 GpuMeshlet.boundCenter/Radius） |
| MeshletMS.hlsl `main` | `meshMain`（import Scene.Scene，gScene.getVertex） |
| MeshletPS.hlsl `main` | `psMain` |
| CullData (BoundingSphere, NormalCone) | 仅 boundCenter + boundRadius（SceneMeshletData 无 normal cone） |
| `ConstantBuffer` | `StructuredBuffer<CBData> gCB` |
| `WavePrefixCountBits`, `WaveActiveCountBits` | 同左 |
| `DispatchMesh(visibleCount, 1, 1, s_Payload)` | 同左 |

## 1:1 映射后运行错误及修复

| 问题 | 错误表现 | 修复 |
|------|----------|------|
| 孤立代码 | Program/State 创建位于 `createProceduralMeshlet()` 外，编译错误 | 将 ProgramDesc、mpMeshletProgram 等移入 `onLoad()`，紧接 `createProceduralMeshlet()` 之后 |
| Camera 头文件 | `Camera/Camera.h` 找不到 | 改用 `Scene/Camera/Camera.h`、`Scene/Camera/CameraController.h` |
| VertexData 歧义 | 与 Falcor::VertexData 冲突 | 重命名为 `MeshletVertexData` |
| 矩阵乘法 | `proj * view` 编译错误（Falcor 矩阵 * 为逐分量） | 改用 `math::mul(proj, view)` |
| HotReloadFlags | `HotReloadFlags::Shader` 不存在 | 改用 `HotReloadFlags::Program` |
| Mesh shader 输出 | `out vertices`/`out indices` 与 dxc 冲突 | 改用 `OutputVertices`/`OutputIndices`（Slang 兼容） |
| ConstantBuffer 绑定 | Falcor ParameterBlock 不支持绑定 Buffer 到 ConstantBuffer | 改用 `StructuredBuffer<T>` 单元素，shader 中 `gConstants[0]` 访问 |
| 相机 | 无内置 camera | 添加 `Camera::create` + `OrbiterCameraController` |
| 视锥体平面 | `normalize(float4)` 对平面不正确 | 使用 `normalizePlane` 仅归一化 xyz，d 同比例 |
| 资产 | Dragon_LOD0.bin, Camera.bin 不在 Falcor 仓库 | 改用 **test_scenes/bunny.pyscene** + SceneMeshletData |

## 资产说明

- **test_scenes/bunny.pyscene**：Falcor 内置场景，含 bunny_dense.obj。路径由 FALCOR_MEDIA_FOLDERS 解析。
- 无需 DirectX-Graphics-Samples 或 WavefrontConverter。

## 简化与待办

- **已实现**：AS+MS+PS 管线、bunny 模型、SceneMeshletData、视锥剔除、Blinn-Phong 着色、meshlet 着色模式切换 (Space)
- **未移植**：Normal cone 剔除（SceneMeshletData 无 normal cone 数据）、FrustumVisualizer、Pick 拾取

## Build/run 验证

```bash
tools\.packman\cmake\bin\cmake.exe --build build/windows-vs2022 --config Debug --target Karma
```

运行：
```bash
# 交互模式（树形 UI 选择）
.\build\windows-vs2022\bin\Debug\Karma.exe

# 直接加载
.\build\windows-vs2022\bin\Debug\Karma.exe --sample Samples/Desktop/MeshletCull

# headless
.\build\windows-vs2022\bin\Debug\Karma.exe --sample Samples/Desktop/MeshletCull --headless
```

预期：蓝色背景 + bunny 模型（meshlet 着色或 Blinn-Phong），Space 切换 meshlet 模式，鼠标旋转/缩放相机。AS 在 dispatch MS 前做视锥剔除。需要 Shader Model 6.5 和 Mesh Shader 支持。
