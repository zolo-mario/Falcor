# Falcor RenderPass 汇总表

## 概述

本文档提供了Falcor框架中所有RenderPass的汇总信息，包括：
- 分类和用途
- 输入输出参数
- 主要配置属性
- 相关shader文件
- 使用场景

## RenderPass分类

### 1. 路径追踪与光线追踪

| Pass名称 | 描述 | NEE | MIS | RTXDI | NRD | SER |
|----------|------|------|-----|-------|-----|-----|
| **PathTracer** | 高级蒙特卡洛路径追踪器 | ✅ | ✅ | ✅ | ✅ | ✅ |
| **MinimalPathTracer** | 极简暴力路径追踪器（验证基准） | ❌ | ❌ | ❌ | ❌ | ❌ |
| **WhittedRayTracer** | Whitted光线追踪器 | ❌ | ❌ | ❌ | ❌ | ❌ |
| **WARDiffPathTracer** | 基于重参数化的可微分路径追踪器 | ❌ | ❌ | ❌ | ❌ | ❌ |

### 2. 几何缓冲区

| Pass名称 | 描述 | 方法 | 支持几何体 |
|----------|------|------|-----------|
| **GBufferRaster** | 光栅化几何缓冲区生成 | Rasterization | TriangleMesh, DisplacedTriangleMesh, Curve |
| **GBufferRT** | 光线追踪几何缓冲区生成 | Ray Tracing | TriangleMesh, DisplacedTriangleMesh, Curve, SDFGrid |
| **VBufferRaster** | 光栅化可见性缓冲区生成 | Rasterization | TriangleMesh, DisplacedTriangleMesh, Curve |
| **VBufferRT** | 光线追踪可见性缓冲区生成 | Ray Tracing | TriangleMesh, DisplacedTriangleMesh, Curve, SDFGrid |

### 3. 后处理

| Pass名称 | 描述 | 类型 |
|----------|------|------|
| **ToneMapper** | 色调映射和自动曝光 | 色调映射 |
| **TAA** | 时间抗锯齿 | 抗锯齿 |
| **SimplePostFX** | 简单后处理效果集合 | 后处理 |
| **SVGFPass** | Spatiotemporal Variance-Guided Filtering | 降噪 |
| **DLSSPass** | NVIDIA DLSS上采样/抗锯齿 | 上采样/抗锯齿 |
| **NRDPass** | NVIDIA实时降噪 | 降噪 |
| **OptixDenoiser_** | OptiX AI降噪 | 降噪 |

### 4. 实用工具

| Pass名称 | 描述 |
|----------|------|
| **BlitPass** | 纹理拷贝和格式转换 |
| **ImageLoader** | 从文件加载图像 |
| **CrossFade** | 两个缓冲区之间的时间渐变 |
| **Composite** | 混合两个缓冲区 |
| **ModulateIllumination** | 使用各种光照组件调制光照 |
| **GaussianBlur** | 高斯模糊后处理 |
| **AccumulatePass** | 时间累积用于渐进式渲染 |
| **OverlaySamplePass** | 演示覆盖UI渲染 |

### 5. 调试与分析

| Pass名称 | 描述 |
|----------|------|
| **SceneDebugger** | 识别资产问题 |
| **PixelInspectorPass** | 检查像素的几何和材质属性 |
| **ColorMapPass** | 应用颜色映射到标量值 |
| **ErrorMeasurePass** | 测量与参考图像的误差 |
| **FLIPPass** | FLIP感知图像差异度量 |
| **InvalidPixelDetectionPass** | 标记NaN/Inf像素 |
| **SplitScreenPass** | 分屏对比 |
| **SideBySidePass** | 并排对比 |

### 6. 专用Pass

| Pass名称 | 描述 |
|----------|------|
| **RTXDIPass** | 使用RTXDI的直接光照 |
| **SDFEditor** | 交互式符号距离函数编辑器 |
| **BSDFOptimizer** | 使用可微分渲染优化BSDF参数 |
| **BSDFViewer** | 交互式BSDF检查工具 |

## 详细规格表

### 路径追踪器

#### PathTracer

**输入输出：**
- 输入：vbuffer, mvec, viewW, sampleCount
- 输出：color, albedo, specularAlbedo, guideNormal, nrd数据

**主要属性：**
- samplesPerPixel (uint32, 默认: 1)
- maxDiffuseBounces (uint32, 默认: 3)
- maxSpecularBounces (uint32, 默认: 3)
- maxTransmissionBounces (uint32, 默认: 10)
- useBSDFSampling (bool, 默认: true)
- useRussianRoulette (bool, 默认: false)
- useNEE (bool, 默认: true)
- useMIS (bool, 默认: true)
- misHeuristic (enum, 默认: Balance)
- emissiveSampler (enum, 默认: LightBVH)
- useRTXDI (bool, 默认: false)
- useSER (bool, 默认: true)
- colorFormat (enum, 默认: LogLuvHDR)

**Shader文件：**
- PathTracer.slang - 核心路径追踪逻辑
- TracePass.rt.slang - 光线追踪着色器
- GeneratePaths.cs.slang - 路径生成
- ResolvePass.cs.slang - 样本解析
- NRDHelpers.slang - NRD集成
- Params.slang - 参数定义

**使用场景：** 生产渲染、高质量离线渲染、基准测试

---

#### MinimalPathTracer

**输入输出：**
- 输入：vbuffer, viewW
- 输出：color

**主要属性：**
- maxBounces (uint, 默认: 3)
- computeDirect (bool, 默认: true)
- useImportanceSampling (bool, 默认: true)

**Shader文件：**
- MinimalPathTracer.rt.slang - 完整光线追踪实现

**使用场景：** 验证基准、算法测试、教学

---

#### WhittedRayTracer

**输入输出：**
- 输入：场景数据
- 输出：color

**主要属性：**
- maxBounces (uint, 默认: 3)
- textureLODMode (enum, 默认: Mip0)
- rayConeMode (enum, 默认: None)
- rayFootprintFilterMode (enum, 默认: Isotropic)
- visualizeSurfaceSpreadAngle (bool, 默认: false)
- useRoughnessToVariance (bool, 默认: false)

**Shader文件：**
- WhittedRayTracer.slang - 主要着色器
- WhittedRayTracerTypes.slang - 类型定义

**使用场景：** 快速预览、教育演示

---

#### WARDiffPathTracer

**输入输出：**
- 输入：场景数据, dLdI（梯度缓冲区）
- 输出：color, gradients

**主要属性：**
- maxBounces (uint, 默认: 可配置)
- differentiationMode (enum: ForwardDiffDebug, BackwardDiff)
- useWAR (bool, 默认: true)
- auxiliarySampleCount (uint, 默认: 16)
- vmfConcentrationParameters (float2)
- boundaryTermBeta (float)
- harmonicWeightsGamma (float)
- antitheticSampling (bool, 默认: false)

**Shader文件：**
- WARDiffPathTracer.slang - 主要可微分追踪器
- Params.slang - 参数定义
- PTUtils.slang - 路径追踪工具
- StaticParams.slang - 静态参数
- WarpedAreaReparam.slang - 重参数化

**使用场景：** 反向渲染、材质优化、可微分学习

---

### 几何缓冲区

#### GBufferRaster / GBufferRT

**输入输出：**
- 输入：场景数据
- 输出：position, normal, tangent, bitangent, faceNormal, diffuseAlbedo, specularAlbedo, emission, opacity, roughness, metallic, IOR, materialID, geometryID, linearDepth, motionVectors

**主要属性：**
- samplePattern (enum: Center, DirectX, Halton, Stratified)
- sampleCount (uint, 默认: 16)
- alphaTest (bool, 默认: true)
- adjustShadingNormals (bool, 默认: true)
- cullMode (enum: Back, Front, None)
- textureLODMode (enum, RT版本)
- depthOfFieldSupport (bool, RT版本)

**Shader文件：**
- DepthPass.3d.slang - 深度预通过（Raster）
- GBufferRaster.3d.slang - 光栅化
- GBufferRT.rt.slang / GBufferRT.cs.slang - 光线追踪
- GBufferHelpers.slang - 共享工具
- GBufferRT.slang - RT接口

**使用场景：** 延迟着色、路径追踪输入、屏幕空间效果

---

#### VBufferRaster / VBufferRT

**输入输出：**
- 输入：场景数据
- 输出：vbuffer (HitInfo格式)

**主要属性：**
- samplePattern (enum)
- sampleCount (uint, 默认: 16)
- alphaTest (bool, 默认: true)
- adjustShadingNormals (bool, 默认: true)
- traceRayInline (bool, RT版本)
- depthOfFieldSupport (bool, RT版本)

**Shader文件：**
- VBufferRaster.3d.slang - 光栅化
- VBufferRT.rt.slang - 光线追踪
- VBufferRT.slang - RT接口

**使用场景：** 路径追踪输入、延迟着色优化

---

### 后处理

#### ToneMapper

**输入输出：**
- 输入：src (HDR颜色)
- 输出：dst (LDR颜色)
- 可选输出：outputLuminance, luminanceLod

**主要属性：**
- exposureMode (enum: AperturePriority, ShutterPriority)
- exposureCompensation (float, 默认: 0.0)
- autoExposure (bool, 默认: true)
- exposureValue (float, 默认: 0.0)
- filmSpeed (float, 默认: 100.0)
- fNumber (float, 默认: 1.0)
- shutterSpeed (float, 默认: 1.0)
- whiteBalance (bool, 默认: true)
- whitePoint (float, 默认: 6500.0)
- operator (enum: Aces, Clamp, Linear, LinearSRGB, Reinhard, ModifiedReinhard, Uc2Hable)
- clampOutput (bool, 默认: true)

**Shader文件：**
- ToneMapperParams.slang - 参数定义
- ToneMapping.ps.slang - 色调映射
- Luminance.ps.slang - 亮度计算

**使用场景：** HDR到LDR转换、显示器准备、艺术风格化

---

#### TAA

**输入输出：**
- 输入：src (当前帧), mvec (运动向量), depth
- 输出：dst (抗锯齿后颜色)

**主要属性：**
- alpha (float, 默认: 0.1)
- colorBoxSigma (float, 默认: 1.0)
- antiFlicker (bool, 默认: true)

**Shader文件：** 内置全屏着色器

**使用场景：** 实时抗锯齿、减少闪烁、时间稳定性

---

#### SVGFPass

**输入输出：**
- 输入：color, albedo, emission, mvec, position/normal/fwidth
- 输出：denoisedColor

**主要属性：**
- filterEnabled (bool, 默认: true)
- filterIterations (uint, 默认: 4)
- feedbackTap (uint, 默认: 1)
- varianceEpsilon (float, 默认: 1e-4)
- phiColor (float, 默认: 10.0)
- phiNormal (float, 默认: 128.0)
- alpha (float, 默认: 0.05)
- momentsAlpha (float, 默认: 0.2)

**Shader文件：**
- SVGFCommon.slang - 共享定义
- SVGFAtrous.ps.slang - A-trous分解
- SVGFFilterMoments.ps.slang - 矩滤波
- SVGFFinalModulate.ps.slang - 最终调制
- SVGFPackLinearZAndNormal.ps.slang - 打包
- SVGFReproject.ps.slang - 重投影

**使用场景：** 实时降噪、低样本路径追踪

---

#### DLSSPass

**输入输出：**
- 输入：color (低分辨率), mvec, depth, exposure
- 输出：upscaledColor

**主要属性：**
- enabled (bool, 默认: false)
- profile (enum: MaxPerf, Balanced, MaxQuality)
- motionVectorScale (enum: Absolute, Relative)
- useHDR (bool, 默认: true)
- sharpness (float, 默认: 0.0)
- exposureValue (float)
- outputSizeSelection (enum)

**Shader文件：** 使用NGX SDK（内部）

**使用场景：** 实时上采样、性能优化

---

#### NRDPass

**输入输出：**
- 输入：color, albedo, normal, mvec, depth, viewDir
- 输出：denoisedDiffuse, denoisedSpecular

**主要属性：**
- denoisingMethod (enum: RelaxDiffuseSpecular, RelaxDiffuse, ReblurDiffuseSpecular, SpecularReflectionMv, SpecularDeltaMv)
- useWorldSpaceMotion (bool, 默认: false)
- maxIntensity (float, 默认: 1000)
- disocclusionThreshold (float, 默认: 2.0)

**Shader文件：**
- PackRadiance.cs.slang - 辐射度打包

**使用场景：** 实时降噪、高质量路径追踪

---

#### OptixDenoiser_

**输入输出：**
- 输入：color (必需), albedo (可选引导), normal (可选引导), mvec (时间)
- 输出：denoisedColor

**主要属性：**
- model (enum: LDR, HDR, Temporal)
- denoiseAlpha (bool, 默认: false)
- blendFactor (float, 默认: 0.0)

**Shader文件：**
- ConvertTexToBuf.cs.slang - 纹理到缓冲区转换
- ConvertNormalsToBuf.cs.slang - 法线转换
- ConvertMotionVectorInputs.cs.slang - 运动向量转换
- ConvertBufToTex.ps.slang - 缓冲区到纹理转换

**使用场景：** AI降噪、高质量离线渲染

---

#### AccumulatePass

**输入输出：**
- 输入：src
- 输出：dst

**主要属性：**
- enabled (bool, 默认: true)
- autoReset (bool, 默认: true)
- precisionMode (enum: Double, Single, SingleCompensated)
- maxFrameCount (uint, 默认: 0=无限)
- overflowMode (enum: Stop, Reset, EMA)
- outputFormat (ResourceFormat, 默认: RGBA32Float)
- outputSizeSelection (enum)
- fixedOutputSize (uint2, 默认: 512x512)

**Shader文件：**
- Accumulate.cs.slang - 累积着色器

**使用场景：** 渐进式渲染、地面真值、长时间采样

---

### 实用工具

#### BlitPass

**输入输出：**
- 输入：src
- 输出：dst

**主要属性：**
- filterMode (enum: Linear, Point, Nearest)
- outputFormat (ResourceFormat)

**使用场景：** 纹理拷贝、格式转换、渲染目标设置

---

#### ImageLoader

**输入输出：**
- 输出：loadedTexture

**主要属性：**
- imagePath (string)
- arraySlice (uint)
- mipLevel (uint)
- generateMipmaps (bool)
- loadAssRGB (bool)

**使用场景：** 纹理加载、参考图像加载

---

#### CrossFade

**输入输出：**
- 输入：bufferA, bufferB
- 输出：fadedBuffer

**主要属性：**
- scaleA (float)
- scaleB (float)
- outputFormat (ResourceFormat)
- autoFadeEnabled (bool, 默认: true)
- waitFrameCount (uint, 默认: 10)
- fadeFrameCount (uint, 默认: 100)
- fixedFadeFactor (float)

**Shader文件：** CrossFade.cs.slang

**使用场景：** 时间过渡、比较图像、动画效果

---

#### Composite

**输入输出：**
- 输入：bufferA, bufferB
- 输出：compositeBuffer

**主要属性：**
- mode (enum: Add, Multiply)
- scaleA (float)
- scaleB (float)
- outputFormat (ResourceFormat)

**Shader文件：** Composite.cs.slang

**使用场景：** 多层合成、添加效果

---

#### ModulateIllumination

**输入输出：**
- 输入：emission, diffuseReflectance/radiance, specularReflectance/radiance, deltaReflection emission/reflectance/radiance, deltaTransmission emission/reflectance/radiance, residualRadiance
- 输出：modulatedIllumination

**主要属性：** 各组件的开关

**Shader文件：** ModulateIllumination.cs.slang

**使用场景：** 光照分解、组件可视化

---

#### GaussianBlur

**输入输出：**
- 输入：src
- 输出：blurredTexture

**主要属性：**
- kernelWidth (uint, 默认: 5)
- sigma (float, 默认: 2.0)

**Shader文件：** 两通水平/垂直模糊

**使用场景：** 模糊、低通滤波、辉光基础

---

### 调试与分析

#### SceneDebugger

**输入输出：**
- 输入：场景数据
- 输出：debugVisualization

**主要属性：**
- mode (enum: 各种调试可视化)
- pixelDataDisplay (bool)

**Shader文件：** 调试计算着色器

**使用场景：** 资产调试、问题诊断

---

#### PixelInspectorPass

**输入输出：**
- 输入：vbuffer, depth, mvec等
- 输出：无（调试显示）

**主要属性：**
- selectedPixel (uint2)
- scaleInputsToWindow (bool)
- continuousPicking (bool)

**Shader文件：**
- PixelInspector.cs.slang
- PixelInspectorData.slang

**使用场景：** 像素检查、材质调试、几何调试

---

#### ColorMapPass

**输入输出：**
- 输入：scalarTexture
- 输出：colormapVisualization

**主要属性：**
- colorMapType (enum: Jet, Viridis, Plasma, Magma等)
- channelSelection (uint, 0-3)
- autoRange (bool, 默认: true)
- minValue (float, 手动范围）
- maxValue (float, 手动范围）

**Shader文件：**
- ColorMapPass.ps.slang
- ColorMapParams.slang

**使用场景：** 可视化标量数据、调试输出、热图

---

#### ErrorMeasurePass

**输入输出：**
- 输入：sourceImage, referenceImage (输入或加载）
- 输出：source/reference/difference (可选）

**主要属性：**
- referenceImagePath (string)
- ignoreBackground (bool)
- computeSquaredDifference (bool)
- computeAverage (bool)
- runningError (bool, EMA)
- runningErrorSigma (float)

**Shader文件：** ErrorMeasurer.cs.slang

**使用场景：** 误差测量、比较渲染器、回归测试

---

#### FLIPPass

**输入输出：**
- 输入：referenceImage, testImage
- 输出：flipErrorMap, exposureMap (HDR-FLIP)

**主要属性：**
- enableLDRFlip (bool)
- enableHDRFlip (bool)
- useMagmaColormap (bool)
- clampInput (bool)
- monitorWidth (float)
- monitorDistance (float)
- toneMapperType (enum)
- exposureParameters (HDR)
- computePooledFlipValues (bool)

**Shader文件：**
- FLIPPass.cs.slang
- ComputeLuminance.cs.slang
- ToneMappers.slang

**使用场景：** 感知质量评估、图像质量指标

---

#### InvalidPixelDetectionPass

**输入输出：**
- 输入：color
- 输出：markedTexture

**主要属性：** outputFormat (ResourceFormat)

**Shader文件：** InvalidPixelDetection.ps.slang

**使用场景：** 调试NaN/Inf、数值稳定性检查

---

#### SplitScreenPass / SideBySidePass

**输入输出：**
- 输入：leftImage, rightImage
- 输出：comparisonDisplay

**主要属性：**
- splitLocation (float)
- dividerSize (float)
- showLabels (bool)
- swapSides (bool)

**Shader文件：**
- SplitScreen.ps.slang
- SideBySide.ps.slang

**使用场景：** 图像比较、渲染器对比

---

### 专用Pass

#### RTXDIPass

**输入输出：**
- 输入：vbuffer, mvec
- 输出：directLightingResult

**主要属性：** RTXDI选项

**Shader文件：**
- PrepareSurfaceData.cs.slang - 表面数据准备
- FinalShading.cs.slang - 最终着色

**使用场景：** 高质量直接光照、复杂场景

---

#### SDFEditor

**输入输出：**
- 输入：vbuffer, depth, inputColor
- 输出：sdfVisualization

**主要属性：**
- editingShapeType (enum: sphere, box等）
- operationType (enum: union, subtraction等）
- grid/symmetryPlane manipulation
- previewEnabled (bool)
- autoBakingEnabled (bool)
- bakeBatchSize (uint)
- preservedHistoryCount (uint)

**Shader文件：**
- SDFEditorTypes.slang - 类型定义
- 2D/3D GUI渲染着色器
- Marker2DSet.slang
- Marker2DTypes.slang

**使用场景：** SDF建模、程序化内容、交互式编辑

---

#### BSDFOptimizer

**输入输出：**
- 输入：场景数据, referenceGradients
- 输出：optimizedMaterialVisualization

**主要属性：**
- adamBeta1 (float)
- adamBeta2 (float)
- adamEpsilon (float)
- initialMaterialID (uint)
- referenceMaterialID (uint)
- bsdfSliceResolution (uint)
- learningRates (float3)

**Shader文件：**
- BSDFOptimizer.cs.slang - 优化着色器
- BSDFOptimizerParams.slang - 参数
- BSDFViewer.cs.slang - 可视化

**使用场景：** 材质优化、反向渲染、学习

---

#### BSDFViewer

**输入输出：**
- 输入：场景数据, envMap (可选）
- 输出：bsdfVisualization

**主要属性：**
- selectedMaterial (uint)
- useEnvironmentMap (bool)
- sampleConfiguration

**Shader文件：**
- BSDFViewer.cs.slang - BSDF查看器
- BSDFViewerParams.slang - 参数

**使用场景：** 材质检查、BSDF可视化

---

## 渲染管道示例

### 标准路径追踪管道

```
VBufferRT
    ↓
PathTracer (with NRD)
    ↓
NRDPass
    ↓
AccumulatePass
    ↓
ToneMapper
    ↓
Output
```

### 实时渲染管道

```
GBufferRaster
    ↓
RTXDIPass (direct lighting)
    ↓
SVGFPass (temporal denoising)
    ↓
TAA (anti-aliasing)
    ↓
ToneMapper
    ↓
DLSSPass (upscaling)
    ↓
Output
```

### 离线高质量管道

```
VBufferRT
    ↓
PathTracer (high spp)
    ↓
AccumulatePass (Double precision)
    ↓
OptixDenoiser (Temporal model)
    ↓
ToneMapper
    ↓
Output
```

### 调试管道

```
GBufferRT
    ↓
PathTracer (debug settings)
    ↓
PixelInspectorPass (pixel inspection)
    ↓
ColorMapPass (visualization)
    ↓
ErrorMeasurePass (reference comparison)
    ↓
Output
```

## 性能参考

### 路径追踪器性能

| Pass | 相对性能 | 样本效率 | 内存使用 |
|------|----------|-----------|----------|
| PathTracer (basic) | 1.0x | 高 | 中等 |
| PathTracer + NEE | 1.2x | 很高 | 中等 |
| PathTracer + RTXDI | 0.8x | 极高 | 高 |
| PathTracer + NRD | 1.5x | 中 | 高 |
| MinimalPathTracer | 0.6x | 低 | 低 |

### 降噪器性能

| Pass | 质量 | 速度 | 延迟 |
|------|------|------|------|
| TAA | 低 | 极快 | 低 |
| SVGF | 高 | 中等 | 中等 |
| NRD Relax | 高 | 快 | 低 |
| NRD Reblur | 很高 | 快 | 低 |
| DLSS | 很高 | 快 | 低 |
| OptiX AI | 极高 | 中等 | 中等 |

### 内存使用

| Pass类型 | GPU内存 | 内存带宽 |
|----------|----------|----------|
| GBuffer (Raster) | 低 | 中等 |
| GBuffer (RT) | 高 | 低 |
| PathTracer (1 spp) | 中等 | 高 |
| PathTracer (16 spp) | 很高 | 极高 |
| NRD | 高 | 低 |
| OptiX Denoiser | 很高 | 中等 |

## 推荐配置

### 实时渲染（60 FPS）

```
VBufferRT → RTXDIPass → NRD (Relax) → TAA → ToneMapper → DLSS (Performance)
```

**设置：**
- PathTracer: 1 spp
- NRD: Relax模式
- TAA: alpha=0.1
- DLSS: Performance模式

### 平衡质量（30 FPS）

```
VBufferRT → RTXDIPass → NRD (Reblur) → TAA → ToneMapper
```

**设置：**
- PathTracer: 2 spp
- NRD: Reblur模式
- TAA: alpha=0.1

### 离线高质量（逐帧）

```
VBufferRT → PathTracer (16 spp) → AccumulatePass (Double) → OptixDenoiser (Temporal) → ToneMapper
```

**设置：**
- PathTracer: 16 spp
- AccumulatePass: 双精度
- OptiX: Temporal模式

## 参考文档

- [RenderPass Architecture](./renderpass-architecture.md)
- [PathTracer](./PathTracer.md)
- [MinimalPathTracer](./MinimalPathTracer.md)
- [ToneMapper](./ToneMapper.md)
- [TAA](./TAA.md)
- [AccumulatePass](./AccumulatePass.md)
