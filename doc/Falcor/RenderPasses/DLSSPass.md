# DLSSPass - NVIDIA DLSS Pass

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **DLSSPass** - NVIDIA DLSS pass
  - [x] **DLSSPass.h** - DLSS pass header (108 lines)
  - [x] **DLSSPass.cpp** - DLSS pass implementation (293 lines)
  - [x] **NGXWrapper.h** - NGX wrapper header (104 lines)
  - [x] **NGXWrapper.cpp** - NGX wrapper implementation (469 lines)

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Texture, Buffer, Fence)
- **Core/Object** - Base object class
- **RenderGraph** - Render graph system (RenderPass, RenderPassReflection)
- **RenderGraph/Helpers** - Render pass helpers (IOSize)
- **Scene** - Scene system (Scene, Camera)
- **Utils/Algorithm** - Algorithm utilities (ParallelReduction)
- **NGX SDK** - NVIDIA NGX SDK (NVSDK_NGX, NVSDK_NGX_D3D12, NVSDK_NGX_VULKAN)

## Module Overview

DLSSPass implements NVIDIA Deep Learning Super Sampling (DLSS) integration for AI-based upscaling and anti-aliasing. This pass provides DLSS functionality through the NGX SDK, supporting both D3D12 and Vulkan APIs. The pass handles DLSS initialization, optimal settings query, feature creation, evaluation, and release. It supports multiple quality profiles (MaxPerf, Balanced, MaxQuality), motion vector scaling modes (Absolute, Relative), HDR content, sharpness adjustment, and exposure compensation. The pass integrates with Falcor's render graph system and provides UI controls for configuration.

## Component Specifications

### DLSSPass Class

**File**: [`DLSSPass.h`](Source/RenderPasses/DLSSPass/DLSSPass.h:36)

**Purpose**: NVIDIA DLSS (Deep Learning Super Sampling) pass for AI-based upscaling and anti-aliasing.

**Key Features**:
- DLSS initialization and management
- Optimal settings query
- Multiple quality profiles (MaxPerf, Balanced, MaxQuality)
- Motion vector scaling (Absolute, Relative)
- HDR content support
- Sharpness adjustment
- Exposure compensation
- Output size configuration (Default, Fixed)
- Jitter offset handling
- Internal buffer for format/size conversion
- Graph recompile on size changes

**Public Interface**:

```cpp
class DLSSPass : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(DLSSPass, "DLSSPass", "DL antialiasing/upscaling.");

    enum class Profile : uint32_t
    {
        MaxPerf,
        Balanced,
        MaxQuality,
    };

    FALCOR_ENUM_INFO(
        Profile,
        {
            {Profile::MaxPerf, "MaxPerf"},
            {Profile::Balanced, "Balanced"},
            {Profile::MaxQuality, "MaxQuality"},
        }
    );

    enum class MotionVectorScale : uint32_t
    {
        Absolute, ///< Motion vectors are provided in absolute screen space length (pixels).
        Relative, ///< Motion vectors are provided in relative screen space length (pixels divided by screen width/height).
    };

    FALCOR_ENUM_INFO(
        MotionVectorScale,
        {
            {MotionVectorScale::Absolute, "Absolute"},
            {MotionVectorScale::Relative, "Relative"},
        }
    );

    static ref<DLSSPass> create(ref<Device> pDevice, const Properties& props)
    {
        return make_ref<DLSSPass>(pDevice, props);
    }

    DLSSPass(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;

private:
    void initializeDLSS(RenderContext* pRenderContext);
    void executeInternal(RenderContext* pRenderContext, const RenderData& renderData);

    // Options
    bool mEnabled = true;
    Profile mProfile = Profile::Balanced;
    MotionVectorScale mMotionVectorScale = MotionVectorScale::Absolute;
    bool mIsHDR = true;
    float mSharpness = 0.f;
    float mExposure = 0.f;
    bool mExposureUpdated = true;

    bool mRecreate = true;
    uint2 mInputSize = {};      ///< Input size in pixels.
    uint2 mDLSSOutputSize = {}; ///< DLSS output size in pixels.
    uint2 mPassOutputSize = {}; ///< Pass output size in pixels. If different from DLSS output size, image gets bilinearly resampled.
    RenderPassHelpers::IOSize mOutputSizeSelection = RenderPassHelpers::IOSize::Default; ///< Selected output size.

    ref<Scene> mpScene;
    ref<Texture> mpOutput;   ///< Internal output buffer. This is used if format/size conversion upon output is needed.
    ref<Texture> mpExposure; ///< Texture of size 1x1 holding exposure value.

    std::unique_ptr<NGXWrapper> mpNGXWrapper;
};
```

**Public Members**: None (all members are private)

**Private Members**:
- `bool mEnabled` - DLSS enabled flag (default true)
- `Profile mProfile` - Quality profile (default Balanced)
- `MotionVectorScale mMotionVectorScale` - Motion vector scale (default Absolute)
- `bool mIsHDR` - HDR content flag (default true)
- `float mSharpness` - Sharpness value (default 0.f, range -1.f to 1.f)
- `float mExposure` - Exposure value (default 0.f, range -10.f to 10.f)
- `bool mExposureUpdated` - Exposure updated flag
- `bool mRecreate` - Recreate flag
- `uint2 mInputSize` - Input size in pixels
- `uint2 mDLSSOutputSize` - DLSS output size in pixels
- `uint2 mPassOutputSize` - Pass output size in pixels
- `RenderPassHelpers::IOSize mOutputSizeSelection` - Output size selection (default Default)
- `ref<Scene> mpScene` - Scene reference
- `ref<Texture> mpOutput` - Internal output buffer
- `ref<Texture> mpExposure` - Exposure texture (1x1)
- `std::unique_ptr<NGXWrapper> mpNGXWrapper` - NGX wrapper

**Private Methods**:
- `void initializeDLSS(RenderContext* pRenderContext)` - Initialize DLSS
- `void executeInternal(RenderContext* pRenderContext, const RenderData& renderData)` - Execute internal

**Public Methods**:
- `Properties getProperties() const override` - Get properties
- `RenderPassReflection reflect(const CompileData& compileData) override` - Reflect input/output
- `void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override` - Set scene
- `void execute(RenderContext* pRenderContext, const RenderData& renderData) override` - Execute pass
- `void renderUI(Gui::Widgets& widget) override` - Render UI

### NGXWrapper Class

**File**: [`NGXWrapper.h`](Source/RenderPasses/DLSSPass/NGXWrapper.h:45)

**Purpose**: Wrapper around NGX functionality for DLSS, providing focus on calls specific to NGX.

**Key Features**:
- NGX initialization and shutdown
- DLSS initialization, evaluation, and release
- Optimal settings query
- D3D12 and Vulkan support
- Format-aware resource handling
- Error handling and logging

**Public Interface**:

```cpp
class NGXWrapper
{
public:
    struct OptimalSettings
    {
        float sharpness;
        uint2 optimalRenderSize;
        uint2 minRenderSize;
        uint2 maxRenderSize;
    };

    /// Constructor. Throws an exception if unable to initialize NGX.
    NGXWrapper(ref<Device> pDevice, const std::filesystem::path& applicationDataPath, const std::filesystem::path& featureSearchPath);
    ~NGXWrapper();

    /// Query optimal DLSS settings for a given resolution and performance/quality profile.
    OptimalSettings queryOptimalSettings(uint2 displaySize, NVSDK_NGX_PerfQuality_Value perfQuality) const;

    /// Initialize DLSS. Throws an exception if unable to initialize.
    void initializeDLSS(
        RenderContext* pRenderContext,
        uint2 maxRenderSize,
        uint2 displayOutSize,
        Texture* pTarget,
        bool isContentHDR,
        bool depthInverted,
        NVSDK_NGX_PerfQuality_Value perfQuality = NVSDK_NGX_PerfQuality_Value_MaxPerf
    );

    /// Release DLSS.
    void releaseDLSS();

    /// Checks if DLSS is initialized.
    bool isDLSSInitialized() const { return mpFeature != nullptr; }

    /// Evaluate DLSS.
    bool evaluateDLSS(
        RenderContext* pRenderContext,
        Texture* pUnresolvedColor,
        Texture* pResolvedColor,
        Texture* pMotionVectors,
        Texture* pDepth,
        Texture* pExposure,
        bool resetAccumulation = false,
        float sharpness = 0.0f,
        float2 jitterOffset = {0.f, 0.f},
        float2 motionVectorScale = {1.f, 1.f}
    ) const;

private:
    void initializeNGX(const std::filesystem::path& applicationDataPath, const std::filesystem::path& featureSearchPath);
    void shutdownNGX();

    ref<Device> mpDevice;
    bool mInitialized = false;

    NVSDK_NGX_Parameter* mpParameters = nullptr;
    NVSDK_NGX_Handle* mpFeature = nullptr;
};
```

**Public Members**:
- `struct OptimalSettings` - Optimal settings structure
  - `float sharpness` - Sharpness value
  - `uint2 optimalRenderSize` - Optimal render size
  - `uint2 minRenderSize` - Minimum render size
  - `uint2 maxRenderSize` - Maximum render size

**Private Members**:
- `ref<Device> mpDevice` - Device reference
- `bool mInitialized` - Initialized flag
- `NVSDK_NGX_Parameter* mpParameters` - NGX parameters
- `NVSDK_NGX_Handle* mpFeature` - NGX feature handle

**Private Methods**:
- `void initializeNGX(const std::filesystem::path& applicationDataPath, const std::filesystem::path& featureSearchPath)` - Initialize NGX
- `void shutdownNGX()` - Shutdown NGX

**Public Methods**:
- `OptimalSettings queryOptimalSettings(uint2 displaySize, NVSDK_NGX_PerfQuality_Value perfQuality) const` - Query optimal settings
- `void initializeDLSS(...)` - Initialize DLSS
- `void releaseDLSS()` - Release DLSS
- `bool isDLSSInitialized() const` - Check if DLSS initialized
- `bool evaluateDLSS(...)` - Evaluate DLSS

## Technical Details

### NGX Initialization

**Purpose**: Initialize NGX SDK and query DLSS capabilities.

**Features**:
- D3D12 and Vulkan support
- Feature search path configuration
- Application data path configuration
- DLSS availability check
- Driver version check
- Capability parameters query

**Initialization Process**:
```cpp
void NGXWrapper::initializeNGX(const std::filesystem::path& applicationDataPath, const std::filesystem::path& featureSearchPath)
{
    NVSDK_NGX_Result result = NVSDK_NGX_Result_Fail;

    NVSDK_NGX_FeatureCommonInfo featureInfo = {};
    const wchar_t* pathList[] = {featureSearchPath.c_str()};
    featureInfo.PathListInfo.Length = 1;
    featureInfo.PathListInfo.Path = const_cast<wchar_t**>(&pathList[0]);

    switch (mpDevice->getType())
    {
    case Device::Type::D3D12:
        result = NVSDK_NGX_D3D12_Init(kAppID, applicationDataPath.c_str(), mpDevice->getNativeHandle().as<ID3D12Device*>(), &featureInfo);
        break;
    case Device::Type::Vulkan:
        result = NVSDK_NGX_VULKAN_Init(
            kAppID,
            applicationDataPath.c_str(),
            mpDevice->getNativeHandle(0).as<VkInstance>(),
            mpDevice->getNativeHandle(1).as<VkPhysicalDevice>(),
            mpDevice->getNativeHandle(2).as<VkDevice>(),
            nullptr,
            nullptr,
            &featureInfo
        );
        break;
    }

    if (NVSDK_NGX_FAILED(result))
    {
        if (result == NVSDK_NGX_Result_FAIL_FeatureNotSupported || result == NVSDK_NGX_Result_FAIL_PlatformError)
        {
            FALCOR_THROW("NVIDIA NGX is not available on this hardware/platform " + resultToString(result));
        }
        else
        {
            FALCOR_THROW("Failed to initialize NGX " + resultToString(result));
        }
    }

    mInitialized = true;

    switch (mpDevice->getType())
    {
    case Device::Type::D3D12:
        THROW_IF_FAILED(NVSDK_NGX_D3D12_GetCapabilityParameters(&mpParameters));
        break;
    case Device::Type::Vulkan:
        THROW_IF_FAILED(NVSDK_NGX_VULKAN_GetCapabilityParameters(&mpParameters));
        break;
    }

    // Driver version check
    int dlssAvailable = 0;
    result = mpParameters->Get(NVSDK_NGX_Parameter_SuperSampling_Available, &dlssAvailable);
    if (NVSDK_NGX_FAILED(result) || !dlssAvailable)
    {
        FALCOR_THROW("NVIDIA DLSS not available on this hardware/platform " + resultToString(result));
    }
}
```

### DLSS Initialization

**Purpose**: Initialize DLSS feature with optimal settings.

**Features**:
- Optimal settings query
- Feature creation flags
- HDR support
- Depth inversion support
- Performance quality selection
- Target texture handling

**Initialization Process**:
```cpp
void NGXWrapper::initializeDLSS(
    RenderContext* pRenderContext,
    uint2 maxRenderSize,
    uint2 displayOutSize,
    Texture* pTarget,
    bool isContentHDR,
    bool depthInverted,
    NVSDK_NGX_PerfQuality_Value perfQuality
)
{
    unsigned int creationNodeMask = 1;
    unsigned int visibilityNodeMask = 1;

    // Next create features
    int createFlags = NVSDK_NGX_DLSS_Feature_Flags_None;
    createFlags |= NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
    createFlags |= isContentHDR ? NVSDK_NGX_DLSS_Feature_Flags_IsHDR : 0;
    createFlags |= depthInverted ? NVSDK_NGX_DLSS_Feature_Flags_DepthInverted : 0;

    NVSDK_NGX_DLSS_Create_Params dlssParams = {};

    dlssParams.Feature.InWidth = maxRenderSize.x;
    dlssParams.Feature.InHeight = maxRenderSize.y;
    dlssParams.Feature.InTargetWidth = displayOutSize.x;
    dlssParams.Feature.InTargetHeight = displayOutSize.y;
    dlssParams.Feature.InPerfQualityValue = perfQuality;
    dlssParams.InFeatureCreateFlags = createFlags;

    switch (mpDevice->getType())
    {
    case Device::Type::D3D12:
        pRenderContext->submit();
        ID3D12GraphicsCommandList* pCommandList =
            pRenderContext->getLowLevelData()->getCommandBufferNativeHandle().as<ID3D12GraphicsCommandList*>();
        THROW_IF_FAILED(NGX_D3D12_CREATE_DLSS_EXT(pCommandList, creationNodeMask, visibilityNodeMask, &mpFeature, mpParameters, &dlssParams)
        );
        pRenderContext->submit();
        break;
    case Device::Type::Vulkan:
        pRenderContext->submit();
        VkCommandBuffer vkCommandBuffer = pRenderContext->getLowLevelData()->getCommandBufferNativeHandle().as<VkCommandBuffer>();
        THROW_IF_FAILED(
            NGX_VULKAN_CREATE_DLSS_EXT(vkCommandBuffer, creationNodeMask, visibilityNodeMask, &mpFeature, mpParameters, &dlssParams)
        );
        pRenderContext->submit();
        break;
    }
}
```

### DLSS Evaluation

**Purpose**: Evaluate DLSS for upscaling and anti-aliasing.

**Features**:
- DLSS evaluation
- Jitter offset handling
- Motion vector scaling
- Sharpness adjustment
- Exposure compensation
- Reset accumulation
- Resource barriers
- Format-aware resource handling

**D3D12 Evaluation**:
```cpp
bool NGXWrapper::evaluateDLSS(...) const
{
    if (!mpFeature)
        return false;

    // In DLSS v2, the target is already upsampled (while in v1, upsampling is handled in a later pass)
    FALCOR_ASSERT(pResolvedColor->getWidth() > pUnresolvedColor->getWidth() && pResolvedColor->getHeight() > pUnresolvedColor->getHeight());

    bool success = true;

    switch (mpDevice->getType())
    {
    case Device::Type::D3D12:
    {
        pRenderContext->resourceBarrier(pUnresolvedColor, Resource::State::ShaderResource);
        pRenderContext->resourceBarrier(pMotionVectors, Resource::State::ShaderResource);
        pRenderContext->resourceBarrier(pDepth, Resource::State::ShaderResource);
        pRenderContext->resourceBarrier(pResolvedColor, Resource::State::UnorderedAccess);

        ID3D12Resource* unresolvedColorBuffer = pUnresolvedColor->getNativeHandle().as<ID3D12Resource*>();
        ID3D12Resource* motionVectorsBuffer = pMotionVectors->getNativeHandle().as<ID3D12Resource*>();
        ID3D12Resource* resolvedColorBuffer = pResolvedColor->getNativeHandle().as<ID3D12Resource*>();
        ID3D12Resource* depthBuffer = pDepth->getNativeHandle().as<ID3D12Resource*>();
        ID3D12Resource* exposureBuffer = pExposure ? pExposure->getNativeHandle().as<ID3D12Resource*>() : nullptr;

        NVSDK_NGX_D3D12_DLSS_Eval_Params evalParams = {};

        evalParams.Feature.pInColor = unresolvedColorBuffer;
        evalParams.Feature.pInOutput = resolvedColorBuffer;
        evalParams.Feature.InSharpness = sharpness;
        evalParams.pInDepth = depthBuffer;
        evalParams.pInMotionVectors = motionVectorsBuffer;
        evalParams.InJitterOffsetX = jitterOffset.x;
        evalParams.InJitterOffsetY = jitterOffset.y;
        evalParams.InReset = resetAccumulation ? 1 : 0;
        evalParams.InRenderSubrectDimensions.Width = pUnresolvedColor->getWidth();
        evalParams.InRenderSubrectDimensions.Height = pUnresolvedColor->getHeight();
        evalParams.InMVScaleX = motionVectorScale.x;
        evalParams.InMVScaleY = motionVectorScale.y;
        evalParams.pInExposureTexture = exposureBuffer;

        ID3D12GraphicsCommandList* pCommandList =
            pRenderContext->getLowLevelData()->getCommandBufferNativeHandle().as<ID3D12GraphicsCommandList*>();
        NVSDK_NGX_Result result = NGX_D3D12_EVALUATE_DLSS_EXT(pCommandList, mpFeature, mpParameters, &evalParams);
        if (NVSDK_NGX_FAILED(result))
        {
            logWarning("Failed to NGX_D3D12_EVALUATE_DLSS_EXT for DLSS: {}", resultToString(result));
            success = false;
        }

        pRenderContext->setPendingCommands(true);
        pRenderContext->uavBarrier(pResolvedColor);
        pRenderContext->submit();
        break;
    }
    // ... Vulkan case similar
    }

    return success;
}
```

### Optimal Settings Query

**Purpose**: Query optimal DLSS settings for given resolution and quality profile.

**Features**:
- Optimal render size computation
- Min/max render size computation
- Sharpness value
- Performance quality selection

**Query Process**:
```cpp
NGXWrapper::OptimalSettings NGXWrapper::queryOptimalSettings(uint2 displaySize, NVSDK_NGX_PerfQuality_Value perfQuality) const
{
    OptimalSettings settings;

    THROW_IF_FAILED(NGX_DLSS_GET_OPTIMAL_SETTINGS(
        mpParameters,
        displaySize.x,
        displaySize.y,
        perfQuality,
        &settings.optimalRenderSize.x,
        &settings.optimalRenderSize.y,
        &settings.maxRenderSize.x,
        &settings.maxRenderSize.y,
        &settings.minRenderSize.x,
        &settings.minRenderSize.y,
        &settings.sharpness
    ));

    // Depending on what version of DLSS DLL is being used, a sharpness of > 1.f was possible.
    settings.sharpness = math::clamp(settings.sharpness, -1.f, 1.f);

    return settings;
}
```

### Jitter Offset Handling

**Purpose**: Handle camera jitter for DLSS.

**Features**:
- X-jitter: left-to-right (no flip)
- Y-jitter: top-to-bottom (flip to bottom-to-top)
- Camera jitter extraction
- Offset scaling to input size

**Jitter Offset Computation**:
```cpp
// In DLSS X-jitter should go left-to-right, Y-jitter should go top-to-bottom.
// Falcor is using math::perspective() that gives coordinate system with
// X from -1 to 1, left-to-right, and Y from -1 to 1, bottom-to-top.
// Therefore, we need to flip the Y-jitter only.
const auto& camera = mpScene->getCamera();
float2 jitterOffset = float2(camera->getJitterX(), -camera->getJitterY()) * float2(inputSize);
```

### Motion Vector Scaling

**Purpose**: Scale motion vectors for DLSS.

**Modes**:
- **Absolute**: Motion vectors are provided in absolute screen space length (pixels)
- **Relative**: Motion vectors are provided in relative screen space length (pixels divided by screen width/height)

**Scaling Computation**:
```cpp
float2 motionVectorScale = float2(1.f, 1.f);
if (mMotionVectorScale == MotionVectorScale::Relative)
    motionVectorScale = inputSize;
```

### Exposure Compensation

**Purpose**: Apply exposure compensation for HDR content.

**Features**:
- Exposure value storage in 1x1 texture
- Exposure update on change
- Pow2 exposure scaling
- Texture data update

**Exposure Computation**:
```cpp
float exposure = pow(2.f, mExposure);
pRenderContext->updateTextureData(mpExposure.get(), &exposure);
```

### Output Size Configuration

**Purpose**: Configure output size for DLSS pass.

**Modes**:
- **Default**: Output is sized based on requirements of connected passes
- **Fixed**: Output is fixed to optimal size determined by DLSS

**Size Computation**:
```cpp
const uint2 sz = RenderPassHelpers::calculateIOSize(mOutputSizeSelection, mDLSSOutputSize, compileData.defaultTexDims);

r.addOutput(kOutput, "Color output")
    .format(ResourceFormat::RGBA32Float)
    .bindFlags(ResourceBindFlags::UnorderedAccess | ResourceBindFlags::RenderTarget)
    .texture2D(sz.x, sz.y);
```

### Internal Buffer Usage

**Purpose**: Use internal buffer for format/size conversion.

**Conditions**:
- Pass output size differs from DLSS output size
- Pass output format differs from DLSS output format
- Bilinear resampling from internal buffer to pass output

**Buffer Creation**:
```cpp
mpOutput = mpDevice->createTexture2D(
    mDLSSOutputSize.x,
    mDLSSOutputSize.y,
    ResourceFormat::RGBA32Float,
    1,
    1,
    nullptr,
    ResourceBindFlags::UnorderedAccess | ResourceBindFlags::ShaderResource
);
```

### Bilinear Resampling

**Purpose**: Resample upscaled DLSS result to pass output if needed.

**Conditions**:
- Pass output size differs from DLSS output size
- Pass output format differs from DLSS output format

**Resampling**:
```cpp
// Resample upscaled result from DLSS to the pass output if needed.
if (useInternalBuffer)
    pRenderContext->blit(mpOutput->getSRV(), pOutput->getRTV());
```

### Property Parsing

**Purpose**: Parse configuration properties from Python or serialization.

**Properties**:
- `enabled` - DLSS enabled flag
- `outputSize` - Output size selection
- `profile` - Quality profile
- `motionVectorScale` - Motion vector scale
- `isHDR` - HDR content flag
- `sharpness` - Sharpness value
- `exposure` - Exposure value

**Parsing Process**:
```cpp
for (const auto& [key, value] : props)
{
    if (key == kEnabled)
        mEnabled = value;
    else if (key == kOutputSize)
        mOutputSizeSelection = value;
    else if (key == kProfile)
        mProfile = value;
    else if (key == kMotionVectorScale)
        mMotionVectorScale = value;
    else if (key == kIsHDR)
        mIsHDR = value;
    else if (key == kSharpness)
        mSharpness = value;
    else if (key == kExposure)
    {
        mExposure = value;
        mExposureUpdated = true;
    }
    else
        logWarning("Unknown property '{}' in a DLSSPass properties.", key);
}
```

### Property Serialization

**Purpose**: Serialize configuration properties.

**Serialization Process**:
```cpp
Properties DLSSPass::getProperties() const
{
    Properties props;
    props[kEnabled] = mEnabled;
    props[kOutputSize] = mOutputSizeSelection;
    props[kProfile] = mProfile;
    props[kMotionVectorScale] = mMotionVectorScale;
    props[kIsHDR] = mIsHDR;
    props[kSharpness] = mSharpness;
    props[kExposure] = mExposure;
    return props;
}
```

### Render Graph Integration

**Input Channels**:
- `color` - Color input (ShaderResource)
- `depth` - Depth input (ShaderResource)
- `mvec` - Motion vectors input (ShaderResource)

**Output Channels**:
- `output` - Color output (UnorderedAccess | RenderTarget, RGBA32Float)

**Reflection**:
```cpp
RenderPassReflection DLSSPass::reflect(const CompileData& compileData)
{
    RenderPassReflection r;
    const uint2 sz = RenderPassHelpers::calculateIOSize(mOutputSizeSelection, mDLSSOutputSize, compileData.defaultTexDims);

    r.addInput(kColorInput, "Color input").bindFlags(ResourceBindFlags::ShaderResource);
    r.addInput(kDepthInput, "Depth input").bindFlags(ResourceBindFlags::ShaderResource);
    r.addInput(kMotionVectorsInput, "Motion vectors input").bindFlags(ResourceBindFlags::ShaderResource);
    r.addOutput(kOutput, "Color output")
        .format(ResourceFormat::RGBA32Float)
        .bindFlags(ResourceBindFlags::UnorderedAccess | ResourceBindFlags::RenderTarget)
        .texture2D(sz.x, sz.y);

    return r;
}
```

### Scene Integration

**Purpose**: Integrate with Falcor's scene system.

**Features**:
- Camera jitter extraction
- Scene reference management
- Camera access for jitter

**Scene Setup**:
```cpp
void DLSSPass::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    mpScene = pScene;
}
```

### UI Integration

**Purpose**: Provide UI controls for DLSS configuration.

**Controls**:
- Enable checkbox
- Output size dropdown (triggers graph recompile)
- Profile dropdown (MaxPerf, Balanced, MaxQuality)
- Motion vector scale dropdown (Absolute, Relative)
- HDR checkbox
- Sharpness slider (-1.f to 1.f)
- Exposure slider (-10.f to 10.f)
- Resolution display (input, DLSS output, pass output)

**UI Rendering**:
```cpp
void DLSSPass::renderUI(Gui::Widgets& widget)
{
    widget.checkbox("Enable", mEnabled);

    // Controls for output size.
    // When output size requirements change, we'll trigger a graph recompile to update the render pass I/O sizes.
    if (widget.dropdown("Output size", mOutputSizeSelection))
        requestRecompile();
    widget.tooltip(
        "Specifies pass output size.\n"
        "'Default' means that the output is sized based on requirements of connected passes.\n"
        "'Fixed' means that the output is fixed to the optimal size determined by DLSS.\n"
        "If the output is of a different size than the DLSS output resolution, the image will be rescaled bilinearly.",
        true
    );

    if (mEnabled)
    {
        mRecreate |= widget.dropdown("Profile", mProfile);

        widget.dropdown("Motion vector scale", mMotionVectorScale);
        widget.tooltip(
            "Absolute: Motion vectors are provided in absolute screen space length (pixels)\n"
            "Relative: Motion vectors are provided in relative screen space length (pixels divided by screen width/height)."
        );

        mRecreate |= widget.checkbox("HDR", mIsHDR);
        widget.tooltip("Enable if input color is HDR.");

        widget.slider("Sharpness", mSharpness, -1.f, 1.f);
        widget.tooltip("Sharpening value between 0.0 and 1.0.");

        if (widget.var("Exposure", mExposure, -10.f, 10.f, 0.01f))
            mExposureUpdated = true;

        widget.text("Input resolution: " + std::to_string(mInputSize.x) + "x" + std::to_string(mInputSize.y));
        widget.text("DLSS output resolution: " + std::to_string(mDLSSOutputSize.x) + "x" + std::to_string(mDLSSOutputSize.y));
        widget.text("Pass output resolution: " + std::to_string(mPassOutput.x) + "x" + std::to_string(mPassOutputSize.y));
    }
}
```

### Error Handling

**Purpose**: Handle NGX SDK errors and exceptions.

**Error Types**:
- Feature not supported
- Platform error
- Initialization failure
- Evaluation failure
- Driver version mismatch

**Error Handling**:
```cpp
#define THROW_IF_FAILED(call)                                                     \
    {                                                                             \
        NVSDK_NGX_Result result_ = call;                                          \
        if (NVSDK_NGX_FAILED(result_))                                            \
            FALCOR_THROW(#call " failed with error {}", resultToString(result_)); \
    }

if (NVSDK_NGX_FAILED(result))
{
    if (result == NVSDK_NGX_Result_FAIL_FeatureNotSupported || result == NVSDK_NGX_Result_FAIL_PlatformError)
    {
        FALCOR_THROW("NVIDIA NGX is not available on this hardware/platform " + resultToString(result));
    }
    else
    {
        FALCOR_THROW("Failed to initialize NGX " + resultToString(result));
    }
}
```

### Result String Conversion

**Purpose**: Convert NGX result codes to human-readable strings.

**Conversion**:
```cpp
std::string resultToString(NVSDK_NGX_Result result)
{
    char buf[1024];
    snprintf(buf, sizeof(buf), "(code: 0x%08x, info: %ls)", result, GetNGXResultAsString(result));
    buf[sizeof(buf) - 1] = '\0';
    return std::string(buf);
}
```

## Architecture Patterns

### Render Pass Pattern
- Inherits from RenderPass base class
- Implements render() method
- Specifies input/output resources
- Manages shader compilation
- Parameter reflection
- Scene integration

### NGX Wrapper Pattern
- Encapsulates NGX SDK functionality
- Provides abstraction over D3D12 and Vulkan
- Handles resource management
- Error handling and logging
- Format-aware resource handling

### DLSS Pattern
- Initialize DLSS with optimal settings
- Evaluate DLSS for upscaling
- Handle jitter offset
- Scale motion vectors
- Apply exposure compensation
- Use internal buffer for format/size conversion

### Resource Management Pattern
- Internal buffer for format/size conversion
- Resource barriers for synchronization
- UAV barriers for unordered access
- Format-aware resource handling

## Code Patterns

### Property Parsing
```cpp
for (const auto& [key, value] : props)
{
    if (key == kEnabled)
        mEnabled = value;
    else if (key == kOutputSize)
        mOutputSizeSelection = value;
    else if (key == kProfile)
        mProfile = value;
    else if (key == kMotionVectorScale)
        mMotionVectorScale = value;
    else if (key == kIsHDR)
        mIsHDR = value;
    else if (key == kSharpness)
        mSharpness = value;
    else if (key == kExposure)
    {
        mExposure = value;
        mExposureUpdated = true;
    }
    else
        logWarning("Unknown property '{}' in a DLSSPass properties.", key);
}
```

### Property Serialization
```cpp
Properties DLSSPass::getProperties() const
{
    Properties props;
    props[kEnabled] = mEnabled;
    props[kOutputSize] = mOutputSizeSelection;
    props[kProfile] = mProfile;
    props[kMotionVectorScale] = mMotionVectorScale;
    props[kIsHDR] = mIsHDR;
    props[kSharpness] = mSharpness;
    props[kExposure] = mExposure;
    return props;
}
```

### Execute Implementation
```cpp
void DLSSPass::executeInternal(RenderContext* pRenderContext, const RenderData& renderData)
{
    // Determine pass I/O sizes based on bound textures.
    const auto& pOutput = renderData.getTexture(kOutput);
    const auto& pColor = renderData.getTexture(kColorInput);
    FALCOR_ASSERT(pColor && pOutput);

    mPassOutputSize = {pOutput->getWidth(), pOutput->getHeight()};
    const uint2 inputSize = {pColor->getWidth(), pColor->getHeight()};

    if (!mEnabled || !mpScene)
    {
        pRenderContext->blit(pColor->getSRV(), pOutput->getRTV());
        return;
    }

    if (mExposureUpdated)
    {
        float exposure = pow(2.f, mExposure);
        pRenderContext->updateTextureData(mpExposure.get(), &exposure);
        mExposureUpdated = false;
    }

    if (mRecreate || any(inputSize != mInputSize))
    {
        mRecreate = false;
        mInputSize = inputSize;

        initializeDLSS(pRenderContext);

        // If pass output is configured to be fixed to DLSS output, but the sizes don't match,
        // we'll trigger a graph recompile to update the pass I/O size requirements.
        // This causes a one frame delay, but unfortunately we don't know the size until after initializeDLSS().
        if (mOutputSizeSelection == RenderPassHelpers::IOSize::Fixed && any(mPassOutputSize != mDLSSOutputSize))
            requestRecompile();
    }

    {
        // Fetch inputs and verify their dimensions.
        auto getInput = [=](const std::string& name)
        {
            auto tex = renderData.getTexture(name);
            if (!tex)
                FALCOR_THROW("DLSSPass: Missing input '{}'", name);
            if (tex->getWidth() != mInputSize.x || tex->getHeight() != mInputSize.y)
                FALCOR_THROW("DLSSPass: Input '{}' has mismatching size. All inputs must be of the same size.", name);
            return tex;
        };

        auto color = getInput(kColorInput);
        auto depth = getInput(kDepthInput);
        auto motionVectors = getInput(kMotionVectorsInput);

        // Determine if we can write directly to render pass output.
        // Otherwise we'll output to an internal buffer and blit to the pass output.
        FALCOR_ASSERT(mpOutput->getWidth() == mDLSSOutputSize.x && mpOutput->getHeight() == mDLSSOutputSize.y);
        bool useInternalBuffer = (any(mDLSSOutputSize != mPassOutputSize) || pOutput->getFormat() != mpOutput->getFormat());

        auto output = useInternalBuffer ? mpOutput : pOutput;

        // In DLSS X-jitter should go left-to-right, Y-jitter should go top-to-bottom.
        // Falcor is using math::perspective() that gives coordinate system with
        // X from -1 to 1, left-to-right, and Y from -1 to 1, bottom-to-top.
        // Therefore, we need to flip the Y-jitter only.
        const auto& camera = mpScene->getCamera();
        float2 jitterOffset = float2(camera->getJitterX(), -camera->getJitterY()) * float2(inputSize);
        float2 motionVectorScale = float2(1.f, 1.f);
        if (mMotionVectorScale == MotionVectorScale::Relative)
            motionVectorScale = inputSize;

        mpNGXWrapper->evaluateDLSS(
            pRenderContext,
            color.get(),
            output.get(),
            motionVectors.get(),
            depth.get(),
            mpExposure.get(),
            false,
            mSharpness,
            jitterOffset,
            motionVectorScale
        );

        // Resample upscaled result from DLSS to the pass output if needed.
        if (useInternalBuffer)
            pRenderContext->blit(mpOutput->getSRV(), pOutput->getRTV());
    }
}
```

### NGX Initialization
```cpp
void NGXWrapper::initializeNGX(const std::filesystem::path& applicationDataPath, const std::filesystem::path& featureSearchPath)
{
    NVSDK_NGX_Result result = NVSDK_NGX_Result_Fail;

    NVSDK_NGX_FeatureCommonInfo featureInfo = {};
    const wchar_t* pathList[] = {featureSearchPath.c_str()};
    featureInfo.PathListInfo.Length = 1;
    featureInfo.PathListInfo.Path = const_cast<wchar_t**>(&pathList[0]);

    switch (mpDevice->getType())
    {
    case Device::Type::D3D12:
        result = NVSDK_NGX_D3D12_Init(kAppID, applicationDataPath.c_str(), mpDevice->getNativeHandle().as<ID3D12Device*>(), &featureInfo);
        break;
    case Device::Type::Vulkan:
        result = NVSDK_NGX_VULKAN_Init(
            kAppID,
            applicationDataPath.c_str(),
            mpDevice->getNativeHandle(0).as<VkInstance>(),
            mpDevice->getNativeHandle(1).as<VkPhysicalDevice>(),
            mpDevice->getNativeHandle(2).as<VkDevice>(),
            nullptr,
            nullptr,
            &featureInfo
        );
        break;
    }

    if (NVSDK_NGX_FAILED(result))
    {
        if (result == NVSDK_NGX_Result_FAIL_FeatureNotSupported || result == NVSDK_NGX_Result_FAIL_PlatformError)
        {
            FALCOR_THROW("NVIDIA NGX is not available on this hardware/platform " + resultToString(result));
        }
        else
        {
            FALCOR_THROW("Failed to initialize NGX " + resultToString(result));
        }
    }

    mInitialized = true;

    switch (mpDevice->getType())
    {
    case Device::Type::D3D12:
        THROW_IF_FAILED(NVSDK_NGX_D3D12_GetCapabilityParameters(&mpParameters));
        break;
    case Device::Type::Vulkan:
        THROW_IF_FAILED(NVSDK_NGX_VULKAN_GetCapabilityParameters(&mpParameters));
        break;
    }

    // Driver version check
    int dlssAvailable = 0;
    result = mpParameters->Get(NVSDK_NGX_Parameter_SuperSampling_Available, &dlssAvailable);
    if (NVSDK_NGX_FAILED(result) || !dlssAvailable)
    {
        FALCOR_THROW("NVIDIA DLSS not available on this hardware/platform " + resultToString(result));
    }
}
```

## Integration Points

### Render Graph Integration
- Input/output specification via reflect()
- Resource binding in execute()
- Property serialization
- Graph recompile on size changes
- Scene-independent pass

### NGX SDK Integration
- NGX initialization and shutdown
- DLSS feature creation and evaluation
- Optimal settings query
- D3D12 and Vulkan support
- Format-aware resource handling
- Error handling and logging

### Scene Integration
- Camera jitter extraction
- Scene reference management
- Camera access for jitter

### UI Integration
- Gui::Widgets for controls
- Resolution display
- Tooltip support
- Interactive controls

### Python Bindings
```cpp
static void registerDLSSPass(pybind11::module& m)
{
    pybind11::class_<DLSSPass, RenderPass, ref<DLSSPass>> pass(m, "DLSSPass");
}
```

### Plugin Registration
```cpp
extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, DLSSPass>();
    ScriptBindings::registerBinding(registerDLSSPass);
}
```

## Use Cases

### AI-Based Upscaling
- DLSS upscaling for improved performance
- Quality vs. performance trade-off
- Multiple quality profiles
- Adaptive resolution scaling

### Anti-Aliasing
- DLSS anti-aliasing
- Temporal accumulation
- Sharpness adjustment
- Jitter compensation

### HDR Rendering
- HDR content support
- Exposure compensation
- Tone mapping integration
- Color space handling

### Research and Development
- DLSS integration testing
- NGX SDK experimentation
- Quality profile comparison
- Performance benchmarking

### Educational Use
- Demonstrate DLSS integration
- Show NGX SDK usage
- Explain upscaling techniques
- Teach anti-aliasing concepts

## Performance Considerations

### GPU Computation
- DLSS evaluation: O(width × height) per frame
- Bilinear resampling: O(width × height) per frame
- Resource barriers: O(1) per frame
- UAV barriers: O(1) per frame

### Memory Usage
- Input textures: O(width × height × sizeof(float4)) bytes
- Output texture: O(DLSS_width × DLSS_height × sizeof(float4)) bytes
- Internal buffer: O(DLSS_width × DLSS_height × sizeof(float4)) bytes
- Exposure texture: 4 bytes (1x1 R32Float)
- NGX internal buffers: O(DLSS_width × DLSS_height × NGX_buffer_size) bytes

### Computational Complexity
- DLSS evaluation: O(width × height) per frame
- Bilinear resampling: O(width × height) per frame
- Overall: O(width × height) per frame

### Performance Optimization
- NGX SDK optimization
- Internal buffer for format/size conversion
- Resource barriers for synchronization
- Minimal texture copies
- Efficient DLSS evaluation

## Limitations

### Feature Limitations
- Requires NVIDIA NGX SDK installation
- Requires NVIDIA RTX GPU
- Limited to D3D12 and Vulkan
- No custom quality profiles
- No advanced filtering options
- No temporal accumulation customization

### NGX SDK Limitations
- Requires specific driver version
- Platform-dependent features
- Limited to supported formats
- No custom feature configuration
- No advanced DLSS options

### Performance Limitations
- DLSS evaluation overhead
- Bilinear resampling overhead
- Resource barrier overhead
- No performance optimization for large textures
- No caching of DLSS results

### Integration Limitations
- Limited to Falcor's render graph system
- Requires specific input formats
- Requires motion vectors input
- Requires depth input
- No external data sources

### UI Limitations
- No preset management
- No undo/redo functionality
- No real-time preview
- Limited configuration options
- No visualization of DLSS parameters

## Best Practices

### DLSS Configuration
- Install NGX SDK properly
- Use appropriate quality profile for use case
- Configure output size correctly
- Enable HDR for HDR content
- Set appropriate sharpness value
- Use correct motion vector scale

### Performance Optimization
- Use internal buffer for format/size conversion
- Minimize resource barriers
- Use appropriate output size
- Test with different quality profiles
- Profile GPU performance

### Debugging
- Check NGX SDK installation
- Verify driver version
- Test with different resolutions
- Monitor GPU performance
- Check for NGX errors
- Validate input formats

### Research and Development
- Study NGX SDK documentation
- Experiment with quality profiles
- Test with different content
- Compare with other upscaling techniques
- Analyze DLSS quality

### Educational Use
- Demonstrate DLSS integration
- Show NGX SDK usage
- Explain upscaling concepts
- Teach anti-aliasing techniques
- Visualize quality trade-offs

### Integration Best Practices
- Connect to appropriate render graph inputs
- Use correct texture formats
- Handle optional inputs/outputs
- Test with different render graphs
- Validate NGX SDK integration
- Ensure proper synchronization
