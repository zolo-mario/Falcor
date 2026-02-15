# D3D12PipelineStateCache 迁移记录

## Scaffold 命令

```bash
python tools/make_new_sample_app.py D3D12PipelineStateCache --path Source/Samples/Desktop
```

## 5.1 DX Sample → Falcor 映射表

| DX 概念 / 代码 | Falcor 对应 | 备注 |
|----------------|-------------|------|
| `LoadPipeline()` | （Falcor 抽象） | Device、swap chain、RTV heap 由 SampleApp 管理 |
| `LoadAssets()` → RootSignature | （Falcor 抽象） | Slang 自动生成 |
| `LoadAssets()` → PSO (11 种) | `Program::create` (Cube + PostEffect) | 原始用 ID3D12PipelineLibrary 缓存，Falcor 用 Slang 编译 |
| `LoadAssets()` → Cube VB/IB | `createBuffer` + `Vao::create` | 顶点/索引 1:1 |
| `LoadAssets()` → Quad VB | `createBuffer` + `Vao::create` | TriangleStrip, 4 顶点 |
| `LoadAssets()` → PerDraw CB | `mpConstantBuffer` + `setBlob` | worldViewProjection |
| `m_intermediateRenderTarget` | `mpIntermediateFbo` | Fbo::create2D(RGBA8Unorm) |
| `ClearRenderTargetView(IntermediateClearColor)` | `pRenderContext->clearFbo(..., intermediateClearColor)` | (0, 0.2, 0.3, 1) |
| `PopulateCommandList()` → Draw cube | `pRenderContext->drawIndexed(..., 36, 0, 0)` | 36 索引 |
| `PopulateCommandList()` → 3x3 后处理 quads | 循环 9 个 effect，每格 `setViewport` + `draw(4, 0)` | 每个 effect 独立 viewport |
| `PSOLibrary` / `ID3D12PipelineLibrary` | （Falcor 抽象） | Falcor/Slang 内部处理 shader 编译与缓存 |
| `Uber Shader` 回退 | 单一 `PostEffect.slang` 用 `effectIndex` switch | 9 种 effect 合并为一个 PSO |
| `SimpleCamera` | `math::matrixFromLookAt` 固定视角 | 简化：固定 (0,0,5) 看原点 |
| 键盘 1-9 切换 effect | `onKeyEvent` + `toggleEffect` | 同原样 |
| 键盘 C/U/L/M (PSO cache) | GUI 说明 | Falcor 内部处理，无对应 API |

## 保留的常量

| 常量 | 原始值 | Falcor 中 |
|------|--------|-----------|
| IntermediateClearColor | `{ 0.0f, 0.2f, 0.3f, 1.0f }` | `kIntermediateClearColor` |
| 3x3 grid | quadsX=3, quadsY=3 | `kQuadsX`, `kQuadsY` |
| 立方体顶点/索引 | 8 顶点 + 36 索引 | 1:1 |
| Quad 顶点 | 4 顶点 (-1,-1)..(1,1), uv (0,1)..(1,0) | 1:1 |
| Effect 索引 | PostBlit=2 .. PostWave=10 | 同左 |
| 灰度权重 | (0.21, 0.72, 0.07) | 1:1 |
| Blur weights/offsets | 15 采样 | 1:1 |
| EdgeDetect threshold/samplePoints | 0.015, 0.001, 9 点 Sobel | 1:1 |

## 5.2 1:1 映射后运行错误及修复

| 问题 | 错误表现 | 修复 |
|------|----------|------|
| FBO 作为纹理采样 | 需 ShaderResource | Fbo::create2D 默认带 ShaderResource |
| 后处理纹理绑定 | 需 Texture 非 RTV | 使用 `getColorTexture(0)` 非 `getRenderTargetView(0)` |
| 3x3 viewport 布局 | 每格需独立 viewport | `mpEffectState->setViewport(0, Viewport(...))` 每 draw 前 |

## 迁移说明：PSO 缓存

原始 D3D12 sample 演示：
- **ID3D12PipelineLibrary**：将编译好的 PSO 存盘，下次加载复用
- **Cached Blobs**：`GetCachedBlob` / `CachedPSO` 作为备选
- **Uber Shader**：专用 PSO 未就绪时用动态分支的 fallback

Falcor 中：
- Slang 负责 shader 编译与缓存，无直接暴露 D3D12 Pipeline Library
- 本迁移保留**视觉效果**（旋转立方体 + 9 种后处理），PSO 缓存由 Falcor 内部处理
- 键盘 C/U/L/M 功能在 GUI 中说明为 N/A

## Build/run 验证

```bash
cmake --build build/windows-vs2022 --config Debug --target D3D12PipelineStateCache
.\build\windows-vs2022\bin\Debug\D3D12PipelineStateCache.exe
```

预期：旋转彩色立方体渲染到中间 RT，再以 3x3 网格显示 9 种后处理效果（Blit、Invert、GrayScale、EdgeDetect、Blur、Warp、Pixelate、Distort、Wave）。按 1-9 可切换各 effect 显示。
