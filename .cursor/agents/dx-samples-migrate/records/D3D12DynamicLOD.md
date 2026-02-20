# D3D12 DynamicLOD 迁移记录

## 概述

DynamicLOD 是 D3D12MeshShaders 系列中最复杂的 sample，使用 **Amplification Shader (AS) + Mesh Shader (MS) + Pixel Shader (PS)** 实现：
- 每实例视锥剔除
- GPU 端动态 LOD 选择
- 多 LOD meshlet 渲染（Dragon 模型 6 档 LOD）

## Scaffold 命令

```bash
python tools/make_new_sample_app.py D3D12DynamicLOD --path Source/Samples/Desktop
```

## 5.1 DX Sample → Falcor 映射表

| DX 概念 / 代码 | Falcor 对应 | 备注 |
|----------------|-------------|------|
| `LoadPipeline()` | （Falcor 抽象） | Device、SwapChain、RTV/DSV 由 SampleApp 管理 |
| `LoadAssets()` → Mesh PSO (AS+MS+PS) | `ProgramDesc().amplificationEntry().meshEntry().psEntry()` | 需 Shader Model 6.5 |
| `LoadAssets()` → Model 加载 | `Model::LoadFromFile()` + `UploadGpuResources()` | MSHL 二进制格式，需 port Model.cpp |
| `LoadAssets()` → Descriptor Table | `ProgramVars::getRootVar()` 绑定 | MeshInfo[8], Vertices[8], Meshlets[8] 等数组 |
| `OnUpdate()` → Constants | `onFrameRender` 前 `mpConstantBuffer->setBlob()` | View, ViewProj, Planes, ViewPosition, RecipTanHalfFovy |
| `PopulateCommandList()` → DispatchMesh | `pRenderContext->drawMeshTasks()` | 调度 AS threadgroups，AS 内部 DispatchMesh |
| `ClearRenderTargetView` | `pRenderContext->clearFbo()` | clearColor (0.0f, 0.2f, 0.4f, 1.0f) |
| `SimpleCamera` | 自实现或 Falcor Camera | WASD 平移、方向键旋转 |
| `RegenerateInstances()` | 实例缓冲 + 立方体网格布局 | + / - 键调整 instance level |

## 依赖与前置条件

### 1. 资源文件

- **Dragon_LOD0.bin ~ Dragon_LOD5.bin**：MSHL 格式 meshlet 模型
- 来源：使用 DirectX-Graphics-Samples 的 **WavefrontConverter** 工具从 OBJ 生成
- 路径：`<DX_SAMPLES>/Samples/Desktop/D3D12MeshShaders/Assets/` 或 Falcor 项目内 `Source/Samples/Desktop/D3D12DynamicLOD/Assets/`

### 2. 需移植的共享代码

| 文件 | 说明 |
|------|------|
| `Span.h` | 已添加，轻量 array view |
| `Shared.h` | 已添加，Constants/DrawParams/Instance/MeshInfo |
| `Model.h` / `Model.cpp` | MSHL 二进制解析 + Falcor Buffer 创建/上传 |
| `SimpleCamera.h` / `SimpleCamera.cpp` | 键盘控制相机，或改用 Falcor FirstPersonCamera |

### 3. Shader 转换 (HLSL → Slang)

| 原始文件 | 作用 |
|----------|------|
| `MeshletAS.hlsl` | Amplification Shader：视锥剔除、LOD 计算、Wave 内建、DispatchMesh |
| `MeshletMS.hlsl` | Mesh Shader：meshlet 顶点/图元输出 |
| `MeshletPS.hlsl` | Pixel Shader：Phong 着色、三种可视化模式 |
| `Common.hlsli` | ROOT_SIG、Payload、MeshInfo、Vertex、Meshlet、IsVisible、ComputeLOD |

**Root Signature 映射**（原始 HLSL）：
```
CBV(b0) Constants
RootConstants(b1, 2) DrawParams
DescriptorTable(CBV(b2,8), SRV(t0,32))  // MeshInfo + Vertices/Meshlets/UniqueVertexIndices/PrimitiveIndices
SRV(t32) Instances
```

Falcor/Slang 使用 parameter block，需用 `var["MeshInfo"]`、`var["Vertices"]` 等绑定。

### 4. 技术要点

- **Wave 内建**：`WavePrefixCountBits`、`WaveActiveCountBits`、`WavePrefixSum`、`WaveReadLaneFirst`、`WaveGetLaneCount` — Slang 支持
- **AS 组大小**：`AS_GROUP_SIZE = 32`（单 wave），避免 wave 间同步
- **MS 组大小**：`MS_GROUP_SIZE = 128`（ROUNDUP(MAX(64,126), 32)）
- **Dispatch 限制**：单次 DispatchMesh 最多 65536 个 AS groups，多批次时需循环并更新 DrawParams

## 5.2 1:1 映射后运行错误及修复

| 问题 | 错误表现 | 修复 |
|------|----------|------|
| （待实现） | - | 当前为 scaffold 状态，需完成 Model、Shader、相机、实例缓冲等移植 |

## 保留的常量

| 常量 | 原始值 | Falcor 中 |
|------|--------|-----------|
| clearColor | `{ 0.0f, 0.2f, 0.4f, 1.0f }` | `float4(0.0f, 0.2f, 0.4f, 1.0f)` |
| c_fovy | `XM_PI / 3.0f` | `M_PI / 3.0f` |
| window title | `L"D3D12 Dynamic LOD"` | `"D3D12 Dynamic LOD"` |
| 相机初始 | `{ 0, 75, 150 }` | 同左 |
| 相机速度 | `150.0f` | 同左 |

## 控件（与原始一致）

| 键 | 功能 |
|----|------|
| WASD | 平移相机 |
| 方向键 | 旋转相机 |
| Space | 切换可视化模式（Flat / Meshlets / LOD） |
| + | 增加实例层级 |
| - | 减少实例层级 |

## Build/Run 验证

```bash
cmake --build build/windows-vs2022 --config Debug --target Karma
.\build\windows-vs2022\bin\Debug\Karma.exe  # 在树中选择 Samples/Desktop/D3D12DynamicLOD
```

**当前状态**：Scaffold 已创建，窗口标题与 clear color 已对齐。完整 mesh shader 管线与资源加载待实现。
