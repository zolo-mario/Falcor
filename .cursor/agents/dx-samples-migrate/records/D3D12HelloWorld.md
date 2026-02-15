# D3D12HelloWorld 迁移记录

## 5.1 DX Sample → Falcor 映射表

| DX 概念 / 代码 | Falcor 对应 | 备注 |
|----------------|-------------|------|
| `D3D12HelloWindow(width, height, name)` | `SampleApp(config)` | 窗口尺寸、标题由 config 配置 |
| `OnInit()` → LoadPipeline | (Falcor 抽象) | Device、SwapChain、CommandQueue 由 SampleApp 管理 |
| `OnInit()` → LoadAssets | (空) | HelloWindow 无 assets，仅创建 command list |
| `OnUpdate()` | (空) | 无每帧更新逻辑 |
| `OnRender()` → PopulateCommandList | `onFrameRender()` | 仅 clear back buffer |
| `ClearRenderTargetView(clearColor)` | `pRenderContext->clearFbo(..., clearColor, ...)` | clearColor = (0, 0.2, 0.4, 1) |
| `OnDestroy()` | `onShutdown()` | HelloWindow 仅 WaitForPreviousFrame，Falcor 自动处理 |
| 窗口标题 `L"D3D12 Hello Window"` | `config.windowDesc.title = "D3D12 Hello Window"` | runMain 中设置 |

## 5.2 1:1 映射后运行错误及修复

| 问题 | 错误表现 | 修复 |
|------|----------|------|
| (无) | - | D3D12HelloWindow 逻辑极简，1:1 映射后无构建/运行错误 |
