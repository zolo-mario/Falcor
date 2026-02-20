# D3D12HDR 迁移记录

## 与 DirectX-Graphics-Samples 原版对比

| 维度 | DirectX-Graphics-Samples 原版 | Falcor 迁移版 |
|------|-------------------------------|---------------|
| **应用框架** | 独立 Win32 exe，继承 `DXSample` | Karma 插件，继承 `SampleBase` |
| **Swap chain 格式** | 8/10/16 bit 运行时切换 (UP/DOWN) | **不实现**，Karma 固定格式 |
| **Color space** | `SetColorSpace1` sRGB/ST.2084/Linear | ImGui 选择 displayCurve，Present PS 做 tone mapping |
| **Display HDR 检测** | `IDXGIOutput6::GetDesc1` | **不实现** |
| **全屏** | SPACE/ALT+ENTER | **不实现**（D3D12Fullscreen 已放弃） |
| **UI** | Direct2D (D3D11On12) | ImGui |
| **渲染管线** | 中间 RT (R16G16B16A16) → Present | 1:1 保留 |

## Scaffold 命令

```bash
python tools/make_new_sample_app.py D3D12HDR --path Source/Samples/Desktop
```

## DX Sample → Falcor 映射表

| DX 概念 / 代码 | Falcor 对应 | 备注 |
|----------------|-------------|------|
| `LoadPipeline()` | （Falcor 抽象） | Device、swap chain 由 Karma 管理 |
| `LoadAssets()` | `onLoad()` | 创建 FBO、VAO、Program |
| `m_intermediateRenderTarget` (R16G16B16A16) | `mpIntermediateFbo` | `Fbo::create2D(..., RGBA16Float, Unknown)` |
| `m_UIRenderTarget` | 移除 | ImGui 由框架叠加 |
| Gradient PSO | `mpGradientProgram` | Gradient.slang |
| Palette PSO | `mpPalette709Program` + `mpPalette2020Program` | Palette709.slang / Palette2020.slang（无 cbuffer） |
| Present PSO ×3 (8/10/16 bit) | 单 `mpPresentProgram` | displayCurve 由 RootConstants 控制 |
| `EnsureSwapChainColorSpace` | 移除 | ImGui 选择 displayCurve |
| `CheckDisplayHDRSupport` | 移除 | |
| `UpdateSwapChainBuffer` | 移除 | |
| `OnKeyDown` VK_PRIOR/VK_NEXT | ImGui dropdown | |
| `OnKeyDown` VK_SPACE | 移除 | |
| UILayer (Direct2D) | `onGuiRender()` ImGui | |

## 1:1 映射后运行错误及修复

| 问题 | 错误表现 | 修复 |
|------|----------|------|
| DISPLAY_CURVE_SRGB 未定义 | Present.slang 链接错误 | 在 Present.slang 内定义 #define，import 不传递 #define |
| useRec2020 未找到 | `No member named 'useRec2020'` | 拆分为 Palette709.slang + Palette2020.slang，无 cbuffer |

## Build/run 验证

```bash
tools\.packman\cmake\bin\cmake.exe --build build/windows-vs2022 --config Debug --target D3D12HDR
```

运行：
```bash
.\build\windows-vs2022\bin\Debug\Karma.exe --sample Samples/Desktop/D3D12HDR
```

预期：SDR 梯度条（上）、HDR 梯度条（下）、Rec.709/Rec.2020 色域三角形，ImGui 显示曲线选择与 reference white nits。
