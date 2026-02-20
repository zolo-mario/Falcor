# D3D12ExecuteIndirect 迁移记录

## 与 DirectX-Graphics-Samples 原版对比

| 维度 | DirectX-Graphics-Samples 原版 | Falcor 迁移版 |
|------|-------------------------------|---------------|
| **应用框架** | 独立 Win32 exe，继承 `DXSample` | Karma 插件，继承 `SampleBase` |
| **入口** | `Main.cpp` → `Win32Application::Run` | `registerPlugin` → Karma 加载 |
| **生命周期** | `OnInit` / `OnUpdate` / `OnRender` / `OnDestroy` | `onLoad` / `onFrameRender` / `onShutdown` |
| **Command Queue** | 双队列：`D3D12_COMMAND_LIST_TYPE_DIRECT` + `D3D12_COMMAND_LIST_TYPE_COMPUTE` | 单 `RenderContext`（继承 ComputeContext） |
| **Command Signature** | `ID3D12CommandSignature`：每命令含 CBV + Draw | **无**，Falcor drawIndirect 仅支持 DrawArgs |
| **Indirect 命令格式** | `IndirectCommand{ cbv, drawArguments }`，per-draw 更新 CBV | **架构调整**：用 `firstInstance` 传递索引 → 因 GFX 未传 firstInstance 改用可见索引方案 |
| **Culling 路径** | Compute 输出 `AppendStructuredBuffer<IndirectCommand>`，ExecuteIndirect 多命令 | Compute 输出 `AppendStructuredBuffer<uint>` 可见索引 + BuildArgs 生成 (3,count,0,0) + 单次 drawIndirect |
| **Non-culling 路径** | ExecuteIndirect 1024 条命令，每命令 (3,1,0,n) | `drawInstanced(3, 1024, 0, 0)`（instanceId 直接索引 gSceneCB） |
| **Vertex 输入** | `float4 position`（CBV 提供 offset） | `float3 position` + `SV_InstanceID` 索引 StructuredBuffer |
| **投影** | `XMMatrixPerspectiveFovLH`（+Z 朝向相机） | `math::perspective`（RH，-Z 朝向），shader 中 worldPos.z = -worldPos.z |
| **Constant Buffer** | 单 buffer 1024×3 条，256B 对齐，含 padding | 每帧独立 buffer（1024 条），无 padding |
| **深度/裁剪** | 默认深度测试、背面剔除 | 禁用深度、禁用背面剔除（避免 Falcor 默认导致全剔除） |
| **Scissor** | culling 时用 `m_cullingScissorRect` 裁剪 | 无，由 compute culling 剔除 draw 数量 |

### Shader 对比

| 原版 (HLSL) | Falcor (Slang) |
|-------------|----------------|
| `shaders.hlsl`：VS 用 `cbuffer SceneConstantBuffer`（单 CBV，per-draw 由 ExecuteIndirect 更新） | `Shaders.slang`：VS 用 `StructuredBuffer<SceneConstantBuffer> gSceneCB[instanceId]` |
| `compute.hlsl`：输出 `AppendStructuredBuffer<IndirectCommand>`，复制 inputCommands[index] | `Compute.slang`：输出 `AppendStructuredBuffer<uint>` 可见索引 |
| — | `ShadersCulling.slang`：culling 路径，`gSceneCB[gVisibleIndices[instanceId]]` |
| — | `BuildArgs.slang`：从 counter 读 count，写入 (3,count,0,0) |

### 数据流对比

**原版**：
```
CommandBuffer(1024条 IndirectCommand{cbv, draw}) 
  → Compute(可见) → AppendStructuredBuffer<IndirectCommand> 
  → ExecuteIndirect(count=UAV) 每命令执行 SetGraphicsRootConstantBufferView + DrawInstanced(3,1,0,n)
```

**Falcor**：
```
Non-culling: drawInstanced(3, 1024, 0, 0) → gSceneCB[instanceId]
Culling: Compute → AppendStructuredBuffer<uint> visibleIndices
       → BuildArgs → (3, count, 0, 0)
       → drawIndirect(1, mpDrawArgsBuffer) → gSceneCB[gVisibleIndices[instanceId]]
```

---

## Scaffold 命令

```bash
python tools/make_new_sample_app.py D3D12ExecuteIndirect --path Source/Samples/Desktop
```

## 5.1 DX Sample → Falcor 映射表

| DX 概念 / 代码 | Falcor 对应 | 备注 |
|----------------|-------------|------|
| 独立 SampleApp 可执行文件 | SampleBase 插件 (add_plugin) | 由 Karma 加载，树路径 Samples/Desktop/D3D12ExecuteIndirect |
| `LoadPipeline()` | （Falcor 抽象） | Device、swap chain、command queue 由 Karma/SampleApp 管理 |
| `LoadPipeline()` 双 command queue (Direct + Compute) | 单 RenderContext | Falcor RenderContext 继承 ComputeContext，compute + graphics 同 queue |
| `LoadAssets()` → RootSignature | （Slang 自动生成） | Graphics: CBV(b0); Compute: SRV(t0,t1) + UAV(u0) + RootConstants |
| `LoadAssets()` → Command Signature (CBV + Draw) | **架构调整** | D3D12 ExecuteIndirect 支持 per-draw CBV；Falcor drawIndirect 仅支持 DrawArgs。改用 firstInstance 传 triangle index，VS 用 StructuredBuffer[instanceID] |
| `LoadAssets()` → PSO (VS+PS) | `Program::create` + `GraphicsState::setProgram` | Shaders.slang |
| `LoadAssets()` → Compute PSO | `ComputePass::create` | Compute.slang |
| `LoadAssets()` → Vertex buffer | `createBuffer` | 单三角形几何 1:1 |
| `LoadAssets()` → Constant buffer | `createStructuredBuffer` × kFrameCount | 每帧独立 buffer（1024 条/帧），避免 getSRV(offset,size) 链接错误 |
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
| SampleApp 无法作为 Karma 插件 | 原为 add_falcor_executable 独立 exe | 改为 SampleBase + add_plugin，继承 SampleBase(pHost)，添加 create()、registerPlugin() |
| CMakeLists.txt 未启用 | 构建时不含 D3D12ExecuteIndirect | 取消 Samples/CMakeLists.txt 中 add_subdirectory(Desktop/D3D12ExecuteIndirect) 注释 |
| Falcor.dll 锁定 | Build 报 LNK1168 无法打开 Falcor.dll | 关闭 Karma 或其他 Falcor 进程后再构建 |
| matrixFromPerspectiveFov 不存在 | C2039/C3861 | 改用 `math::perspective(fovy, aspect, zNear, zFar)` |
| getSRV(offset, size) 导致 LNK2019 | ParameterBlock::setVariable\<ref\<ShaderResourceView\>\> 无法解析 | 改用 per-frame buffers（mpConstantBuffers[kFrameCount]），直接绑定 Buffer 而非 SRV |
| 屏幕无输出（show nothing） | 投影错误、culling 全剔除、初始 offset 在视口外、LH/RH 坐标系、深度测试、背面剔除 | ① mul(cb.projection, pos) 与 Falcor 列主序一致，移除 transpose；② mEnableCulling=false 默认；③ offset.x 初始 -1.5~1.5；④ **D3D12 用 XMMatrixPerspectiveFovLH（+Z 朝向相机），Falcor math::perspective 为 RH（-Z 朝向相机）**，shader 中 worldPos.z = -worldPos.z；⑤ **setDepthEnabled(false)** 避免 depth clear/format 导致全剔除；⑥ **setRasterizerState(CullMode::None)** 禁用背面剔除 |
| 仅显示 1 个白色三角形 | ① C++/Compute.slang SceneConstantBuffer 含 padding，stride 不匹配；② **drawIndirect 多命令时每 draw 的 SV_InstanceID 均为 0**（GFX 未传 firstInstance） | ① 移除 C++ 与 Compute.slang 的 padding；② non-culling 路径改用 **drawInstanced(3, 1024, 0, 0)** 替代 drawIndirect，使 instanceId=0..1023 |
| Culling 模式无输出 | ComputePass 未调用 setVars(mpCullVars)，execute 用内部空 vars，compute 无绑定导致 AppendStructuredBuffer 无输出、count=0 | **mpCullPass->setVars(mpCullVars)** |
| Culling 模式仍只显示 1 个白色三角形 | 同 non-culling：drawIndirect 多命令时 SV_InstanceID 恒为 0，所有 draw 都用 gSceneCB[0] | **可见索引方案**：Compute 输出 `AppendStructuredBuffer<uint>` 可见索引；BuildArgs compute 写入 (3, count, 0, 0)；单次 drawIndirect；ShadersCulling.slang 用 `gSceneCB[gVisibleIndices[instanceId]]` |

## Build/run 验证

```bash
# 使用 packman cmake（若 cmake 不在 PATH）
tools\.packman\cmake\bin\cmake.exe --build build/windows-vs2022 --config Debug --target Karma

# 或标准 cmake
cmake --build build/windows-vs2022 --config Debug --target Karma
```

运行：
```bash
# 交互模式（树形 UI 选择）
.\build\windows-vs2022\bin\Debug\Karma.exe

# 直接加载（无需手动选择）
.\build\windows-vs2022\bin\Debug\Karma.exe --sample Samples/Desktop/D3D12ExecuteIndirect

# headless（无窗口）
.\build\windows-vs2022\bin\Debug\Karma.exe --sample D3D12ExecuteIndirect --headless
```

预期：1024 个彩色三角形水平移动，SPACE 切换 culling 开/关（culling 开时仅绘制视口中心区域内的三角形）。
