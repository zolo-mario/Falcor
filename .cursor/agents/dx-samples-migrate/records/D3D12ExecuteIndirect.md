# D3D12ExecuteIndirect 迁移记录

## Scaffold 命令

```bash
python tools/make_new_sample_app.py D3D12ExecuteIndirect --path Source/Samples/Desktop
```

## 5.1 DX Sample → Falcor 映射表

| DX 概念 / 代码 | Falcor 对应 | 备注 |
|----------------|-------------|------|
| `LoadPipeline()` | （Falcor 抽象） | Device、swap chain、command queue 由 SampleApp 管理 |
| `LoadPipeline()` 双 command queue (Direct + Compute) | 单 RenderContext | Falcor RenderContext 继承 ComputeContext，compute + graphics 同 queue |
| `LoadAssets()` → RootSignature | （Slang 自动生成） | Graphics: CBV(b0); Compute: SRV(t0,t1) + UAV(u0) + RootConstants |
| `LoadAssets()` → Command Signature (CBV + Draw) | **架构调整** | D3D12 ExecuteIndirect 支持 per-draw CBV；Falcor drawIndirect 仅支持 DrawArgs。改用 firstInstance 传 triangle index，VS 用 StructuredBuffer[instanceID] |
| `LoadAssets()` → PSO (VS+PS) | `Program::create` + `GraphicsState::setProgram` | Shaders.slang |
| `LoadAssets()` → Compute PSO | `ComputePass::create` | Compute.slang |
| `LoadAssets()` → Vertex buffer | `createBuffer` | 单三角形几何 1:1 |
| `LoadAssets()` → Constant buffer | `createStructuredBuffer` | 1024×3 条，256B 对齐 |
| `LoadAssets()` → Command buffer | `createStructuredBuffer` | IndirectArg，存 (3,1,0,n) |
| `LoadAssets()` → Processed command buffer | `createStructuredBuffer(..., createCounter=true)` | AppendStructuredBuffer 输出，UAV counter 作 drawIndirect count |
| `OnUpdate()` 动画 | `onFrameRender` 内联 | offset.x += velocity.x，越界重置 |
| `PopulateCommandLists()` Compute | `mpCullPass->execute` | clearUAVCounter → dispatch |
| `ExecuteIndirect` (culling on) | `drawIndirect(..., pCountBuffer=getUAVCounter())` | 使用 count buffer |
| `ExecuteIndirect` (culling off) | `drawIndirect(..., pCountBuffer=nullptr)` | maxCount=1024 |
| `OnKeyDown(VK_SPACE)` | `onKeyEvent(Key::Space)` | 切换 mEnableCulling |
| `ClearRenderTargetView` | `clearFbo` | clearColor (0, 0.2, 0.4, 1) |

## 保留的常量

| 常量 | 原始值 | Falcor 中 |
|------|--------|-----------|
| clearColor | `{ 0.0f, 0.2f, 0.4f, 1.0f }` | `float4(0.0f, 0.2f, 0.4f, 1.0f)` |
| TriangleCount | 1024 | kTriangleCount |
| TriangleHalfWidth | 0.05f | kTriangleHalfWidth |
| TriangleDepth | 1.0f | kTriangleDepth |
| CullingCutoff | 0.5f | kCullingCutoff |
| ComputeThreadBlockSize | 128 | kComputeThreadBlockSize |
| offsetBounds | 2.5f | 同左 |
| FOV | π/4 | math::radians(45.f) |
| window title | `L"D3D12 Execute Indirect"` | `"D3D12 Execute Indirect"` |

## 架构差异说明

D3D12 原版使用 **Command Signature**：每个 indirect command 含 `D3D12_GPU_VIRTUAL_ADDRESS cbv` + `D3D12_DRAW_ARGUMENTS`，即 per-draw 的 CBV 更新。Falcor/Vulkan 的 drawIndirect 仅支持标准 DrawIndirectCommand (vertexCount, instanceCount, firstVertex, firstInstance)，无 per-draw descriptor。

**迁移方案**：用 `firstInstance` 传递三角形索引，VS 通过 `SV_InstanceID`（即 firstInstance）索引 `StructuredBuffer<SceneConstantBuffer>`，实现等价语义。

## 5.2 1:1 映射后运行错误及修复

| 问题 | 错误表现 | 修复 |
|------|----------|------|
| （待验证） | 构建环境 CrayClang 导致 CMake 失败 | 非本 sample 代码问题，需修复构建环境 |

## Build/run 验证

```bash
cmake --build build/windows-vs2022 --config Debug --target Karma
.\build\windows-vs2022\bin\Debug\Karma.exe  # 在树中选择 Samples/Desktop/D3D12ExecuteIndirect
```

预期：1024 个彩色三角形水平移动，SPACE 切换 culling 开/关（culling 开时仅绘制视口中心区域内的三角形）。
