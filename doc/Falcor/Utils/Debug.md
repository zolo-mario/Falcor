# Debug - Debugging Utilities

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **DebugConsole** - Debug console window
- [x] **PixelDebug** - Pixel debugging system
- [x] **WarpProfiler** - Warp-level profiling

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Buffer, Fence)
- **Core/Program** - Shader program management (Program, ShaderVar)
- **Utils/UI** - User interface (Gui)

## Module Overview

The Debug module provides comprehensive debugging and profiling utilities for the Falcor rendering framework. It includes pixel-level debugging with shader print/assert support, warp-level profiling for GPU performance analysis, and a debug console for Windows applications. These utilities are essential for shader debugging, performance optimization, and development workflow.

## Component Specifications

### PixelDebug

**Files**:
- [`PixelDebug.h`](Source/Falcor/Utils/Debug/PixelDebug.h:1) - Pixel debug header
- [`PixelDebug.cpp`](Source/Falcor/Utils/Debug/PixelDebug.cpp:1) - Pixel debug implementation
- [`PixelDebug.slang`](Source/Falcor/Utils/Debug/PixelDebug.slang:1) - Pixel debug shader
- [`PixelDebugTypes.slang`](Source/Falcor/Utils/Debug/PixelDebugTypes.slang:1) - Pixel debug type definitions

**Purpose**: Shader debugging system with print() and assert() support.

**Key Features**:
- Shader-side print() statements
- Shader-side assert() statements
- Pixel selection via mouse click
- Async readback for minimal performance impact
- Type-safe print support (bool, int3, float2, uint4, etc.)
- Assert logging with coordinate tracking
- UI integration for debugging controls
- Configurable buffer capacities

**Core Methods**:

**Constructor**:
- [`PixelDebug(ref<Device> pDevice, uint32_t printCapacity = 100, uint32_t assertCapacity = 100)`](Source/Falcor/Utils/Debug/PixelDebug.h:74) - Constructor
  - Parameters:
    - `pDevice` - GPU device
    - `printCapacity` - Maximum number of print() statements per frame (default 100)
    - `assertCapacity` - Maximum number of assert() statements per frame (default 100)
  - Throws exception on error

**Frame Management**:
- [`beginFrame(RenderContext* pRenderContext, const uint2& frameDim)`](Source/Falcor/Utils/Debug/PixelDebug.h:76) - Begin frame for debugging
  - Parameters:
    - `pRenderContext` - Render context
    - `frameDim` - Frame dimensions (width, height)

- [`endFrame(RenderContext* pRenderContext)`](Source/Falcor/Utils/Debug/PixelDebug.h:77) - End frame and trigger readback
  - Parameters:
    - `pRenderContext` - Render context

**Program Preparation**:
- [`prepareProgram(const ref<Program>& pProgram, const ShaderVar& var)`](Source/Falcor/Utils/Debug/PixelDebug.h:83) - Prepare program for debugging
  - Parameters:
    - `pProgram` - Shader program to prepare
    - `var` - Shader variable to bind debug resources to
  - Performs program specialization and binds resources
  - Doesn't change resource declarations

**UI Integration**:
- [`renderUI(Gui::Widgets& widget)`](Source/Falcor/Utils/Debug/PixelDebug.h:86) - Render UI (reference overload)
- [`renderUI(Gui::Widgets* widget = nullptr)`](Source/Falcor/Utils/Debug/PixelDebug.h:86) - Render UI (pointer overload)
  - Displays print output and assert log
  - Shows selected pixel coordinates
  - Provides debugging controls

**Event Handling**:
- [`onMouseEvent(const MouseEvent& mouseEvent)`](Source/Falcor/Utils/Debug/PixelDebug.h:87) - Handle mouse events
  - Left click to select pixel
  - Updates selected pixel coordinates

**Control**:
- [`enable()`](Source/Falcor/Utils/Debug/PixelDebug.h:89) - Enable debugging features

**Data Structures**:

**PixelDebug Class**:
- [`mpDevice`](Source/Falcor/Utils/Debug/PixelDebug.h:95) - Device reference
- [`mpReflectProgram`](Source/Falcor/Utils/Debug/PixelDebug.h:96) - Program for type reflection
- [`mpCounterBuffer`](Source/Falcor/Utils/Debug/PixelDebug.h:97) - Counter buffer (print, assert) on GPU
- [`mpPrintBuffer`](Source/Falcor/Utils/Debug/PixelDebug.h:98) - Print buffer on GPU
- [`mpAssertBuffer`](Source/Falcor/Utils/Debug/PixelDebug.h:99) - Assert buffer on GPU
- [`mpReadbackBuffer`](Source/Falcor/Utils/Debug/PixelDebug.h:100) - Staging buffer for async readback
- [`mpFence`](Source/Falcor/Utils/Debug/PixelDebug.h:101) - GPU fence for synchronizing readback

**Configuration**:
- [`mEnabled`](Source/Falcor/Utils/Debug/PixelDebug.h:104) - Enable debugging features
- [`mSelectedPixel`](Source/Falcor/Utils/Debug/PixelDebug.h:105) - Currently selected pixel (x, y)
- [`mPrintCapacity`](Source/Falcor/Utils/Debug/PixelDebug.h:119) - Capacity of print buffer in elements
- [`mAssertCapacity`](Source/Falcor/Utils/Debug/PixelDebug.h:120) - Capacity of assert buffer in elements

**Runtime Data**:
- [`mFrameDim`](Source/Falcor/Utils/Debug/PixelDebug.h:108) - Frame dimensions (width, height)
- [`mRunning`](Source/Falcor/Utils/Debug/PixelDebug.h:110) - True when data collection is running
- [`mWaitingForData`](Source/Falcor/Utils/Debug/PixelDebug.h:111) - True if waiting for data on GPU
- [`mDataValid`](Source/Falcor/Utils/Debug/PixelDebug.h:112) - True if data has been read back and is valid
- [`mHashToString`](Source/Falcor/Utils/Debug/PixelDebug.h:114) - Map of string hashes to string values
- [`mPrintData`](Source/Falcor/Utils/Debug/PixelDebug.h:116) - Print data read back from GPU
- [`mAssertData`](Source/Falcor/Utils/Debug/PixelDebug.h:117) - Assert log data read back from GPU

**Technical Details**:

**Host-Side Integration**:
1. Create PixelDebug object
2. Call beginFrame()/endFrame() before and after executing programs with debugging
3. Call prepareProgram() before launching a program to use debugging
4. Call onMouseEvent() and renderUI() from respective callbacks in render pass

**Runtime Usage**:
1. Import PixelDebug.slang in shader
2. Use printSetPixel() in shader to set current pixel
3. Use print() in shader to output values for selected pixel
4. All basic types supported (bool, int3, float2, uint4, etc.)
5. Click left mouse button (or edit coords) to select pixel
6. Use assert() in shader to test condition for being true
7. All pixels tested, failed asserts logged
8. Coordinates of triggering asserts can be used with print() for further debugging

**Performance**:
- Shader code disabled (using macros) when debugging is off
- When enabled, async readback used with minor perf loss
- Configurable buffer capacities to balance memory vs. functionality

**Shader Integration**:
- printSetPixel() - Set current pixel for debugging
- print() - Output values for selected pixel
- assert() - Test condition, log failures with coordinates

**Use Cases**:
- Shader debugging
- Pixel-level analysis
- Condition testing in shaders
- Visual debugging
- Development workflow optimization

### WarpProfiler

**Files**:
- [`WarpProfiler.h`](Source/Falcor/Utils/Debug/WarpProfiler.h:1) - Warp profiler header
- [`WarpProfiler.cpp`](Source/Falcor/Utils/Debug/WarpProfiler.cpp:1) - Warp profiler implementation
- [`WarpProfiler.slang`](Source/Falcor/Utils/Debug/WarpProfiler.slang:1) - Warp profiler shader

**Purpose**: Warp-level profiling for GPU performance analysis.

**Key Features**:
- Warp-level execution tracking
- Multiple profiling bins
- Histogram generation
- CSV export support
- Async readback for minimal overhead
- Configurable bin count

**Core Methods**:

**Constructor**:
- [`WarpProfiler(ref<Device> pDevice, const uint32_t binCount)`](Source/Falcor/Utils/Debug/WarpProfiler.h:56) - Constructor
  - Parameters:
    - `pDevice` - GPU device
    - `binCount` - Number of profiling bins

**Shader Binding**:
- [`bindShaderData(const ShaderVar& var) const`](Source/Falcor/Utils/Debug/WarpProfiler.h:63) - Bind profiler data to shader vars
  - Must be called before profiler can be used
  - Parameters:
    - `var` - Shader vars of program to set data into

**Profiling Control**:
- [`begin(RenderContext* pRenderContext)`](Source/Falcor/Utils/Debug/WarpProfiler.h:69) - Begin profiling
  - Parameters:
    - `pRenderContext` - Render context

- [`end(RenderContext* pRenderContext)`](Source/Falcor/Utils/Debug/WarpProfiler.h:75) - End profiling
  - Parameters:
    - `pRenderContext` - Render context

**Histogram Analysis**:
- [`getWarpHistogram(const uint32_t binIndex, const uint32_t binCount = 1)`](Source/Falcor/Utils/Debug/WarpProfiler.h:84) - Compute warp histogram
  - Parameters:
    - `binIndex` - Index of first profiling bin for histogram
    - `binCount` - Number of profiling bins to include in histogram (default 1)
  - Returns: Histogram with `kWarpSize` buckets
    - First bucket: number of warps with 1 counted element
    - Last bucket: number of warps with `kWarpSize` counted elements

**Export**:
- [`saveWarpHistogramsAsCSV(const std::filesystem::path& path)`](Source/Falcor/Utils/Debug/WarpProfiler.h:91) - Save warp histograms to CSV file
  - Parameters:
    - `path` - File path for CSV output
  - Returns: True if successful, false otherwise

**Data Structures**:

**WarpProfiler Class**:
- [`mpFence`](Source/Falcor/Utils/Debug/WarpProfiler.h:96) - GPU fence for synchronization
- [`mpHistogramBuffer`](Source/Falcor/Utils/Debug/WarpProfiler.h:97) - Histogram buffer on GPU
- [`mpHistogramStagingBuffer`](Source/Falcor/Utils/Debug/WarpProfiler.h:98) - Staging buffer for readback
- [`mBinCount`](Source/Falcor/Utils/Debug/WarpProfiler.h:100) - Number of profiling bins
- [`mHistograms`](Source/Falcor/Utils/Debug/WarpProfiler.h:101) - Histograms for all profiling bins

**Runtime State**:
- [`mActive`](Source/Falcor/Utils/Debug/WarpProfiler.h:103) - True while inside begin()/end() section
- [`mDataWaiting`](Source/Falcor/Utils/Debug/WarpProfiler.h:104) - True when data is waiting for readback

**Constants**:
- [`kWarpSize`](Source/Falcor/Utils/Debug/WarpProfiler.h:49) - Warp size (32, do not change)

**Technical Details**:

**Warp-Level Profiling**:
- Tracks execution at warp granularity (32 threads)
- Measures thread divergence
- Identifies inefficient execution patterns
- Provides histogram of warp activity

**Histogram Structure**:
- `kWarpSize` buckets (32)
- Each bucket counts warps with specific active thread counts
- Bucket 0: warps with 1 active thread
- Bucket 31: warps with 32 active threads (fully active)

**Profiling Workflow**:
1. Create WarpProfiler with desired bin count
2. Bind shader data with bindShaderData()
3. Call begin() before profiling section
4. Execute shader code with warp profiling
5. Call end() after profiling section
6. Read back data with getWarpHistogram()
7. Analyze or export results

**Performance Analysis**:
- High divergence: many warps with low active thread counts
- Good utilization: many warps with high active thread counts
- Optimize: reduce branching, improve memory access patterns

**Use Cases**:
- GPU performance optimization
- Thread divergence analysis
- Shader optimization
- Performance profiling
- Kernel efficiency analysis

### DebugConsole

**Files**:
- [`DebugConsole.h`](Source/Falcor/Utils/Debug/DebugConsole.h:1) - Debug console header

**Purpose**: Windows debug console with stream redirection.

**Key Features**:
- Opens console window
- Redirects std::cout, std::cerr, std::cin
- Automatic cleanup on destruction
- Optional wait for key press
- Stream restoration on exit

**Core Methods**:

**Constructor**:
- [`DebugConsole(bool waitForKey = true)`](Source/Falcor/Utils/Debug/DebugConsole.h:52) - Constructor
  - Parameters:
    - `waitForKey` - If true, console waits for key press before closing (default true)
  - Opens console window
  - Redirects standard streams to console

**Destructor**:
- [`~DebugConsole()`](Source/Falcor/Utils/Debug/DebugConsole.h:75) - Destructor
  - Flushes streams
  - Waits for key press if enabled
  - Restores original streams
  - Closes console window

**Control**:
- [`pause() const`](Source/Falcor/Utils/Debug/DebugConsole.h:96) - Pause and wait for key press
  - Displays "Press any key to continue..."
  - Waits for user input

- [`flush() const`](Source/Falcor/Utils/Debug/DebugConsole.h:103) - Flush output streams
  - Flushes std::cout and std::cerr

**Data Structures**:

**DebugConsole Class**:
- [`mCout`](Source/Falcor/Utils/Debug/DebugConsole.h:110) - Output file stream for cout
- [`mCerr`](Source/Falcor/Utils/Debug/DebugConsole.h:111) - Output file stream for cerr
- [`mCin`](Source/Falcor/Utils/Debug/DebugConsole.h:112) - Input file stream for cin
- [`mPrevCout`](Source/Falcor/Utils/Debug/DebugConsole.h:113) - Previous cout stream buffer
- [`mPrevCerr`](Source/Falcor/Utils/Debug/DebugConsole.h:114) - Previous cerr stream buffer
- [`mPrevCin`](Source/Falcor/Utils/Debug/DebugConsole.h:115) - Previous cin stream buffer
- [`mWaitForKey`](Source/Falcor/Utils/Debug/DebugConsole.h:118) - Wait for key before closing

**Technical Details**:

**Platform**:
- Windows-only (FALCOR_WINDOWS)
- Uses Windows API (AllocConsole, FreeConsole)
- Console output: "CONOUT$"
- Console error: "CONERR$"
- Console input: "CONIN$"

**Stream Redirection**:
- std::cout redirected to console output
- std::cerr redirected to console error
- std::cin redirected to console input
- Original stream buffers saved for restoration

**Lifecycle**:
1. Constructor opens console and redirects streams
2. Application writes to std::cout/std::cerr
3. Output appears in console window
4. Destructor flushes, waits for key (optional), restores streams, closes console

**Use Cases**:
- Debugging Windows applications
- Console output for GUI applications
- Development workflow
- Logging and debugging
- User interaction in console

## Architecture Patterns

### Debugging Pattern
- Shader-side debugging support
- Async readback for minimal overhead
- Type-safe operations
- Configurable capacities

### Profiling Pattern
- GPU-side data collection
- Async readback
- Histogram analysis
- CSV export support

### Console Pattern
- RAII-based resource management
- Stream redirection
- Automatic cleanup
- Platform-specific implementation

## Technical Details

### Pixel Debug Implementation

**GPU Buffers**:
- Counter buffer: tracks print/assert counts
- Print buffer: stores print records
- Assert buffer: stores assert records
- Readback buffer: staging for async transfer

**Synchronization**:
- GPU fence for readback synchronization
- Async transfer to minimize overhead
- Data validity tracking

**Shader Integration**:
- Macros for conditional compilation
- Type-safe print functions
- Assert with coordinate tracking
- Pixel selection system

### Warp Profiler Implementation

**Warp Tracking**:
- 32 threads per warp (kWarpSize)
- Histogram of active thread counts
- Multiple profiling bins
- Efficient GPU-side counting

**Data Flow**:
1. Shader increments counters in histogram buffer
2. End profiling triggers readback
3. Staging buffer used for async transfer
4. Fence ensures synchronization
5. CPU processes histogram data

**Analysis**:
- Bucket 0: warps with 1 active thread (high divergence)
- Bucket 31: warps with 32 active threads (perfect utilization)
- Identify optimization opportunities

### Debug Console Implementation

**Windows API**:
- AllocConsole: Create console window
- FreeConsole: Close console window
- CONOUT$: Console output stream
- CONERR$: Console error stream
- CONIN$: Console input stream

**Stream Management**:
- Save original stream buffers
- Redirect to console streams
- Restore on destruction
- RAII-based cleanup

## Progress Log

- **2026-01-07T19:11:19Z**: Debug sub-module analysis completed. Analyzed PixelDebug, WarpProfiler, and DebugConsole classes. Documented pixel debugging system with shader print/assert support, warp-level profiling for GPU performance analysis, and Windows debug console with stream redirection. Created comprehensive technical specification with detailed code patterns, data structures, and use cases.

## Next Steps

Proceed to analyze Image sub-module to understand image processing utilities (AsyncTextureLoader, Bitmap, ImageIO, ImageProcessing, TextureAnalyzer, TextureManager).
