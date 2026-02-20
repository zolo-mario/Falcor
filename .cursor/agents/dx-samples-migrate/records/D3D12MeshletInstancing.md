# D3D12MeshletInstancing 迁移记录

## 与 DirectX-Graphics-Samples 原版对比（Falcor Bunny 版）

本迁移使用 **Falcor bunny 场景**、**SceneMeshletData** 和 **Falcor 相机系统**，替代原版 ToyRobot.bin 与 MeshletModel。

| 维度 | DirectX-Graphics-Samples 原版 | Falcor 迁移版（Bunny） |
|------|-------------------------------|------------------------|
| **应用框架** | 独立 Win32 exe，继承 `DXSample` | Karma 插件，继承 `SampleBase` |
| **模型** | ToyRobot.bin（MeshletModel） | test_scenes/bunny.pyscene（Scene + SceneMeshletData） |
| **Meshlet 数据** | MeshletModel::loadFromFile + uploadGpuResources | Scene::getMeshletData() → SceneMeshletData |
| **Meshlet 格式** | GpuMeshlet (VertCount, VertOffset, PrimCount, PrimOffset) | GpuMeshletDesc (vertexOffset, triangleOffset, vertexCount, triangleCount, instanceID, ...) |
| **顶点数据** | StructuredBuffer\<Vertex\> (Position, Normal) | gScene.getVertex() → StaticVertexData |
| **三角形索引** | PackedTriangle (10+10+10 bit) | gMeshletTriangles (uint32 × 3) |
| **相机** | 自定义 SimpleCamera | Scene 内建 Camera + OrbiterCameraController |
| **着色器** | MeshletInstancing.ms.slang | MeshletInstancingBunny.ms.slang |

## Scaffold 命令

```bash
python tools/make_new_sample_app.py D3D12MeshletInstancing --path Source/Samples/Desktop
```

## DX Sample → Falcor 映射表

| DX 概念 / 代码 | Falcor 对应 | 备注 |
|----------------|-------------|------|
| 独立 SampleApp 可执行文件 | SampleBase 插件 (add_plugin) | 由 Karma 加载，树路径 Samples/Desktop/D3D12MeshletInstancing |
| `LoadPipeline()` | （Falcor 抽象） | Device、swap chain 由 Karma 管理 |
| `LoadAssets()` → Model | `SceneBuilder` + `Scene::getMeshletData` | 加载 test_scenes/bunny.pyscene |
| `LoadAssets()` → Mesh Shader PSO | `Program::create` + Scene 集成 | addShaderModules, getSceneDefines, getTypeConformances |
| `LoadAssets()` → ToyRobot.bin | 移除 | 改用 bunny.pyscene |
| `SimpleCamera` | `mpScene->setCameraController(Orbiter)` | Scene 内建相机 |
| `OnUpdate()` → View/ViewProj | `gScene.camera.getViewProj()` | Shader 内从 Scene 获取 |
| `PopulateCommandList()` → DispatchMesh | `drawMeshTasks(meshletCount * instanceCount, 1, 1)` | 按 meshlet 分批（每批 InstanceOffset=0），groupCount = meshletCount × instanceCount |
| RootConstants (DrawParams, MeshInfo) | `mpDrawParamsBuffer`, `mpMeshInfoBuffer` | InstanceCount, InstanceOffset; MeshletCount, MeshletOffset |
| `RegenerateInstances()` | `regenerateInstances()` | 立方体网格布局，用 `mpScene->getSceneBounds().radius()` 计算 spacing |
| `ClearRenderTargetView` | `pRenderContext->clearFbo` | clearColor (0, 0.2, 0.4, 1) |
| `SetGraphicsRootShaderResourceView` | `var["gMeshlets"]`, `var["Instances"]` 等 | SceneMeshletData + Instance buffer |

## 1:1 映射后运行错误及修复

| 问题 | 错误表现 | 修复 |
|------|----------|------|
| ToyRobot.bin 依赖 | 原版需外部资产 | 改用 Falcor bunny.pyscene，无需 ToyRobot |
| MeshletModel 格式 | Vertices, UniqueVertexIndices, PrimitiveIndices | 改用 SceneMeshletData (gMeshletVertices, gMeshletTriangles) |
| View/ViewProj 传递 | 原版 C++ 每帧更新 | Shader 内 `gScene.camera.getViewProj()` |
| Input::Key::Plus 非法 | C2838/C2065（Windows 宏冲突） | 改用 `Input::Key::Equal`（主键盘 + 为 Shift+=） |
| 绑定 buffer 到 cbuffer | Error trying to bind buffer to a non SRV/UAV variable | 改用 `var["CB"]["gDrawMeshlets"]`、`var["DrawParamsCB"]["DrawParams_data"]` 等直接设标量/结构成员 |
| **instance > 1 时模型严重变形（飞面）** | 多实例时顶点位置错误，模型撕裂 | 见下方「多实例变形修复」 |

## 多实例变形修复（instance > 1 飞面问题）

**现象**：实例数 > 1 时，模型顶点位置错误，出现严重变形或“飞面”。

**根因与修复**：

| 根因 | 修复 |
|------|------|
| 矩阵乘法顺序与 DirectX 原版不一致 | Shader 中改为 `mul(float4(vtx.position, 1.0), inst.World)`（行向量 × 矩阵），与 DirectX sample 一致；C++ 侧存储 `math::transpose(world)` |
| 按 instance 分批时 InstanceOffset > 0 导致参数/索引错误 | 改为按 meshlet 分批：每批 `InstanceOffset=0`、`InstanceCount=mInstanceCount`，只改变 `MeshletOffset`/`MeshletCount`；`groupCount = meshletCount * mInstanceCount` |
| last meshlet packing 在多实例时顶点/三角形索引错误 | 当 `InstanceCount <= 32` 时禁用 packing（`MeshletInstancingBunny.ms.slang` 第 98 行） |

**相关文件**：`D3D12MeshletInstancing.cpp`（分批逻辑）、`MeshletInstancingBunny.ms.slang`（矩阵乘法、packing 逻辑）。

## 资产说明

- **test_scenes/bunny.pyscene**：Falcor 内置场景，含 bunny_dense.obj。路径由 FALCOR_MEDIA_FOLDERS 解析。
- 无需 DirectX-Graphics-Samples 或 ToyRobot.bin。

## Build/run 验证

```bash
tools\.packman\cmake\bin\cmake.exe --build build/windows-vs2022 --config Debug --target Karma
```

运行：
```bash
# 交互模式（树形 UI 选择）
.\build\windows-vs2022\bin\Debug\Karma.exe

# 直接加载
.\build\windows-vs2022\bin\Debug\Karma.exe --sample Samples/Desktop/D3D12MeshletInstancing

# headless
.\build\windows-vs2022\bin\Debug\Karma.exe --sample Samples/Desktop/D3D12MeshletInstancing --headless
```

预期：蓝色背景 + bunny 多实例（立方体网格）、+/- 调整实例密度、Space 切换 meshlet 着色、鼠标旋转/缩放相机。
