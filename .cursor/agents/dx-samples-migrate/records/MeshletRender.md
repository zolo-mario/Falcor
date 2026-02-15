# MeshletRender 迁移记录

## Scaffold 命令

```bash
python tools/make_new_sample_app.py MeshletRender --path Source/Samples/Desktop
```

## 5.1 DX Sample → Falcor 映射表

| DX 概念 / 代码 | Falcor 对应 | 备注 |
|----------------|-------------|------|
| `LoadPipeline()` | （Falcor 抽象） | Device、swap chain、RTV/DSV 由 SampleApp 管理 |
| `LoadAssets()` → Mesh Shader PSO | `Program::create` + `meshEntry` + `psEntry` | MeshletRender.slang，SM 6.5 |
| `LoadAssets()` → RootSignature | （Slang 自动生成） | CBV(b0), RootConstants(b1), SRV(t0-t3) |
| `LoadAssets()` → Model | `MeshletModel::loadFromFile` + `uploadGpuResources` | 移植 Model.h/cpp，.bin 格式 |
| `LoadAssets()` → Constant buffer | `createBuffer(..., ResourceBindFlags::Constant)` | World, WorldView, WorldViewProj, DrawMeshlets |
| `OnUpdate()` → 相机 + 矩阵 | `updateCamera` + `updateConstants` | SimpleCamera 逻辑内联 |
| `PopulateCommandList()` → DispatchMesh | `drawMeshTasks(subset.Count, 1, 1)` | 每 subset 一次，需更新 MeshInfo |
| `SetGraphicsRootConstantBufferView(0, ...)` | `var["CB"] = mpConstantBuffer` | 主常量缓冲 |
| `SetGraphicsRoot32BitConstant(1, ...)` | `var["MeshInfoCB"] = mpMeshInfoBuffer` | IndexBytes, MeshletOffset 每 subset 更新 |
| `SetGraphicsRootShaderResourceView(2-5, ...)` | `var["Vertices"]` 等 | Vertices, Meshlets, UniqueVertexIndices, PrimitiveIndices |
| `ClearRenderTargetView` + `ClearDepthStencilView` | `pRenderContext->clearFbo` | clearColor (0, 0.2, 0.4, 1) |
| `OMSetRenderTargets` | `mpGraphicsState->setFbo(mpFbo)` | 带 depth 的 FBO |
| `D3D12_FEATURE_SHADER_MODEL` 6.5 | `isShaderModelSupported(ShaderModel::SM6_5)` | 启动时检查 |
| `D3D12_FEATURE_D3D12_OPTIONS7` MeshShaderTier | （Falcor 抽象） | 由 SM 6.5 隐含 |

## 保留的常量

| 常量 | 原始值 | Falcor 中 |
|------|--------|-----------|
| clearColor | `{ 0.0f, 0.2f, 0.4f, 1.0f }` | `float4(0.0f, 0.2f, 0.4f, 1.0f)` |
| 相机初始位置 | `{ 0, 75, 150 }` | `float3(0, 75, 150)` |
| 移动速度 | 150 | `moveSpeed = 150.f` |
| FOV | π/3 | `3.14159f / 3.f` |
| DrawMeshlets | true | `c.DrawMeshlets = 1` |
| window title | `L"D3D12 MeshletRender"` | `"D3D12 MeshletRender"` |

## 5.2 1:1 映射后运行错误及修复

| 问题 | 错误表现 | 修复 |
|------|----------|------|
| createConstantBuffer 不存在 | 编译错误 | 使用 `createBuffer(size, ResourceBindFlags::Constant, MemoryType::Upload)` |
| Dragon_LOD0.bin 缺失 | 仅蓝色背景，无模型 | 需运行 D3D12 WavefrontConverter 生成；CMake 支持从 DirectX-Graphics-Samples/Assets 或 data/ 复制 |

## 资产说明

- **Dragon_LOD0.bin**：由 DirectX-Graphics-Samples 的 D3D12WavefrontConverter 从 Dragon.obj 生成。
- 若 DirectX-Graphics-Samples 与 Falcor 为兄弟目录，CMake 会尝试从 `../DirectX-Graphics-Samples/Samples/Desktop/D3D12MeshShaders/Assets/Dragon_LOD0.bin` 复制。
- 或可将生成的 bin 放入 `Source/Samples/Desktop/MeshletRender/data/`。

## Build/run 验证

```bash
cmake --build build/windows-vs2022 --config Debug --target MeshletRender
.\build\windows-vs2022\bin\Debug\MeshletRender.exe
```

预期：蓝色背景 + Dragon 模型（meshlet 着色模式），WASD 移动，方向键旋转视角。
