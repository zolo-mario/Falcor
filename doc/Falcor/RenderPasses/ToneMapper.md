# ToneMapper - Tone Mapping Pass

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **ToneMapper** - Tone mapping pass
  - [x] **ToneMapper.h** - Tone mapper header (172 lines)
  - [x] **ToneMapper.cpp** - Tone mapper implementation (550 lines)
  - [x] **ToneMapperParams.slang** - Parameter structures (71 lines)
  - [x] **ToneMapping.ps.slang** - Tone mapping shader (168 lines)
  - [x] **Luminance.ps.slang** - Luminance computation shader (42 lines)

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Texture, Formats)
- **Core/Object** - Base object class
- **Core/Pass/FullScreenPass** - Full-screen pass
- **RenderGraph** - Render graph system (RenderPass, RenderPassReflection)
- **RenderGraph/RenderPassHelpers** - Render pass helpers (IOSize)
- **Utils/Color/ColorUtils** - Color utilities (calculateWhiteBalanceTransformRGB_Rec709)
- **Utils/UI** - UI utilities (Gui)

## Module Overview

ToneMapper module provides tone mapping for HDR to LDR conversion with auto-exposure and eye-adaptation support. It implements multiple tone mapping operators including Linear, Reinhard, Reinhard Modified, Heji Hable ALU, Hable Uc2 (Uncharted 2), and ACES filmic tone mapping. The pass supports manual exposure control with camera parameters (f-number, shutter speed, film speed), auto-exposure with luminance-based adaptation, white balance with color temperature control, and output clamping. It integrates with scene metadata for automatic camera parameter setup and provides comprehensive UI controls for real-time adjustment.

## Component Specifications

### ToneMapper Class

**File**: [`ToneMapper.h`](Source/RenderPasses/ToneMapper/ToneMapper.h:38)

**Purpose**: Tone mapping pass for HDR to LDR conversion with auto-exposure and eye-adaptation.

**Public Interface**:

```cpp
class ToneMapper : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(
        ToneMapper,
        "ToneMapper",
        {"Tone-map a color-buffer. The resulting buffer is always in [0, 1] range. The pass supports auto-exposure and eye-adaptation."}
    );

    using Operator = ToneMapperOperator;

    enum class ExposureMode
    {
        AperturePriority, // Keep aperture constant when modifying EV
        ShutterPriority,  // Keep shutter constant when modifying EV
    };

    FALCOR_ENUM_INFO(
        ExposureMode,
        {
            {ExposureMode::AperturePriority, "AperturePriority"},
            {ExposureMode::ShutterPriority, "ShutterPriority"},
        }
    );

    static ref<ToneMapper> create(ref<Device> pDevice, const Properties& props) { return make_ref<ToneMapper>(pDevice, props); }

    ToneMapper(ref<Device> pDevice, const Properties& props);

    Properties getProperties() const override;
    RenderPassReflection reflect(const CompileData& compileData) override;
    void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    void renderUI(Gui::Widgets& widget) override;
    void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;

    // Scripting functions
    void setExposureCompensation(float exposureCompensation);
    void setAutoExposure(bool autoExposure);
    void setExposureValue(float exposureValue);
    void setFilmSpeed(float filmSpeed);
    void setWhiteBalance(bool whiteBalance);
    void setWhitePoint(float whitePoint);
    void setOperator(Operator op);
    void setClamp(bool clamp);
    void setWhiteMaxLuminance(float maxLuminance);
    void setWhiteScale(float whiteScale);
    void setFNumber(float fNumber);
    void setShutter(float shutter);
    void setExposureMode(ExposureMode mode);

    float getExposureCompensation() const { return mExposureCompensation; }
    bool getAutoExposure() const { return mAutoExposure; }
    float getExposureValue() const { return mExposureValue; }
    float getFilmSpeed() const { return mFilmSpeed; }
    bool getWhiteBalance() const { return mWhiteBalance; }
    float getWhitePoint() { return mWhitePoint; }
    Operator getOperator() const { return mOperator; }
    bool getClamp() const { return mClamp; }
    float getWhiteMaxLuminance() const { return mWhiteMaxLuminance; }
    float getWhiteScale() const { return mWhiteScale; }
    float getFNumber() const { return mFNumber; }
    float getShutter() const { return mShutter; }
    ExposureMode getExposureMode() const { return mExposureMode; }

private:
    void parseProperties(const Properties& props);
    void createToneMapPass();
    void createLuminancePass();
    void createLuminanceFbo(const ref<Texture>& pSrc);
    void updateWhiteBalanceTransform();
    void updateColorTransform();
    void updateExposureValue();

    ref<FullScreenPass> mpToneMapPass;
    ref<FullScreenPass> mpLuminancePass;
    ref<Fbo> mpLuminanceFbo;
    ref<Sampler> mpPointSampler;
    ref<Sampler> mpLinearSampler;

    /// Selected output size.
    RenderPassHelpers::IOSize mOutputSizeSelection = RenderPassHelpers::IOSize::Default;
    /// Output format (uses default when set to ResourceFormat::Unknown).
    ResourceFormat mOutputFormat = ResourceFormat::Unknown;
    /// Output size in pixels when 'Fixed' size is selected.
    uint2 mFixedOutputSize = {512, 512};

    /// Use scene metadata for setting up tonemapper when loading a scene.
    bool mUseSceneMetadata = true;

    /// Exposure compensation (in F-stops).
    float mExposureCompensation = 0.f;
    /// Enable auto exposure.
    bool mAutoExposure = false;
    /// Exposure value (EV), derived from fNumber, shutter, and film speed; only used when auto exposure is disabled.
    float mExposureValue = 0.0f;
    /// Film speed (ISO), only used when auto exposure is disabled.
    float mFilmSpeed = 100.f;
    /// Lens speed
    float mFNumber = 1.f;
    /// Reciprocal of shutter time
    float mShutter = 1.f;

    /// Enable white balance.
    bool mWhiteBalance = false;
    /// White point (K).
    float mWhitePoint = 6500.0f;

    /// Tone mapping operator.
    Operator mOperator = Operator::Aces;
    /// Clamp output to [0,1].
    bool mClamp = true;

    /// Parameter used in ModifiedReinhard operator.
    float mWhiteMaxLuminance = 1.0f;
    /// Parameter used in Uc2Hable operator.
    float mWhiteScale = 11.2f;

    // Pre-computed fields based on above settings
    /// Color balance transform in RGB space.
    float3x3 mWhiteBalanceTransform;
    /// Source illuminant in RGB (the white point to which image is transformed to conform to).
    float3 mSourceWhite;
    /// Final color transform with exposure value baked in.
    float3x3 mColorTransform;

    bool mRecreateToneMapPass = true;
    bool mUpdateToneMapPass = true;

    ExposureMode mExposureMode = ExposureMode::AperturePriority;
};
```

**Public Members**: None (all members are private)

**Private Members**:
- `ref<FullScreenPass> mpToneMapPass` - Tone mapping full-screen pass
- `ref<FullScreenPass> mpLuminancePass` - Luminance computation full-screen pass
- `ref<Fbo> mpLuminanceFbo` - Luminance frame buffer object
- `ref<Sampler> mpPointSampler` - Point sampler for texture sampling
- `ref<Sampler> mpLinearSampler` - Linear sampler for texture sampling
- `RenderPassHelpers::IOSize mOutputSizeSelection` - Output size selection (Default, Fixed, MatchInput)
- `ResourceFormat mOutputFormat` - Output format
- `uint2 mFixedOutputSize` - Fixed output size in pixels
- `bool mUseSceneMetadata` - Use scene metadata flag
- `float mExposureCompensation` - Exposure compensation in F-stops
- `bool mAutoExposure` - Auto-exposure flag
- `float mExposureValue` - Exposure value (EV)
- `float mFilmSpeed` - Film speed (ISO)
- `float mFNumber` - Lens speed (f-number)
- `float mShutter` - Reciprocal of shutter time
- `bool mWhiteBalance` - White balance flag
- `float mWhitePoint` - White point in Kelvin
- `Operator mOperator` - Tone mapping operator
- `bool mClamp` - Clamp output flag
- `float mWhiteMaxLuminance` - White max luminance (ModifiedReinhard)
- `float mWhiteScale` - White scale (HableUc2)
- `float3x3 mWhiteBalanceTransform` - White balance transform
- `float3 mSourceWhite` - Source illuminant
- `float3x3 mColorTransform` - Final color transform
- `bool mRecreateToneMapPass` - Recreate tone map pass flag
- `bool mUpdateToneMapPass` - Update tone map pass flag
- `ExposureMode mExposureMode` - Exposure mode

**Private Methods**:
- `void parseProperties(const Properties& props)` - Parse properties
- `void createToneMapPass()` - Create tone mapping pass
- `void createLuminancePass()` - Create luminance pass
- `void createLuminanceFbo(const ref<Texture>& pSrc)` - Create luminance FBO
- `void updateWhiteBalanceTransform()` - Update white balance transform
- `void updateColorTransform()` - Update color transform
- `void updateExposureValue()` - Update exposure value

**Public Methods**:
- `Properties getProperties() const override` - Get properties
- `RenderPassReflection reflect(const CompileData& compileData) override` - Reflect input/output
- `void execute(RenderContext* pRenderContext, const RenderData& renderData) override` - Execute pass
- `void renderUI(Gui::Widgets& widget) override` - Render UI
- `void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override` - Set scene
- Multiple setter/getter methods for parameters

### ToneMapperParams.slang

**File**: [`ToneMapperParams.slang`](Source/RenderPasses/ToneMapper/ToneMapperParams.slang:1)

**Purpose**: Parameter structures for tone mapping.

**Features**:
- ToneMapperOperator enum (6 operators)
- ToneMapperParams struct (host-device shared)

**ToneMapperOperator Enum**:
```cpp
enum class ToneMapperOperator : uint32_t
{
    Linear,           ///< Linear mapping
    Reinhard,         ///< Reinhard operator
    ReinhardModified, ///< Reinhard operator with maximum white intensity
    HejiHableAlu,     ///< John Hable's ALU approximation of Jim Heji's filmic operator
    HableUc2,         ///< John Hable's filmic tone-mapping used in Uncharted 2
    Aces,             ///< Aces Filmic Tone-Mapping
};
```

**ToneMapperParams Struct**:
```cpp
struct ToneMapperParams
{
    float whiteScale;
    float whiteMaxLuminance;
    float _pad0;
    float _pad1;
    float3x4 colorTransform;
};
```

### ToneMapping.ps.slang

**File**: [`ToneMapping.ps.slang`](Source/RenderPasses/ToneMapper/ToneMapping.ps.slang:1)

**Purpose**: Tone mapping pixel shader.

**Features**:
- Multiple tone mapping operators
- Auto-exposure support
- Color grading with white balance
- Output clamping
- Rec.709 luminance computation

**Tone Mapping Operators**:
- `toneMapLinear` - Linear mapping (no transformation)
- `toneMapReinhard` - Reinhard operator: L / (L + 1)
- `toneMapReinhardModified` - Reinhard with max luminance: L * (1 + L / (whiteMaxLuminance²)) / (1 + L)
- `toneMapHejiHableAlu` - John Hable's ALU approximation with sRGB conversion
- `toneMapHableUc2` - Uncharted 2 filmic tone map with shoulder, linear, and toe regions
- `toneMapAces` - ACES filmic tone mapping with pre-exposure cancellation

**Auto-Exposure**:
- Average luminance from mip level 16
- Exposure key: 0.042
- Exposure scale: kExposureKey / avgLuminance
- Log luminance computation

**Color Grading**:
- White balance transform (3x3 matrix)
- Exposure compensation (2^exposureCompensation)
- Manual exposure scale (normConstant * filmSpeed / (shutter * fNumber²))
- Final transform: whiteBalance * exposureScale * manualExposureScale

**Clamping**:
- Optional output clamping to [0, 1]
- Saturate function

### Luminance.ps.slang

**File**: [`Luminance.ps.slang`](Source/RenderPasses/ToneMapper/Luminance.ps.slang:1)

**Purpose**: Luminance computation for auto-exposure.

**Features**:
- Rec.709 luminance weights (0.299, 0.587, 0.114)
- Log luminance computation
- Minimum luminance clamping (0.0001)
- Single channel output

## Technical Details

### Constructor

**Algorithm**:
```cpp
ToneMapper::ToneMapper(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    parseProperties(props);

    createLuminancePass();
    createToneMapPass();

    updateWhiteBalanceTransform();

    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(TextureFilteringMode::Point, TextureFilteringMode::Point, TextureFilteringMode::Point);
    mpPointSampler = mpDevice->createSampler(samplerDesc);
    samplerDesc.setFilterMode(TextureFilteringMode::Linear, TextureFilteringMode::Linear, TextureFilteringMode::Point);
    mpLinearSampler = mpDevice->createSampler(samplerDesc);
}
```

**Features**:
- Property parsing
- Luminance pass creation
- Tone mapping pass creation
- White balance transform initialization
- Point sampler creation
- Linear sampler creation

### ParseProperties

**Algorithm**:
```cpp
void ToneMapper::parseProperties(const Properties& props)
{
    for (const auto& [key, value] : props)
    {
        if (key == kOutputSize)
            mOutputSizeSelection = value;
        else if (key == kOutputFormat)
            mOutputFormat = value;
        else if (key == kFixedOutputSize)
            mFixedOutputSize = value;
        else if (key == kUseSceneMetadata)
            mUseSceneMetadata = value;
        else if (key == kExposureCompensation)
            setExposureCompensation(value);
        else if (key == kAutoExposure)
            setAutoExposure(value);
        else if (key == kFilmSpeed)
            setFilmSpeed(value);
        else if (key == kWhiteBalance)
            setWhiteBalance(value);
        else if (key == kWhitePoint)
            setWhitePoint(value);
        else if (key == kOperator)
            setOperator(value);
        else if (key == kClamp)
            setClamp(value);
        else if (key == kWhiteMaxLuminance)
            setWhiteMaxLuminance(value);
        else if (key == kWhiteScale)
            setWhiteScale(value);
        else if (key == kFNumber)
            setFNumber(value);
        else if (key == kShutter)
            setShutter(value);
        else if (key == kExposureMode)
            setExposureMode(value);
        else
            logWarning("Unknown property '{}' in a ToneMapping properties.", key);
    }
}
```

**Features**:
- Output size parsing
- Output format parsing
- Fixed output size parsing
- Scene metadata flag parsing
- Exposure compensation parsing
- Auto-exposure flag parsing
- Film speed parsing
- White balance flag parsing
- White point parsing
- Operator parsing
- Clamp flag parsing
- White max luminance parsing
- White scale parsing
- F-number parsing
- Shutter parsing
- Exposure mode parsing
- Unknown property warning

### GetProperties

**Algorithm**:
```cpp
Properties ToneMapper::getProperties() const
{
    Properties props;
    props[kOutputSize] = mOutputSizeSelection;
    if (mOutputSizeSelection == RenderPassHelpers::IOSize::Fixed)
        props[kFixedOutputSize] = mFixedOutputSize;
    if (mOutputFormat != ResourceFormat::Unknown)
        props[kOutputFormat] = mOutputFormat;
    props[kUseSceneMetadata] = mUseSceneMetadata;
    props[kExposureCompensation] = mExposureCompensation;
    props[kAutoExposure] = mAutoExposure;
    props[kFilmSpeed] = mFilmSpeed;
    props[kWhiteBalance] = mWhiteBalance;
    props[kWhitePoint] = mWhitePoint;
    props[kOperator] = mOperator;
    props[kClamp] = mClamp;
    props[kWhiteMaxLuminance] = mWhiteMaxLuminance;
    props[kWhiteScale] = mWhiteScale;
    props[kFNumber] = mFNumber;
    props[kShutter] = mShutter;
    props[kExposureMode] = mExposureMode;
    return props;
}
```

**Features**:
- Output size serialization
- Output format serialization
- Fixed output size serialization (conditional)
- Scene metadata flag serialization
- Exposure compensation serialization
- Auto-exposure flag serialization
- Film speed serialization
- White balance flag serialization
- White point serialization
- Operator serialization
- Clamp flag serialization
- White max luminance serialization
- White scale serialization
- F-number serialization
- Shutter serialization
- Exposure mode serialization

### Reflect

**Algorithm**:
```cpp
RenderPassReflection ToneMapper::reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    const uint2 sz = RenderPassHelpers::calculateIOSize(mOutputSizeSelection, mFixedOutputSize, compileData.defaultTexDims);

    reflector.addInput(kSrc, "Source texture");
    auto& output = reflector.addOutput(kDst, "Tone-mapped output texture").texture2D(sz.x, sz.y);
    if (mOutputFormat != ResourceFormat::Unknown)
    {
        output.format(mOutputFormat);
    }

    return reflector;
}
```

**Features**:
- Input channel: src (Source texture)
- Output channel: dst (Tone-mapped output texture)
- Output size calculation
- Output format specification (conditional)

### Execute

**Algorithm**:
```cpp
void ToneMapper::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    auto pSrc = renderData.getTexture(kSrc);
    auto pDst = renderData.getTexture(kDst);
    FALCOR_ASSERT(pSrc && pDst);

    // Issue warning if image will be resampled. The render pass supports this but image quality may suffer.
    if (pSrc->getWidth() != pDst->getWidth() || pSrc->getHeight() != pDst->getHeight())
    {
        logWarning("ToneMapper pass I/O has different dimensions. The image will be resampled.");
    }

    ref<Fbo> pFbo = Fbo::create(mpDevice);
    pFbo->attachColorTarget(pDst, 0);

    // Run luminance pass if auto exposure is enabled
    if (mAutoExposure)
    {
        createLuminanceFbo(pSrc);

        auto var = mpLuminancePass->getRootVar();
        var["gColorTex"] = pSrc;
        var["gColorSampler"] = mpLinearSampler;

        mpLuminancePass->execute(pRenderContext, mpLuminanceFbo);
        mpLuminanceFbo->getColorTexture(0)->generateMips(pRenderContext);
    }

    // Run main pass
    if (mRecreateToneMapPass)
    {
        createToneMapPass();
        mUpdateToneMapPass = true;
        mRecreateToneMapPass = false;
    }

    if (mUpdateToneMapPass)
    {
        updateWhiteBalanceTransform();
        updateColorTransform();

        ToneMapperParams params;
        params.whiteScale = mWhiteScale;
        params.whiteMaxLuminance = mWhiteMaxLuminance;
        params.colorTransform = float3x4(mColorTransform);
        mpToneMapPass->getRootVar()["PerImageCB"]["gParams"].setBlob(&params, sizeof(params));
        mUpdateToneMapPass = false;
    }

    auto var = mpToneMapPass->getRootVar();
    var["gColorTex"] = pSrc;
    var["gColorSampler"] = mpPointSampler;

    if (mAutoExposure)
    {
        var["gLuminanceTexSampler"] = mpLinearSampler;
        var["gLuminanceTex"] = mpLuminanceFbo->getColorTexture(0);
    }

    mpToneMapPass->execute(pRenderContext, pFbo);
}
```

**Features**:
- Input/output validation
- Resampling warning
- Luminance pass execution (conditional)
- Tone map pass recreation (conditional)
- Tone map pass update (conditional)
- White balance transform update
- Color transform update
- Parameter update
- Tone map pass execution
- Auto-exposure support

### CreateLuminanceFbo

**Algorithm**:
```cpp
void ToneMapper::createLuminanceFbo(const ref<Texture>& pSrc)
{
    bool createFbo = mpLuminanceFbo == nullptr;
    ResourceFormat srcFormat = pSrc->getFormat();
    uint32_t bytesPerChannel = getFormatBytesPerBlock(srcFormat) / getFormatChannelCount(srcFormat);

    // Find required texture size and format
    ResourceFormat luminanceFormat = (bytesPerChannel == 4) ? ResourceFormat::R32Float : ResourceFormat::R16Float;
    uint32_t requiredHeight = fstd::bit_floor(pSrc->getHeight());
    uint32_t requiredWidth = fstd::bit_floor(pSrc->getWidth());

    if (createFbo == false)
    {
        createFbo = (requiredWidth != mpLuminanceFbo->getWidth()) || (requiredHeight != mpLuminanceFbo->getHeight()) ||
                    (luminanceFormat != mpLuminanceFbo->getColorTexture(0)->getFormat());
    }

    if (createFbo)
    {
        Fbo::Desc desc;
        desc.setColorTarget(0, luminanceFormat);
        mpLuminanceFbo = Fbo::create2D(mpDevice, requiredWidth, requiredHeight, desc, 1, Fbo::kAttachEntireMipLevel);
    }
}
```

**Features**:
- Lazy FBO creation
- Format detection (R32Float for 4-byte channels, R16Float otherwise)
- Size calculation (bit_floor for power-of-2)
- FBO recreation on size/format change
- Single color target

### UpdateExposureValue

**Algorithm**:
```cpp
void ToneMapper::updateExposureValue()
{
    mExposureValue = std::log2(mShutter * mFNumber * mFNumber);
}
```

**Features**:
- EV calculation from shutter and f-number
- Log2 computation

### RenderUI

**Algorithm**:
```cpp
void ToneMapper::renderUI(Gui::Widgets& widget)
{
    // Controls for output size.
    // When output size requirements change, we'll trigger a graph recompile to update render pass I/O sizes.
    if (widget.dropdown("Output size", mOutputSizeSelection))
        requestRecompile();
    if (mOutputSizeSelection == RenderPassHelpers::IOSize::Fixed)
    {
        if (widget.var("Size in pixels", mFixedOutputSize, 32u, 16384u))
            requestRecompile();
    }

    if (auto exposureGroup = widget.group("Exposure", true))
    {
        mUpdateToneMapPass |= exposureGroup.var(
            "Exposure Compensation", mExposureCompensation, kExposureCompensationMin, kExposureCompensationMax, 0.1f, false, "%.1f"
        );

        mRecreateToneMapPass |= exposureGroup.checkbox("Auto Exposure", mAutoExposure);

        if (!mAutoExposure)
        {
            if (auto exposureMode = mExposureMode; exposureGroup.dropdown("Exposure mode", exposureMode))
            {
                setExposureMode(exposureMode);
            }

            if (exposureGroup.var("Exposure Value (EV)", mExposureValue, kExposureValueMin, kExposureValueMax, 0.1f, false, "%.1f"))
            {
                setExposureValue(mExposureValue);
            }

            mUpdateToneMapPass |= exposureGroup.var("Film Speed (ISO)", mFilmSpeed, kFilmSpeedMin, kFilmSpeedMax, 0.1f, false, "%.1f");

            if (exposureGroup.var("f-Number", mFNumber, kFNumberMin, kFNumberMax, 0.1f, false, "%.1f"))
            {
                setFNumber(mFNumber);
            }

            if (exposureGroup.var("Shutter", mShutter, kShutterMin, kShutterMax, 0.1f, false, "%.1f"))
            {
                setShutter(mShutter);
            }
        }
    }

    if (auto colorgradingGroup = widget.group("Color Grading", true))
    {
        mUpdateToneMapPass |= colorgradingGroup.checkbox("White Balance", mWhiteBalance);

        if (mWhiteBalance)
        {
            if (colorgradingGroup.var("White Point (K)", mWhitePoint, kWhitePointMin, kWhitePointMax, 5.f, false, "%.0f"))
            {
                updateWhiteBalanceTransform();
                mUpdateToneMapPass = true;
            }

            // Display color widget for currently chosen white point.
            // We normalize color so that max(RGB) = 1 for display purposes.
            float3 w = mSourceWhite;
            w = w / std::max(std::max(w.r, w.g), w.b);
            colorgradingGroup.rgbColor("", w);
        }
    }

    if (auto tonemappingGroup = widget.group("Tonemapping", true))
    {
        if (auto op = mOperator; tonemappingGroup.dropdown("Operator", op))
        {
            setOperator(op);
        }

        if (mOperator == Operator::ReinhardModified)
        {
            mUpdateToneMapPass |= tonemappingGroup.var("White Luminance", mWhiteMaxLuminance, 0.1f, FLT_MAX, 0.2f);
        }
        else if (mOperator == Operator::HableUc2)
        {
            mUpdateToneMapPass |= tonemappingGroup.var("Linear White", mWhiteScale, 0.f, 100.f, 0.01f);
        }

        mRecreateToneMapPass |= tonemappingGroup.checkbox("Clamp Output", mClamp);
    }
}
```

**Features**:
- Output size dropdown (triggers recompile)
- Fixed output size slider (triggers recompile)
- Exposure group:
  - Exposure compensation slider (-12 to +12 F-stops)
  - Auto-exposure checkbox
  - Exposure mode dropdown (AperturePriority, ShutterPriority)
  - Exposure value slider (EV)
  - Film speed slider (ISO 1-6400)
  - f-number slider (0.1-100)
  - Shutter slider (0.1-10000)
- Color grading group:
  - White balance checkbox
  - White point slider (1905-25000 K)
  - White point color display
- Tonemapping group:
  - Operator dropdown (6 operators)
  - White luminance slider (ReinhardModified)
  - Linear white slider (HableUc2)
  - Clamp output checkbox

### SetScene

**Algorithm**:
```cpp
void ToneMapper::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    if (pScene && mUseSceneMetadata)
    {
        const Scene::Metadata& metadata = pScene->getMetadata();

        if (metadata.filmISO)
            setFilmSpeed(metadata.filmISO.value());
        if (metadata.fNumber)
            setFNumber(metadata.fNumber.value());
        if (metadata.shutterSpeed)
            setShutter(metadata.shutterSpeed.value());
    }
}
```

**Features**:
- Scene metadata integration
- Film speed from scene metadata
- f-number from scene metadata
- Shutter speed from scene metadata
- Conditional metadata usage

### SetExposureCompensation

**Algorithm**:
```cpp
void ToneMapper::setExposureCompensation(float exposureCompensation)
{
    mExposureCompensation = std::clamp(exposureCompensation, kExposureCompensationMin, kExposureCompensationMax);
    mUpdateToneMapPass = true;
}
```

**Features**:
- Clamping (-12 to +12 F-stops)
- Update flag

### SetAutoExposure

**Algorithm**:
```cpp
void ToneMapper::setAutoExposure(bool autoExposure)
{
    mAutoExposure = autoExposure;
    mRecreateToneMapPass = true;
}
```

**Features**:
- Auto-exposure flag
- Recreate flag

### SetExposureValue

**Algorithm**:
```cpp
void ToneMapper::setExposureValue(float exposureValue)
{
    mExposureValue = std::clamp(exposureValue, kExposureValueMin, kExposureValueMax);

    switch (mExposureMode)
    {
    case ExposureMode::AperturePriority:
        // Set shutter based on EV and aperture.
        mShutter = std::pow(2.f, mExposureValue) / (mFNumber * mFNumber);
        mShutter = std::clamp(mShutter, kShutterMin, kShutterMax);
        break;
    case ExposureMode::ShutterPriority:
        // Set aperture based on EV and shutter.
        mFNumber = std::sqrt(std::pow(2.f, mExposureValue) / mShutter);
        mFNumber = std::clamp(mFNumber, kFNumberMin, kFNumberMax);
        break;
    default:
        FALCOR_UNREACHABLE();
    }

    updateExposureValue();

    mUpdateToneMapPass = true;
}
```

**Features**:
- Clamping (derived from shutter and f-number)
- Aperture priority: shutter = 2^EV / fNumber²
- Shutter priority: fNumber = sqrt(2^EV / shutter)
- EV update
- Update flag

### SetFilmSpeed

**Algorithm**:
```cpp
void ToneMapper::setFilmSpeed(float filmSpeed)
{
    mFilmSpeed = std::clamp(filmSpeed, kFilmSpeedMin, kFilmSpeedMax);
    mUpdateToneMapPass = true;
}
```

**Features**:
- Clamping (ISO 1-6400)
- Update flag

### SetWhiteBalance

**Algorithm**:
```cpp
void ToneMapper::setWhiteBalance(bool whiteBalance)
{
    mWhiteBalance = whiteBalance;
    mUpdateToneMapPass = true;
}
```

**Features**:
- White balance flag
- Update flag

### SetWhitePoint

**Algorithm**:
```cpp
void ToneMapper::setWhitePoint(float whitePoint)
{
    mWhitePoint = std::clamp(whitePoint, kWhitePointMin, kWhitePointMax);
    mUpdateToneMapPass = true;
}
```

**Features**:
- Clamping (1905-25000 K)
- Update flag

### SetOperator

**Algorithm**:
```cpp
void ToneMapper::setOperator(Operator op)
{
    if (op != mOperator)
    {
        mOperator = op;
        mRecreateToneMapPass = true;
    }
}
```

**Features**:
- Operator change detection
- Recreate flag

### SetClamp

**Algorithm**:
```cpp
void ToneMapper::setClamp(bool clamp)
{
    if (clamp != mClamp)
    {
        mClamp = clamp;
        mRecreateToneMapPass = true;
    }
}
```

**Features**:
- Clamp change detection
- Recreate flag

### SetWhiteMaxLuminance

**Algorithm**:
```cpp
void ToneMapper::setWhiteMaxLuminance(float maxLuminance)
{
    mWhiteMaxLuminance = maxLuminance;
    mUpdateToneMapPass = true;
}
```

**Features**:
- White max luminance (ModifiedReinhard)
- Update flag

### SetWhiteScale

**Algorithm**:
```cpp
void ToneMapper::setWhiteScale(float whiteScale)
{
    mWhiteScale = std::max(0.001f, whiteScale);
    mUpdateToneMapPass = true;
}
```

**Features**:
- White scale (HableUc2)
- Minimum clamping (0.001)
- Update flag

### SetFNumber

**Algorithm**:
```cpp
void ToneMapper::setFNumber(float fNumber)
{
    mFNumber = std::clamp(fNumber, kFNumberMin, kFNumberMax);
    updateExposureValue();
    mUpdateToneMapPass = true;
}
```

**Features**:
- Clamping (0.1-100)
- EV update
- Update flag

### SetShutter

**Algorithm**:
```cpp
void ToneMapper::setShutter(float shutter)
{
    mShutter = std::clamp(shutter, kShutterMin, kShutterMax);
    updateExposureValue();
    mUpdateToneMapPass = true;
}
```

**Features**:
- Clamping (0.1-10000)
- EV update
- Update flag

### SetExposureMode

**Algorithm**:
```cpp
void ToneMapper::setExposureMode(ExposureMode mode)
{
    mExposureMode = mode;
}
```

**Features**:
- Exposure mode setting
- No update flag

### CreateLuminancePass

**Algorithm**:
```cpp
void ToneMapper::createLuminancePass()
{
    mpLuminancePass = FullScreenPass::create(mpDevice, kLuminanceFile);
}
```

**Features**:
- Full-screen pass creation
- Shader file: Luminance.ps.slang

### CreateToneMapPass

**Algorithm**:
```cpp
void ToneMapper::createToneMapPass()
{
    DefineList defines;
    defines.add("_TONE_MAPPER_OPERATOR", std::to_string(static_cast<uint32_t>(mOperator)));
    if (mAutoExposure)
        defines.add("_TONE_MAPPER_AUTO_EXPOSURE");
    if (mClamp)
        defines.add("_TONE_MAPPER_CLAMP");

    mpToneMapPass = FullScreenPass::create(mpDevice, kToneMappingFile, defines);
}
```

**Features**:
- Operator define
- Auto-exposure define (conditional)
- Clamp define (conditional)
- Full-screen pass creation
- Shader file: ToneMapping.ps.slang

### UpdateWhiteBalanceTransform

**Algorithm**:
```cpp
void ToneMapper::updateWhiteBalanceTransform()
{
    // Calculate color transform for current white point.
    mWhiteBalanceTransform = mWhiteBalance ? calculateWhiteBalanceTransformRGB_Rec709(mWhitePoint) : float3x3::identity();
    // Calculate source illuminant, i.e. color that transforms to a pure white (1, 1, 1) output at current color settings.
    mSourceWhite = mul(inverse(mWhiteBalanceTransform), float3(1, 1, 1));
}
```

**Features**:
- White balance transform calculation (Rec.709)
- Identity transform when white balance disabled
- Source illuminant calculation
- ColorUtils integration

### UpdateColorTransform

**Algorithm**:
```cpp
void ToneMapper::updateColorTransform()
{
    // Exposure scale due to exposure compensation.
    float exposureScale = pow(2.f, mExposureCompensation);
    float manualExposureScale = 1.f;
    if (!mAutoExposure)
    {
        float normConstant = 1.f / 100.f;
        manualExposureScale = (normConstant * mFilmSpeed) / (mShutter * mFNumber * mFNumber);
    }
    // Calculate final transform.
    mColorTransform = mWhiteBalanceTransform * exposureScale * manualExposureScale;
}
```

**Features**:
- Exposure compensation scale (2^exposureCompensation)
- Manual exposure scale (normConstant * filmSpeed / (shutter * fNumber²))
- Final transform composition (whiteBalance * exposureCompensation * manualExposure)
- Auto-exposure handling

## Architecture Patterns

### Render Pass Pattern

ToneMapper inherits from [`RenderPass`](Source/Falcor/RenderGraph/RenderPass.h:1) base class:
- Implements standard render pass interface
- Input/output specification (reflect)
- Execute method
- Property serialization (getProperties)
- Scene integration (setScene)
- UI rendering (renderUI)

### Full-Screen Pass Pattern

ToneMapper uses full-screen passes:
- Luminance pass (Luminance.ps.slang)
- Tone mapping pass (ToneMapping.ps.slang)
- Single dispatch per pass per frame
- Pixel shader execution

### Two-Pass Pattern

ToneMapper uses two-pass approach:
- Luminance pass (auto-exposure)
- Tone mapping pass (main)
- Conditional execution (luminance only for auto-exposure)
- FBO for intermediate results

### Auto-Exposure Pattern

ToneMapper implements auto-exposure:
- Luminance computation (log luminance)
- Mip chain generation
- Average luminance from mip level 16
- Exposure key (0.042)
- Exposure scale (kExposureKey / avgLuminance)

### Color Grading Pattern

ToneMapper implements color grading:
- White balance transform (Rec.709)
- Exposure compensation (2^exposureCompensation)
- Manual exposure scale (normConstant * filmSpeed / (shutter * fNumber²))
- Final transform composition

## Code Patterns

### Property Serialization Pattern

```cpp
Properties ToneMapper::getProperties() const
{
    Properties props;
    props[kOutputSize] = mOutputSizeSelection;
    if (mOutputSizeSelection == RenderPassHelpers::IOSize::Fixed)
        props[kFixedOutputSize] = mFixedOutputSize;
    if (mOutputFormat != ResourceFormat::Unknown)
        props[kOutputFormat] = mOutputFormat;
    props[kUseSceneMetadata] = mUseSceneMetadata;
    props[kExposureCompensation] = mExposureCompensation;
    props[kAutoExposure] = mAutoExposure;
    props[kFilmSpeed] = mFilmSpeed;
    props[kWhiteBalance] = mWhiteBalance;
    props[kWhitePoint] = mWhitePoint;
    props[kOperator] = mOperator;
    props[kClamp] = mClamp;
    props[kWhiteMaxLuminance] = mWhiteMaxLuminance;
    props[kWhiteScale] = mWhiteScale;
    props[kFNumber] = mFNumber;
    props[kShutter] = mShutter;
    props[kExposureMode] = mExposureMode;
    return props;
}
```

### Property Parsing Pattern

```cpp
void ToneMapper::parseProperties(const Properties& props)
{
    for (const auto& [key, value] : props)
    {
        if (key == kOutputSize)
            mOutputSizeSelection = value;
        else if (key == kOutputFormat)
            mOutputFormat = value;
        else if (key == kFixedOutputSize)
            mFixedOutputSize = value;
        // ... more properties
        else
            logWarning("Unknown property '{}' in a ToneMapping properties.", key);
    }
}
```

### Lazy Resource Creation Pattern

```cpp
void ToneMapper::createLuminanceFbo(const ref<Texture>& pSrc)
{
    bool createFbo = mpLuminanceFbo == nullptr;
    ResourceFormat srcFormat = pSrc->getFormat();
    uint32_t bytesPerChannel = getFormatBytesPerBlock(srcFormat) / getFormatChannelCount(srcFormat);

    // Find required texture size and format
    ResourceFormat luminanceFormat = (bytesPerChannel == 4) ? ResourceFormat::R32Float : ResourceFormat::R16Float;
    uint32_t requiredHeight = fstd::bit_floor(pSrc->getHeight());
    uint32_t requiredWidth = fstd::bit_floor(pSrc->getWidth());

    if (createFbo == false)
    {
        createFbo = (requiredWidth != mpLuminanceFbo->getWidth()) || (requiredHeight != mpLuminanceFbo->getHeight()) ||
                    (luminanceFormat != mpLuminanceFbo->getColorTexture(0)->getFormat());
    }

    if (createFbo)
    {
        Fbo::Desc desc;
        desc.setColorTarget(0, luminanceFormat);
        mpLuminanceFbo = Fbo::create2D(mpDevice, requiredWidth, requiredHeight, desc, 1, Fbo::kAttachEntireMipLevel);
    }
}
```

### Shader Recompile Pattern

```cpp
void ToneMapper::createToneMapPass()
{
    DefineList defines;
    defines.add("_TONE_MAPPER_OPERATOR", std::to_string(static_cast<uint32_t>(mOperator)));
    if (mAutoExposure)
        defines.add("_TONE_MAPPER_AUTO_EXPOSURE");
    if (mClamp)
        defines.add("_TONE_MAPPER_CLAMP");

    mpToneMapPass = FullScreenPass::create(mpDevice, kToneMappingFile, defines);
}
```

### Plugin Registration Pattern

```cpp
extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, ToneMapper>();
    ScriptBindings::registerBinding(regToneMapper);
}
```

## Integration Points

### Render Graph Integration

ToneMapper integrates with render graph system:
- Input channels: 1 (src)
- Output channels: 1 (dst)
- Output size configuration (Default, Fixed, MatchInput)
- Output format configuration
- Resampling support (with warning)

### Scene Integration

ToneMapper integrates with scene system:
- Scene metadata integration (filmISO, fNumber, shutterSpeed)
- Automatic camera parameter setup
- Conditional metadata usage (mUseSceneMetadata)

### ColorUtils Integration

ToneMapper integrates with color utilities:
- White balance transform calculation (calculateWhiteBalanceTransformRGB_Rec709)
- Rec.709 color space
- Color temperature support (1905-25000 K)

### Scripting Integration

ToneMapper integrates with scripting:
- Python bindings for all parameters
- Property serialization
- Enum conversion (stringToEnum, enumToString)

### UI Integration

ToneMapper integrates with UI system:
- Grouped controls (Exposure, Color Grading, Tonemapping)
- Dropdown menus (Output size, Exposure mode, Operator)
- Sliders (Exposure compensation, Exposure value, Film speed, f-number, Shutter, White point, White luminance, Linear white)
- Checkboxes (Auto exposure, White balance, Clamp output)
- Color display (White point)
- Recompile trigger (Output size changes)

## Use Cases

### Rendering Applications

HDR to LDR conversion:
- Tone mapping for display
- Auto-exposure for dynamic scenes
- White balance for color correction
- Multiple tone mapping operators

### Real-Time Rendering

Real-time rendering applications:
- Real-time tone mapping
- Auto-exposure adaptation
- Interactive parameter adjustment
- Performance optimization

### Cinematic Rendering

Cinematic rendering applications:
- Filmic tone mapping (ACES, HableUc2)
- Manual exposure control
- Camera parameter simulation
- Color grading support

### Photography Simulation

Photography simulation applications:
- Camera parameter simulation (f-number, shutter, film speed)
- Exposure value calculation
- Exposure mode selection (AperturePriority, ShutterPriority)
- Scene metadata integration

### Debugging and Analysis

Debugging and analysis use cases:
- Tone mapping comparison
- Exposure analysis
- Color balance analysis
- Luminance analysis

### Research and Development

Research and development use cases:
- Tone mapping algorithm research
- Auto-exposure algorithm development
- Color grading research
- Camera parameter simulation

### Educational Use

Educational use cases:
- Tone mapping demonstration
- HDR to LDR conversion
- Auto-exposure explanation
- Color balance demonstration

## Performance Considerations

### GPU Computation

**Luminance Pass**:
- Full-screen pixel shader: O(width × height) operations
- Log luminance computation: O(1) per pixel
- Mip chain generation: O(width × height) operations
- Total: O(width × height) per frame

**Tone Mapping Pass**:
- Full-screen pixel shader: O(width × height) operations
- Tone mapping operator: O(1) per pixel
- Color transform: O(9) per pixel (3x3 matrix multiply)
- Auto-exposure: O(1) per pixel (luminance lookup)
- Total: O(width × height) per frame

**Total GPU Computation**:
- Luminance pass: O(width × height) per frame (conditional)
- Tone mapping pass: O(width × height) per frame
- Total: O(width × height) per frame (auto-exposure), O(width × height) per frame (manual exposure)

### Memory Usage

**Luminance Pass**:
- Luminance texture: O(bit_floor(width) × bit_floor(height) × sizeof(float16/float32)) bytes
- Mip chain: O(bit_floor(width) × bit_floor(height) × sizeof(float16/float32) × log2(min(width, height))) bytes
- Total: O(bit_floor(width) × bit_floor(height) × sizeof(float16/float32) × (1 + log2(min(width, height)))) bytes

**Tone Mapping Pass**:
- Tone map texture: O(width × height × sizeof(float4)) bytes
- Total: O(width × height × 16) bytes

**Total Memory Usage**:
- Luminance pass: O(bit_floor(width) × bit_floor(height) × sizeof(float16/float32) × (1 + log2(min(width, height)))) bytes (conditional)
- Tone mapping pass: O(width × height × 16) bytes
- Total: O(width × height × (16 + sizeof(float16/float32) × (1 + log2(min(width, height))))) bytes (auto-exposure), O(width × height × 16) bytes (manual exposure)

### Computational Complexity

**Luminance Pass**:
- Log luminance computation: O(1) per pixel
- Total: O(width × height) per frame

**Tone Mapping Pass**:
- Tone mapping operator: O(1) per pixel
- Color transform: O(9) per pixel
- Auto-exposure: O(1) per pixel
- Total: O(width × height) per frame

**Total Computational Complexity**:
- Luminance pass: O(width × height) per frame (conditional)
- Tone mapping pass: O(width × height) per frame
- Overall: O(width × height) per frame

### Program Management Overhead

**Shader Compilation**:
- Luminance pass: O(1) on creation
- Tone mapping pass: O(1) on operator/auto-exposure/clamp change
- Total: O(1) on parameter changes

**FBO Creation**:
- Luminance FBO: O(1) on size/format change
- Total: O(1) on size/format changes

**Parameter Update**:
- White balance transform: O(1) on white balance change
- Color transform: O(1) on exposure change
- Total: O(1) on parameter changes

## Limitations

### Feature Limitations

**Tone Mapping Limitations**:
- Limited to 6 tone mapping operators
- No custom tone mapping operators
- No temporal tone mapping
- No adaptive tone mapping
- No local tone mapping
- No tone mapping presets
- No tone mapping curves

**Auto-Exposure Limitations**:
- Simple average luminance (no histogram)
- No temporal adaptation
- No exposure smoothing
- No exposure clamping
- No exposure key adjustment
- No exposure adaptation speed control

**Color Grading Limitations**:
- Limited to white balance
- No color curves
- No color wheels
- No saturation control
- No contrast control
- No gamma control
- No lift/gamma/gain

**Camera Parameter Limitations**:
- Limited to f-number, shutter, film speed
- No ISO auto
- No aperture priority auto
- No shutter priority auto
- No manual mode
- No exposure compensation limits

**Integration Limitations**:
- Limited to Falcor's scene system
- Requires scene metadata for auto-setup
- No external data sources
- No scripting support for custom operators
- Limited to Rec.709 color space

### Performance Limitations

**Performance Limitations**:
- No performance optimization
- No adaptive quality settings
- No performance profiling tools
- No benchmarking capabilities
- No statistics display

### UI Limitations

**UI Limitations**:
- No preset management
- No undo/redo functionality
- No real-time preview
- No comparison tools
- No histogram display
- No luminance graph

## Best Practices

### Tone Mapping Best Practices

**Operator Selection**:
- Use Linear for no tone mapping
- Use Reinhard for simple tone mapping
- Use ReinhardModified for better highlight handling
- Use HejiHableAlu for filmic look with sRGB
- Use HableUc2 for Uncharted 2 filmic look
- Use Aces for ACES filmic look (recommended)

**Auto-Exposure Configuration**:
- Enable auto-exposure for dynamic scenes
- Disable auto-exposure for cinematic control
- Use appropriate exposure key (default: 0.042)
- Test with different exposure keys
- Monitor average luminance

**Manual Exposure Configuration**:
- Use appropriate exposure value for scene
- Set film speed (ISO) for camera simulation
- Set f-number for depth of field
- Set shutter for motion blur
- Use aperture priority for fixed aperture
- Use shutter priority for fixed shutter

**Color Grading Configuration**:
- Enable white balance for color correction
- Set appropriate white point (6500K for daylight)
- Test with different white points
- Monitor source illuminant color
- Use scene metadata for auto-setup

**Clamping Configuration**:
- Enable clamping for display output
- Disable clamping for intermediate results
- Test with and without clamping
- Monitor out-of-range values

**Output Configuration**:
- Use Default size for match input
- Use Fixed size for specific resolution
- Use appropriate output format (sRGB for display)
- Test with different output formats
- Monitor resampling warnings

### Performance Optimization

**Performance Optimization**:
- Disable auto-exposure for static scenes
- Use appropriate tone mapping operator
- Test with different output sizes
- Profile GPU performance
- Optimize shader compilation

**Memory Optimization**:
- Use appropriate output format (R16Float for 16-bit channels)
- Monitor memory usage
- Test with different resolutions
- Optimize FBO creation

### Debugging

**Debugging Best Practices**:
- Test with different tone mapping operators
- Verify auto-exposure behavior
- Check white balance transform
- Monitor color transform values
- Validate exposure calculations
- Test with different camera parameters

**Rendering Pipeline Debugging**:
- Verify luminance computation
- Check tone mapping operator output
- Validate color grading results
- Test with different input formats
- Monitor resampling behavior

**Performance Debugging**:
- Profile luminance pass performance
- Profile tone mapping pass performance
- Measure memory bandwidth
- Test with different resolutions
- Optimize shader compilation

### Research and Development

**Research and Development**:
- Study tone mapping algorithms
- Analyze auto-exposure techniques
- Research color grading methods
- Investigate camera parameter simulation
- Develop new tone mapping operators

**Educational Use**:
- Demonstrate tone mapping concepts
- Show HDR to LDR conversion
- Explain auto-exposure algorithms
- Teach color grading techniques
- Visualize camera parameters

## Related Components

### RenderGraph/RenderPass

Render pass base class:
- `RenderPass::reflect()` - Define input/output resources
- `RenderPass::execute()` - Execute render pass
- `RenderPass::renderUI()` - Render UI controls
- `RenderPass::getProperties()` - Serialize configuration
- `RenderPass::setScene()` - Set scene

### Core/Pass/FullScreenPass

Full-screen pass:
- `FullScreenPass::create()` - Create full-screen pass
- `FullScreenPass::execute()` - Execute full-screen pass
- `FullScreenPass::getRootVar()` - Get root variable

### Utils/Color/ColorUtils

Color utilities:
- `calculateWhiteBalanceTransformRGB_Rec709()` - Calculate white balance transform
- Rec.709 color space support
- Color temperature support

### Scene

Scene system:
- `Scene::getMetadata()` - Get scene metadata
- `Scene::Metadata::filmISO` - Film speed (ISO)
- `Scene::Metadata::fNumber` - Lens speed (f-number)
- `Scene::Metadata::shutterSpeed` - Shutter speed

## Progress Log

- **2026-01-07T22:30:00Z**: Completed ToneMapper analysis. Analyzed ToneMapper.h (172 lines), ToneMapper.cpp (550 lines), ToneMapperParams.slang (71 lines), ToneMapping.ps.slang (168 lines), and Luminance.ps.slang (42 lines). Created comprehensive technical specification covering tone mapping operators (Linear, Reinhard, ReinhardModified, HejiHableAlu, HableUc2, Aces), auto-exposure with luminance-based adaptation, manual exposure with camera parameters (f-number, shutter, film speed), white balance with color temperature control, color grading with exposure compensation and color transforms, output clamping, scene metadata integration, two-pass execution (luminance pass + tone mapping pass), render graph integration, UI controls, scripting support, performance considerations, limitations, and best practices. Marked ToneMapper as Complete.
