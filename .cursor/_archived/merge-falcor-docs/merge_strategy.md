# Falcor 文档合并策略

## 目标
将 235 个 MD 文件合并为 45 个文件，保证负载均衡和内容内聚。

## 合并原则
1. **功能内聚**: 相关功能的文件合并在一起
2. **负载均衡**: 每个目标文件约包含 4-6 个源文件
3. **保持独立性**: 组织良好的子模块（如 RenderGraph）保持独立
4. **内容整合**: Doc 文档按主题合并到技术文件中
5. **清理链接**: 移除所有内部链接，内容完全整合

## 合并映射表

### Core 模块 (63个文件 → 12个文件)

| 目标文件 | 源文件数量 | 源文件 |
|---------|-----------|--------|
| merged/Core/API/Core-API-Basics.md | 5 | Core/API/API.md, Core/API/Aftermath.md, Core/API/NativeHandle.md, Core/API/NativeFormats.md, Core/API/Handles.md |
| merged/Core/API/Core-API-Resources.md | 7 | Core/API/Device.md, Core/API/Resource.md, Core/API/Buffer.md, Core/API/Texture.md, Core/API/FBO.md, Core/API/ResourceViews.md, Core/API/ParameterBlock.md |
| merged/Core/API/Core-API-Contexts.md | 4 | Core/API/RenderContext.md, Core/API/ComputeContext.md, Core/API/CopyContext.md, Core/API/BlitContext.md |
| merged/Core/API/Core-API-States.md | 5 | Core/API/GraphicsStateObject.md, Core/API/ComputeStateObject.md, Core/API/BlendState.md, Core/API/DepthStencilState.md, Core/API/RasterizerState.md |
| merged/Core/API/Core-API-Raytracing.md | 4 | Core/API/RtStateObject.md, Core/API/RtAccelerationStructure.md, Core/API/ShaderTable.md, Core/API/Raytracing.md |
| merged/Core/API/Core-API-Memory.md | 7 | Core/API/GpuMemoryHeap.md, Core/API/GpuTimer.md, Core/API/QueryHeap.md, Core/API/Fence.md, Core/API/FencedPool.md, Core/API/Swapchain.md, Core/API/VAO.md |
| merged/Core/API/Core-API-D3D12.md | 8 | Core/API/Shared/Shared.md + Shared/ 下其他 7 个文件 |
| merged/Core/Pass/Core-Pass-System.md | 5 | Core/Pass/Pass.md, Core/Pass/FullScreenPass.md, Core/Pass/ComputePass.md, Core/Pass/RasterPass.md, Core/Pass/BaseGraphicsPass.md |
| merged/Core/Program/Core-Program-System.md | 8 | Core/Program/Program.md, Core/Program/ProgramManager.md, Core/Program/ProgramVars.md, Core/Program/ProgramReflection.md, Core/Program/ShaderVar.md, Core/Program/RtBindingTable.md, Core/Program/ProgramVersion.md, Core/Program/DefineList.md |
| merged/Core/State/Core-State-Management.md | 4 | Core/State/State.md, Core/State/GraphicsState.md, Core/State/ComputeState.md, Core/State/StateGraph.md |
| merged/Core/Platform/Core-Platform-Abstraction.md | 7 | Core/Platform/Platform.md, Core/Platform/OS.md, Core/Platform/MonitorInfo.md, Core/Platform/ProgressBar.md, Core/Platform/MemoryMappedFile.md, Core/Platform/LockFile.md, Core/Platform/PlatformHandles.md |
| merged/Core/Core-Overview.md | 1 | Core/Core.md |

### Utils 模块 (34个文件 → 6个文件)

| 目标文件 | 源文件数量 | 源文件 |
|---------|-----------|--------|
| merged/Utils/Utils-String-IO.md | 5 | Utils/StringUtils.md, Utils/StringFormatters.md, Utils/PathResolving.md, Utils/BinaryFileStream.md, Utils/Properties.md |
| merged/Utils/Utils-Sampling-Algorithms.md | 2 | Utils/Sampling.md, Utils/Algorithm.md |
| merged/Utils/Utils-Math-Color.md | 4 | Utils/Math.md, Utils/Color.md, Utils/NumericRange.md, Utils/HostDeviceShared.md |
| merged/Utils/Utils-Image-Processing.md | 2 | Utils/Image.md, Utils/TermColor.md |
| merged/Utils/Utils-Threading-Debug.md | 8 | Utils/Threading.md, Utils/TaskManager.md, Utils/Logger.md, Utils/Debug.md, Utils/SplitBuffer.md, Utils/fast_vector.md, Utils/ObjectID.md, Utils/ObjectIDPython.md |
| merged/Utils/Utils-Core-Utilities.md | 6 | Utils/Utils.md, Utils/Utils-UI.md, Utils/CoreUtilities.md, Utils/AlignedAllocator.md, Utils/BufferAllocator.md, Utils/Dictionary.md |
| merged/Utils/Utils-Special-Features.md | 7 | Utils/IndexedVector.md, Utils/SharedCache.md, Utils/Attributes.md, Utils/NVAPI.md, Utils/CudaUtils.md, Utils/CudaRuntime.md, Utils/CryptoUtils.md, Utils/SlangUtils.md |

### Scene 模块 (47个文件 → 9个文件)

| 目标文件 | 源文件数量 | 源文件 |
|---------|-----------|--------|
| merged/Scene/Scene-Core-System.md | 8 | Scene/Scene.md, Scene/SceneCore/SceneCore.md, Scene/SceneCore/Scene.md, Scene/SceneCore/IScene.md, Scene/SceneCore/SceneCache.md, Scene/SceneCache.md, Scene/SceneCore/SceneBuilder.md, Scene/SceneCore/SceneBuilderDump.md |
| merged/Scene/Scene-Geometry-Mesh.md | 6 | Scene/SceneCore/VertexData.md, Scene/SceneCore/VertexAttrib.md, Scene/SceneCore/TriangleMesh.md, Scene/SceneCore/Transform.md, Scene/VertexAttrib.md, Scene/TriangleMesh.md, Scene/Transform.md |
| merged/Scene/Scene-Geometry-Shading.md | 3 | Scene/SceneCore/ShadingData.md, Scene/SceneCore/Shading.md, Scene/SceneCore/HitInfo.md |
| merged/Scene/Scene-Raytracing.md | 5 | Scene/SceneCore/Raytracing.md, Scene/SceneCore/RaytracingInline.md, Scene/SceneCore/SceneRayQueryInterface.md, Scene/SceneCore/Intersection.md, Scene/SceneCore/NullTrace.md |
| merged/Scene/Scene-Raster-Import.md | 3 | Scene/SceneCore/Raster.md, Scene/SceneCore/Importer.md, Scene/SceneCore/MeshIO.md |
| merged/Scene/Scene-Material-Light.md | 3 | Scene/Material.md, Scene/Lights.md, Scene/SceneCore/SceneDefines.md |
| merged/Scene/Scene-Components.md | 3 | Scene/Camera.md, Scene/Animation.md, Scene/Curves.md |
| merged/Scene/Scene-Displacement.md | 4 | Scene/Displacement/Displacement.md, Scene/Displacement/DisplacementData.md, Scene/Displacement/DisplacementMapping.md, Scene/Displacement/DisplacementUpdate.md, Scene/Displacement/DisplacementUpdateTask.md |
| merged/Scene/Scene-Volume-SDF.md | 11 | Scene/Volume/ 下 6 个文件, Scene/SDFs/ 下 5 个文件 |

### RenderPasses 模块 (28个文件 → 8个文件)

| 目标文件 | 源文件数量 | 源文件 |
|---------|-----------|--------|
| merged/RenderPasses/Path-Tracing.md | 5 | RenderPasses/PathTracer.md, RenderPasses/MinimalPathTracer.md, RenderPasses/WARDiffPathTracer.md, RenderPasses/WhittedRayTracer.md, Doc/renderpasses/PathTracer.md, Doc/renderpasses/MinimalPathTracer.md, Doc/usage/path-tracer.md |
| merged/RenderPasses/GBuffer-Accumulation.md | 4 | RenderPasses/GBuffer.md, RenderPasses/AccumulatePass.md, Doc/renderpasses/AccumulatePass.md, Doc/usage/render-passes.md |
| merged/RenderPasses/Anti-Aliasing.md | 2 | RenderPasses/TAA.md, Doc/renderpasses/TAA.md |
| merged/RenderPasses/Post-Processing.md | 4 | RenderPasses/ToneMapper.md, RenderPasses/SVGFPass.md, RenderPasses/SimplePostFX.md, Doc/renderpasses/ToneMapper.md |
| merged/RenderPasses/Denoising-Upscaling.md | 4 | RenderPasses/DLSSPass.md, RenderPasses/NRDPass.md, RenderPasses/OptixDenoiser.md, RenderPasses/RTXDIPass.md |
| merged/RenderPasses/Debug-Utils.md | 7 | RenderPasses/DebugPasses.md, RenderPasses/SceneDebugger.md, RenderPasses/PixelInspectorPass.md, RenderPasses/Utils.md, RenderPasses/SDFEditor.md, RenderPasses/ErrorMeasurePass.md, RenderPasses/FLIPPass.md |
| merged/RenderPasses/Special-Passes.md | 6 | RenderPasses/BSDFViewer.md, RenderPasses/BSDFOptimizer.md, RenderPasses/BlitPass.md, RenderPasses/ImageLoader.md, RenderPasses/ModulateIllumination.md, RenderPasses/OverlaySamplePass.md |
| merged/RenderPasses/RenderPasses-Overview.md | 1 | RenderPasses/RenderPasses.md |

### Rendering 模块 (5个文件 → 3个文件)

| 目标文件 | 源文件数量 | 源文件 |
|---------|-----------|--------|
| merged/Rendering/Rendering-System.md | 1 | Rendering/Rendering.md |
| merged/Rendering/Lighting-Material.md | 3 | Rendering/Lights/Lights.md, Rendering/Materials.md, Rendering/Lights/EmissiveLightSampler.md, Rendering/Utils/PixelStats.md, Doc/usage/materials.md |
| merged/Rendering/Advanced-Rendering.md | 2 | Rendering/RTXDI.md, Rendering/Volumes.md |

### Doc 模块 (22个文件 → 3个文件)

| 目标文件 | 源文件数量 | 源文件 |
|---------|-----------|--------|
| merged/Doc/Doc-Shaders.md | 7 | Doc/shaders/ 下所有 7 个文件 |
| merged/Doc/Doc-Usage.md | 5 | Doc/usage/scene-creation.md, Doc/usage/scenes.md, Doc/usage/scene-formats.md, Doc/usage/custom-primitives.md, Doc/usage/scripting.md |
| merged/Doc/Doc-Tools.md | 2 | Doc/usage/sdf-editor.md, Doc/usage/environment-variables.md |

### 其他模块 (保持独立)

| 目标文件 | 源文件数量 | 说明 |
|---------|-----------|------|
| merged/RenderGraph/RenderGraph.md | 1 | 独立模块 |
| merged/Testing/Testing.md | 1 | 独立模块 |
| merged/INDEX.md | 1 | 索引文件 |
| merged/Summary.md | 1 | 总结文件 |
| merged/Falcor.md | 1 | 主文件 |

## 负载均衡统计

| 模块 | 目标文件数 | 平均源文件/目标文件 |
|------|-----------|-------------------|
| Core | 12 | 5.25 |
| Utils | 7 | 4.86 |
| Scene | 9 | 5.22 |
| RenderPasses | 8 | 3.5 |
| Rendering | 3 | 1.67 |
| Doc | 3 | 7.33 |
| 其他 | 5 | 0.2 |
| **总计** | **48** | **4.90** |

## 脚本功能
- 自动读取源文件
- 清理 Obsidian 链接 [[...]]
- 合并内容并添加标题分隔
- 生成目录结构
- 创建合并后的文件夹结构
