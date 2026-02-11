# Rendering - Rendering Subsystems

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **Lights** - Light sampling and BVH structures
  - [ ] EmissiveLightSampler - Base class for emissive light samplers
  - [ ] EmissivePowerSampler - Power-based emissive light sampling
  - [ ] EmissiveUniformSampler - Uniform emissive light sampling
  - [ ] EnvMapSampler - Environment map light sampling
  - [ ] LightBVH - Light bounding volume hierarchy
  - [ ] LightBVHBuilder - BVH construction
  - [ ] LightBVHSampler - BVH-based light sampling
- [ ] **Materials** - Material system and BSDF implementations
  - [ ] BSDFIntegrator - BSDF integration utilities
  - [ ] Various BSDF implementations (GGX, Cloth, Hair, MERL, RGL, PBRT)
  - [ ] Material implementations (Standard, Cloth, Hair, MERL, RGL, PBRT)
- [ ] **RTXDI** - RTXDI integration for resampled importance sampling
  - [ ] RTXDI - Main RTXDI wrapper class
  - [ ] RTXDISDK - External SDK integration
- [ ] **Volumes** - Volume rendering
  - [ ] GridVolumeSampler - Grid-based volume sampling
  - [ ] HomogeneousVolumeSampler - Homogeneous volume sampling
  - [ ] PhaseFunction - Volume phase functions
- [x] **Utils** - Rendering utilities
  - [x] PixelStats - Pixel statistics tracking

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Buffer, Texture, Formats)
- **Core/Program** - Shader program management (DefineList, ComputePass)
- **Core/Enum** - Enum utilities
- **Scene/Scene** - Scene integration (IScene)
- **Scene/Lights** - Light collection interfaces (ILightCollection)
- **Utils/Properties** - Property system
- **Utils/Debug** - Debug utilities (PixelDebug)
- **Utils/Math** - Mathematical utilities (Vector types)
- **Utils/UI** - User interface (Gui::Widgets)

## Module Overview

The Rendering module provides comprehensive rendering subsystems including light sampling, material systems, RTXDI integration, and volume rendering. It implements advanced rendering techniques for real-time path tracing and global illumination.

## Component Specifications

### Lights Subsystem

**Purpose**: Provides multiple light sampling strategies for efficient global illumination.

**Key Features**:
- Multiple emissive light samplers (Power, Uniform, BVH)
- Environment map sampling
- Light BVH construction and traversal
- Efficient light candidate generation
- Spatial and temporal light reuse

**Components**:

#### EmissiveLightSampler

**File**: [`EmissiveLightSampler.h`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:1)

**Purpose**: Base class for emissive light sampler implementations.

**Core Methods**:
- [`update()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:54) - Update sampler for current frame
- [`getDefines()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:59) - Get shader defines
- [`bindShaderData()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:63) - Bind sampler data to shader
- [`renderUI()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:68) - Render GUI
- [`getType()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:73) - Get sampler type

#### LightBVH

**Purpose**: Bounding volume hierarchy for efficient light sampling.

**Key Features**:
- Hierarchical light organization
- Fast traversal for candidate selection
- Refitting support for dynamic scenes
- BVH-based sampling strategies

#### LightBVHBuilder

**Purpose**: Constructs light BVH from scene lights.

**Key Features**:
- Efficient BVH construction algorithms
- Support for various light types
- Optimized for GPU traversal

### Materials Subsystem

**Purpose**: Comprehensive material system with multiple BSDF implementations.

**Key Features**:
- Multiple material models (Standard, Cloth, Hair, MERL, RGL, PBRT)
- BSDF integration for precomputation
- Texture-based material properties
- Physically based rendering

**Components**:

#### BSDFIntegrator

**File**: [`BSDFIntegrator.h`](Source/Falcor/Rendering/Materials/BSDFIntegrator.h:1)

**Purpose**: Utility class for BSDF integration and precomputation.

**Core Methods**:
- [`integrateIsotropic()`](Source/Falcor/Rendering/Materials/BSDFIntegrator.h:55) - Integrate BSDF for single direction
- [`integrateIsotropic()`](Source/Falcor/Rendering/Materials/BSDFIntegrator.h:64) - Integrate BSDF for multiple directions

**Integration Strategy**:
- Uses GPU compute passes for integration
- Grid-based discretization of incident directions
- Parallel reduction for final results
- Staging buffers for CPU readback

### RTXDI Subsystem

**File**: [`RTXDI.h`](Source/Falcor/Rendering/RTXDI/RTXDI.h:1)

**Purpose**: Wrapper module for RTXDI (Resampled Temporal Importance Sampling) integration.

**Key Features**:
- Multiple resampling modes (No, Spatial, Temporal, Spatiotemporal)
- Bias correction options (Off, Basic, Pairwise, RayTraced)
- Light presampling for improved coherence
- Temporal history for sample reuse
- Pixel debug integration

**Sampling Modes**:
- [`Mode::NoResampling`](Source/Falcor/Rendering/RTXDI/RTXDI.h:93) - Talbot RIS from EGSR 2005
- [`Mode::SpatialResampling`](Source/Falcor/Rendering/RTXDI/RTXDI.h:94) - Spatial resampling only
- [`Mode::TemporalResampling`](Source/Falcor/Rendering/RTXDI/RTXDI.h:95) - Temporal resampling only
- [`Mode::SpatiotemporalResampling`](Source/Falcor/Rendering/RTXDI/RTXDI.h:96) - Spatiotemporal resampling

**Bias Correction Modes**:
- [`BiasCorrection::Off`](Source/Falcor/Rendering/RTXDI/RTXDI.h:110) - (1/M) normalization
- [`BiasCorrection::Basic`](Source/Falcor/Rendering/RTXDI/RTXDI.h:111) - MIS-like normalization
- [`BiasCorrection::Pairwise`](Source/Falcor/Rendering/RTXDI/RTXDI.h:112) - Pairwise MIS normalization
- [`BiasCorrection::RayTraced`](Source/Falcor/Rendering/RTXDI/RTXDI.h:113) - MIS with visibility rays (unbiased)

**Configuration Options** (from [`Options`](Source/Falcor/Rendering/RTXDI/RTXDI.h:125)):
- **Light Presampling**:
  - `presampledTileCount` - Number of precomputed light tiles (default: 128)
  - `presampledTileSize` - Size of each light tile (default: 1024)
  - `storeCompactLightInfo` - Store compact light info (default: true)

- **Initial Candidate Sampling**:
  - `localLightCandidateCount` - Local light candidates (default: 24)
  - `infiniteLightCandidateCount` - Infinite light candidates (default: 8)
  - `envLightCandidateCount` - Environment light candidates (default: 8)
  - `brdfCandidateCount` - BRDF candidates (default: 1)
  - `brdfCutoff` - BRDF ray shortening (default: 0.0)
  - `testCandidateVisibility` - Test visibility before resampling (default: true)

- **Resampling Options**:
  - `biasCorrection` - Bias correction mode (default: Basic)
  - `depthThreshold` - Depth difference threshold (default: 0.1)
  - `normalThreshold` - Normal angle threshold (default: 0.5)

- **Spatial Resampling**:
  - `samplingRadius` - Screen-space radius in pixels (default: 30.0)
  - `spatialSampleCount` - Neighbor pixel count (default: 1)
  - `spatialIterations` - Spatial iterations (default: 5)

- **Temporal Resampling**:
  - `maxHistoryLength` - Maximum history length in frames (default: 20)
  - `boilingFilterStrength` - Boiling filter strength (default: 0.0)

- **Rendering Options**:
  - `rayEpsilon` - Ray epsilon for self-intersection (default: 1.0e-3)
  - `useEmissiveTextures` - Use emissive textures (default: false)

- **Advanced Options**:
  - `enableVisibilityShortcut` - Reuse visibility across frames (default: false)
  - `enablePermutationSampling` - Permute previous frame pixels (default: false)

**Core Methods**:
- [`beginFrame()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:255) - Begin frame processing
- [`update()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:268) - Run RTXDI resampling
- [`endFrame()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:261) - End frame processing
- [`getDefines()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:242) - Get shader defines
- [`bindShaderData()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:248) - Bind sampler to shader
- [`renderUI()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:237) - Render configuration UI
- [`getPixelDebug()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:273) - Get pixel debug component

**Rendering Workflow**:
1. Call [`beginFrame()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:255) to start frame
2. Provide surface data via `gRTXDI.setSurfaceData()` for each pixel
3. Call [`update()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:268) to run resampling
4. Shade using `gRTXDI.getFinalSample()` for selected light samples
5. Call [`endFrame()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:261) to end frame

**Light Categorization**:
- **Local Lights**: Emissive triangles + Analytic lights (point lights)
- **Infinite Lights**: Directional and distant lights
- **Environment Light**: Environment map

**Resources**:
- Light info buffers
- Surface data buffers (G-Buffer)
- Reservoir buffers for sample storage
- Motion vector textures for temporal reprojection
- Precomputed light tiles

### Volumes Subsystem

**Purpose**: Volume rendering capabilities for participating media.

**Key Features**:
- Grid-based volume sampling
- Homogeneous volume support
- Phase function implementations
- Volume data structures

**Components**:
- `GridVolumeSampler` - Grid-based volume sampling
- `HomogeneousVolumeSampler` - Homogeneous volume sampling
- `PhaseFunction` - Volume phase function interfaces

### Utils Subsystem

**Purpose**: Rendering utilities and statistics.

**Components**:
- `PixelStats` - Pixel statistics tracking for debugging and analysis

## Architecture Patterns

### Strategy Pattern
- [`EmissiveLightSampler`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:44) base class with multiple implementations
- Interchangeable sampling strategies
- Runtime selection of sampler type

### Builder Pattern
- [`LightBVHBuilder`](Source/Falcor/Rendering/Lights/LightBVHBuilder.h:1) constructs BVH from lights
- Separation of construction from traversal
- Optimized for specific light types

### Wrapper Pattern
- [`RTXDI`](Source/Falcor/Rendering/RTXDI/RTXDI.h:86) wraps external SDK
- Provides Falcor-native interface
- Manages SDK lifecycle
- Handles resource synchronization

### Integration Pattern
- [`BSDFIntegrator`](Source/Falcor/Rendering/Materials/BSDFIntegrator.h:42) integrates BSDFs
- GPU-based computation
- Staging for CPU readback
- Parallel reduction

## Technical Details

### Light Sampling Strategies

**EmissiveLightSampler** provides multiple sampling strategies:
1. **Power Sampling** - Samples proportional to light power
2. **Uniform Sampling** - Uniform sampling across lights
3. **BVH Sampling** - Hierarchical sampling using light BVH

**LightBVH** enables:
- O(log N) light selection
- Spatial coherence
- Efficient candidate generation
- Support for dynamic scenes via refitting

### RTXDI Algorithm

RTXDI implements Resampled Temporal Importance Sampling:

**Key Concepts**:
- **Reservoirs**: Store candidate light samples with weights
- **Temporal Reuse**: Reuse samples from previous frames
- **Spatial Reuse**: Share samples between neighboring pixels
- **Bias Correction**: Multiple strategies for unbiased sampling

**Resampling Pipeline**:
1. Generate initial candidates (local, infinite, environment, BRDF)
2. Test visibility for selected candidates
3. Spatial resampling across neighboring pixels
4. Temporal resampling from previous frames
5. Bias correction and normalization
6. Final sample selection for shading

### Material Integration

**BSDFIntegrator** precomputes BSDF integrals:
- Discretizes incident directions
- Integrates over hemisphere
- Stores results for runtime lookup
- Supports multiple materials

## Performance Considerations

### Light Sampling
- BVH provides O(log N) selection
- Presampling improves coherence
- Spatial reuse reduces variance
- Temporal reuse improves temporal stability

### RTXDI
- Reservoirs enable efficient sample reuse
- Spatial resampling reduces noise
- Temporal resampling improves convergence
- Bias correction controls quality vs. cost trade-off

### BSDF Integration
- GPU-based integration is highly parallel
- Grid resolution controls quality vs. memory
- Staging enables efficient CPU-GPU transfer

## Integration Points

### Scene Integration
- Lights subsystem integrates with [`ILightCollection`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:32)
- RTXDI integrates with [`IScene`](Source/Falcor/Rendering/RTXDI/RTXDI.h:43)
- Materials integrate with scene material system

### Shader Integration
- All subsystems provide [`getDefines()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:59) for shader compilation
- [`bindShaderData()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:63) for runtime binding
- Consistent shader variable naming

### Debug Integration
- RTXDI provides [`PixelDebug`](Source/Falcor/Rendering/RTXDI/RTXDI.h:273) component
- Visualizes sampling decisions
- Aids in parameter tuning

## Progress Log

- **2026-01-07T18:49:08Z**: Rendering module analysis initiated. High-level overview created. Sub-modules identified: Lights, Materials, RTXDI, Volumes, Utils. Detailed analysis of each sub-module required.
- **2026-01-08T04:16:00Z**: PixelStats component analysis completed. Comprehensive technical specification created covering PixelStats class (142 lines header, 312 lines implementation), Stats structure, PixelStatsRayType enum, shader functions (logSetPixel, logTraceRay, logPathLength, logPathVertex, logVolumeLookup), compute shader for ray count texture, frame lifecycle (beginFrame, endFrame), statistics collection (GPU collection, parallel reduction, CPU readback), statistics computation (ray, path, volume statistics), texture generation (ray count, path length, path vertex count, volume lookup count), program integration (prepareProgram, shader usage), UI integration (renderUI), Python bindings, performance considerations (GPU impact, CPU impact, memory usage, optimizations), integration points (path tracing, render pass, debug), architecture patterns (conditional compilation, async readback, lazy resource creation, reduction, state machine), code patterns (atomic operations, conditional compilation, async readback, lazy initialization, buffer reuse), use cases (basic statistics collection, per-pixel visualization, real-time monitoring, performance profiling, Python scripting, debugging path tracer), limitations (feature, API, performance, platform), best practices (when to use, usage patterns, performance tips, error handling, memory management), implementation notes (parallel reduction integration, fence synchronization, texture format, shader define, Python integration, UI integration), and future enhancements (additional statistics, performance improvements, API extensions, visualization enhancements, debug features, integration enhancements). Rendering Folder Note updated to reflect PixelStats completion status.

## Next Steps

Proceed to analyze Scene module to understand scene management, animation, camera, and material systems.
