# RTXDI - Resampled Temporal Importance Sampling

## Module State Machine

**Status**: In Progress

## Dependency Graph

### Components (Pending Analysis)

- [ ] **RTXDI** - Main RTXDI wrapper class
  - [ ] RTXDI.cpp - Implementation
  - [ ] RTXDI.h - Interface
- [ ] **RTXDISDK** - External SDK integration (conditional compilation)
  - [ ] rtxdi/RTXDI.h - External SDK header
  - [ ] rtxdi/RTXDI.slang - External SDK shader interface

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Buffer, Texture)
- **Core/Enum** - Enum utilities
- **Utils/Properties** - Property system
- **Utils/Debug** - Debug utilities (PixelDebug)
- **Scene/Scene** - Scene integration (IScene)
- **Scene/Lights** - Light collection interfaces (ILightCollection)

## Module Overview

The RTXDI module provides integration with NVIDIA's RTXDI (Resampled Temporal Importance Sampling) SDK. It enables high-quality real-time global illumination with efficient light sample reuse through temporal and spatial resampling strategies.

## Component Specifications

### RTXDI Main Class

**File**: [`RTXDI.h`](Source/Falcor/Rendering/RTXDI/RTXDI.h:1)

**Purpose**: Wrapper module for RTXDI integration into Falcor's path tracer.

**Key Features**:
- Multiple resampling modes (No, Spatial, Temporal, Spatiotemporal)
- Bias correction strategies (Off, Basic, Pairwise, RayTraced)
- Light presampling for improved coherence
- Temporal history for sample reuse
- Pixel debug integration
- Scene light categorization
- Dynamic scene update support

**Sampling Modes** (from [`Mode`](Source/Falcor/Rendering/RTXDI/RTXDI.h:91) enum):

1. **NoResampling** - Talbot RIS from EGSR 2005 "Importance Resampling for Global Illumination"
   - Basic importance resampling without spatial/temporal reuse
   - Fastest but highest variance

2. **SpatialResampling** - Spatial resampling only
   - Reuses samples across neighboring pixels
   - Reduces spatial noise
   - Single iteration

3. **TemporalResampling** - Temporal resampling only
   - Reuses samples from previous frames
   - Reduces temporal flickering
   - Uses history buffer

4. **SpatiotemporalResampling** - Spatiotemporal resampling
   - Combines spatial and temporal reuse
   - Best quality for most scenes
   - Default mode

**Bias Correction Modes** (from [`BiasCorrection`](Source/Falcor/Rendering/RTXDI/RTXDI.h:108) enum):

1. **Off** - (1/M) normalization
   - Very biased but very fast
   - Simple normalization

2. **Basic** - MIS-like normalization
   - Assumes every sample is visible
   - Faster than Pairwise
   - Good default for most cases

3. **Pairwise** - Pairwise MIS normalization
   - Assumes every sample is visible
   - Better bias correction
   - Higher computational cost

4. **RayTraced** - MIS with visibility rays
   - Unbiased
   - Highest quality
   - Highest computational cost

**Configuration Options** (from [`Options`](Source/Falcor/Rendering/RTXDI/RTXDI.h:125) struct):

#### Light Presampling Options

- `presampledTileCount` (default: 128)
  - Number of precomputed light tiles
  - Controls granularity of presampling
  - Higher values improve coherence but increase memory

- `presampledTileSize` (default: 1024)
  - Size of each precomputed light tile (number of samples)
  - Total samples = presampledTileCount × presampledTileSize
  - Larger tiles improve spatial coherence

- `storeCompactLightInfo` (default: true)
  - Store compact light info for precomputed light tiles
  - Improves cache coherence
  - Slightly higher memory usage

#### Initial Candidate Sampling Options

- `localLightCandidateCount` (default: 24)
  - Number of initial local light candidate samples
  - Includes emissive triangles and analytic lights
  - Higher values improve quality but increase cost

- `infiniteLightCandidateCount` (default: 8)
  - Number of initial infinite light candidate samples
  - Includes directional and distant lights
  - Lower values acceptable due to smooth nature

- `envLightCandidateCount` (default: 8)
  - Number of initial environment light candidate samples
  - Environment light sampling
  - Can be increased for complex environment maps

- `brdfCutoff` (default: 0.0)
  - Value in range [0, 1] to determine BRDF ray shortening
  - 0 to disable shortening
  - Higher values reduce BRDF sampling cost but may increase bias

- `testCandidateVisibility` (default: true)
  - Test visibility on selected candidate sample before resampling
  - Improves quality by discarding occluded candidates
  - Increases computational cost

#### Resampling Options

- `biasCorrection` (default: Basic)
  - Bias correction mode selection
  - Controls quality vs. cost trade-off
  - See Bias Correction Modes above

- `depthThreshold` (default: 0.1)
  - Relative depth difference at which pixels are classified too far apart
  - 0.1 = 10% depth difference
  - Controls spatial reuse radius

- `normalThreshold` (default: 0.5)
  - Cosine of angle between normals for spatial reuse
  - Lower values allow more reuse
  - Higher values reduce artifacts from normal differences

#### Spatial Resampling Options

- `samplingRadius` (default: 30.0)
  - Screen-space radius for spatial resampling, measured in pixels
  - Larger radius increases reuse but may cause blurring
  - Should be adjusted based on scene content

- `spatialSampleCount` (default: 1)
  - Number of neighbor pixels considered for resampling
  - Higher values improve quality but increase cost
  - Spatiotemporal mode always uses 1

- `spatialIterations` (default: 5)
  - Number of spatial resampling passes
  - Only used in SpatialResampling mode
  - Spatiotemporal mode always uses 1 iteration
  - Higher values improve convergence

#### Temporal Resampling Options

- `maxHistoryLength` (default: 20)
  - Maximum history length for temporal reuse, measured in frames
  - Longer history improves stability but increases memory
  - Should be balanced with scene complexity

- `boilingFilterStrength` (default: 0.0)
  - 0 = off, 1 = full strength
  - Reduces temporal artifacts (boiling)
  - Higher values reduce temporal noise but may blur

#### Rendering Options

- `rayEpsilon` (default: 1.0e-3)
  - Ray epsilon for avoiding self-intersection of visibility rays
  - Small value to prevent self-occlusion
  - May need adjustment for scene scale

- `useEmissiveTextures` (default: false)
  - Use emissive textures to return final sample incident radiance
  - true is slower and noisier
  - false uses preintegrated values (faster, less noise)
  - Recommended false for most cases
  - Enable only for lights where high-frequency detail is vital

#### Advanced Options

- `enableVisibilityShortcut` (default: false)
  - Reuse visibility across frames to reduce cost
  - Requires careful setup to avoid bias / numerical blowups
  - Experimental feature

- `enablePermutationSampling` (default: false)
  - Enables permuting pixels sampled from previous frame
  - Noisier but more denoiser friendly
  - Experimental feature

**Core Methods**:

#### Frame Management

- [`beginFrame()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:255)
  - Must be called once at beginning of each frame
  - Parameters: RenderContext, frameDim
  - Initializes RTXDI state for new frame
  - Updates reservoir buffers

- [`update()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:268)
  - Run RTXDI resampling
  - Must be called once between beginFrame() and endFrame()
  - Parameters: RenderContext, motion vectors
  - Performs all resampling passes
  - Generates final samples for shading

- [`endFrame()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:261)
  - Must be called once at end of each frame
  - Parameters: RenderContext
  - Finalizes frame state
  - Swaps reservoir buffers for temporal reuse

#### Configuration

- [`setOptions()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:227)
  - Set configuration options
  - Triggers recompilation if needed
  - Updates light presampling if changed

- [`getOptions()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:232)
  - Returns current configuration options
  - Provides read-only access to configuration

#### Shader Integration

- [`getDefines()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:242)
  - Get list of shader defines for using RTXDI sampler
  - Includes mode-specific defines
  - Required for shader compilation

- [`bindShaderData()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:248)
  - Bind RTXDI sampler to shader variable
  - Always bound to global "gRTXDI" variable
  - Parameters: rootVar (root shader variable)

#### UI and Debug

- [`renderUI()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:237)
  - Render GUI for configuration
  - Returns true if options were changed
  - Provides interactive parameter tuning

- [`getPixelDebug()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:273)
  - Returns pixel debug component
  - Enables visualization of RTXDI decisions
  - Aids in parameter tuning

#### Static Methods

- [`isInstalled()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:216)
  - Check if RTXDI SDK is installed
  - Returns true if FALCOR_HAS_RTXDI is defined
  - Enables conditional compilation

**Light Categorization** (from [`mLights`](Source/Falcor/Rendering/RTXDI/RTXDI.h:310) struct):

RTXDI categorizes lights into three types:

1. **Local Lights** - `getLocalLightCount()`
   - Emissive triangles (triangle lights)
   - Local analytic lights (point lights)
   - Total = emissiveLightCount + localAnalyticLightCount
   - First index: 0

2. **Infinite Lights** - `getInfiniteLightCount()`
   - Directional lights
   - Distant lights
   - Total = infiniteAnalyticLightCount
   - First index: getLocalLightCount()

3. **Environment Light** - `getEnvLightIndex()`
   - Environment map light
   - Single light if present
   - Index: getLocalLightCount() + getInfiniteLightCount()

**Light Tracking**:
- `analyticLightIDs` - Sorted list of analytic light IDs
- `prevEmissiveLightCount` - Previous frame emissive light count
- `prevLocalAnalyticLightCount` - Previous frame analytic light count
- Enables efficient light updates

**Resources** (from RTXDI integration):

- `mpAnalyticLightIDBuffer` - Buffer storing analytic light IDs
- `mpLightInfoBuffer` - Buffer storing information about all lights
- `mpLocalLightPdfTexture` - Texture storing PDF for local lights
- `mpEnvLightLuminanceTexture` - Texture storing environment light luminance
- `mpEnvLightPdfTexture` - Texture storing environment light PDF
- `mpLightTileBuffer` - Buffer storing precomputed light tiles (RIS buffer)
- `mpCompactLightInfoBuffer` - Optional buffer for compact light info
- `mpReservoirBuffer` - Buffer storing light reservoirs
- `mpSurfaceDataBuffer` - Buffer storing surface data for current and previous frames
- `mpNeighborOffsetsBuffer` - Buffer storing neighbor offsets for sampling

**Compute Passes**:

- `mpReflectTypes` - Helper pass for reflecting type information
- `mpUpdateLightsPass` - Update light infos and light PDF texture
- `mpUpdateEnvLightPass` - Update environment light luminance and PDF texture

**Presampling Passes**:

- `mpPresampleLocalLightsPass` - Presample local lights into light tiles
- `mpPresampleEnvLightPass` - Presample environment light into light tiles

**Candidate Generation**:

- `mpGenerateCandidatesPass` - Generate initial candidates
- `mpTestCandidateVisibilityPass` - Test visibility for selected candidate

**Resampling Passes**:

- `mpSpatialResamplingPass` - Spatial only resampling
- `mpTemporalResamplingPass` - Temporal only resampling
- `mpSpatiotemporalResamplingPass` - Spatiotemporal resampling

**Runtime State**:

- `mFrameIndex` - Current frame index (for temporal reuse)
- `mFrameDim` - Current frame dimension in pixels
- `mLastFrameReservoirID` - Index of reservoir containing last frame's output
- `mCurrentSurfaceBufferIndex` - Index of surface buffer used for current frame (0 or 1)
- `mPrevCameraData` - Previous frame's camera data (for temporal reprojection)

**Update Flags** (from [`mFlags`](Source/Falcor/Rendering/RTXDI/RTXDI.h:332) struct):

- `updateEmissiveLights` - Emissive triangles have changed (moved, enabled/disabled)
- `updateEmissiveLightsFlux` - Emissive triangles have changed intensities
- `updateAnalyticLights` - Analytic lights have changed (enabled/disabled)
- `updateAnalyticLightsFlux` - Analytic lights have changed intensities
- `updateEnvLight` - Environment light has changed (env map, intensity, enabled/disabled)
- `recompileShaders` - Shaders need recompilation on next beginFrame() call
- `clearReservoirs` - Reservoirs need to be cleared on next beginFrame() call

## Architecture Patterns

### Wrapper Pattern
- [`RTXDI`](Source/Falcor/Rendering/RTXDI/RTXDI.h:86) wraps external SDK
- Provides Falcor-native interface
- Manages SDK lifecycle
- Handles resource synchronization
- Conditional compilation based on SDK availability

### State Machine Pattern
- Frame-based state management
- Double-buffered reservoir storage
- Temporal history tracking
- Update flag accumulation

### Resource Management Pattern
- Explicit buffer allocation
- Lifetime tracking per frame
- Efficient memory reuse
- Double-buffering for temporal stability

## Technical Details

### RTXDI Algorithm Overview

RTXDI implements Resampled Temporal Importance Sampling:

**Key Concepts**:

1. **Reservoirs**
   - Store candidate light samples with weights
   - Enable efficient sample reuse
   - Maintain multiple candidates per pixel

2. **Temporal Reuse**
   - Reuse samples from previous frames
   - Reduces temporal variance
   - Requires motion vectors for reprojection

3. **Spatial Reuse**
   - Share samples between neighboring pixels
   - Reduces spatial noise
   - Uses Poisson disk sampling pattern

4. **Bias Correction**
   - Multiple strategies for unbiased sampling
   - Normalization based on visibility
   - Trade-off between quality and cost

### Resampling Pipeline

**Phase 1: Candidate Generation**
1. Generate initial candidates from:
   - Local lights (emissive + analytic)
   - Infinite lights (directional + distant)
   - Environment light
   - BRDF sampling

2. Test visibility for selected candidates
3. Store in reservoirs

**Phase 2: Spatial Resampling**
1. For each pixel, sample from neighboring reservoirs
2. Use Poisson disk sampling pattern
3. Depth and normal thresholding for valid neighbors
4. Merge candidates into current reservoir

**Phase 3: Temporal Resampling**
1. Reproject previous frame samples using motion vectors
2. Test depth and normal compatibility
3. Merge temporal candidates into current reservoir
4. Update temporal history

**Phase 4: Bias Correction**
1. Normalize reservoir weights
2. Apply selected bias correction strategy
3. Ensure unbiased sampling

**Phase 5: Final Sample Selection**
1. Select final sample from reservoir
2. Provide to shader for shading
3. Return both data and full structures for further processing

### Light Presampling

**Purpose**: Precompute light tiles for improved coherence.

**Process**:
1. Sample lights into tiles
2. Store light information and PDF
3. Use compact representation for better cache behavior
4. Enable fast candidate generation from tiles

**Benefits**:
- Improved spatial coherence
- Reduced variance
- Better cache utilization
- Faster candidate generation

### Surface Data Management

**Purpose**: Provide surface information for RTXDI resampling.

**Data Required**:
- World-space position
- Surface normal
- Material ID
- Texture coordinates
- Motion vectors (for temporal reuse)

**Double Buffering**:
- Current frame surface data
- Previous frame surface data
- Enables temporal reuse without read-after-write hazards

### Pixel Debug Integration

**Purpose**: Visualize RTXDI decisions for debugging and tuning.

**Visualizations**:
- Candidate selection
- Resampling decisions
- Spatial reuse patterns
- Temporal reuse patterns
- Bias correction effects

## Performance Considerations

### Computational Cost

**Candidate Generation**:
- O(L) where L = total light count
- Dominated by light sampling cost
- Visibility testing adds ray tracing cost

**Spatial Resampling**:
- O(N × K) where N = pixels, K = spatialSampleCount
- Memory bandwidth dominated
- Poisson disk sampling reduces variance

**Temporal Resampling**:
- O(N) where N = pixels
- Motion vector reprojection cost
- History buffer access

**Bias Correction**:
- Basic: O(1) per sample
- Pairwise: O(K) per sample where K = reservoir size
- RayTraced: O(K + R) where R = visibility rays

### Memory Usage

**Reservoir Buffers**:
- Size: frameWidth × frameHeight × reservoirSize
- Double-buffered for temporal stability
- Includes candidate data, weights, M

**Light Tiles**:
- Size: presampledTileCount × presampledTileSize
- Compact representation reduces memory
- Improves cache behavior

**Surface Data**:
- Size: frameWidth × frameHeight
- Double-buffered for temporal reuse
- Includes position, normal, material, etc.

**History Buffers**:
- Size: maxHistoryLength × frameWidth × frameHeight
- Stores previous frame reservoirs
- Enables temporal reuse

### Quality vs. Cost Trade-offs

**Mode Selection**:
- NoResampling: Fastest, highest variance
- SpatialResampling: Faster, moderate variance
- TemporalResampling: Slower, low temporal variance
- SpatiotemporalResampling: Slowest, best overall quality

**Bias Correction**:
- Off: Fastest, highest bias
- Basic: Fast, moderate bias
- Pairwise: Slower, low bias
- RayTraced: Slowest, unbiased

**Candidate Count**:
- Higher values: Better quality, higher cost
- Lower values: Faster convergence, higher variance
- Should be tuned per scene

## Integration Points

### Scene Integration

- Integrates with [`IScene`](Source/Falcor/Rendering/RTXDI/RTXDI.h:43)
- Tracks light changes via update flags
- Categorizes lights into local, infinite, environment
- Efficient light updates

### Shader Integration

- Provides [`getDefines()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:242) for compilation
- [`bindShaderData()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:248) for runtime binding
- Consistent global variable naming ("gRTXDI")

### Debug Integration

- Provides [`PixelDebug`](Source/Falcor/Rendering/RTXDI/RTXDI.h:273) component
- Visualizes sampling decisions
- Aids in parameter tuning

### Rendering Workflow

**Required Steps** (from documentation):

1. Call [`beginFrame()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:255) to start frame
2. Provide surface data via `gRTXDI.setSurfaceData()` for each pixel
3. Call [`update()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:268) to run resampling
4. Shade using `gRTXDI.getFinalSample()` for selected light samples
5. Call [`endFrame()`](Source/Falcor/Rendering/RTXDI/RTXDI.h:261) to end frame

## Progress Log

- **2026-01-07T18:51:16Z**: RTXDI subsystem analysis completed. Comprehensive documentation created covering all configuration options, resampling modes, bias correction strategies, and integration points. Module marked as In Progress pending detailed component analysis.

## Next Steps

Analyze Materials subsystem to understand material system and BSDF implementations.
