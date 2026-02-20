# MeshletRender 迁移记录

## 与 DirectX-Graphics-Samples 原版对比（Falcor Bunny 版）

本迁移使用 **Falcor bunny 场景**、**SceneMeshletData** 和 **Falcor 相机系统**，替代原版 Dragon.bin 与自定义 SimpleCamera。

| 维度 | DirectX-Graphics-Samples 原版 | Falcor 迁移版（Bunny） |
|------|-------------------------------|------------------------|
| **应用框架** | 独立 Win32 exe，继承 `DXSample` | Karma 插件，继承 `SampleBase` |
| **模型** | Dragon_LOD0.bin（MeshletModel） | test_scenes/bunny.pyscene（Scene + SceneMeshletData） |
| **Meshlet 数据** | MeshletModel::loadFromFile + uploadGpuResources | Scene::getMeshletData() → SceneMeshletData |
| **Meshlet 格式** | GpuMeshlet (VertCount, VertOffset, PrimCount, PrimOffset) | GpuMeshletDesc (vertexOffset, triangleOffset, vertexCount, triangleCount, instanceID, ...) |
| **顶点数据** | StructuredBuffer\<Vertex\> (Position, Normal) | gScene.getVertex() → StaticVertexData |
| **三角形索引** | PackedTriangle (10+10+10 bit) | gMeshletTriangles (uint32 × 3) |
| **相机** | 自定义 SimpleCamera (WASD + 方向键) | Scene 内建 Camera + OrbiterCameraController |
| **着色器** | MeshletRender.slang | MeshletRenderBunny.ms.slang |

## Scaffold 命令

```bash
python tools/make_new_sample_app.py MeshletRender --path Source/Samples/Desktop
```

## DX Sample → Falcor 映射表

| DX 概念 / 代码 | Falcor 对应 | 备注 |
|----------------|-------------|------|
| 独立 SampleApp 可执行文件 | SampleBase 插件 (add_plugin) | 由 Karma 加载，树路径 Samples/Desktop/MeshletRender |
| `LoadPipeline()` | （Falcor 抽象） | Device、swap chain 由 Karma 管理 |
| `LoadAssets()` → Model | `SceneBuilder` + `Scene::getMeshletData` | 加载 test_scenes/bunny.pyscene |
| `LoadAssets()` → Mesh Shader PSO | `Program::create` + Scene 集成 | addShaderModules, getSceneDefines, getTypeConformances |
| `LoadAssets()` → RootSignature | （Slang 自动生成） | CB, gMeshlets, gMeshletVertices, gMeshletTriangles, gScene |
| `OnUpdate()` → 相机 | `mpScene->update()` + `setCameraController(Orbiter)` | 转发 onKeyEvent/onMouseEvent 到 Scene |
| `PopulateCommandList()` → DispatchMesh | `drawMeshTasks(mMeshletCount, 1, 1)` | 单次 dispatch，SceneMeshletData 已展开 |
| `ClearRenderTargetView` | `pRenderContext->clearFbo` | clearColor (0, 0.2, 0.4, 1) |
| `OMSetRenderTargets` | `mpGraphicsState->setFbo(mpFbo)` | 带 depth 的 FBO |

## 1:1 映射后运行错误及修复

| 问题 | 错误表现 | 修复 |
|------|----------|------|
| setCameraControllerType 不存在 | C2039 | 使用 `mpScene->setCameraController(Scene::CameraControllerType::Orbiter)` |
| Dragon_LOD0.bin 依赖 | 原版需外部资产 | 改用 Falcor bunny.pyscene，无需 Dragon |

## 资产说明

- **test_scenes/bunny.pyscene**：Falcor 内置场景，含 bunny_dense.obj。路径由 FALCOR_MEDIA_FOLDERS 解析。
- 无需 DirectX-Graphics-Samples 或 WavefrontConverter。

## Build/run 验证

```bash
tools\.packman\cmake\bin\cmake.exe --build build/windows-vs2022 --config Debug --target Karma
```

运行：
```bash
# 交互模式（树形 UI 选择）
.\build\windows-vs2022\bin\Debug\Karma.exe

# 直接加载
.\build\windows-vs2022\bin\Debug\Karma.exe --sample Samples/Desktop/MeshletRender

# headless
.\build\windows-vs2022\bin\Debug\Karma.exe --sample Samples/Desktop/MeshletRender --headless
```

预期：蓝色背景 + bunny 模型（meshlet 着色模式，按 meshlet 索引着色），鼠标旋转/缩放相机。
