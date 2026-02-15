# MeshletCull 迁移记录

## Scaffold 命令

```bash
python tools/make_new_sample_app.py MeshletCull --path Source/Samples/Desktop
```

## 5.1 DX Sample → Falcor 映射表

| DX 概念 / 代码 | Falcor 对应 | 备注 |
|----------------|-------------|------|
| `LoadPipeline()` | （Falcor 抽象） | Device、swap chain、RTV/DSV heap 由 SampleApp 管理 |
| `LoadAssets()` → AS+MS+PS PSO | `ProgramDesc().amplificationEntry().meshEntry().psEntry()` | MeshletCull.slang 替代 MeshletAS/MS/PS.hlsl |
| `D3DX12_MESH_SHADER_PIPELINE_STATE_DESC` | `Program::create` + `GraphicsState::setVao(nullptr)` | Mesh pipeline 无 VAO |
| `DispatchMesh(meshletCount/32, 1, 1)` | `drawMeshTasks(asGroupCount, 1, 1)` | 调度 AS 组，AS 内部 DispatchMesh 调度 MS |
| `m_rootSignature` (from AS) | Slang 自动生成 | 无需显式 root signature |
| `LoadAssets()` → Model (.bin) | `createProceduralMeshlet()` | 当前用程序化单三角形 meshlet，未移植 .bin 加载 |
| `m_constantBuffer`, `m_objects[].InstanceResource` | `mpConstantsBuffer`, `mpInstanceBuffer` | 常量 + 实例数据 |
| `SimpleCamera` | `Camera::create` + `OrbiterCameraController` | 相机 (0,15,40) 看向原点 |
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

| 原始 | Falcor |
|------|--------|
| MeshletAS.hlsl `ampMain` | MeshletCull.slang `ampMain` |
| MeshletMS.hlsl `main` | MeshletCull.slang `meshMain` |
| MeshletPS.hlsl `main` | MeshletCull.slang `psMain` |
| MeshletCommon.hlsli, MeshletUtils.hlsli | 内联到 MeshletCull.slang |
| `ConstantBuffer<X> : register(b0)` | `ConstantBuffer<X> gX : register(b0)` |
| `ByteAddressBuffer` | `ByteAddressBuffer` |
| `WavePrefixCountBits`, `WaveActiveCountBits` | 同左（Slang 支持） |
| `DispatchMesh(visibleCount, 1, 1, s_Payload)` | 同左 |

## 5.2 1:1 映射后运行错误及修复

| 问题 | 错误表现 | 修复 |
|------|----------|------|
| 相机 | 无内置 camera | 添加 `Camera::create` + `OrbiterCameraController` |
| 视锥体平面 | `normalize(float4)` 对平面不正确 | 使用 `normalizePlane` 仅归一化 xyz，d 同比例 |
| 资产 | Dragon_LOD0.bin, Camera.bin 不在 Falcor 仓库 | 使用程序化单三角形 meshlet 验证管线 |

## 简化与待办

- **已实现**：AS+MS+PS 管线、程序化三角形、相机、Blinn-Phong 着色、meshlet 着色模式切换 (Space)
- **未移植**：Model .bin 加载、FrustumVisualizer、CullDataVisualizer、Pick 拾取、Debug Camera

## Build/run 验证

```bash
cmake --build build/windows-vs2022 --config Debug --target MeshletCull
.\build\windows-vs2022\bin\Debug\MeshletCull.exe
```

预期：蓝色背景 + 单三角形（Blinn-Phong 或 meshlet 着色），Space 切换 meshlet 模式。需要 Shader Model 6.5 和 Mesh Shader 支持。
