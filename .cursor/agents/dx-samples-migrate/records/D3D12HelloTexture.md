# D3D12HelloTexture 迁移记录

## Scaffold 命令

```bash
python tools/make_new_sample_app.py D3D12HelloTexture --path Source/Samples/Desktop
```

## 5.1 DX Sample → Falcor 映射表

| DX 概念 / 代码 | Falcor 对应 | 备注 |
|----------------|-------------|------|
| `LoadPipeline()` | （Falcor 抽象） | 同 HelloWindow |
| `LoadAssets()` → RootSignature | （Falcor 抽象） | SRV + Sampler，Slang 自动生成 |
| `LoadAssets()` → PSO (VS+PS) | `Program::create` + `GraphicsState::setProgram` | HelloTexture.slang 替代 shaders.hlsl |
| `LoadAssets()` → Vertex buffer | `mpVertexBuffer = createBuffer(...)` | 顶点数据 1:1，POSITION + TEXCOORD |
| `LoadAssets()` → VAO | `Vao::create(TriangleList, pLayout, buffers)` | 无 index buffer |
| `D3D12_INPUT_ELEMENT_DESC` | `VertexBufferLayout::addElement` | POSITION RGB32Float, TEXCOORD RG32Float |
| `LoadAssets()` → Texture 256×256 | `createTexture2D(..., textureData.data(), ...)` | RGBA8Unorm，checkerboard 数据 |
| `GenerateTextureData()` | `generateTextureData()` 静态函数 | 1:1 黑白棋盘格 |
| `LoadAssets()` → SRV heap + CreateSRV | `mpTexture` + `var["g_texture"] = mpTexture` | Falcor 自动管理 descriptor |
| `D3D12_STATIC_SAMPLER_DESC` (Point, Border) | `Sampler::Desc().setFilterMode(Point).setAddressingMode(Border)` | `mpSampler` + `var["g_sampler"] = mpSampler` |
| `SetDescriptorHeaps` + `SetGraphicsRootDescriptorTable` | `mpVars->getRootVar()["g_texture"] = ...` | 需通过 getRootVar() 获取 ShaderVar |
| `DepthStencilState.DepthEnable = FALSE` | `DepthStencilState::Desc().setDepthEnabled(false)` | 无 depth |
| `DrawInstanced(3, 1, 0, 0)` | `pRenderContext->draw(mpState, mpVars, 3, 0)` | 3 顶点 |
| 顶点坐标 `0.25f * m_aspectRatio` | `aspectRatio = size.x / size.y` | onLoad 时从 window 获取 |

## 保留的常量

| 常量 | 原始值 | Falcor 中 |
|------|--------|-----------|
| clearColor | `{ 0.0f, 0.2f, 0.4f, 1.0f }` | `float4(0.0f, 0.2f, 0.4f, 1.0f)` |
| TextureWidth/Height | 256 | `kTextureWidth`, `kTextureHeight` |
| TexturePixelSize | 4 | `kTexturePixelSize` |
| 顶点 0 (top) | `(0, 0.25*ar, 0)`, uv `(0.5, 0)` | 同左 |
| 顶点 1 (right) | `(0.25, -0.25*ar, 0)`, uv `(1, 1)` | 同左 |
| 顶点 2 (left) | `(-0.25, -0.25*ar, 0)`, uv `(0, 1)` | 同左 |
| window title | `L"D3D12 Hello Texture"` | `"D3D12 Hello Texture"` |

## 5.2 1:1 映射后运行错误及修复

| 问题 | 错误表现 | 修复 |
|------|----------|------|
| l-value specifies const object | `mpVars["g_texture"] = mpTexture` 编译错误 | 使用 `auto var = mpVars->getRootVar(); var["g_texture"] = mpTexture;` |

## Build/run 验证

```bash
cmake --build build/windows-vs2022 --config Debug --target Karma
.\build\windows-vs2022\bin\Debug\Karma.exe  # 在树中选择 Samples/Desktop/D3D12HelloTexture
```
预期：蓝色背景 + 带黑白棋盘格纹理的三角形。
