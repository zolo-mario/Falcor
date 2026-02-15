# D3D12HelloConstantBuffers 迁移记录

## Scaffold 命令

```bash
python tools/make_new_sample_app.py D3D12HelloConstantBuffers --path Source/Samples/Desktop
```

## 5.1 DX Sample → Falcor 映射表

| DX 概念 / 代码 | Falcor 对应 | 备注 |
|----------------|-------------|------|
| `LoadPipeline()` | （Falcor 抽象） | 同 HelloWindow |
| `LoadAssets()` → RootSignature (CBV descriptor table) | （Falcor 抽象） | Slang 根据 cbuffer 自动生成 |
| `LoadAssets()` → PSO (VS+PS) | `Program::create` + `GraphicsState::setProgram` | HelloConstantBuffers.slang 替代 shaders.hlsl |
| `LoadAssets()` → Vertex buffer | `mpVertexBuffer = createBuffer(...)` | 顶点数据 1:1 |
| `LoadAssets()` → Constant buffer (256-byte) | **不**创建 Buffer；cbuffer 由 ProgramVars 内部管理 | Falcor cbuffer 非 SRV/UAV，不能绑定 Buffer |
| `LoadAssets()` → CBV descriptor heap + CreateConstantBufferView | 无需显式绑定 | 每帧通过 `ShaderVar::setBlob` 上传 |
| `LoadAssets()` → VAO | `Vao::create(TriangleList, pLayout, buffers)` | 无 index buffer |
| `OnUpdate()` → offset.x += translationSpeed, wrap at ±offsetBounds | `onFrameRender` 内 `mConstantBufferData.offset.x += 0.005f`，>1.25 时置 -1.25 | 1:1 逻辑 |
| `OnUpdate()` → memcpy to mapped CB | `mpVars->getRootVar()["SceneConstantBuffer"].setBlob(&mConstantBufferData, sizeof(...))` | 每帧通过 ShaderVar 上传 |
| `PopulateCommandList()` → SetGraphicsRootDescriptorTable(CBV) | 由 mpVars 绑定，draw 时自动应用 | Falcor 管理 |
| `DepthStencilState.DepthEnable = FALSE` | `DepthStencilState::Desc().setDepthEnabled(false)` | 无 depth |
| 顶点坐标 `0.25f * m_aspectRatio` | `aspectRatio = size.x / size.y` | onLoad 时从 window 获取 |

## 保留的常量

| 常量 | 原始值 | Falcor 中 |
|------|--------|-----------|
| clearColor | `{ 0.0f, 0.2f, 0.4f, 1.0f }` | `float4(0.0f, 0.2f, 0.4f, 1.0f)` |
| translationSpeed | `0.005f` | `0.005f` |
| offsetBounds | `1.25f` | `1.25f` |
| 顶点 0 (top) | `(0, 0.25*ar, 0)`, red | 同左 |
| 顶点 1 (right) | `(0.25, -0.25*ar, 0)`, green | 同左 |
| 顶点 2 (left) | `(-0.25, -0.25*ar, 0)`, blue | 同左 |
| SceneConstantBuffer | `float4 offset; float4 padding[15]` (256 bytes) | 同左 |
| window title | `L"D3D12 Hello Const Buffers"` (DX 项目名) | `"D3D12 Hello Constant Buffers"` |

## 5.2 1:1 映射后运行错误及修复

| 问题 | 错误表现 | 修复 |
|------|----------|------|
| cbuffer 绑定方式错误 | `Error trying to bind buffer to a non SRV/UAV variable`（ParameterBlock.cpp:479） | Falcor 的 cbuffer 不能绑定 `Buffer`；改用 `mpVars->getRootVar()["SceneConstantBuffer"].setBlob(&data, sizeof(data))` 每帧上传 |
| CullMode | 三角形不显示（仅蓝色背景） | `RasterizerState::Desc().setCullMode(CullMode::None)`（与 HelloTriangle 相同） |

## Build/run 验证

```bash
cmake --build build/windows-vs2022 --config Debug --target D3D12HelloConstantBuffers
.\build\windows-vs2022\bin\Debug\D3D12HelloConstantBuffers.exe
```

预期：蓝色背景 + 红绿蓝三角形水平往复移动（offset.x 动画）。
