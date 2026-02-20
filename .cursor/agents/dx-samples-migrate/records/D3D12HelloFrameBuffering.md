# D3D12HelloFrameBuffering 迁移记录

## FrameBuffering 概念与 Falcor 实现

### D3D12 Frame Buffering 概念

**Frame Buffering**（帧缓冲）是 D3D12 中保证 CPU/GPU 并行、避免资源竞争的核心机制：

1. **多 back buffer**：Swap chain 有多个 back buffer（如 FrameCount=2），GPU 在渲染一帧时，CPU 可准备下一帧。
2. **每帧 Command Allocator**：`ID3D12CommandAllocator` 只能在其关联的 command list 在 GPU 上执行完毕后才能 `Reset`。因此每帧需要独立的 allocator（`m_commandAllocators[FrameCount]`）。
3. **Fence 同步**：用 `ID3D12Fence` 跟踪 GPU 完成进度。`MoveToNextFrame()` 在切换 frame index 前，若下一帧的 back buffer 仍被 GPU 使用，则 `WaitForSingleObjectEx` 等待 fence。
4. **流程**：`PopulateCommandList` → `ExecuteCommandLists` → `Present` → `MoveToNextFrame`（Signal fence、更新 frameIndex、必要时等待）。

### Falcor 的实现方式

Falcor 将上述逻辑**完全抽象**，应用层无需手动管理：

| D3D12 概念 | Falcor 对应 |
|------------|-------------|
| 多 back buffer + 每帧 RTV | `Swapchain` 管理 `imageCount` 张 swap chain 图像；`SampleApp` 用 `mpTargetFBO` 作为渲染目标，再 `copyResource` 到 `acquireNextImage()` 得到的 swap chain 图像 |
| 每帧 Command Allocator | `Device` 使用 `kInFlightFrameCount = 3` 个 `ITransientResourceHeap`，每帧轮换 |
| Fence 同步 | `Device::mpFrameFence` + `Device::endFrame()`：`submit` 后若已排队帧数 > kInFlightFrameCount 则 `mpFrameFence->wait()`，再 `synchronizeAndReset` transient heap、`signal` fence |
| MoveToNextFrame / WaitForGpu | `Device::endFrame()` 统一处理；`SampleApp` 主循环在 `onFrameRender` 后调用 `mpSwapchain->present()` 和 `mpDevice->endFrame()` |

**关键代码位置**：
- `Device::endFrame()`（`Source/Falcor/Core/API/Device.cpp`）：提交、等待超量帧、切换 transient heap、signal fence
- `SampleApp` 主循环（`Source/Falcor/Core/SampleApp.cpp`）：`onFrameRender` → `acquireNextImage` → `copyResource` → `submit` → `present` → `endFrame`

迁移时只需实现 `onFrameRender` 的渲染逻辑，frame buffering 由 Falcor 自动处理。

---

## Scaffold 命令

```bash
python tools/make_new_sample_app.py D3D12HelloFrameBuffering --path Source/Samples/Desktop
```

## 5.1 DX Sample → Falcor 映射表

| DX 概念 / 代码 | Falcor 对应 | 备注 |
|----------------|-------------|------|
| `LoadPipeline()` | （Falcor 抽象） | Device、swap chain、RTV heap、frame resources 均由 Falcor 管理 |
| `FrameCount = 2`（多帧缓冲） | （Falcor 抽象） | Falcor 内部处理 frame buffering、fences、command allocators |
| `m_commandAllocators[FrameCount]` | （Falcor 抽象） | 每帧 command allocator 由 Falcor 管理 |
| `m_fence` / `m_fenceValues` / `MoveToNextFrame()` / `WaitForGpu()` | （Falcor 抽象） | GPU 同步由 Falcor 内部处理 |
| `LoadAssets()` → RootSignature | （Falcor 抽象） | 空 root signature，Slang 自动生成 |
| `LoadAssets()` → PSO (VS+PS) | `Program::create` + `GraphicsState::setProgram` | HelloFrameBuffering.slang 替代 shaders.hlsl |
| `LoadAssets()` → Vertex buffer | `mpVertexBuffer = createBuffer(...)` | 顶点数据 1:1（与 HelloTriangle 相同） |
| `LoadAssets()` → VAO | `Vao::create(TriangleList, pLayout, buffers)` | 无 index buffer |
| `PopulateCommandList()` | 内联到 `onFrameRender` | clear + draw |
| `ClearRenderTargetView(clearColor)` | `pRenderContext->clearFbo(..., clearColor, ...)` | clearColor 一致 |
| `DrawInstanced(3, 1, 0, 0)` | `pRenderContext->draw(mpState, mpVars, 3, 0)` | 3 顶点 |
| 窗口标题 `L"D3D12 Hello Frame Buffering"` | `config.windowDesc.title = "D3D12 Hello Frame Buffering"` | 在 runMain 中设置 |

## 保留的常量

| 常量 | 原始值 | Falcor 中 |
|------|--------|-----------|
| clearColor | `{ 0.0f, 0.2f, 0.4f, 1.0f }` | `float4(0.0f, 0.2f, 0.4f, 1.0f)` |
| 顶点 0 (top) | `(0, 0.25*ar, 0)`, red | 同左 |
| 顶点 1 (right) | `(0.25, -0.25*ar, 0)`, green | 同左 |
| 顶点 2 (left) | `(-0.25, -0.25*ar, 0)`, blue | 同左 |
| window title | `L"D3D12 Hello Frame Buffering"` | `"D3D12 Hello Frame Buffering"` |

## 迁移说明

HelloFrameBuffering 与 HelloTriangle 的**视觉输出完全相同**（蓝色背景 + 红绿蓝三角形）。原始 sample 的核心价值是展示**多帧缓冲**（FrameCount=2、每帧 command allocator、fence 同步、MoveToNextFrame/WaitForGpu）。Falcor 已将上述逻辑抽象，因此迁移后仅需实现与 HelloTriangle 相同的渲染逻辑，frame buffering 由 Falcor 自动处理。

## 5.2 1:1 映射后运行错误及修复

| 问题 | 错误表现 | 修复 |
|------|----------|------|
| PS 未被调用 | 三角形不显示（仅蓝色背景） | 同 HelloTriangle：顶点为 CW winding，添加 `RasterizerState::create(Desc().setCullMode(CullMode::None))` |

## Build/run 验证

```bash
.\build_vs2022.ps1 --target Karma
.\build\windows-vs2022\bin\Debug\Karma.exe  # 在树中选择 Samples/Desktop/D3D12HelloFrameBuffering
```

预期：蓝色窗口 + 红绿蓝三角形，标题 "D3D12 Hello Frame Buffering"。
