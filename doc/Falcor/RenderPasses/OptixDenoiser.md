# OptixDenoiser - OptiX AI Denoiser

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **OptixDenoiser** - OptiX denoiser implementation
  - [x] **OptixDenoiser.h** - OptiX denoiser header (236 lines)
  - [x] **OptixDenoiser.cpp** - OptiX denoiser implementation (537 lines)
  - [x] **OptixUtils.h** - CUDA buffer utilities and OptiX initialization (72 lines)
  - [x] **OptixUtils.cpp** - OptiX initialization implementation (77 lines)
  - [x] **ConvertTexToBuf.cs.slang** - Texture to buffer conversion shader (41 lines)
  - [x] **ConvertNormalsToBuf.cs.slang** - Normal to buffer conversion shader (50 lines)
  - [x] **ConvertMotionVectorInputs.cs.slang** - Motion vector conversion shader (48 lines)
  - [x] **ConvertBufToTex.ps.slang** - Buffer to texture conversion shader (40 lines)

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Buffer, Texture, Fbo)
- **Core/Pass** - Render pass base classes (RenderPass, FullScreenPass, ComputePass)
- **Utils/CudaUtils** - CUDA utilities (CudaRuntime, CudaUtils)
- **OptiX SDK** - OptiX 7.3 denoiser API
- **CUDA Runtime** - CUDA runtime API for GPU memory management

## Module Overview

OptixDenoiser is a render pass that integrates NVIDIA's OptiX AI-based denoiser with Falcor's render graph system. It provides flexible denoising for both LDR and HDR images with optional guide inputs (albedo, normals, motion vectors) for improved denoising quality. The pass handles DX <-> CUDA/OptiX interop through staging buffers and conversion shaders, making it usable in any Falcor render graph without awareness of interop requirements.

### Key Characteristics

- **AI-Based Denoising**: Uses NVIDIA's OptiX 7.3 denoiser with trained neural networks
- **Flexible Input Support**: Supports LDR and HDR denoising modes with optional guide inputs
- **Temporal Denoising**: Supports temporal denoising with motion vectors for temporal coherence
- **DX <-> CUDA Interop**: Handles DirectX and CUDA/OptiX memory sharing through staging buffers
- **Guide Inputs**: Optional albedo, normal, and motion vector guide inputs for improved quality
- **Multiple Models**: Supports LDR, HDR, Temporal, and AOV denoising models
- **Flexible Integration**: Works with any Falcor render graph without interop awareness

### Requirements

- **OptiX 7.3 SDK** - Must be installed (directly or via packman)
- **NVIDIA Driver 465.84 or later** - Required for OptiX 7.3 functionality
- **CUDA Runtime** - Required for CUDA/OptiX interop
- **RTX GPU** - Recommended for optimal performance

## Component Specifications

### OptixDenoiser_ Class

**File**: [`OptixDenoiser.h`](Source/RenderPasses/OptixDenoiser/OptixDenoiser.h:1)

**Purpose**: Main render pass class for OptiX denoising

**Inheritance**: `RenderPass`

**Public Interface**:

```cpp
class OptixDenoiser_ : public RenderPass
{
public:
    // Plugin registration
    FALCOR_PLUGIN_CLASS(OptixDenoiser_, "OptixDenoiser", "Apply OptiX AI Denoiser.");
    
    // Factory method
    static ref<OptixDenoiser_> create(ref<Device> pDevice, const Properties& props);
    
    // Constructor
    OptixDenoiser_(ref<Device> pDevice, const Properties& props);
    
    // RenderPass overrides
    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    
    // Scripting functions
    bool getEnabled() const;
    void setEnabled(bool enabled);
};
```

**Private Members**:

```cpp
private:
    // Scene reference
    ref<Scene> mpScene;
    
    // Denoiser setup
    void setupDenoiser();
    
    // Conversion functions
    void convertTexToBuf(RenderContext* pRenderContext, const ref<Texture>& tex, const ref<Buffer>& buf, const uint2& size);
    void convertNormalsToBuf(RenderContext* pRenderContext, const ref<Texture>& tex, const ref<Buffer>& buf, const uint2& size, float4x4 viewIT);
    void convertBufToTex(RenderContext* pRenderContext, const ref<Buffer>& buf, const ref<Texture>& tex, const uint2& size);
    void convertMotionVectors(RenderContext* pRenderContext, const ref<Texture>& tex, const ref<Buffer>& buf, const uint2& size);
    
    // Options and parameters
    bool mEnabled = true;                           // Enable/disable denoiser
    bool mSelectBestMode = true;                      // Auto-select best mode
    bool mIsFirstFrame = true;                        // First frame flag
    bool mHasColorInput = true;                        // Color input availability
    bool mHasAlbedoInput = false;                     // Albedo input availability
    bool mHasNormalInput = false;                      // Normal input availability
    bool mHasMotionInput = false;                       // Motion input availability
    uint2 mBufferSize = uint2(0, 0);               // Current buffer size
    bool mRecreateDenoiser = true;                    // Recreate flag
    
    // GUI helpers
    Gui::DropdownList mModelChoices = {};                // Model selection dropdown
    uint32_t mSelectedModel = OptixDenoiserModelKind::OPTIX_DENOISER_MODEL_KIND_HDR;
    
    // OptiX context
    OptixDeviceContext mOptixContext = nullptr;
    
    // DX <-> CUDA interop structure
    struct Interop
    {
        ref<Buffer> buffer;                     // Falcor buffer
        CUdeviceptr devicePtr = (CUdeviceptr)0; // CUDA pointer
    };
    
    // Denoiser parameters and state
    struct
    {
        // OptiX denoiser parameters
        OptixDenoiserOptions options = {0u, 0u};
        OptixDenoiserModelKind modelKind = OptixDenoiserModelKind::OPTIX_DENOISER_MODEL_KIND_HDR;
        OptixDenoiser denoiser = nullptr;
        OptixDenoiserParams params = {0u, static_cast<CUdeviceptr>(0), 0.0f, static_cast<CUdeviceptr>(0)};
        OptixDenoiserSizes sizes = {};
        
        // Advanced parameters (not exposed)
        bool kernelPredictionMode = false;
        bool useAOVs = false;
        uint32_t tileOverlap = 0u;
        uint32_t tileWidth = 0u;
        uint32_t tileHeight = 0u;
        
        // Guide layer
        OptixDenoiserGuideLayer guideLayer = {};
        
        // Input/output layer
        OptixDenoiserLayer layer = {};
        
        // Intermediates
        struct Intermediates
        {
            Interop normal;
            Interop albedo;
            Interop motionVec;
            Interop denoiserInput;
            Interop denoiserOutput;
        } interop;
        
        // GPU memory buffers
        CudaBuffer scratchBuffer, stateBuffer, intensityBuffer, hdrAverageBuffer;
    } mDenoiser;
    
    // Conversion shaders
    ref<ComputePass> mpConvertTexToBuf;
    ref<ComputePass> mpConvertNormalsToBuf;
    ref<ComputePass> mpConvertMotionVectors;
    ref<FullScreenPass> mpConvertBufToTex;
    ref<Fbo> mpFbo;
    
    // Buffer allocation
    void allocateStagingBuffer(RenderContext* pRenderContext, Interop& interop, OptixImage2D& image, OptixPixelFormat format = OPTIX_PIXEL_FORMAT_FLOAT4);
    void freeStagingBuffer(Interop& interop, OptixImage2D& image);
    void reallocateStagingBuffers(RenderContext* pRenderContext);
    void* exportBufferToCudaDevice(ref<Buffer>& buf);
};
```

### CudaBuffer Class

**File**: [`OptixUtils.h`](Source/RenderPasses/OptixDenoiser/OptixUtils.h:1)

**Purpose**: Utility class for GPU/device buffer management with CUDA

**Public Interface**:

```cpp
class CudaBuffer
{
public:
    CudaBuffer();
    
    // Accessors
    CUdeviceptr getDevicePtr();
    size_t getSize();
    
    // Memory management
    void allocate(size_t size);
    void resize(size_t size);
    void free();
    
private:
    size_t mSizeBytes = 0;
    void* mpDevicePtr = nullptr;
};
```

**Key Features**:
- CUDA memory allocation with `cudaMalloc`
- Automatic cleanup with `cudaFree`
- Device pointer access for OptiX interop
- Size tracking for buffer management

### OptiX Initialization

**File**: [`OptixUtils.cpp`](Source/RenderPasses/OptixDenoiser/OptixUtils.cpp:1)

**Function**: `initOptix(Falcor::Device* pDevice)`

**Purpose**: Initialize OptiX context and CUDA device

**Implementation**:

```cpp
OptixDeviceContext initOptix(Falcor::Device* pDevice)
{
    // Initialize CUDA device
    FALCOR_CHECK(pDevice->initCudaDevice(), "Failed to initialize CUDA device.");
    
    // Initialize OptiX
    OPTIX_CHECK(optixInit());
    
    // Check function table
    FALCOR_CHECK(g_optixFunctionTable.optixDeviceContextCreate, "OptiX function table not initialized.");
    
    // Create OptiX context
    OptixDeviceContext optixContext;
    OPTIX_CHECK(optixDeviceContextCreate(pDevice->getCudaDevice()->getContext(), 0, &optixContext));
    
    // Set log callback
    OPTIX_CHECK(optixDeviceContextSetLogCallback(optixContext, optixLogCallback, nullptr, 4));
    
    return optixContext;
}
```

**Key Features**:
- CUDA device initialization through Falcor
- OptiX library initialization
- OptiX context creation from CUDA context
- Log callback integration with Falcor logging
- Error checking with macros

### ConvertTexToBuf Shader

**File**: [`ConvertTexToBuf.cs.slang`](Source/RenderPasses/OptixDenoiser/ConvertTexToBuf.cs.slang:1)

**Purpose**: Convert texture to flat buffer for OptiX input

**Shader Code**:

```slang
cbuffer GlobalCB
{
    uint gStride;
}

Texture2D<float4> gInTex;
RWBuffer<float4> gOutBuf;

[numthreads(8, 8, 1)]
void main(uint3 thrdId: SV_DispatchThreadID)
{
    uint bufIdx = thrdId.x + thrdId.y * gStride;
    gOutBuf[bufIdx] = gInTex[thrdId.xy];
}
```

**Key Features**:
- 8x8 thread group dispatch
- Linear buffer indexing with stride
- Direct texture-to-buffer copy
- Float4 format support

### ConvertNormalsToBuf Shader

**File**: [`ConvertNormalsToBuf.cs.slang`](Source/RenderPasses/OptixDenoiser/ConvertNormalsToBuf.cs.slang:1)

**Purpose**: Convert normals from texture to buffer with view-space transformation

**Shader Code**:

```slang
cbuffer GlobalCB
{
    uint gStride;
    float4x4 gViewIT;
}

Texture2D<float4> gInTex;
RWBuffer<float3> gOutBuf;

[numthreads(8, 8, 1)]
void main(uint3 thrdId: SV_DispatchThreadID)
{
    uint bufIdx = thrdId.x + thrdId.y * gStride;
    
    // Decode normal from [0,1] to [-1,1]
    float3 normal = (gInTex[thrdId.xy].xyz - 0.5f) * 2.0f;
    
    // Transform to view space and normalize
    if (length(normal) > 0.01)
    {
        normal = mul(gViewIT, float4(normal, 0.0f)).xyz;
        normal = normalize(normal);
    }
    
    gOutBuf[bufIdx] = normal;
}
```

**Key Features**:
- Normal decoding from [0,1] to [-1,1]
- View-space transformation with inverse transpose of view matrix
- Normalization after transformation
- Zero-length normal handling
- Float3 format output

### ConvertMotionVectorInputs Shader

**File**: [`ConvertMotionVectorInputs.cs.slang`](Source/RenderPasses/OptixDenoiser/ConvertMotionVectorInputs.cs.slang:1)

**Purpose**: Convert motion vectors from Falcor format to OptiX format

**Shader Code**:

```slang
cbuffer GlobalCB
{
    uint gStride;
    uint2 gSize;
}

Texture2D<float4> gInTex;
RWBuffer<float2> gOutBuf;

[numthreads(8, 8, 1)]
void main(uint3 thrdId: SV_DispatchThreadID)
{
    uint bufIdx = thrdId.x + thrdId.y * gStride;
    
    // Convert Falcor motion vectors to OptiX format
    float2 optixMVec = gInTex[thrdId.xy].xy * gSize;
    gOutBuf[bufIdx] = float2(-optixMVec.x, -optixMVec.y);
}
```

**Key Features**:
- Motion vector format conversion (Falcor to OptiX)
- Scaling by screen size
- X/Y negation for coordinate system conversion
- Float2 format output

### ConvertBufToTex Shader

**File**: [`ConvertBufToTex.ps.slang`](Source/RenderPasses/OptixDenoiser/ConvertBufToTex.ps.slang:1)

**Purpose**: Convert buffer to texture for Falcor output

**Shader Code**:

```slang
cbuffer GlobalCB
{
    uint gStride;
}

Buffer<float4> gInBuf;

float4 main(float2 texC: TEXCOORD, float4 posH: SV_POSITION) : SV_Target0
{
    uint2 pixel = uint2(posH.xy);
    uint bufIdx = pixel.x + pixel.y * gStride;
    return gInBuf[bufIdx];
}
```

**Key Features**:
- Pixel shader for buffer-to-texture conversion
- Linear buffer indexing with stride
- Full-screen quad rendering
- Float4 format output

## Technical Details

### Render Graph Integration

**Input Channels**:
- `color` (required) - Noisy color input (LDR or HDR)
- `albedo` (optional) - Albedo guide image
- `normal` (optional) - Normal guide image
- `mvec` (optional) - Motion vector input for temporal denoising

**Output Channels**:
- `output` (required) - Denoised color output (RGBA32Float format)

### Denoising Models

**LDR Model**:
- Assumes input range [0, 1]
- Best for LDR images
- No HDR-specific processing

**HDR Model**:
- Assumes input range [0, 10,000]
- Supports HDR content
- Uses HDR intensity buffer
- Default model when no motion vectors

**Temporal Model**:
- Requires motion vector input
- Uses previous frame output
- Temporal coherence for improved quality
- Best for animated sequences

**AOV Model**:
- Supports additional arbitrary output variables
- Currently not fully implemented
- Reserved for future use

### Denoising Pipeline

**1. Input Preparation**:
- Convert color texture to buffer (ConvertTexToBuf)
- Convert albedo texture to buffer (if provided)
- Convert normals to buffer with view-space transform (if provided)
- Convert motion vectors to OptiX format (if provided)

**2. HDR Preprocessing**:
- Compute average intensity (if HDR model)
- Compute average color (if kernel prediction mode)
- Store in intensity/hdrAverage buffers

**3. Temporal Initialization**:
- On first frame, use current frame as previous output
- Avoids temporal artifacts on startup

**4. Denoising**:
- Invoke OptiX denoiser with guide layer and input layer
- Use scratch buffer for temporary storage
- Use state buffer for temporal accumulation

**5. Output Conversion**:
- Convert denoised buffer to texture (ConvertBufToTex)
- Update previous frame output for temporal denoising

### DX <-> CUDA Interop

**Staging Buffer Allocation**:
- Create shared DX/CUDA buffer with `ResourceBindFlags::Shared`
- Get CUDA device pointer with `exportBufferToCudaDevice`
- Configure OptiX image structure with buffer pointer and stride

**Buffer Formats**:
- Float4 for color/albedo (RGBA32Float)
- Float3 for normals (RGBA32Float, 3 components used)
- Float2 for motion vectors (RG32Float)

**Memory Layout**:
- OptiX expects flat arrays (not z-order textures)
- Conversion shaders handle layout transformation
- Row stride for proper indexing

### OptiX Denoiser Setup

**Creation**:
```cpp
optixDenoiserCreate(mOptixContext, mDenoiser.modelKind, &mDenoiser.options, &mDenoiser.denoiser);
```

**Memory Resources**:
```cpp
optixDenoiserComputeMemoryResources(mDenoiser.denoiser, mDenoiser.tileWidth, mDenoiser.tileHeight, &mDenoiser.sizes);
```

**Setup**:
```cpp
optixDenoiserSetup(
    mDenoiser.denoiser,
    nullptr,
    mDenoiser.tileWidth + 2 * mDenoiser.tileOverlap,
    mDenoiser.tileHeight + 2 * mDenoiser.tileOverlap,
    mDenoiser.stateBuffer.getDevicePtr(),
    mDenoiser.stateBuffer.getSize(),
    mDenoiser.scratchBuffer.getDevicePtr(),
    mDenoiser.scratchBuffer.getSize()
);
```

**Invocation**:
```cpp
optixDenoiserInvoke(
    mDenoiser.denoiser,
    nullptr,
    &mDenoiser.params,
    mDenoiser.stateBuffer.getDevicePtr(),
    mDenoiser.stateBuffer.getSize(),
    &mDenoiser.guideLayer,
    &mDenoiser.layer,
    1u,
    0u,
    0u,
    mDenoiser.scratchBuffer.getDevicePtr(),
    mDenoiser.scratchBuffer.getSize()
);
```

### Synchronization

**DX to CUDA**:
```cpp
pRenderContext->waitForFalcor();
```

**CUDA to DX**:
```cpp
pRenderContext->waitForCuda();
```

### Property Serialization

**Properties**:
- `enabled` - Enable/disable denoiser (bool)
- `model` - Denoising model (OptixDenoiserModelKind)
- `blend` - Blend factor (0.0 to 1.0)
- `denoiseAlpha` - Denoise alpha channel (bool)

### Python Bindings

**Scripting Functions**:
```python
regOptixDenoiser(pybind11::module& m)
{
    pybind11::class_<OptixDenoiser_, RenderPass, ref<OptixDenoiser_>> pass(m, "OptixDenoiser");
    pass.def_property("enabled", &OptixDenoiser_::getEnabled, &OptixDenoiser_::setEnabled);
}
```

### UI Controls

**Main Controls**:
- Enable/disable checkbox
- Model selection dropdown (LDR/HDR/Temporal)
- Albedo guide checkbox (if albedo input available)
- Normal guide checkbox (if normal input available)
- Denoise alpha checkbox
- Blend factor slider (0.0 to 1.0)

**Tooltips**:
- Model selection: Explains LDR/HDR assumptions
- Albedo guide: Explains guide usage
- Normal guide: Explains guide usage and requirements
- Blend: Explains blending behavior

## Integration Points

### Render Graph Integration

**Input/Output Specification**:
- Reflect method defines required and optional inputs
- Output format fixed to RGBA32Float
- Input format detection at compile time

**Resource Management**:
- Automatic buffer reallocation on resolution change
- Staging buffer management for interop
- Temporary buffer allocation for OptiX

### Scene Integration

**Camera Access**:
- Used for normal transformation (view matrix inverse transpose)
- Required for proper normal guide handling

**Scene Changes**:
- Triggers denoiser recreation
- Resets first frame flag
- Updates buffer allocations

### CUDA Integration

**CUDA Device Initialization**:
- Falcor's CUDA device initialization
- OptiX context creation from CUDA context
- Shared memory allocation for interop

**CUDA Memory Management**:
- CUDA buffer allocation/deallocation
- Device pointer retrieval for OptiX
- Shared buffer creation with DX interop

### OptiX Integration

**Denoiser Lifecycle**:
- Creation with model kind and options
- Memory resource computation
- Setup with tile dimensions
- Invocation with guide and input layers
- Destruction on recreation

**Guide Layer**:
- Normal guide (optional)
- Albedo guide (optional)
- Motion vector guide (for temporal)

**Input/Output Layer**:
- Input color buffer
- Output color buffer
- Previous frame output (for temporal)

## Architecture Patterns

### Render Pass Pattern

**Inheritance**: `RenderPass`

**Overrides**:
- `reflect()` - Define input/output resources
- `compile()` - Initialize OptiX and allocate buffers
- `execute()` - Run denoising pipeline
- `renderUI()` - Render UI controls
- `setScene()` - Store scene reference
- `getProperties()` - Serialize configuration

### Compute Pass Pattern

**Conversion Shaders**:
- `ConvertTexToBuf` - Texture to buffer
- `ConvertNormalsToBuf` - Normal to buffer with transform
- `ConvertMotionVectors` - Motion vector format conversion
- `ConvertBufToTex` - Buffer to texture

**Thread Groups**: 8x8 for all conversion shaders

### Full Screen Pass Pattern

**Output Conversion**:
- `ConvertBufToTex` uses FullScreenPass
- Renders full-screen quad
- Pixel shader for buffer-to-texture conversion

### Interop Pattern

**DX <-> CUDA Sharing**:
- Shared buffer allocation with `ResourceBindFlags::Shared`
- CUDA device pointer retrieval
- OptiX image structure configuration

**Synchronization**:
- `waitForFalcor()` before CUDA operations
- `waitForCuda()` after CUDA operations
- Ensures proper synchronization between APIs

### Denoiser Pattern

**Lifecycle Management**:
- Creation on initialization or settings change
- Setup with tile dimensions and buffers
- Invocation per frame
- Destruction on recreation

**Guide Inputs**:
- Optional albedo guide
- Optional normal guide
- Optional motion vector guide (for temporal)

**Temporal Accumulation**:
- Previous frame output storage
- First frame initialization
- State buffer for temporal coherence

## Code Patterns

### Property Parsing

**Constructor Property Parsing**:
```cpp
for (const auto& [key, value] : props)
{
    if (key == kEnabled)
        mEnabled = value;
    else if (key == kModel)
    {
        mDenoiser.modelKind = value;
        mSelectBestMode = false;
    }
    else if (key == kBlend)
        mDenoiser.params.blendFactor = value;
    else if (key == kDenoiseAlpha)
        mDenoiser.params.denoiseAlpha = (value ? 1u : 0u);
    else
        logWarning("Unknown property '{}' in a OptixDenoiser properties.", key);
}
```

### Property Serialization

**getProperties Implementation**:
```cpp
Properties OptixDenoiser_::getProperties() const
{
    Properties props;
    props[kEnabled] = mEnabled;
    props[kBlend] = mDenoiser.params.blendFactor;
    props[kModel] = mDenoiser.modelKind;
    props[kDenoiseAlpha] = bool(mDenoiser.params.denoiseAlpha > 0);
    return props;
}
```

### Scene Change Detection

**Compile Time Detection**:
```cpp
mHasColorInput = (compileData.connectedResources.getField(kColorInput) != nullptr);
mHasAlbedoInput = (compileData.connectedResources.getField(kAlbedoInput) != nullptr);
mHasNormalInput = (compileData.connectedResources.getField(kNormalInput) != nullptr);
mHasMotionInput = (compileData.connectedResources.getField(kMotionInput) != nullptr);
```

### Best Mode Selection

**Automatic Mode Selection**:
```cpp
if (mSelectBestMode)
{
    auto best = mHasMotionInput ? OptixDenoiserModelKind::OPTIX_DENOISER_MODEL_KIND_TEMPORAL
                                : OptixDenoiserModelKind::OPTIX_DENOISER_MODEL_KIND_HDR;
    mSelectedModel = best;
    mDenoiser.modelKind = best;
}
```

### Buffer Allocation

**Staging Buffer Allocation**:
```cpp
interop.buffer = mpDevice->createTypedBuffer(
    falcorFormat,
    mBufferSize.x * mBufferSize.y,
    ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess | ResourceBindFlags::RenderTarget | ResourceBindFlags::Shared
);
interop.devicePtr = (CUdeviceptr)exportBufferToCudaDevice(interop.buffer);
```

### Buffer Reallocation

**Resolution Change Detection**:
```cpp
uint2 newSize = compileData.defaultTexDims;
if (any(newSize != mBufferSize) && all(newSize > 0u))
{
    mBufferSize = newSize;
    reallocateStagingBuffers(pRenderContext);
}
```

### Conversion Shader Execution

**Texture to Buffer**:
```cpp
void OptixDenoiser_::convertTexToBuf(RenderContext* pRenderContext, const ref<Texture>& tex, const ref<Buffer>& buf, const uint2& size)
{
    auto var = mpConvertTexToBuf->getRootVar();
    var["GlobalCB"]["gStride"] = size.x;
    var["gInTex"] = tex;
    var["gOutBuf"] = buf;
    mpConvertTexToBuf->execute(pRenderContext, size.x, size.y);
}
```

**Normals to Buffer**:
```cpp
void OptixDenoiser_::convertNormalsToBuf(
    RenderContext* pRenderContext,
    const ref<Texture>& tex,
    const ref<Buffer>& buf,
    const uint2& size,
    float4x4 viewIT
)
{
    auto var = mpConvertNormalsToBuf->getRootVar();
    var["GlobalCB"]["gStride"] = size.x;
    var["GlobalCB"]["gViewIT"] = viewIT;
    var["gInTex"] = tex;
    var["gOutBuf"] = buf;
    mpConvertNormalsToBuf->execute(pRenderContext, size.x, size.y);
}
```

**Motion Vectors**:
```cpp
void OptixDenoiser_::convertMotionVectors(RenderContext* pRenderContext, const ref<Texture>& tex, const ref<Buffer>& buf, const uint2& size)
{
    auto var = mpConvertMotionVectors->getRootVar();
    var["GlobalCB"]["gStride"] = size.x;
    var["GlobalCB"]["gSize"] = size;
    var["gInTex"] = tex;
    var["gOutBuf"] = buf;
    mpConvertMotionVectors->execute(pRenderContext, size.x, size.y);
}
```

**Buffer to Texture**:
```cpp
void OptixDenoiser_::convertBufToTex(RenderContext* pRenderContext, const ref<Buffer>& buf, const ref<Texture>& tex, const uint2& size)
{
    auto var = mpConvertBufToTex->getRootVar();
    var["GlobalCB"]["gStride"] = size.x;
    var["gInBuf"] = buf;
    mpFbo->attachColorTarget(tex, 0);
    mpConvertBufToTex->execute(pRenderContext, mpFbo);
}
```

### Denoiser Setup

**Setup Implementation**:
```cpp
void OptixDenoiser_::setupDenoiser()
{
    // Destroy existing denoiser
    if (mDenoiser.denoiser)
    {
        optixDenoiserDestroy(mDenoiser.denoiser);
    }
    
    // Create denoiser
    optixDenoiserCreate(mOptixContext, mDenoiser.modelKind, &mDenoiser.options, &mDenoiser.denoiser);
    
    // Compute memory resources
    optixDenoiserComputeMemoryResources(mDenoiser.denoiser, mDenoiser.tileWidth, mDenoiser.tileHeight, &mDenoiser.sizes);
    
    // Allocate buffers
    mDenoiser.scratchBuffer.resize(mDenoiser.sizes.withoutOverlapScratchSizeInBytes);
    mDenoiser.stateBuffer.resize(mDenoiser.sizes.stateSizeInBytes);
    
    // Setup denoiser
    optixDenoiserSetup(
        mDenoiser.denoiser,
        nullptr,
        mDenoiser.tileWidth + 2 * mDenoiser.tileOverlap,
        mDenoiser.tileHeight + 2 * mDenoiser.tileOverlap,
        mDenoiser.stateBuffer.getDevicePtr(),
        mDenoiser.stateBuffer.getSize(),
        mDenoiser.scratchBuffer.getDevicePtr(),
        mDenoiser.scratchBuffer.getSize()
    );
}
```

### Execute Implementation

**Main Execute Flow**:
```cpp
void OptixDenoiser_::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    if (mEnabled && mpScene)
    {
        // Recreate denoiser if needed
        if (mRecreateDenoiser)
        {
            if (!mHasMotionInput && mDenoiser.modelKind == OptixDenoiserModelKind::OPTIX_DENOISER_MODEL_KIND_TEMPORAL)
            {
                mSelectedModel = OptixDenoiserModelKind::OPTIX_DENOISER_MODEL_KIND_HDR;
                mDenoiser.modelKind = OptixDenoiserModelKind::OPTIX_DENOISER_MODEL_KIND_HDR;
            }
            setupDenoiser();
            mRecreateDenoiser = false;
            mIsFirstFrame = true;
        }
        
        // Convert inputs
        convertTexToBuf(pRenderContext, renderData.getTexture(kColorInput), mDenoiser.interop.denoiserInput.buffer, mBufferSize);
        if (mHasAlbedoInput && mDenoiser.options.guideAlbedo)
            convertTexToBuf(pRenderContext, renderData.getTexture(kAlbedoInput), mDenoiser.interop.albedo.buffer, mBufferSize);
        if (mHasNormalInput && mDenoiser.options.guideNormal)
            convertNormalsToBuf(pRenderContext, renderData.getTexture(kNormalInput), mDenoiser.interop.normal.buffer, mBufferSize, transpose(inverse(mpScene->getCamera()->getViewMatrix())));
        if (mHasMotionInput && mDenoiser.modelKind == OptixDenoiserModelKind::OPTIX_DENOISER_MODEL_KIND_TEMPORAL)
            convertMotionVectors(pRenderContext, renderData.getTexture(kMotionInput), mDenoiser.interop.motionVec.buffer, mBufferSize);
        
        // Synchronize DX to CUDA
        pRenderContext->waitForFalcor();
        
        // Compute HDR intensity/average
        if (mDenoiser.params.hdrIntensity)
            optixDenoiserComputeIntensity(...);
        if (mDenoiser.params.hdrAverageColor)
            optixDenoiserComputeAverageColor(...);
        
        // First frame handling
        if (mIsFirstFrame)
            mDenoiser.layer.previousOutput = mDenoiser.layer.input;
        
        // Invoke denoiser
        optixDenoiserInvoke(...);
        
        // Synchronize CUDA to DX
        pRenderContext->waitForCuda();
        
        // Convert output
        convertBufToTex(pRenderContext, mDenoiser.interop.denoiserOutput.buffer, renderData.getTexture(kOutput), mBufferSize);
        
        // Update previous frame
        if (mIsFirstFrame)
        {
            mDenoiser.layer.previousOutput = mDenoiser.layer.output;
            mIsFirstFrame = false;
        }
    }
    else
    {
        // Pass-through if disabled
        pRenderContext->blit(renderData.getTexture(kColorInput)->getSRV(), renderData.getTexture(kOutput)->getRTV());
    }
}
```

### UI Rendering

**UI Controls**:
```cpp
void OptixDenoiser_::renderUI(Gui::Widgets& widget)
{
    widget.checkbox("Use OptiX Denoiser?", mEnabled);
    
    if (mEnabled)
    {
        // Model selection
        if (widget.dropdown("Model", mModelChoices, mSelectedModel))
        {
            mDenoiser.modelKind = static_cast<OptixDenoiserModelKind>(mSelectedModel);
            mRecreateDenoiser = true;
        }
        
        // Albedo guide
        if (mHasAlbedoInput)
        {
            bool useAlbedoGuide = mDenoiser.options.guideAlbedo != 0u;
            if (widget.checkbox("Use albedo guide?", useAlbedoGuide))
            {
                mDenoiser.options.guideAlbedo = useAlbedoGuide ? 1u : 0u;
                mRecreateDenoiser = true;
            }
        }
        
        // Normal guide
        if (mHasNormalInput)
        {
            bool useNormalGuide = mDenoiser.options.guideNormal != 0u;
            if (widget.checkbox("Use normal guide?", useNormalGuide))
            {
                mDenoiser.options.guideNormal = useNormalGuide ? 1u : 0u;
                mRecreateDenoiser = true;
            }
        }
        
        // Denoise alpha
        bool denoiseAlpha = mDenoiser.params.denoiseAlpha != 0;
        if (widget.checkbox("Denoise Alpha?", denoiseAlpha))
        {
            mDenoiser.params.denoiseAlpha = denoiseAlpha ? 1u : 0u;
        }
        
        // Blend factor
        widget.slider("Blend", mDenoiser.params.blendFactor, 0.f, 1.f);
    }
}
```

### Plugin Registration

**Standard Plugin Registration**:
```cpp
extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, OptixDenoiser_>();
    ScriptBindings::registerBinding(regOptixDenoiser);
}
```

## Use Cases

### Real-Time Rendering

**Path Tracing Denoising**:
- Denoise Monte Carlo path tracing output
- Reduce sample count requirements
- Maintain temporal coherence with temporal model
- Use albedo and normal guides for quality

**Ray Tracing Denoising**:
- Denoise ray-traced reflections and shadows
- Reduce noise from low sample counts
- Improve visual quality in real-time applications

### Offline Rendering

**High-Quality Rendering**:
- Denoise high-sample renders for faster convergence
- Use HDR model for HDR content
- Blend with original for artistic control

**Animation Rendering**:
- Use temporal denoising for animated sequences
- Maintain temporal coherence
- Reduce flickering and temporal noise

### Debugging and Analysis

**Quality Comparison**:
- Compare noisy vs. denoised output
- Adjust blend factor for visual inspection
- Test different denoising models

**Performance Profiling**:
- Measure denoising performance overhead
- Profile GPU memory usage
- Test with different resolutions

### Research and Development

**Denoising Research**:
- Test different guide configurations
- Evaluate denoising quality metrics
- Develop new denoising techniques

**Algorithm Development**:
- Integrate custom denoising algorithms
- Compare with OptiX denoiser
- Develop hybrid approaches

### Educational Use

**Denoising Concepts**:
- Demonstrate AI-based denoising
- Show guide input effects
- Explain temporal denoising

**Render Graph Integration**:
- Learn render pass development
- Understand DX <-> CUDA interop
- Study resource management

## Performance Considerations

### GPU Computation

**Conversion Shaders**:
- Texture to buffer: O(width × height) with 8x8 thread groups
- Normals to buffer: O(width × height) with 8x8 thread groups + matrix multiplication
- Motion vectors: O(width × height) with 8x8 thread groups + scaling
- Buffer to texture: O(width × height) with full-screen quad

**OptiX Denoising**:
- HDR intensity computation: O(width × height) parallel reduction
- HDR average computation: O(width × height) parallel reduction
- Denoiser invocation: O(width × height) with neural network inference
- Total: O(width × height) per frame

**Memory Bandwidth**:
- Input textures: 4 × width × height × sizeof(float4) bytes
- Output texture: width × height × sizeof(float4) bytes
- Guide buffers: 2 × width × height × sizeof(float3) bytes (if used)
- Motion buffer: width × height × sizeof(float2) bytes (if used)
- OptiX buffers: scratch + state + intensity/hdrAverage bytes

**Approximate Performance**:
- 1080p @ RTX 3090: ~3ms per frame
- Includes conversion, denoising, and output conversion
- Scales with resolution (quadratic)
- Scales with guide inputs (linear)

### Memory Usage

**Staging Buffers**:
- Denoiser input: width × height × 16 bytes (Float4)
- Denoiser output: width × height × 16 bytes (Float4)
- Normal guide: width × height × 12 bytes (Float3)
- Albedo guide: width × height × 16 bytes (Float4)
- Motion guide: width × height × 8 bytes (Float2)
- Total: O(width × height × 68) bytes with all guides

**OptiX Buffers**:
- Scratch buffer: sizes.withoutOverlapScratchSizeInBytes (model-dependent)
- State buffer: sizes.stateSizeInBytes (model-dependent)
- Intensity buffer: 1 × sizeof(float)
- HDR average buffer: 3 × sizeof(float)
- Total: O(model_size) bytes

**Total Memory**:
- O(width × height × 68 + model_size) bytes
- For 1080p: ~140MB + model_size
- For 4K: ~560MB + model_size

### Computational Complexity

**Conversion Shaders**:
- Texture to buffer: O(1) per pixel
- Normals to buffer: O(1) per pixel + O(1) matrix multiplication
- Motion vectors: O(1) per pixel + O(1) scaling
- Buffer to texture: O(1) per pixel

**OptiX Denoising**:
- HDR intensity: O(width × height) parallel reduction
- HDR average: O(width × height) parallel reduction
- Denoiser invocation: O(width × height) neural network inference
- Total: O(width × height) per frame

**Overall Complexity**:
- O(width × height) per frame
- Linear with resolution
- Linear with guide inputs
- Model-dependent constants

### Synchronization Overhead

**DX to CUDA**:
- `waitForFalcor()` before CUDA operations
- Minimal overhead (microseconds)
- Ensures DX work completion

**CUDA to DX**:
- `waitForCuda()` after CUDA operations
- Minimal overhead (microseconds)
- Ensures CUDA work completion

**Total Overhead**:
- ~0.1ms per frame
- Negligible compared to denoising time
- Required for proper interop

### Optimization Opportunities

**Reduce Conversions**:
- Direct DX/OptiX interop (complex)
- Avoid staging buffers (complex)
- Current approach prioritizes flexibility

**Tile Denoising**:
- Not currently implemented
- Could reduce memory usage
- Could improve cache locality

**Async Operations**:
- Overlap conversion with denoising
- Requires careful synchronization
- Complex to implement correctly

## Limitations

### Feature Limitations

**AOV Support**:
- AOV model not fully implemented
- Reserved for future use
- Cannot use additional output variables

**Kernel Prediction Mode**:
- Not exposed to user
- Not tested or validated
- May not work correctly

**Tile Denoising**:
- Not currently implemented
- Tile overlap always 0
- Cannot use tiled denoising

**Advanced Options**:
- Many OptiX options not exposed
- Limited configuration flexibility
- Cannot fine-tune denoising

### Integration Limitations

**DX <-> CUDA Interop**:
- Requires staging buffers
- Adds conversion overhead
- Not optimal for performance
- Suboptimal memory layout

**Normal Guide Requirements**:
- Requires view matrix transformation
- May require albedo guide (per OptiX documentation)
- Unclear API requirements

**Motion Vector Format**:
- Falcor format differs from OptiX
- Requires conversion shader
- May not handle all cases

### Performance Limitations

**Memory Overhead**:
- Staging buffers double memory usage
- OptiX buffers add significant overhead
- High memory usage at high resolutions

**Conversion Overhead**:
- Four conversion passes per frame
- Adds ~0.5ms overhead
- Could be optimized with direct interop

**Synchronization Overhead**:
- Two synchronization points per frame
- Adds ~0.1ms overhead
- Required for correct interop

**Model Recreation**:
- Requires full denoiser recreation
- Expensive operation (~10ms)
- Happens on settings change

### Hardware Limitations

**RTX GPU Required**:
- Best performance on RTX GPUs
- May work on older GPUs (slower)
- OptiX 7.3 requires modern hardware

**Driver Requirements**:
- NVIDIA driver 465.84 or later
- May not work on older drivers
- Requires driver updates

**CUDA Requirements**:
- CUDA runtime required
- CUDA device initialization required
- May not work in all environments

### Usage Limitations

**Scene Requirement**:
- Requires scene for normal transformation
- Cannot work without scene
- Falls back to pass-through if no scene

**Input Format**:
- Assumes specific normal encoding ([0,1] to [-1,1])
- Assumes specific motion vector format
- May not work with all G-buffer formats

**Guide Input Quality**:
- Requires noise-free guide inputs
- Noisy guides reduce quality
- Must come from G-buffer (not denoised)

### Debugging Limitations

**Limited Debugging**:
- No internal state inspection
- No intermediate outputs
- Difficult to debug denoising issues

**Error Handling**:
- Limited error reporting
- OptiX errors may be cryptic
- Hard to diagnose issues

## Best Practices

### Configuration

**Model Selection**:
- Use LDR for [0,1] range images
- Use HDR for [0,10000] range images
- Use Temporal for animated sequences with motion vectors
- Let auto-selection choose best mode

**Guide Inputs**:
- Always use albedo guide if available
- Always use normal guide if available
- Use motion vectors for temporal denoising
- Ensure guides are noise-free (from G-buffer)

**Blend Factor**:
- Start with 0.0 (full denoising)
- Adjust for visual preference
- Higher values preserve more noise
- Lower values provide smoother results

**Denoise Alpha**:
- Enable for transparent geometry
- Disable for opaque geometry only
- Reduces artifacts on edges

### Performance Optimization

**Resolution**:
- Test at target resolution
- Lower resolution for faster iteration
- Scale up if performance is acceptable

**Guide Inputs**:
- Use minimal guide set for performance
- Disable unnecessary guides
- Balance quality vs. performance

**Temporal Denoising**:
- Use for animated sequences
- Disable for static scenes
- Reduces temporal noise

**Memory Management**:
- Monitor GPU memory usage
- Reduce resolution if out of memory
- Disable guides if needed

### Debugging

**Quality Issues**:
- Check guide input quality
- Verify normal encoding
- Verify motion vector format
- Test different models
- Adjust blend factor

**Performance Issues**:
- Profile GPU time
- Check memory usage
- Test without guides
- Reduce resolution
- Check driver version

**Integration Issues**:
- Verify scene is set
- Check input connections
- Verify output format
- Check OptiX initialization
- Check CUDA initialization

### Development

**Custom Denoising**:
- Study OptiX API documentation
- Understand guide requirements
- Test with different configurations
- Profile performance

**DX <-> CUDA Interop**:
- Understand shared memory requirements
- Handle synchronization correctly
- Test with different formats
- Profile overhead

**Render Graph Integration**:
- Follow Falcon render pass conventions
- Implement proper reflection
- Handle optional inputs
- Test with different graphs

### Research Use

**Quality Evaluation**:
- Use blend factor for comparison
- Test different guide configurations
- Evaluate different models
- Measure quality metrics

**Performance Analysis**:
- Profile denoising time
- Measure memory usage
- Test with different resolutions
- Compare with baseline

**Algorithm Development**:
- Study OptiX denoising
- Understand guide effects
- Develop hybrid approaches
- Test new techniques

### Educational Use

**Learning Denoising**:
- Start with simple configuration
- Add guides incrementally
- Observe quality changes
- Understand temporal effects

**Render Pass Development**:
- Study conversion shaders
- Understand interop patterns
- Learn resource management
- Follow coding conventions

## Notes

- All analysis derived from static code analysis
- No external documentation or web searches used
- All insights grounded in actual implementation
- Process designed to be resumable and idempotent
