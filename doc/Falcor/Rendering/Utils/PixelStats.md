# PixelStats - Runtime Statistics Collection

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Sub-modules

None (Leaf component)

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Buffer, Texture, Fence)
- **Core/Pass** - Compute pass management (ComputePass)
- **Utils/Algorithm** - Parallel reduction (ParallelReduction)
- **Utils/UI** - GUI widgets (Gui::Widgets)
- **Utils/Logger** - Logging utilities
- **Utils/Scripting** - Python bindings

## Module Overview

PixelStats provides comprehensive runtime statistics collection for path tracing and ray tracing applications. It tracks per-pixel ray counts, path lengths, path vertex counts, and volume lookups on the GPU, with efficient reduction to CPU for analysis and visualization. The system is designed for minimal performance impact when disabled and provides detailed per-pixel statistics when enabled.

## Component Specifications

### PixelStats Class

**Files**:
- [`PixelStats.h`](Source/Falcor/Rendering/Utils/PixelStats.h:1) - Main class header (142 lines)
- [`PixelStats.cpp`](Source/Falcor/Rendering/Utils/PixelStats.cpp:1) - Implementation (312 lines)
- [`PixelStats.slang`](Source/Falcor/Rendering/Utils/PixelStats.slang:1) - Shader functions (80 lines)
- [`PixelStatsShared.slang`](Source/Falcor/Rendering/Utils/PixelStatsShared.slang:1) - Shared definitions (41 lines)
- [`PixelStats.cs.slang`](Source/Falcor/Rendering/Utils/PixelStats.cs.slang:1) - Compute shader (52 lines)

**Purpose**: Helper class for collecting runtime stats in path tracer.

**Key Features**:
- Per-pixel ray count tracking (visibility, closest hit)
- Path length statistics
- Path vertex count tracking
- Volume lookup count tracking
- GPU-based statistics collection
- Parallel reduction for efficient aggregation
- Async CPU readback with fence synchronization
- Per-pixel texture generation for visualization
- Enable/disable for performance control
- Python bindings for script access
- GUI integration for real-time display

### Stats Structure

**File**: [`PixelStats.h`](Source/Falcor/Rendering/Utils/PixelStats.h:50)

**Purpose**: Aggregated statistics structure.

**Members**:
- `visibilityRays` - Total visibility rays (uint32_t)
- `closestHitRays` - Total closest hit rays (uint32_t)
- `totalRays` - Total rays (visibility + closest hit) (uint32_t)
- `pathVertices` - Total path vertices (uint32_t)
- `volumeLookups` - Total volume lookups (uint32_t)
- `avgVisibilityRays` - Average visibility rays per pixel (float)
- `avgClosestHitRays` - Average closest hit rays per pixel (float)
- `avgTotalRays` - Average total rays per pixel (float)
- `avgPathLength` - Average path length per pixel (float)
- `avgPathVertices` - Average path vertices per pixel (float)
- `avgVolumeLookups` - Average volume lookups per pixel (float)

### PixelStatsRayType Enum

**File**: [`PixelStatsShared.slang`](Source/Falcor/Rendering/Utils/PixelStatsShared.slang:33)

**Purpose**: Ray type classification for statistics.

**Values**:
- `Visibility = 0` - Visibility rays (shadow rays)
- `ClosestHit = 1` - Closest hit rays (primary rays)
- `Count = 2` - Total ray type count

## Technical Details

### Core Data Structures

**PixelStats Class Members**:

**Helper Objects**:
- [`mpParallelReduction`](Source/Falcor/Rendering/Utils/PixelStats.h:116) - ParallelReduction helper for GPU aggregation
- [`mpComputeRayCount`](Source/Falcor/Rendering/Utils/PixelStats.h:140) - ComputePass for total ray count texture

**GPU Resources**:
- [`mpReductionResult`](Source/Falcor/Rendering/Utils/PixelStats.h:117) - CPU-mappable buffer for reduction results
- [`mpFence`](Source/Falcor/Rendering/Utils/PixelStats.h:118) - GPU fence for synchronization
- [`mpStatsRayCount[kRayTypeCount]`](Source/Falcor/Rendering/Utils/PixelStats.h:133) - Per-pixel ray count textures (R32Uint)
- [`mpStatsRayCountTotal`](Source/Falcor/Rendering/Utils/PixelStats.h:134) - Per-pixel total ray count texture (R32Uint)
- [`mpStatsPathLength`](Source/Falcor/Rendering/Utils/PixelStats.h:135) - Per-pixel path length texture (R32Uint)
- [`mpStatsPathVertexCount`](Source/Falcor/Rendering/Utils/PixelStats.h:136) - Per-pixel path vertex count texture (R32Uint)
- [`mpStatsVolumeLookupCount`](Source/Falcor/Rendering/Utils/PixelStats.h:137) - Per-pixel volume lookup count texture (R32Uint)

**Configuration**:
- [`mEnabled`](Source/Falcor/Rendering/Utils/PixelStats.h:121) - Enable pixel statistics flag
- [`mEnableLogging`](Source/Falcor/Rendering/Utils/PixelStats.h:122) - Enable logging to logfile flag

**Runtime State**:
- [`mRunning`](Source/Falcor/Rendering/Utils/PixelStats.h:125) - True between begin/end calls
- [`mWaitingForData`](Source/Falcor/Rendering/Utils/PixelStats.h:126) - True if waiting for GPU data
- [`mFrameDim`](Source/Falcor/Rendering/Utils/PixelStats.h:127) - Frame dimensions (width, height)
- [`mStatsValid`](Source/Falcor/Rendering/Utils/PixelStats.h:129) - True if stats are valid
- [`mRayCountTextureValid`](Source/Falcor/Rendering/Utils/PixelStats.h:130) - True if ray count texture is valid
- [`mStatsBuffersValid`](Source/Falcor/Rendering/Utils/PixelStats.h:138) - True if per-pixel buffers contain valid data
- [`mStats`](Source/Falcor/Rendering/Utils/PixelStats.h:131) - Cached statistics structure

### Core Methods

**Construction**:
- [`PixelStats(ref<Device> pDevice)`](Source/Falcor/Rendering/Utils/PixelStats.cpp:59) - Constructor, initializes compute pass

**Frame Management**:
- [`beginFrame(RenderContext* pRenderContext, const uint2& frameDim)`](Source/Falcor/Rendering/Utils/PixelStats.cpp:65) - Begin frame, prepare state and buffers
- [`endFrame(RenderContext* pRenderContext)`](Source/Falcor/Rendering/Utils/PixelStats.cpp:110) - End frame, trigger reduction and readback

**Program Preparation**:
- [`prepareProgram(const ref<Program>& pProgram, const ShaderVar& var)`](Source/Falcor/Rendering/Utils/PixelStats.cpp:138) - Add defines and bind resources to shader

**Statistics Access**:
- [`getStats(PixelStats::Stats& stats)`](Source/Falcor/Rendering/Utils/PixelStats.cpp:199) - Fetch latest stats (async readback)
- [`getRayCountTexture(RenderContext* pRenderContext)`](Source/Falcor/Rendering/Utils/PixelStats.cpp:211) - Get per-pixel ray count texture
- [`getPathLengthTexture()`](Source/Falcor/Rendering/Utils/PixelStats.cpp:245) - Get per-pixel path length texture
- [`getPathVertexCountTexture()`](Source/Falcor/Rendering/Utils/PixelStats.cpp:251) - Get per-pixel path vertex count texture
- [`getVolumeLookupCountTexture()`](Source/Falcor/Rendering/Utils/PixelStats.cpp:257) - Get per-pixel volume lookup count texture

**UI and Logging**:
- [`renderUI(Gui::Widgets& widget)`](Source/Falcor/Rendering/Utils/PixelStats.cpp:159) - Render GUI for configuration and stats display

**Protected Methods**:
- [`copyStatsToCPU()`](Source/Falcor/Rendering/Utils/PixelStats.cpp:263) - Async copy stats from GPU to CPU
- [`computeRayCountTexture(RenderContext* pRenderContext)`](Source/Falcor/Rendering/Utils/PixelStats.cpp:225) - Compute total ray count texture

### Shader Functions

**File**: [`PixelStats.slang`](Source/Falcor/Rendering/Utils/PixelStats.slang:1)

**Global Resources**:
- `RWTexture2D<uint> gStatsRayCount[2]` - Per-pixel ray count stats (Visibility, ClosestHit)
- `RWTexture2D<uint> gStatsPathLength` - Per-pixel path length
- `RWTexture2D<uint> gStatsPathVertexCount` - Per-pixel path vertex count
- `RWTexture2D<uint> gStatsVolumeLookupCount` - Per-pixel volume lookup count
- `static uint2 gPixelStatsPixel` - Current pixel coordinate

**Functions**:

**logSetPixel(uint2 pixel)** (from [`logSetPixel()`](Source/Falcor/Rendering/Utils/PixelStats.slang:47)):
- Sets current pixel coordinate for logging
- Only active when `_PIXEL_STATS_ENABLED` is defined

**logTraceRay(PixelStatsRayType rayType)** (from [`logTraceRay()`](Source/Falcor/Rendering/Utils/PixelStats.slang:54)):
- Logs a ray trace operation
- Uses `InterlockedAdd` for thread-safe increment
- Increments appropriate ray type counter

**logPathLength(uint pathLength)** (from [`logPathLength()`](Source/Falcor/Rendering/Utils/PixelStats.slang:61)):
- Logs path length for current pixel
- Direct assignment (not atomic, assumes single thread per pixel)

**logPathVertex()** (from [`logPathVertex()`](Source/Falcor/Rendering/Utils/PixelStats.slang:68)):
- Logs a path vertex
- Uses `InterlockedAdd` for thread-safe increment

**logVolumeLookup()** (from [`logVolumeLookup()`](Source/Falcor/Rendering/Utils/PixelStats.slang:75)):
- Logs a volume lookup
- Uses `InterlockedAdd` for thread-safe increment

### Compute Shader

**File**: [`PixelStats.cs.slang`](Source/Falcor/Rendering/Utils/PixelStats.cs.slang:1)

**Purpose**: Compute pass for computing per-pixel total ray count.

**Resources**:
- `Texture2D<uint> gStatsRayCount[2]` - Input ray count textures
- `RWTexture2D<uint> gStatsRayCountTotal` - Output total ray count texture
- `cbuffer CB` - Constant buffer with `gFrameDim` (uint2)

**Main Function** (from [`main()`](Source/Falcor/Rendering/Utils/PixelStats.cs.slang:41)):
- Thread group size: 16x16x1
- For each pixel, sums all ray type counts
- Writes total to output texture
- Bounds checking against frame dimensions

### Frame Lifecycle

**beginFrame()** (from [`PixelStats::beginFrame()`](Source/Falcor/Rendering/Utils/PixelStats.cpp:65)):
1. Validate state (not already running)
2. Set running flag
3. Store frame dimensions
4. Mark previous data as invalid
5. If enabled:
   - Create ParallelReduction helper if needed
   - Create reduction result buffer if needed
   - Create or resize stats textures if frame size changed
   - Clear all stats textures to zero

**endFrame()** (from [`PixelStats::endFrame()`](Source/Falcor/Rendering/Utils/PixelStats.cpp:110)):
1. Validate state (must be running)
2. Clear running flag
3. If enabled:
   - Create fence if needed
   - Execute parallel reduction for each ray type
   - Execute parallel reduction for path length
   - Execute parallel reduction for path vertex count
   - Execute parallel reduction for volume lookup count
   - Submit command list
   - Signal fence
   - Set waiting for data flag
   - Mark stats buffers as valid

### Statistics Collection

**GPU Collection**:
- Per-pixel counters stored in RWTexture2D<uint>
- Atomic operations for thread safety
- Shader functions called from path tracing code
- Conditional compilation via `_PIXEL_STATS_ENABLED` define

**Parallel Reduction** (from [`PixelStats::endFrame()`](Source/Falcor/Rendering/Utils/PixelStats.cpp:121)):
- Uses ParallelReduction helper
- Reduces each texture to single uint4 result
- Results stored in GPU buffer with byte offsets
- Offsets: 0 (visibility), 16 (closest hit), 32 (path length), 48 (path vertices), 64 (volume lookups)

**CPU Readback** (from [`PixelStats::copyStatsToCPU()`](Source/Falcor/Rendering/Utils/PixelStats.cpp:263)):
1. Wait for fence signal
2. Map reduction result buffer
3. Extract uint4 values for each stat type
4. Compute averages by dividing by pixel count
5. Populate Stats structure
6. Unmap buffer
7. Mark stats as valid

### Statistics Computation

**Ray Statistics**:
- `visibilityRays` = result[0].x
- `closestHitRays` = result[1].x
- `totalRays` = visibilityRays + closestHitRays

**Path Statistics**:
- `pathVertices` = result[2].x (from path vertex count reduction)
- `avgPathLength` = totalPathLength / numPixels
- `avgPathVertices` = pathVertices / numPixels

**Volume Statistics**:
- `volumeLookups` = result[3].x (from volume lookup count reduction)
- `avgVolumeLookups` = volumeLookups / numPixels

**Averages**:
- All averages computed per pixel (divide by frame width × height)
- Floating-point division for precision

### Texture Generation

**Ray Count Texture** (from [`PixelStats::computeRayCountTexture()`](Source/Falcor/Rendering/Utils/PixelStats.cpp:225)):
- Lazy generation (only when requested)
- Compute pass sums ray types per pixel
- Output: R32Uint texture with total rays per pixel
- Thread group: 16x16x1

**Per-Pixel Textures**:
- Path Length: R32Uint, per-pixel path length
- Path Vertex Count: R32Uint, per-pixel vertex count
- Volume Lookup Count: R32Uint, per-pixel volume lookups

### Program Integration

**prepareProgram()** (from [`PixelStats::prepareProgram()`](Source/Falcor/Rendering/Utils/PixelStats.cpp:138)):
- Adds `_PIXEL_STATS_ENABLED` define if enabled
- Removes define if disabled
- Binds all stats textures to shader variables
- Uses array indexing for ray count textures

**Shader Usage**:
```cpp
// In path tracing shader:
logSetPixel(pixelCoord);
logTraceRay(PixelStatsRayType::Visibility);
logPathVertex();
logPathLength(currentPathLength);
logVolumeLookup();
```

### UI Integration

**renderUI()** (from [`PixelStats::renderUI()`](Source/Falcor/Rendering/Utils/PixelStats.cpp:159)):
- Checkbox to enable/disable stats collection
- Tooltip explaining performance impact
- Displays all statistics when available
- Formatting with fixed precision (3 decimal places)
- Optional logging to logfile
- Detailed tooltip explaining path vertex count

### Python Bindings

**File**: [`PixelStats.cpp`](Source/Falcor/Rendering/Utils/PixelStats.cpp:302)

**Binding** (from [`FALCOR_SCRIPT_BINDING(PixelStats)`](Source/Falcor/Rendering/Utils/PixelStats.cpp:302)):
- Class name: `PixelStats`
- Properties:
  - `enabled` (read/write) - Enable/disable stats
  - `stats` (read-only) - Dictionary with all stats
- Stats conversion function `toPython()` creates dictionary with all stat fields

### Performance Considerations

**GPU Impact**:
- Minimal when disabled (no resource binding)
- Atomic operations on per-pixel counters
- Parallel reduction overhead
- Additional texture memory (5 textures per frame)
- Compute pass for ray count texture

**CPU Impact**:
- Async readback with fence synchronization
- Minimal CPU work (just averages computation)
- UI rendering overhead

**Memory Usage**:
- Per-pixel textures: 4 × (width × height × 4 bytes)
- Reduction buffer: (ray types + 3) × 16 bytes = 80 bytes
- Total: ~16 bytes per pixel + overhead

**Optimizations**:
- Lazy texture creation (only when needed)
- Conditional compilation for zero overhead when disabled
- Efficient parallel reduction
- Async readback to avoid blocking

### Integration Points

**Path Tracing Integration**:
- Call `logSetPixel()` at start of pixel shader
- Call `logTraceRay()` for each ray
- Call `logPathVertex()` for each path vertex
- Call `logPathLength()` at end of path
- Call `logVolumeLookup()` for each volume sample

**Render Pass Integration**:
- Call `beginFrame()` before rendering
- Call `prepareProgram()` for each shader
- Call `endFrame()` after rendering
- Use `getStats()` for analysis
- Use texture getters for visualization

**Debug Integration**:
- Per-pixel textures for visualization
- Real-time UI display
- Optional logging to file

## Architecture Patterns

### Conditional Compilation Pattern
- `_PIXEL_STATS_ENABLED` define controls all stats code
- Zero overhead when disabled
- Shader and CPU code both conditionally compiled

### Async Readback Pattern
- GPU fence for synchronization
- Non-blocking `getStats()` call
- Lazy data fetching on demand
- State tracking for availability

### Lazy Resource Creation Pattern
- Textures created on first use
- Recreated only when frame size changes
- ParallelReduction helper created on first enable
- Fence created on first endFrame

### Reduction Pattern
- Parallel reduction for GPU aggregation
- Single uint4 per stat type
- Byte offsets for multiple stats in one buffer
- Efficient memory access patterns

### State Machine Pattern
- Running state between begin/end
- Waiting for data state during async readback
- Valid flags for cached data
- Assertions for state validation

## Code Patterns

### Atomic Operations
```cpp
// Shader:
InterlockedAdd(gStatsRayCount[rayType][pixel], 1);
```
- Thread-safe increment
- Used for ray counts, path vertices, volume lookups

### Conditional Compilation
```cpp
#ifdef _PIXEL_STATS_ENABLED
    // Stats code
#endif
```
- Zero overhead when disabled
- Applied to both CPU and GPU code

### Async Readback
```cpp
// End frame:
pRenderContext->signal(mpFence.get());
mWaitingForData = true;

// Get stats:
if (mWaitingForData) {
    mpFence->wait();
    // Map and read data
}
```
- Non-blocking end frame
- Lazy data fetching

### Lazy Initialization
```cpp
if (!mpParallelReduction) {
    mpParallelReduction = std::make_unique<ParallelReduction>(mpDevice);
    mpReductionResult = mpDevice->createBuffer(...);
}
```
- Create on first use
- Avoids unnecessary allocation

### Buffer Reuse
```cpp
// Multiple stats in single buffer with offsets:
mpParallelReduction->execute<uint4>(..., mpReductionResult, i * sizeof(uint4));
```
- Efficient memory usage
- Byte offset for positioning

## Use Cases

### Basic Statistics Collection
```cpp
// Initialize:
PixelStats stats(device);
stats.setEnabled(true);

// Frame loop:
stats.beginFrame(renderContext, frameDim);
stats.prepareProgram(program, var);
// ... render path tracer ...
stats.endFrame(renderContext);

// Get stats:
PixelStats::Stats s;
if (stats.getStats(s)) {
    std::cout << "Avg rays: " << s.avgTotalRays << std::endl;
}
```

### Per-Pixel Visualization
```cpp
// Get ray count texture:
auto rayCountTex = stats.getRayCountTexture(renderContext);
// Use texture for visualization (heatmap, etc.)
```

### Real-Time Monitoring
```cpp
// In UI render:
stats.renderUI(widget);
// Shows checkbox and all statistics
```

### Performance Profiling
```cpp
// Enable stats:
stats.setEnabled(true);

// Run path tracer
// Check stats for ray counts, path lengths

// Disable for production:
stats.setEnabled(false);
```

### Python Scripting
```python
# Access stats from Python
stats.enabled = True
# ... render ...
s = stats.stats
print(f"Average rays: {s['avgTotalRays']}")
```

### Debugging Path Tracer
```cpp
// Enable logging:
stats.renderUI(widget);
widget.checkbox("Enable logging", enableLogging);

// Check per-pixel data:
auto pathLengthTex = stats.getPathLengthTexture();
auto vertexCountTex = stats.getPathVertexCountTexture();
// Visualize for debugging
```

## Limitations

### Feature Limitations
- Only tracks two ray types (visibility, closest hit)
- No ray direction or origin tracking
- No per-ray timing information
- No per-material statistics
- No per-light statistics

### API Limitations
- Must call begin/end in correct order
- Cannot get stats while running
- Ray count texture requires separate compute pass
- Per-pixel textures only available after endFrame

### Performance Limitations
- Atomic operations add overhead
- Parallel reduction adds GPU time
- Async readback still requires synchronization
- Additional memory usage for textures

### Platform Limitations
- Requires compute shader support
- Requires atomic operations support
- Requires RWTexture2D support

## Best Practices

### When to Use
- Path tracing performance analysis
- Ray count optimization
- Path length validation
- Volume rendering debugging
- Research and development

### Usage Patterns
- Enable only when needed
- Disable for production builds
- Use per-pixel textures for visualization
- Use getStats() for quantitative analysis
- Use UI for real-time monitoring

### Performance Tips
- Disable stats when not needed
- Avoid frequent getStats() calls
- Use async readback to avoid blocking
- Reuse textures across frames
- Minimize atomic operations

### Error Handling
- Check return value of getStats()
- Handle invalid stats gracefully
- Validate frame dimensions
- Check for fence creation failures

### Memory Management
- Textures recreated on size change
- Reduction buffer reused
- Fence created once
- ParallelReduction created once

## Implementation Notes

### Parallel Reduction Integration
- Uses ParallelReduction helper
- Reduces each texture independently
- Results stored in single buffer with offsets
- Type: uint4 for efficient packing

### Fence Synchronization
- Created on first endFrame
- Signaled after reduction
- Waited on in copyStatsToCPU
- Ensures GPU completion before CPU read

### Texture Format
- All stats textures: R32Uint
- Efficient for atomic operations
- Sufficient for ray counts
- Directly usable in shaders

### Shader Define
- `_PIXEL_STATS_ENABLED` controls all stats code
- Added/removed by prepareProgram
- Checked in all shader functions
- Zero overhead when undefined

### Python Integration
- toPython() function converts Stats to dict
- Property access for enabled flag
- Read-only stats property
- Automatic conversion on access

### UI Integration
- Checkbox for enable/disable
- Tooltip for performance warning
- Detailed stats display
- Optional logging to file
- Fixed precision formatting

## Future Enhancements

### Additional Statistics
- Per-ray timing information
- Per-material statistics
- Per-light statistics
- Ray direction distribution
- Path termination reasons

### Performance Improvements
- Reduce atomic operation overhead
- Batch reduction operations
- More efficient texture layout
- GPU-side statistics computation

### API Extensions
- Per-pixel statistics query
- Histogram generation
- Percentile computation
- Statistical analysis functions
- Export to CSV/JSON

### Visualization Enhancements
- Heatmap generation
- Per-pixel overlay
- Real-time graphing
- Comparison views
- Difference visualization

### Debug Features
- Per-ray logging
- Path tracing visualization
- Volume sampling visualization
- Interactive inspection
- Breakpoint support

### Integration Enhancements
- Automatic integration with path tracers
- Shader library for easy use
- Preset configurations
- Performance profiling mode
- Benchmark mode
