# D3D12HelloTriangle 迁移记录

## 5.1 DX Sample → Falcor 映射表

| DX 概念 / 代码 | Falcor 对应 | 备注 |
|----------------|-------------|------|
| `LoadPipeline()` | （Falcor 抽象） | 同 HelloWindow |
| `LoadAssets()` → RootSignature | （Falcor 抽象） | 空 root signature，Slang 自动生成 |
| `LoadAssets()` → PSO (VS+PS) | `Program::create` + `GraphicsState::setProgram` | HelloTriangle.slang 替代 shaders.hlsl |
| `LoadAssets()` → Vertex buffer | `mpVertexBuffer = createBuffer(...)` | 顶点数据 1:1 |
| `LoadAssets()` → VAO | `Vao::create(TriangleList, pLayout, buffers)` | 无 index buffer |
| `D3D12_INPUT_ELEMENT_DESC` | `VertexBufferLayout::addElement` | POSITION RGB32Float, COLOR RGBA32Float |
| `DepthStencilState.DepthEnable = FALSE` | `DepthStencilState::Desc().setDepthEnabled(false)` | 无 depth |
| `DrawInstanced(3, 1, 0, 0)` | `pRenderContext->draw(mpState, mpVars, 3, 0)` | 3 顶点 |
| 顶点坐标 `0.25f * m_aspectRatio` | `aspectRatio = size.x / size.y` | onLoad 时从 window 获取 |

## 5.2 1:1 映射后运行错误及修复

| 问题 | 错误表现 | 修复 |
|------|----------|------|
| PS 未被调用 | 三角形不显示（仅蓝色背景） | 顶点为 CW  winding，默认 Back cull 剔除；添加 `RasterizerState::create(Desc().setCullMode(CullMode::None))` |
