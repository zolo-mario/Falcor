# D3D12Bundles 迁移记录

## Scaffold 命令

```bash
python tools/make_new_sample_app.py D3D12Bundles --path Source/Samples/Desktop
```

## 5.1 DX Sample → Falcor 映射表

| DX 概念 / 代码 | Falcor 对应 | 备注 |
|----------------|-------------|------|
| `LoadPipeline()` | （Falcor 抽象） | Device、swap chain、descriptor heaps 均由 SampleApp 管理 |
| `LoadAssets()` → RootSignature | （Falcor 抽象） | SRV + Sampler + CBV，Slang 自动生成 |
| `LoadAssets()` → PSO1/PSO2 | `Program::create` + `GraphicsState::setProgram` | MeshSimple.slang、MeshAlt.slang 替代 shader_mesh_*.hlsl |
| `LoadAssets()` → Vertex/Index buffer | `createBuffer` + `Vao::create(..., mpIndexBuffer, R32Uint)` | 从 occcity.bin 加载 |
| `LoadAssets()` → Texture BC1 | `createTexture2D(1024,1024, BC1Unorm, ..., data)` | occcity.bin 前 524288 字节 |
| `FrameResource` + per-city CBV | `mpConstantBuffer` + `setBlob` 每帧每 draw | 单 CB 每 draw 更新，等价 30 个 CBV |
| `ExecuteBundle` / `PopulateCommandList` | `drawIndexed` 循环 30 次 | Falcor 无 bundle API，直接录制等价命令 |
| `SimpleCamera` (WASD+方向键) | `OrbiterCameraController` | 轨道相机，setModelParams(center, radius, dist) |
| `CreateFrameResources` + 3 帧 | （简化） | 无显式 frame resource 轮转，Falcor 内部管理 |
| `CityRowCount=10, CityColumnCount=3` | `kCityRowCount`, `kCityColumnCount` | 30 个 city 实例 |
| 交替 PSO1/PSO2 | `usePso1 ? mpStateSimple : mpStateAlt` | 每 city 切换 pixel shader |
| `SetCityPositions(8, -8)` | `mModelMatrices` 循环，y=0.02*(i*col+j) | 防止 z-fighting |

## 保留的常量

| 常量 | 原始值 | Falcor 中 |
|------|--------|-----------|
| clearColor | `{ 0.0f, 0.2f, 0.4f, 1.0f }` | `float4(0.0f, 0.2f, 0.4f, 1.0f)` |
| CityRowCount | 10 | `kCityRowCount` |
| CityColumnCount | 3 | `kCityColumnCount` |
| 相机初始位置 | `{ 8, 8, 30 }` | `setPosition(float3(8,8,30))` |
| City 间距 | intervalX=8, intervalZ=-8 | `cityOffsetX = j*8`, `cityOffsetZ = i*-8` |
| alt pixel filter | `float3(0.25, 1.0, 0.25)` | 同左 |
| occcity.bin 布局 | Vertex 524288, Index 1344536 | `kVertexDataOffset`, `kIndexDataOffset` |
| window title | `L"D3D12 Bundles"` | `"D3D12 Bundles"` |

## 资产依赖

- **occcity.bin**：需从 DirectX-Graphics-Samples 复制
  - 来源：`Samples/Desktop/D3D12Bundles/src/occcity.bin`
  - 目标：exe 同目录，或 `Source/Samples/Desktop/D3D12Bundles/data/`
  - CMake 可选：若 `../DirectX-Graphics-Samples` 存在则 POST_BUILD 自动复制

## 5.2 1:1 映射后运行错误及修复

| 问题 | 错误表现 | 修复 |
|------|----------|------|
| indexed draw | `draw(..., kNumIndices, 0)` 错误 | 使用 `drawIndexed(..., kNumIndices, 0, 0)` |
| Camera API | `getProjectionMatrix` 不存在 | 使用 `getProjMatrix()` |
| 双 PSO 的 vars | 单 mpVars 与 mpProgramAlt 不兼容 | 创建 `mpVarsSimple`、`mpVarsAlt` 分别绑定 |
| CB 绑定 | `var["cb0"] = mpConstantBuffer` 报错 "bind buffer to non SRV/UAV variable" | 改用 `var["cb0"]["g_mWorldViewProj"] = mvp` 直接写入 ProgramVars 内置 CB |
| **无显示** | 运行后窗口仅蓝色背景，无 city 网格 | 待排查：depth、culling、MVP、顶点/索引数据、纹理格式等 |

## Build/run 验证

```bash
# 确保 occcity.bin 已复制（见资产依赖）
cmake --build build/windows-vs2022 --config Debug --target D3D12Bundles
.\build\windows-vs2022\bin\Debug\D3D12Bundles.exe
```

预期：蓝色背景 + 30 个 city 网格（交替纹理/滤镜着色），可鼠标旋转相机。

**已知问题**：当前仅显示蓝色背景，无 mesh 渲染。待排查。
