# D3D12RaytracingHelloWorld 迁移记录

## 与 DirectX-Graphics-Samples 原版对比

| 维度 | DirectX-Graphics-Samples 原版 | Falcor 迁移版 |
|------|-------------------------------|---------------|
| **应用框架** | 独立 Win32 exe，继承 `DXSample` | Karma 插件，继承 `SampleBase` |
| **几何来源** | 手动构建 BLAS/TLAS（单三角形） | 同上，使用 `RtAccelerationStructure` + `buildAccelerationStructure` |
| **Scene 系统** | 无，standalone raytracing | 无，直接调用 `pRenderContext->raytrace()` |
| **坐标空间** | 屏幕空间（NDC），正交射线 | 同上，1:1 保留 |
| **生命周期** | `OnInit` / `OnUpdate` / `OnRender` / `OnDestroy` | `onLoad` / `onFrameRender` / `onShutdown` |

---

## Scaffold 命令

```bash
python tools/make_new_sample_app.py D3D12RaytracingHelloWorld --path Source/Samples/Desktop/D3D12Raytracing
```

需在 `Source/Samples/CMakeLists.txt` 添加 `add_subdirectory(Desktop/D3D12Raytracing/D3D12RaytracingHelloWorld)`。

## 5.1 DX Sample → Falcor 映射表

| DX 概念 / 代码 | Falcor 对应 | 备注 |
|----------------|-------------|------|
| `OnInit()` / `CreateDeviceDependentResources` | `onLoad()` | 合并为单次加载 |
| `CreateRaytracingInterfaces` | （Falcor 抽象） | 无需实现 |
| `CreateRootSignatures` | （Slang 自动生成） | 由 shader 反射生成 |
| `CreateRaytracingPipelineStateObject` | `ProgramDesc` + `Program::create` | D3D12RaytracingHelloWorld.rt.slang |
| `CreateDescriptorHeap` | （Falcor 抽象） | 由 RtProgramVars 管理 |
| `BuildGeometry` | `mpVertexBuffer`、`mpIndexBuffer` | `createBuffer`，顶点 (0,-0.7,1), (-0.7,0.7,1), (0.7,0.7,1) |
| `BuildAccelerationStructures` | `RtAccelerationStructure::create` + `buildAccelerationStructure` | BLAS → TLAS，单实例单位矩阵 |
| `BuildShaderTables` | `RtBindingTable` + `RtProgramVars` | create(1, 1, 1)：1 miss、1 ray type、1 geometry |
| `CreateRaytracingOutputResource` | `mpRtOut = createTexture2D(..., UAV \| SRV)` | RGBA8Unorm |
| `DoRaytracing` | `pRenderContext->raytrace(...)` | 直接调用，不经过 Scene |
| `CopyRaytracingOutputToBackbuffer` | `pRenderContext->blit(mpRtOut->getSRV(), pTargetFbo->getRenderTargetView(0))` | 1:1 |
| `UpdateForSizeChange` | `onResize()` + `updateViewportConstants()` | viewport/stencil 随宽高比 |
| `RaytracingAccelerationStructure Scene` | `ParameterBlock<RtScene>` 含 `[root] rtAccel` | 自定义参数块，非 gScene |
| `ConstantBuffer g_rayGenCB` | `var["g_rayGenCB"]["viewport"]` 等逐字段绑定 | 避免 setVariable 链接错误 |
| `TraceRay(Scene, ...)` | `TraceRay(gRtScene.rtAccel, ...)` | 使用自定义 TLAS |

## 保留的常量

| 常量 | 原始值 | Falcor 中 |
|------|--------|-----------|
| viewport | {-1,-1,1,1} | mRayGenCB.viewport |
| stencil border | 0.1f | updateViewportConstants |
| 顶点 | (0,-0.7,1), (-0.7,0.7,1), (0.7,0.7,1) | buildAccelerationStructures |
| ray.TMin | 0.001 | 0.001f |
| ray.TMax | 10000 | 10000.0f |
| payload | float4 color | RayPayload |
| maxRecursionDepth | 1 | setMaxTraceRecursionDepth(1) |
| payloadSize | 16 (float4) | setMaxPayloadSize(16) |

## 5.2 1:1 映射后运行错误及修复

| 问题 | 错误表现 | 修复 |
|------|----------|------|
| float4x4(1) 转换失败 | C2440 无法从 int 转换为 float4x4 | 使用 `instanceDesc.transform[0][0]=1` 等手动设置单位矩阵，避免 `setTransform(float4x4::identity())` 链接错误 |
| RayGenConstantBuffer setVariable 链接错误 | LNK2019 无法解析 setVariable<RayGenConstantBuffer> | 改为逐字段绑定：`var["g_rayGenCB"]["viewport"]["left"] = ...` |
| [root] 属性未知 | warning 31000: unknown attribute 'root' | 添加 `import Utils.Attributes;` |
| DispatchRaysIndex 类型转换 | error 39999: no overload for float2 applicable to vector<uint,3> | 使用 `uint2 launchIndex = DispatchRaysIndex().xy` 和 `float2(launchIndex) / float2(launchDim)` |
| scratch buffer 零大小 | 潜在 createBuffer(0) 失败 | 使用 `std::max(blasPrebuild.scratchDataSize, 256ull)` |

## 关键文件

- [Source/Samples/Desktop/D3D12Raytracing/D3D12RaytracingHelloWorld/D3D12RaytracingHelloWorld.cpp](Source/Samples/Desktop/D3D12Raytracing/D3D12RaytracingHelloWorld/D3D12RaytracingHelloWorld.cpp)
- [Source/Samples/Desktop/D3D12Raytracing/D3D12RaytracingHelloWorld/D3D12RaytracingHelloWorld.rt.slang](Source/Samples/Desktop/D3D12Raytracing/D3D12RaytracingHelloWorld/D3D12RaytracingHelloWorld.rt.slang)

## 验证

- Build: `.\build_vs2022.ps1 --target Karma`
- Run: `Karma.exe --sample Samples/Desktop/D3D12Raytracing/D3D12RaytracingHelloWorld`
- 视觉：中心三角形显示重心着色（红绿蓝渐变），外围 uv 渐变，背景黑色
