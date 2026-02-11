# SimplePostFX - Simple Post-Processing Effects

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **SimplePostFX** - Simple post-processing effects
  - [x] **SimplePostFX.h** - Simple post FX header (135 lines)
  - [x] **SimplePostFX.cpp** - Simple post FX implementation (391 lines)
  - [x] **SimplePostFX.cs.slang** - Simple post FX shader (241 lines)

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Texture, Buffer, Formats, Sampler)
- **Core/Object** - Base object class
- **Core/Program** - Shader program management (ComputePass, Program)
- **RenderGraph** - Render graph system (RenderPass, RenderPassReflection, RenderPassHelpers)
- **Utils/Color** - Color utilities (ColorHelpers, luminance)
- **Utils/UI** - UI utilities (Gui)

## Module Overview

SimplePostFX is a comprehensive post-processing effects pass implementing multiple image enhancement techniques including bloom, vignette, chromatic aberration, barrel distortion, saturation curves, and color grading. The pass uses an image pyramid with 8 levels for efficient multi-scale bloom computation, with optional star effects, lens distortion, and ASC-CDL-style color grading. It provides a wipe effect for transitioning between processed and unprocessed images, with extensive UI controls for all parameters.

## Component Specifications

### SimplePostFX Class

**File**: [`SimplePostFX.h`](Source/RenderPasses/SimplePostFX/SimplePostFX.h:35)

**Purpose**: Simple set of post-processing effects.

**Public Interface**:

```cpp
class SimplePostFX : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(SimplePostFX, "SimplePostFX", "Simple set of post effects.");

    static ref<SimplePostFX> create(ref<Device> pDevice, const Properties& props) { return make_ref<SimplePostFX>(pDevice, props); }

    SimplePostFX(ref<Device> pDevice, const Properties& props);

    Properties getProperties() const override;
    RenderPassReflection reflect(const CompileData& compileData) override;
    void compile(RenderContext* pRenderContext, const CompileData& compileData) override {}
    void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    void renderUI(Gui::Widgets& widget) override;
    void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override {}

    // Getter methods for all parameters
    bool getEnabled() const { return mEnabled; }
    float getWipe() const { return mWipe; }
    float getBloomAmount() const { return mBloomAmount; }
    float getStarAmount() const { return mStarAmount; }
    float getStarAngle() const { return mStarAngle; }
    float getVignetteAmount() const { return mVignetteAmount; }
    float getChromaticAberrationAmount() const { return mChromaticAberrationAmount; }
    float getBarrelDistortAmount() const { return mBarrelDistortAmount; }
    float3 getSaturationCurve() const { return mSaturationCurve; }
    float3 getColorOffset() const { return mColorOffset; }
    float3 getColorScale() const { return mColorScale; }
    float3 getColorPower() const { return mColorPower; }
    float getColorOffsetScalar() const { return mColorOffsetScalar; }
    float getColorScaleScalar() const { return mColorScaleScalar; }
    float getColorPowerScalar() const { return mColorPowerScalar; }

    // Setter methods for all parameters
    void setEnabled(bool e) { mEnabled = e; }
    void setWipe(float v) { mWipe = v; }
    void setBloomAmount(float v) { mBloomAmount = v; }
    void setStarAmount(float v) { mStarAmount = v; }
    void setStarAngle(float v) { mStarAngle = v; }
    void setVignetteAmount(float v) { return mVignetteAmount = v; }
    void setChromaticAberrationAmount(float v) { mChromaticAberrationAmount = v; }
    void setBarrelDistortAmount(float v) { mBarrelDistortAmount = v; }
    void setSaturationCurve(float3 v) { mSaturationCurve = v; }
    void setColorOffset(float3 v) { mColorOffset = v; }
    void setColorScale(float3 v) { mColorScale = v; }
    void setColorPower(float3 v) { mColorPower = v; }
    void setColorOffsetScalar(float v) { mColorOffsetScalar = v; }
    void setColorScaleScalar(float v) { mColorScaleScalar = v; }
    void setColorPowerScalar(float v) { mColorPowerScalar = v; }

private:
    void preparePostFX(RenderContext* pRenderContext, uint32_t width, uint32_t height);

    const static int kNumLevels = 8;

    /// Selected output size.
    RenderPassHelpers::IOSize mOutputSizeSelection = RenderPassHelpers::IOSize::Default;
    /// Output size in pixels when 'Fixed' size is selected.
    uint2 mFixedOutputSize = {512, 512};

    ref<ComputePass> mpDownsamplePass;
    ref<ComputePass> mpUpsamplePass;
    ref<ComputePass> mpPostFXPass;

    /// Image pyramid, fine to coarse, full res down in steps of 4x (16x area).
    ref<Texture> mpPyramid[kNumLevels + 1];
    ref<Sampler> mpLinearSampler;

    /// Wipe across to see effect without fx. 0 <= all effect, 1 >= disabled.
    float mWipe = 0.f;
    /// Enable the entire pass.
    bool mEnabled = true;

    /// Amount of bloom.
    float mBloomAmount = 0.f;
    /// How much of a 6 pointed star to add to the bloom kernel.
    float mStarAmount = 0.f;
    /// Angle of star rays.
    float mStarAngle = 0.1f;
    /// Amount of circular vignetting.
    float mVignetteAmount = 0.f;
    /// Amount of radial chromatic aberration.
    float mChromaticAberrationAmount = 0.f;
    /// Amount of barrel distortion.
    float mBarrelDistortAmount = 0.f;
    /// Saturation amount for shadows, midtones and highlights.
    float3 mSaturationCurve = float3(1.f, 1.f, 1.f);
    /// Color offset, tints shadows.
    float3 mColorOffset = float3(0.5f, 0.5f, 0.5f);
    /// Color scale, tints highlights.
    float3 mColorScale = float3(0.5f, 0.5f, 0.5f);
    /// Color power (gamma), tints midtones.
    float3 mColorPower = float3(0.5f, 0.5f, 0.5f);

    // Above colors are also offered as scalars for ease of UI and also to set negative colors.

    /// Luma offset, crushes shadows if negative.
    float mColorOffsetScalar = 0.f;
    /// Luma scale, effectively another exposure control.
    float mColorScaleScalar = 0.f;
    /// Luma power, ie a gamma curve.
    float mColorPowerScalar = 0.f;
};
```

**Public Members**: None (all members are private)

**Private Members**:
- `ref<ComputePass> mpDownsamplePass` - Downsample compute pass
- `ref<ComputePass> mpUpsamplePass` - Upsample compute pass
- `ref<ComputePass> mpPostFXPass` - PostFX compute pass
- `ref<Texture> mpPyramid[kNumLevels + 1]` - Image pyramid (9 textures)
- `ref<Sampler> mpLinearSampler` - Linear sampler
- `RenderPassHelpers::IOSize mOutputSizeSelection` - Output size selection
- `uint2 mFixedOutputSize` - Fixed output size (512x512)
- `float mWipe` - Wipe effect (0.0f = all effect, >=1.0f = disabled)
- `bool mEnabled` - Enable/disable entire pass
- `float mBloomAmount` - Bloom amount
- `float mStarAmount` - Star amount
- `float mStarAngle` - Star angle
- `float mVignetteAmount` - Vignette amount
- `float mChromaticAberrationAmount` - Chromatic aberration amount
- `float mBarrelDistortAmount` - Barrel distortion amount
- `float3 mSaturationCurve` - Saturation curve (shadow, midtone, highlight)
- `float3 mColorOffset` - Color offset
- `float3 mColorScale` - Color scale
- `float3 mColorPower` - Color power (gamma)
- `float mColorOffsetScalar` - Luma offset scalar
- `float mColorScaleScalar` - Luma scale scalar
- `float mColorPowerScalar` - Luma power scalar

**Private Methods**:
- `void preparePostFX(RenderContext* pRenderContext, uint32_t width, uint32_t height)` - Prepare postFX textures

**Public Methods**:
- `static void registerBindings(pybind11::module& m)` - Register Python bindings

### SimplePostFX.cs.slang Shader

**File**: [`SimplePostFX.cs.slang`](Source/RenderPasses/SimplePostFX/SimplePostFX.cs.slang:1)

**Purpose**: Simple post-processing effects shader.

**Constant Buffer (PerFrameCB)**:
```cpp
cbuffer PerFrameCB
{
    uint2 gResolution;
    float2 gInvRes;
    float gBloomAmount;
    float gVignetteAmount;
    float gChromaticAberrationAmount;
    float2 gBarrelDistort;
    float3 gSaturationCurve;
    float3 gColorOffset;
    float3 gColorScale;
    float3 gColorPower;
    float gStar;
    float2 gStarDir1;
    float2 gStarDir2;
    float2 gStarDir3;
    float gWipe;
    bool gInPlace;
};
```

**Shader Resources**:
- `Texture2D<float4> gSrc` - Source texture
- `Texture2D<float4> gBloomed` - Bloomed texture (from pyramid)
- `RWTexture2D<float4> gDst` - Destination texture (read-write)
- `SamplerState gLinearSampler` - Linear sampler

**Entry Points**:

1. **downsample** - Downsample shader (5x5 bilinear tap kernel)
```cpp
[numthreads(16, 16, 1)]
void downsample(uint3 dispatchThreadId: SV_DispatchThreadID)
```
- Purpose: Create image pyramid by downsampling source texture
- Algorithm: 5x5 bilinear tap kernel (effective 4x4 footprint)
- Offsets: Chosen to sample from binomial 1 3 3 1
- Weight: 0.25f (normalized)
- Dispatch: 16x16 thread groups
- Output: Writes to pyramid levels

2. **upsample** - Upsample shader (3x3 bilinear tap kernel with optional star effects)
```cpp
[numthreads(16, 16, 1)]
void upsample(uint3 dispatchThreadId: SV_DispatchThreadID)
```
- Purpose: Upsample pyramid levels with bloom and optional star effects
- Algorithm: 3x3 bilinear tap kernel (effective 6x6 footprint)
- Offsets: Chosen to sample from binomial 1 5 10 10 5 1
- Weights: Calculated from binomial coefficients
- Star effects: Optional 6-point star pattern with configurable amount and angle
- Dispatch: 16x16 thread groups
- Output: Blends current level with coarser level

3. **runPostFX** - Post-processing shader
```cpp
[numthreads(16, 16, 1)]
void runPostFX(uint3 dispatchThreadId: SV_DispatchThreadID)
```
- Purpose: Apply all post-processing effects to final image
- Effects:
  - Barrel distortion: Radial distortion with configurable amount
  - Chromatic aberration: RGB channel offset with configurable amount
  - Vignette: Circular falloff with configurable amount
  - Saturation curves: ASC-CDL style saturation with shadow/midtone/highlight control
  - Color offset/scale/power: ASC-CDL style color grading
  - Wipe effect: Horizontal wipe based on pixel position
- Dispatch: 16x16 thread groups
- Output: Final processed image

## Technical Details

### Render Graph Integration

**Input Channels**:
- `src` (required): Source texture (RGBA32Float)

**Output Channels**:
- `dst` (required): Post-processed output texture (RGBA32Float)

**Reflection**:
```cpp
RenderPassReflection reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    const uint2 sz = RenderPassHelpers::calculateIOSize(mOutputSizeSelection, mFixedOutputSize, compileData.defaultTexDims);

    reflector.addInput(kSrc, "Source texture").bindFlags(ResourceBindFlags::ShaderResource);
    reflector.addOutput(kDst, "post-effected output texture")
        .bindFlags(ResourceBindFlags::RenderTarget | ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess)
        .format(ResourceFormat::RGBA32Float)
        .texture2D(sz.x, sz.y);
    return reflector;
}
```

**Features**:
- Single input channel (source texture)
- Single output channel (post-processed texture)
- Configurable output size (Default, Fixed, Custom)
- Fixed output size: 512x512 pixels

### Execute Implementation

**Implementation**:
```cpp
void SimplePostFX::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    auto pSrc = renderData.getTexture(kSrc);
    auto pDst = renderData.getTexture(kDst);
    FALCOR_ASSERT(pSrc && pDst);

    // Issue error and disable pass if I/O size doesn't match
    if (getEnabled() && (pSrc->getWidth() != pDst->getWidth() || pSrc->getHeight() != pDst->getHeight()))
    {
        logError("SimplePostFX I/O sizes don't match. The pass will be disabled.");
        mEnabled = false;
    }
    const uint2 resolution = uint2(pSrc->getWidth(), pSrc->getHeight());

    // If we have 'identity' settings, we can just copy input to output
    if (getEnabled() == false || getWipe() >= 1.f || (
        getBloomAmount() == 0.f &&
        getChromaticAberrationAmount() == 0.f &&
        getBarrelDistortAmount() == 0.f &&
        all(getSaturationCurve() == float3(1.f)) &&
        all(getColorOffset() == float3(0.5f)) &&
        all(getColorScale() == float3(0.5f)) &&
        all(getColorPower() == float3(0.5f)) &&
        getColorOffsetScalar() == 0.f &&
        getColorScaleScalar() == 0.f &&
        getColorPowerScalar() == 0.f
        ))
    {
        // Wipe is all the way across, which corresponds to no effect
        pRenderContext->blit(pSrc->getSRV(), pDst->getRTV());
        return;
    }

    // Prepare postFX textures
    preparePostFX(pRenderContext, resolution.x, resolution.y);

    // Downsampling phase (8 levels of 4x downsampling)
    if (getBloomAmount() > 0.f)
    {
        for (int level = 0; level < kNumLevels; ++level)
        {
            uint2 res = {std::max(1u, resolution.x >> (level + 1)), std::max(1u, resolution.y >> (level + 1))};
            float2 invres = float2(1.f / res.x, 1.f / res.y);
            // Execute downsample pass
            mpDownsamplePass->execute(pRenderContext, uint3(res, 1));
        }
    }

    // Upsampling phase (8 levels of 4x upsampling with bloom and star effects)
    {
        for (int level = kNumLevels - 1; level >= 0; --level)
        {
            uint2 res = {std::max(1u, resolution.x >> level), std::max(1u, resolution.y >> level)};
            float2 invres = float2(1.f / res.x, 1.f / res.y);
            bool wantStar = level == 1 || level == 2;
            // Execute upsample pass
            mpUpsamplePass->execute(pRenderContext, uint3(res, 1));
        }
    }

    // PostFX phase (apply all effects)
    {
        auto var = mpPostFXPass->getRootVar();
        var["PerFrameCB"]["gResolution"] = resolution;
        var["PerFrameCB"]["gInvRes"] = float2(1.f / resolution.x, 1.f / resolution.y);
        var["PerFrameCB"]["gVignetteAmount"] = getVignetteAmount();
        var["PerFrameCB"]["gChromaticAberrationAmount"] = getChromaticAberrationAmount() * (1.f / 64.f);
        float barrel = getBarrelDistortAmount() * 0.125f;
        var["PerFrameCB"]["gBarrelDistort"] = float2(1.f / (1.f + 4.f * barrel), barrel);
        float3 satcurve = getSaturationCurve();
        // Fit a quadratic through 3 points
        satcurve.y -= satcurve.x;
        satcurve.z -= satcurve.x;
        float A = 2.f * satcurve.z - 4.f * satcurve.y;
        float B = satcurve.z - A;
        float C = satcurve.x;
        var["PerFrameCB"]["gSaturationCurve"] = float3(A, B, C);
        var["PerFrameCB"]["gColorOffset"] = getColorOffset() + getColorOffsetScalar() - 0.5f;
        var["PerFrameCB"]["gColorScale"] = getColorScale() * std::exp2(1.f + 2.f * getColorScaleScalar());
        var["PerFrameCB"]["gColorPower"] = exp2(3.f * (0.5f - getColorPower() - getColorPowerScalar()));
        var["PerFrameCB"]["gWipe"] = mWipe * resolution.x;
        var["gBloomed"] = getBloomAmount() > 0.f ? mpPyramid[0] : pSrc;
        var["gSrc"] = pSrc;
        var["gDst"] = pDst;
        var["gLinearSampler"] = mpLinearSampler;
        mpPostFXPass->execute(pRenderContext, uint3(resolution, 1));
    }
}
```

**Features**:
- I/O size validation with automatic disable on mismatch
- Identity optimization (direct blit when all effects disabled)
- Three-phase pipeline:
  1. Downsampling: 8 levels of 4x downsampling using 5x5 bilinear kernel
  2. Upsampling: 8 levels of 4x upsampling using 3x3 bilinear kernel with bloom and star effects
  3. PostFX: Apply all lens and color effects
- Image pyramid with 9 textures (8 levels + source)
- Configurable output size (Default, Fixed, Custom)
- Wipe effect for horizontal transition
- Automatic recompile on output size change

### Property Serialization

**Implementation**:
```cpp
Properties SimplePostFX::getProperties() const
{
    Properties props;
    props[kEnabled] = getEnabled();
    props[kOutputSize] = mOutputSizeSelection;
    if (mOutputSizeSelection == RenderPassHelpers::IOSize::Fixed)
        props[kFixedOutputSize] = mFixedOutputSize;
    props[kWipe] = getWipe();
    props[kBloomAmount] = getBloomAmount();
    props[kStarAmount] = getStarAmount();
    props[kStarAngle] = getStarAngle();
    props[kVignetteAmount] = getVignetteAmount();
    props[kChromaticAberrationAmount] = getChromaticAberrationAmount();
    props[kBarrelDistortAmount] = getBarrelDistortAmount();
    props[kSaturationCurve] = getSaturationCurve();
    props[kColorOffset] = getColorOffset();
    props[kColorScale] = getColorScale();
    props[kColorPower] = getColorPower();
    props[kColorOffsetScalar] = getColorOffsetScalar();
    props[kColorScaleScalar] = getColorScaleScalar();
    props[kColorPowerScalar] = getColorPowerScalar();
    return props;
}
```

**Features**:
- Complete serialization of all parameters
- Output size selection (Default, Fixed, Custom)
- Fixed output size specification (512x512)
- All effect parameters serialized

### UI Rendering

**Implementation**:
```cpp
void SimplePostFX::renderUI(Gui::Widgets& widget)
{
    // Controls for output size
    if (widget.dropdown("Output size", mOutputSizeSelection))
        requestRecompile();
    if (mOutputSizeSelection == RenderPassHelpers::IOSize::Fixed)
    {
        if (widget.var("Size in pixels", mFixedOutputSize, 32u, 16384u))
            requestRecompile();
    }

    // PostFX options
    widget.checkbox("Enable post fx", mEnabled);
    widget.slider("Wipe", mWipe, 0.f, 1.f);

    // Lens FX group
    if (auto group = widget.group("Lens FX", true))
    {
        group.slider("Bloom", mBloomAmount, 0.f, 1.f);
        group.slider("Bloom Star", mStarAmount, 0.f, 1.f);
        group.slider("Star Angle", mStarAngle, 0.f, 1.f, true);
        group.slider("Vignette", mVignetteAmount, 0.f, 1.f);
        group.slider("Chromatic Aberration", mChromaticAberrationAmount, 0.f, 1.f);
        group.slider("Barrel Distortion", mBarrelDistortAmount, 0.f, 1.f);
        if (group.button("reset this group"))
        {
            mBloomAmount = 0.f;
            mStarAmount = 0.f;
            mStarAngle = 0.1f;
            mVignetteAmount = 0.f;
            mChromaticAberrationAmount = 0.f;
            mBarrelDistortAmount = 0.f;
        }
    }

    // Saturation group
    if (auto group = widget.group("Saturation", true))
    {
        group.slider("Shadow Saturation", mSaturationCurve.x, 0.f, 2.f);
        group.slider("Midtone Saturation", mSaturationCurve.y, 0.f, 2.f);
        group.slider("Highlight Saturation", mSaturationCurve.z, 0.f, 2.f);
        if (group.button("reset this group"))
        {
            mSaturationCurve = float3(1.f);
        }
    }

    // Offset/Power/Scale (luma) group
    if (auto group = widget.group("Offset/Power/Scale (luma)", true))
    {
        group.slider("Luma Offset (Shadows)", mColorOffsetScalar, -1.f, 1.f);
        group.slider("Luma Power (Midtones)", mColorPowerScalar, -1.f, 1.f);
        group.slider("Luma Scale (Highlights)", mColorScaleScalar, -1.f, 1.f);
        if (group.button("reset this group"))
        {
            mColorOffsetScalar = 0.f;
            mColorPowerScalar = 0.f;
            mColorScaleScalar = 0.f;
        }
    }

    // Offset/Power/Scale (color) group
    if (auto group = widget.group("Offset/Power/Scale (color)", true))
    {
        if (group.button("reset##1"))
            mColorOffset = float3(0.5f, 0.5f, 0.5f);
        group.rgbColor("Color Offset (Shadows)", mColorOffset, true);

        if (group.button("reset##2"))
            mColorPower = float3(0.5f, 0.5f, 0.5f);
        group.rgbColor("Color Power (Midtones)", mColorPower, true);

        if (group.button("reset##3"))
            mColorScale = float3(0.5f, 0.5f, 0.5f);
        group.rgbColor("Color Scale (Highlights)", mColorScale, true);

        if (group.button("reset this group"))
        {
            mColorOffset = float3(0.5f, 0.5f, 0.5f);
            mColorPower = float3(0.5f, 0.5f, 0.5f);
            mColorScale = float3(0.5f, 0.5f, 0.5f);
        }
    }
}
```

**Features**:
- Output size control (dropdown, fixed size specification)
- Enable/disable checkbox
- Wipe slider
- Lens FX group with sliders and reset button
- Saturation group with shadow/midtone/highlight sliders and reset button
- Luma group with offset/power/scale sliders and reset button
- Color group with offset/power/scale RGB controls and reset buttons
- Automatic recompile on output size change

### Bloom Implementation

**Algorithm**:
- Image pyramid with 8 levels (4x downsampling per level)
- Downsampling: 5x5 bilinear tap kernel (effective 4x4 footprint)
- Upsampling: 3x3 bilinear tap kernel (effective 6x6 footprint)
- Blend: Linear interpolation between current and coarser level based on bloom amount
- Star effects: Optional 6-point star pattern added to upsampling

**Kernel Weights**:
- 5x5 downsampling: Binomial coefficients (1 3 3 1)
- 3x3 upsampling: Binomial coefficients (1 5 10 10 5 1)
- Normalized to sum to 1.0f

**Star Pattern**:
- 6-point star with configurable amount and angle
- 3 rays at 120° intervals (configurable angle offset)
- Brute force line sampling with brightness falloff (1/(10 + |i|))

### Lens Effects

**Vignette**:
- Circular falloff based on radial distance from center
- Formula: `max(0.f, 1.f - r_squared * gVignetteAmount)`
- Cheap circular falloff approximation

**Chromatic Aberration**:
- RGB channel offset based on radial distance
- Formula: `uv_circular *= barrel` where `barrel = 1.f + r_squared * gBarrelDistort.y`
- RGB channels offset by `ndc / ca` where `ca = 1.f + gChromaticAberrationAmount`
- Simulates lens chromatic aberration

**Barrel Distortion**:
- Radial distortion based on squared distance
- Formula: `barrel = 1.f + r_squared * gBarrelDistort.y`
- Scale factor: `1.f / (1.f + 4.f * barrel)` chosen to keep corners of 16:9 viewport fixed

### Saturation Curves

**ASC-CDL Style**:
- Quadratic curve through 3 points (shadow, midtone, highlight)
- Saturation curve: `satcurve = float3(A, B, C)` where:
  - `A = 2.f * satcurve.z - 4.f * satcurve.y`
  - `B = satcurve.z - A`
  - `C = satcurve.x`
- Luma-based saturation: `saturated_luma = saturate(luma / (luma + 0.5f))`
- Final saturation: `max(0.f, gSaturationCurve.z + squashed_luma * gSaturationCurve.y + squashed_luma * squashed_luma * gSaturationCurve.x)`

### Color Grading

**ASC-CDL Style**:
- Color offset: `gColorOffset + gColorOffsetScalar - 0.5f` (tints shadows)
- Color scale: `gColorScale * std::exp2(1.f + 2.f * gColorScaleScalar())` (tints highlights)
- Color power: `exp2(3.f * (0.5f - gColorPower - gColorPowerScalar()))` (gamma curve for midtones)
- Formula: `col = pow(max(0.f, col * gColorScale + gColorOffset), gColorPower)`

### Wipe Effect

- Horizontal wipe based on pixel position
- Formula: `if (pixelPos.x < gWipe) output = gSrc[pixelPos]`
- Wipe amount in pixels: `gWipe * resolution.x`
- Used for transitioning between processed and unprocessed images

## Architecture Patterns

### Render Pass Pattern

- Inherits from [`RenderPass`](Source/Falcor/RenderGraph/RenderPass.h:1) base class
- Implements standard render pass interface:
  - `reflect()` - Defines input/output resources
  - `execute()` - Executes post-processing pipeline
  - `renderUI()` - Renders comprehensive UI
  - `getProperties()` - Serializes configuration
  - `setScene()` - Empty (no scene integration)
  - `compile()` - Empty

### Compute Pass Pattern

- Uses three compute passes:
  - `mpDownsamplePass` - Downsample shader (downsample entry point)
  - `mpUpsamplePass` - Upsample shader (upsample entry point)
  - `mpPostFXPass` - PostFX shader (runPostFX entry point)
- 16x16 thread groups for all passes
- Single dispatch per pass per frame

### Image Pyramid Pattern

- Multi-scale representation with 8 levels
- Fine to coarse: Each level is 4x smaller than previous
- Total pyramid: 9 textures (8 levels + source)
- Downsampling: 5x5 bilinear tap kernel
- Upsampling: 3x3 bilinear tap kernel
- In-place update for most levels (except last step)

### Bloom Pattern

- Energy-preserving weighted sum of gaussians
- Downsampling kernel: Wide to avoid aliasing artifacts
- Upsampling kernel: Blend with coarser layer
- Bloom amount: Linear interpolation between current and coarser
- Optional star effects: 6-point star pattern

### Lens Effects Pattern

- Multiple lens simulation effects:
  - Vignette: Circular falloff
  - Chromatic aberration: RGB channel offset
  - Barrel distortion: Radial distortion
- All effects based on radial distance from center
- Configurable intensity for each effect

### ASC-CDL Pattern

- ASC-CDL (Academy Color Decision List) style color grading
- Saturation curves: Quadratic through 3 points (shadow, midtone, highlight)
- Color grading: Offset/scale/power with luma-based controls
- Shadow tinting via color offset
- Highlight tinting via color scale
- Midtone gamma via color power

### Property Pattern

- Complete property serialization
- Getter/setter methods for all parameters
- Scalar and vector parameters for ease of UI
- Reset buttons for grouped parameters
- Automatic recompile on output size change

## Code Patterns

### Property Serialization Pattern

```cpp
Properties SimplePostFX::getProperties() const
{
    Properties props;
    props[kEnabled] = getEnabled();
    props[kOutputSize] = mOutputSizeSelection;
    if (mOutputSizeSelection == RenderPassHelpers::IOSize::Fixed)
        props[kFixedOutputSize] = mFixedOutputSize;
    props[kWipe] = getWipe();
    props[kBloomAmount] = getBloomAmount();
    props[kStarAmount] = getStarAmount();
    props[kStarAngle] = getStarAngle();
    props[kVignetteAmount] = getVignetteAmount();
    props[kChromaticAberrationAmount] = getChromaticAberrationAmount();
    props[kBarrelDistortAmount] = getBarrelDistortAmount();
    props[kSaturationCurve] = getSaturationCurve();
    props[kColorOffset] = getColorOffset();
    props[kColorScale] = getColorScale();
    props[kColorPower] = getColorPower();
    props[kColorOffsetScalar] = getColorOffsetScalar();
    props[kColorScaleScalar] = getColorScaleScalar();
    props[kColorPowerScalar] = getColorPowerScalar();
    return props;
}
```

### Identity Optimization Pattern

```cpp
// If we have 'identity' settings, we can just copy input to output
if (getEnabled() == false || getWipe() >= 1.f || (
    getBloomAmount() == 0.f &&
    getChromaticAberrationAmount() == 0.f &&
    getBarrelDistortAmount() == 0.f &&
    all(getSaturationCurve() == float3(1.f)) &&
    all(getColorOffset() == float3(0.5f)) &&
    all(getColorScale() == float3(0.5f)) &&
    all(getColorPower() == float3(0.5f)) &&
    getColorOffsetScalar() == 0.f &&
    getColorScaleScalar() == 0.f &&
    getColorPowerScalar() == 0.f
    ))
{
    // Wipe is all the way across, which corresponds to no effect
    pRenderContext->blit(pSrc->getSRV(), pDst->getRTV());
    return;
}
```

### Image Pyramid Preparation Pattern

```cpp
void SimplePostFX::preparePostFX(RenderContext* pRenderContext, uint32_t width, uint32_t height)
{
    for (int res = 0; res < kNumLevels + 1; ++res)
    {
        ref<Texture>& pBuf = mpPyramid[res];
        if (getBloomAmount() <= 0.f)
        {
            pBuf = nullptr;
        }
        else
        {
            uint32_t w = std::max(1u, width >> res);
            uint32_t h = std::max(1u, height >> res);
            if (!pBuf || pBuf->getWidth() != w || pBuf->getHeight() != h)
            {
                pBuf = mpDevice->createTexture2D(
                    w, h, ResourceFormat::RGBA16Float, 1, 1, nullptr, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess
                );
                FALCOR_ASSERT(pBuf);
            }
        }
    }
}
```

### Downsampling Pattern

```cpp
// Downsampling phase (8 levels of 4x downsampling)
if (getBloomAmount() > 0.f)
{
    for (int level = 0; level < kNumLevels; ++level)
    {
        uint2 res = {std::max(1u, resolution.x >> (level + 1)), std::max(1u, resolution.y >> (level + 1))};
        float2 invres = float2(1.f / res.x, 1.f / res.y);
        auto var = mpDownsamplePass->getRootVar();
        var["gLinearSampler"] = mpLinearSampler;
        for (int level = 0; level < kNumLevels; ++level)
        {
            uint2 res = {std::max(1u, resolution.x >> (level + 1)), std::max(1u, resolution.y >> (level + 1))};
            float2 invres = float2(1.f / res.x, 1.f / res.y);
            var["PerFrameCB"]["gResolution"] = res;
            var["PerFrameCB"]["gInvRes"] = invres;
            var["gSrc"] = level ? mpPyramid[level] : pSrc;
            var["gDst"] = mpPyramid[level + 1];
            mpDownsamplePass->execute(pRenderContext, uint3(res, 1));
        }
    }
}
```

### Upsampling Pattern

```cpp
// Upsampling phase (8 levels of 4x upsampling with bloom and star effects)
{
    auto var = mpUpsamplePass->getRootVar();
    var["gLinearSampler"] = mpLinearSampler;
    var["PerFrameCB"]["gBloomAmount"] = getBloomAmount();
    var["gSrc"] = pSrc;
    for (int level = kNumLevels - 1; level >= 0; --level)
    {
        uint2 res = {std::max(1u, resolution.x >> level), std::max(1u, resolution.y >> level)};
        float2 invres = float2(1.f / res.x, 1.f / res.y);
        bool wantStar = level == 1 || level == 2;
        var["PerFrameCB"]["gResolution"] = res;
        var["PerFrameCB"]["gInvRes"] = invres;
        if (wantStar)
        {
            float ang = getStarAngle();
            var["PerFrameCB"]["gStarDir1"] = float2(std::sin(ang), std::cos(ang)) * invres * 2.f;
            ang += float(M_PI) / 3.f;
            var["PerFrameCB"]["gStarDir2"] = float2(std::sin(ang), std::cos(ang)) * invres * 2.f;
            ang += float(M_PI) / 3.f;
            var["PerFrameCB"]["gStarDir3"] = float2(std::sin(ang), std::cos(ang)) * invres * 2.f;
        }
        var["gBloomed"] = mpPyramid[level + 1];
        var["gDst"] = mpPyramid[level];
        mpUpsamplePass->execute(pRenderContext, uint3(res, 1));
    }
}
```

### PostFX Pattern

```cpp
// PostFX phase (apply all effects)
{
    auto var = mpPostFXPass->getRootVar();
    var["PerFrameCB"]["gResolution"] = resolution;
    var["PerFrameCB"]["gInvRes"] = float2(1.f / resolution.x, 1.f / resolution.y);
    var["PerFrameCB"]["gVignetteAmount"] = getVignetteAmount();
    var["PerFrameCB"]["gChromaticAberrationAmount"] = getChromaticAberrationAmount() * (1.f / 64.f);
    float barrel = getBarrelDistortAmount() * 0.125f;
    var["PerFrameCB"]["gBarrelDistort"] = float2(1.f / (1.f + 4.f * barrel), barrel);
    float3 satcurve = getSaturationCurve();
    // Fit a quadratic through 3 points
    satcurve.y -= satcurve.x;
    satcurve.z -= satcurve.x;
    float A = 2.f * satcurve.z - 4.f * satcurve.y;
    float B = satcurve.z - A;
    float C = satcurve.x;
    var["PerFrameCB"]["gSaturationCurve"] = float3(A, B, C);
    var["PerFrameCB"]["gColorOffset"] = getColorOffset() + getColorOffsetScalar() - 0.5f;
    var["PerFrameCB"]["gColorScale"] = getColorScale() * std::exp2(1.f + 2.f * getColorScaleScalar());
    var["PerFrameCB"]["gColorPower"] = exp2(3.f * (0.5f - getColorPower() - getColorPowerScalar()));
    var["PerFrameCB"]["gWipe"] = mWipe * resolution.x;
    var["gBloomed"] = getBloomAmount() > 0.f ? mpPyramid[0] : pSrc;
    var["gSrc"] = pSrc;
    var["gDst"] = pDst;
    var["gLinearSampler"] = mpLinearSampler;
    mpPostFXPass->execute(pRenderContext, uint3(resolution, 1));
}
```

### Plugin Registration Pattern

```cpp
extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, SimplePostFX>();
    ScriptBindings::registerBinding(regSimplePostFX);
}
```

## Integration Points

### Render Graph Integration

The pass integrates with render graph system:
- Input channels: 1 required (src)
- Output channels: 1 required (dst)
- Reflects input/output resources via `reflect()` method
- Executes post-processing pipeline in `execute()` method
- Configurable output size (Default, Fixed, Custom)
- Automatic recompile on output size change

### Compute Pass Integration

The pass integrates with compute pass system:
- Three compute passes: downsample, upsample, runPostFX
- Entry points: downsample, upsample, runPostFX
- 16x16 thread groups for all passes
- Image pyramid with 9 textures
- Linear sampler for texture sampling

### Utils/Color Integration

The pass integrates with color utilities:
- Luminance computation via `luminance()` function
- ColorHelpers for color operations
- ASC-CDL style color grading
- Saturation curves with shadow/midtone/highlight control

### Utils/UI Integration

The pass provides comprehensive UI:
- Output size control (dropdown, fixed size)
- Enable/disable checkbox
- Wipe slider
- Lens FX group (bloom, star, vignette, chromatic aberration, barrel distortion)
- Saturation group (shadow, midtone, highlight)
- Luma group (offset, power, scale)
- Color group (offset, power, scale)
- Reset buttons for all groups
- Automatic recompile on output size change

## Use Cases

### Post-Processing

Post-processing applications:
- Bloom effects for glow and soft lighting
- Vignette for cinematic darkening at edges
- Chromatic aberration for lens simulation
- Barrel distortion for fisheye effect
- Saturation curves for artistic color control
- Color grading for HDR to LDR conversion
- Wipe effect for transitions

### Cinematic Effects

Cinematic applications:
- Film-like bloom with star effects
- Lens distortion for wide-angle shots
- Color grading for film look
- Vignette for cinematic framing
- ASC-CDL color grading for Hollywood look

### Artistic Effects

Artistic applications:
- Custom saturation curves for artistic control
- Color offset for shadow/highlight tinting
- Color scale for highlight intensity
- Color power for midtone gamma
- Star effects for artistic bloom

### Debugging and Analysis

Debugging applications:
- Individual effect testing
- Effect combination analysis
- Parameter tuning
- Visual quality assessment

### Research and Development

Research applications:
- Bloom algorithm research
- Lens effect simulation
- Color grading techniques
- ASC-CDL implementation study
- Multi-scale processing research

### Educational Use

Educational applications:
- Demonstrate post-processing techniques
- Show image pyramid construction
- Explain bloom algorithms
- Teach lens effects
- Visualize color grading

## Performance Considerations

### GPU Computation

- Downsampling: O(width × height / 256) dispatches (16x16 thread groups)
- Upsampling: O(width × height / 256) dispatches (16x16 thread groups)
- PostFX: O(width × height / 256) dispatches (16x16 thread groups)
- Total: O(width × height / 256 × 3) dispatches per frame

### Memory Usage

- Image pyramid: O(width × height × 16 × 9) bytes (RGBA16Float, 9 levels)
- Source texture: O(width × height × 16) bytes (RGBA32Float)
- Destination texture: O(width × height × 16) bytes (RGBA32Float)
- Total: O(width × height × (16 × 9 + 16 + 16)) bytes

### Computational Complexity

- Downsampling: O(width × height / 256) per frame, 5x5 bilinear taps = 25 samples per pixel
- Upsampling: O(width × height / 256) per frame, 3x3 bilinear taps = 9 samples per pixel, optional 6-star pattern
- PostFX: O(width × height / 256) per frame, barrel distortion, chromatic aberration, vignette, saturation curves, color grading
- Overall: O(width × height) per frame

### Thread Group Size

- All passes: 16x16 thread groups (256 threads per group)
- Efficient for modern GPUs
- Good balance between occupancy and granularity

## Limitations

### Feature Limitations

- No scene integration (empty setScene)
- No ray tracing or advanced rendering
- Limited to 2D post-processing
- No temporal effects
- No motion blur
- No depth of field
- No film grain
- No color space conversion (assumes linear RGB)

### Performance Limitations

- High memory usage for image pyramid (9 textures)
- Multiple dispatches per frame (3 passes)
- No async compute
- No performance optimization for specific hardware
- No adaptive quality settings

### UI Limitations

- Complex UI with many controls
- No preset management
- No undo/redo functionality
- No real-time preview of individual effects
- Limited parameter validation
- No help system integration

### Algorithm Limitations

- Fixed 8-level image pyramid
- Fixed kernel sizes (5x5, 3x3)
- No adaptive kernel selection
- No edge-aware filtering
- No temporal stability
- Star effects limited to 6-point pattern

### Integration Limitations

- Limited to render graph system
- No external data sources
- No scripting support (except property serialization)
- No event handling (only UI)
- No scene integration

## Best Practices

### Bloom Configuration

- Use appropriate bloom amount for scene
- Balance star amount and angle for desired effect
- Test with different image pyramid levels
- Monitor memory usage for large resolutions
- Use identity optimization when bloom disabled

### Lens Effects Configuration

- Adjust vignette amount for cinematic look
- Use chromatic aberration for lens simulation
- Apply barrel distortion for wide-angle shots
- Balance all lens effects for realistic look
- Test individual effects before combining

### Color Grading Configuration

- Use ASC-CDL style for Hollywood look
- Adjust saturation curves for artistic control
- Use color offset for shadow tinting
- Use color scale for highlight intensity
- Use color power for midtone gamma
- Test with different color grading settings

### Output Size Configuration

- Use Default for automatic size matching
- Use Fixed for consistent output size
- Use Custom for specific requirements
- Test with different resolutions
- Monitor performance impact

### Performance Optimization

- Use appropriate output size for scene
- Monitor GPU memory usage
- Test with different thread group sizes
- Profile individual passes
- Optimize for target hardware

### Debugging

- Test individual effects in isolation
- Verify image pyramid construction
- Check bloom blending
- Validate lens effects
- Analyze color grading results

### Research and Development

- Study bloom algorithms
- Research lens effect simulation
- Investigate color grading techniques
- Develop new post-processing effects
- Optimize for performance

### Educational Use

- Demonstrate post-processing concepts
- Show image pyramid construction
- Explain bloom algorithms
- Teach lens effects
- Visualize color grading
- Provide clear parameter explanations

## Related Components

### RenderGraph/RenderPass

Render pass base class:
- `RenderPass::reflect()` - Define input/output resources
- `RenderPass::execute()` - Execute render pass
- `RenderPass::renderUI()` - Render UI controls
- `RenderPass::getProperties()` - Serialize configuration

### RenderGraph/RenderPassHelpers

Render pass helpers:
- `RenderPassHelpers::calculateIOSize()` - Calculate I/O size
- `RenderPassHelpers::IOSize` - Output size enumeration

### Core/Program

Program management:
- `ComputePass::create()` - Create compute pass
- `ComputePass::execute()` - Execute compute pass
- `ComputePass::getRootVar()` - Get root variable

### Utils/Color/ColorHelpers

Color utilities:
- `luminance()` - Luminance computation
- Color space conversions
- Color grading utilities

### Utils/UI/Gui

UI framework:
- Provides checkbox, var, group, text, dropdown, button controls
- Simple UI widget interface
- Real-time control updates

## Progress Log

- **2026-01-07T22:19:00Z**: Completed SimplePostFX analysis. Analyzed SimplePostFX.h (135 lines), SimplePostFX.cpp (391 lines), and SimplePostFX.cs.slang (241 lines). Created comprehensive technical specification covering image pyramid with 8 levels, bloom implementation with 5x5/3x3 bilinear kernels, star effects with 6-point pattern, lens effects (vignette, chromatic aberration, barrel distortion), ASC-CDL style saturation curves and color grading, wipe effect, property serialization, UI rendering with grouped controls, and integration with render graph, compute pass, color utilities, and UI systems.
