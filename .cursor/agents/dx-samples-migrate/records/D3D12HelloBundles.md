# D3D12HelloBundles 迁移记录

## Scaffold 命令

```bash
python tools/make_new_sample_app.py D3D12HelloBundles --path Source/Samples/Desktop
```

## 5.1 DX Sample → Falcor 映射表

| DX 概念 / 代码 | Falcor 对应 | 备注 |
|----------------|-------------|------|
| `LoadPipeline()` | （Falcor 抽象） | 同 HelloWindow |
| `LoadAssets()` → RootSignature | （Falcor 抽象） | 空 root signature，Slang 自动生成 |
| `LoadAssets()` → PSO (VS+PS) | `Program::create` + `GraphicsState::setProgram` | HelloBundles.slang 替代 shaders.hlsl，与 HelloTriangle 相同 |
| `LoadAssets()` → Vertex buffer | `mpVertexBuffer = createBuffer(...)` | 顶点数据 1:1 |
| `LoadAssets()` → VAO | `Vao::create(TriangleList, pLayout, buffers)` | 无 index buffer |
| `D3D12_INPUT_ELEMENT_DESC` | `VertexBufferLayout::addElement` | POSITION RGB32Float, COLOR RGBA32Float |
| `CreateCommandAllocator(BUNDLE)` + `CreateCommandList(BUNDLE)` | （Falcor 抽象） | Falcor 不暴露 D3D12 bundle API |
| Bundle 预录制: SetRootSignature, IASet*, DrawInstanced(3) | 直接 `pRenderContext->draw(3, 0)` | 语义等价，相同视觉输出 |
| `PopulateCommandList()` → ExecuteBundle(m_bundle) | `pRenderContext->draw(mpState, mpVars, 3, 0)` | 单次 draw 等价于执行 bundle |
| `DepthStencilState.DepthEnable = FALSE` | `DepthStencilState::Desc().setDepthEnabled(false)` | 无 depth |
| 顶点坐标 `0.25f * m_aspectRatio` | `aspectRatio = size.x / size.y` | onLoad 时从 window 获取 |

## 保留的常量

| 常量 | 原始值 | Falcor 中 |
|------|--------|-----------|
| clearColor | `{ 0.0f, 0.2f, 0.4f, 1.0f }` | `float4(0.0f, 0.2f, 0.4f, 1.0f)` |
| 顶点 0 (top) | `(0, 0.25*ar, 0)`, red | 同左 |
| 顶点 1 (right) | `(0.25, -0.25*ar, 0)`, green | 同左 |
| 顶点 2 (left) | `(-0.25, -0.25*ar, 0)`, blue | 同左 |
| window title | `L"D3D12 Hello Bundles"` | `"D3D12 Hello Bundles"` |

## 5.2 1:1 映射后运行错误及修复

| 问题 | 错误表现 | 修复 |
|------|----------|------|
| （无） | - | 与 HelloTriangle 相同，需 CullMode::None；bundle 映射为直接 draw 无额外问题 |

## Bundle 映射说明

D3D12 Hello Bundles 的核心是演示 **Command Bundle**：将 Draw 命令预录制到 bundle，每帧通过 `ExecuteBundle` 执行。Falcor SampleApp 不暴露 D3D12 bundle 接口，迁移时：

- **语义等价**：`ExecuteBundle(bundle)` 其中 bundle 仅含 `DrawInstanced(3,1,0,0)` → 映射为 `pRenderContext->draw(mpState, mpVars, 3, 0)`
- **视觉输出**：与 HelloTriangle 完全一致（蓝色背景 + 红绿蓝三角形）

## Build/run 验证

```bash
cmake --build build/windows-vs2022 --config Debug --target D3D12HelloBundles
.\build\windows-vs2022\bin\Debug\D3D12HelloBundles.exe
```
预期：蓝色背景 + 红绿蓝三角形（与 HelloTriangle 相同）。
