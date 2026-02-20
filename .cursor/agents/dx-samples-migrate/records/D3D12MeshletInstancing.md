# D3D12MeshletInstancing 迁移记录

## Scaffold 命令

```bash
python tools/make_new_sample_app.py D3D12MeshletInstancing --path Source/Samples/Desktop
```

## 5.1 DX Sample → Falcor 映射表

| DX 概念 / 代码 | Falcor 对应 | 备注 |
|----------------|-------------|------|
| `LoadPipeline()` | （Falcor 抽象） | Device、swap chain、RTV/DSV heap 由 SampleApp 管理 |
| `LoadPipeline()` → Mesh Shader PSO | `ProgramDesc().addShaderLibrary(...).meshEntry("meshMain").psEntry("psMain").setShaderModel(SM6_5)` | MS+PS 管线，无 amplification shader |
| `LoadAssets()` → Model 加载 | `MeshletModel::loadFromFile()` | 移植 Model.h/cpp，解析 .bin 格式 |
| `LoadAssets()` → ToyRobot.bin | `findToyRobotBin()` | 从 runtime dir 或 DirectX-Graphics-Samples/Assets 查找 |
| `LoadAssets()` → GPU 资源上传 | `MeshletModel::uploadGpuResources()` | createStructuredBuffer/createBuffer |
| `SimpleCamera` | `OrbiterCameraController` | Falcor 内置轨道相机 |
| `OnUpdate()` → View/ViewProj | `mpCamera->getViewMatrix()`, `getViewProjMatrix()` | 每帧更新 constant buffer |
| `PopulateCommandList()` → DispatchMesh | `pRenderContext->drawMeshTasks(state, vars, groupCount, 1, 1)` | 按 meshlet subset 分批 dispatch |
| RootConstants (DrawParams, MeshInfo) | `mpDrawParamsBuffer`, `mpMeshInfoBuffer` | 每批 dispatch 前 setBlob 更新 |
| `RegenerateInstances()` | `regenerateInstances()` | 立方体网格布局实例，+/- 键调整 level |
| `ClearRenderTargetView` + `ClearDepthStencilView` | `pRenderContext->clearFbo(..., clearColor, 1.0f, 0, All)` | clearColor (0, 0.2, 0.4, 1) |
| `SetGraphicsRootConstantBufferView(0, ...)` | `var["Globals"] = mpConstantBuffer` | Scene constants |
| `SetGraphicsRootShaderResourceView(7, ...)` | `var["Instances"] = mpInstanceBuffer` | Instance 变换 |
| `SetGraphicsRootShaderResourceView(3-6, ...)` | `var["Vertices"]`, `var["Meshlets"]`, etc. | 每 mesh 绑定 |
| MeshletMS.hlsl + MeshletPS.hlsl | MeshletInstancing.ms.slang | 合并为单文件，HLSL→Slang 语法 |

## 保留的常量

| 常量 | 原始值 | Falcor 中 |
|------|--------|-----------|
| clearColor | `{ 0.0f, 0.2f, 0.4f, 1.0f }` | `float4(0.0f, 0.2f, 0.4f, 1.0f)` |
| 相机初始 | `Init({0, 5, 35})`, `SetMoveSpeed(60)` | `setPosition(0,5,35)`, `setCameraSpeed(60)` |
| MAX_VERTS / MAX_PRIMS | 64 / 126 | 同左 |
| c_maxGroupDispatchCount | 65536 | 同左 |
| window title | `L"D3D12 Meshlet Instancing"` | `"D3D12 Meshlet Instancing"` |

## 5.2 1:1 映射后运行错误及修复

| 问题 | 错误表现 | 修复 |
|------|----------|------|
| ToyRobot.bin 缺失 | 启动失败，无法加载模型 | CMake 配置从 DirectX-Graphics-Samples 复制；或需先构建 MeshletGenerator 生成 .bin |
| Shader Model 6.5 | 设备不支持 | 需 DirectX 12 Ultimate 兼容 GPU |
| OrbiterCameraController | 无 create() 静态方法 | 使用 `std::make_unique<OrbiterCameraController>(mpCamera)` |
| DrawParams/MeshInfo 每批更新 | Root constants 需每 dispatch 更新 | 创建小 buffer，每批 setBlob 后绑定 |
| ByteAddressBuffer | UniqueVertexIndices 需 raw buffer | 使用 createBuffer，Slang 绑定为 ByteAddressBuffer |

## Build/run 验证

```bash
cmake --build build/windows-vs2022 --config Debug --target Karma
.\build\windows-vs2022\bin\Debug\Karma.exe  # 在树中选择 Samples/Desktop/D3D12MeshletInstancing
```

**前置条件**：ToyRobot.bin 需位于可执行文件同目录。可从 DirectX-Graphics-Samples 构建 MeshletGenerator 后，将 `Samples/Desktop/D3D12MeshShaders/Assets/ToyRobot.bin` 复制到 Falcor 运行时目录，或确保 CMake 能发现并复制该文件。

预期：蓝色背景，ToyRobot 模型多实例渲染，+/- 调整实例密度，Space 切换 meshlet 着色模式。
