# FLIPPass - FLIP Perceptual Error Metric

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **FLIPPass** - FLIP perceptual error metric pass
  - [x] **FLIPPass.h** - FLIP pass header
  - [x] **FLIPPass.cpp** - FLIP pass implementation
  - [x] **FLIPPass.cs.slang** - FLIP compute shader
  - [x] **ComputeLuminance.cs.slang** - Luminance computation shader
  - [x] **ToneMappers.slang** - Tone mapper implementations
  - [x] **flip.hlsli** - FLIP common definitions

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Texture, Formats)
- **Core/Object** - Base object class
- **Core/Platform** - MonitorInfo for monitor information
- **Core/Program** - Shader program management (ComputePass)
- **RenderGraph** - Render graph system (RenderPass, RenderPassReflection)
- **Utils/Algorithm** - ParallelReduction for sum/min/max computation
- **Utils/Color** - Color utilities (ColorHelpers, luminance computation)
- **Utils/Math** - Mathematical utilities (MathConstants)

## Module Overview

The FLIPPass implements the FLIP (Framework for Learning Image Priors) perceptual error metric for comparing rendered images. It supports both LDR-FLIP for low dynamic range images and HDR-FLIP for high dynamic range images. The pass computes perceptually-motivated error values that correlate better with human perception than traditional metrics like MSE or PSNR. It includes tone mappers (ACES, Hable, Reinhard) for HDR-FLIP, color maps (Magma, Viridis) for visualization, and monitor information for accurate pixels-per-degree (PPD) calculation.

## Component Specifications

### FLIPPass Class

**File**: [`FLIPPass.h`](Source/RenderPasses/FLIPPass/FLIPPass.h:37)

**Purpose**: Computes FLIP perceptual error metric between test and reference images.

**Public Interface**:

```cpp
class FLIPPass : public RenderPass
{
public:
    static ref<FLIPPass> create(ref<Device> pDevice, const Properties& props);
    FLIPPass(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
};
```

**Public Members**: None (all members are private/protected)

**Private Members**:
- `bool mEnabled` - Enable FLIP calculation (default: true)
- `bool mUseMagma` - Map FLIP result to magma colormap (default: true)
- `bool mClampInput` - Clamp FLIP input to expected range (default: false)
- `uint mMonitorWidthPixels` - Horizontal monitor resolution (default: 3840)
- `float mMonitorWidthMeters` - Monitor width in meters (default: 0.7f)
- `float mMonitorDistanceMeters` - Monitor distance from viewer in meters (default: 0.7f)
- `bool mIsHDR` - Enable HDR-FLIP computation (default: false)
- `bool mUseCustomExposureParameters` - Use custom exposure parameters (default: false)
- `FLIPToneMapperType mToneMapper` - Tone mapper for HDR-FLIP (default: ACES)
- `float mStartExposure` - Start exposure for HDR-FLIP (default: 0.0f)
- `float mStopExposure` - Stop exposure for HDR-FLIP (default: 0.0f)
- `float mExposureDelta` - Exposure delta for HDR-FLIP (default: 0.0f)
- `uint32_t mNumExposures` - Number of exposures for HDR-FLIP (default: 2)
- `ref<Texture> mpFLIPErrorMapDisplay` - Internal buffer for display output
- `ref<Texture> mpExposureMapDisplay` - Internal buffer for exposure map
- `ref<Buffer> mpLuminance` - Internal buffer for luminance values
- `ref<ComputePass> mpFLIPPass` - Compute pass for FLIP calculation
- `ref<ComputePass> mpComputeLuminancePass` - Compute pass for luminance computation
- `std::unique_ptr<ParallelReduction> mpParallelReduction` - Parallel reduction helper
- `bool mComputePooledFLIPValues` - Compute mean/min/max across frame (default: false)
- `float mAverageFLIP` - Average FLIP value across frame
- `float mMinFLIP` - Minimum FLIP value across frame
- `float mMaxFLIP` - Maximum FLIP value across frame
- `bool mUseRealMonitorInfo` - Use real monitor data from OS (default: false)
- `bool mRecompile` - Recompilation flag (default: true)

**Protected Members**:
- `void updatePrograms()` - Update shader programs with new defines
- `void computeExposureParameters(const float Ymedian, const float Ymax)` - Compute exposure parameters for HDR-FLIP
- `void parseProperties(const Properties& props)` - Parse configuration properties

### FLIPToneMapperType Enumeration

**File**: [`ToneMappers.slang`](Source/RenderPasses/FLIPPass/ToneMappers.slang:42)

**Purpose**: Enumerates tone mapper options for HDR-FLIP.

**Values**:
- `ACES = 0` - ACES filmic tone mapper
- `Hable = 1` - Hable tone mapper
- `Reinhard = 2` - Reinhard tone mapper

### FLIP Compute Shader

**File**: [`FLIPPass.cs.slang`](Source/RenderPasses/FLIPPass/FLIPPass.cs.slang:28)

**Purpose**: Computes FLIP perceptual error metric between test and reference images.

**Shader Resources**:
- `Texture2D gTestImage` - Test image texture
- `Texture2D gReferenceImage` - Reference image texture
- `RWTexture2D<float4> gFLIPErrorMap` - High-precision FLIP error map output
- `RWTexture2D<float4> gFLIPErrorMapDisplay` - Low-precision FLIP error map for display
- `RWTexture2D<float4> gExposureMapDisplay` - HDR-FLIP exposure map for display

**Constant Buffer** (`PerFrameCB`):
- `uint2 gResolution` - Image resolution
- `bool gIsHDR` - Enable HDR-FLIP
- `bool gUseMagma` - Use magma colormap
- `bool gClampInput` - Clamp input to expected range
- `float gStartExposure` - Start exposure for HDR-FLIP
- `float gExposureDelta` - Exposure delta for HDR-FLIP
- `uint gNumExposures` - Number of exposures for HDR-FLIP
- `uint gMonitorWidthPixels` - Horizontal monitor resolution
- `float gMonitorWidthMeters` - Monitor width in meters
- `float gMonitorDistance` - Monitor distance from viewer

**Thread Group Size**: 32x32x1

**Main Function** (`main`):
```cpp
[numthreads(32, 32, 1)]
void main(uint3 globalId: SV_DispatchThreadID, uint3 groupThreadId: SV_GroupThreadId)
{
    uint2 pixel = globalId.xy;
    float value = FLIP(pixel);

    if (isnan(value) || isinf(value) || value < 0.0f || value > 1.0f)
    {
        gFLIPErrorMap[pixel] = float4(1.0f, 0.0f, 0.0f, 1.0f);
        gFLIPErrorMapDisplay[pixel] = float4(1.0f, 0.0f, 0.0f, 1.0f);
    }
    else
    {
        float3 col = gUseMagma ? MagmaMap[int(value * 255.0f + 0.5f)] : float3(value);
        gFLIPErrorMap[pixel] = float4(col, value);
        gFLIPErrorMapDisplay[pixel] = float4(sRGBToLinear(col), 1.0f);
    }
}
```

**Key Functions**:
- `ppd()` - Compute pixels per degree from monitor information
- `getPixel(Texture2D image, int2 pixel, float exposure)` - Get pixel with tone mapping
- `calculateWeight(float dist2, float4 ab)` - Calculate CSF filter weight
- `HyAB(float3 a, float3 b)` - Hunt-Pointer-Magnussen color difference
- `Hunt(float3 color)` - Hunt color space transformation
- `redistributeErrors(float colorDifference, float featureDifference)` - Redistribute errors based on perceptual cutoff
- `LDRFLIP(uint2 pixel, float exposure)` - Compute LDR-FLIP error
- `HDRFLIP(uint2 pixel)` - Compute HDR-FLIP error (maximum over exposures)
- `FLIP(uint2 pixel)` - Dispatch to LDR-FLIP or HDR-FLIP

### ComputeLuminance Shader

**File**: [`ComputeLuminance.cs.slang`](Source/RenderPasses/FLIPPass/ComputeLuminance.cs.slang:28)

**Purpose**: Compute luminance of an RGB image for HDR-FLIP exposure parameter computation.

**Shader Resources**:
- `Texture2D<float4> gInputImage` - Input image texture
- `RWBuffer<float> gOutputLuminance` - Output luminance buffer

**Constant Buffer** (`PerFrameCB`):
- `uint2 gResolution` - Image resolution

**Thread Group Size**: 16x16x1

**Main Function** (`computeLuminance`):
```cpp
[numthreads(16, 16, 1)]
void computeLuminance(uint3 dispatchThreadId: SV_DispatchThreadID)
{
    if (any(dispatchThreadId.xy >= gResolution))
        return;
    const uint2 pixelPos = dispatchThreadId.xy;
    const float3 color = gInputImage[pixelPos].xyz;
    gOutputLuminance[pixelPos.x + gResolution.x * pixelPos.y] = luminance(color);
}
```

### Tone Mappers

**File**: [`ToneMappers.slang`](Source/RenderPasses/FLIPPass/ToneMappers.slang:60)

**Purpose**: Implement tone mappers for HDR-FLIP.

**Function** (`toneMap`):
```cpp
float3 toneMap(float3 col, FLIPToneMapperType toneMapper = FLIPToneMapperType::ACES)
{
    float k0, k1, k2, k3, k4, k5;

    if (toneMapper == FLIPToneMapperType::ACES)
    {
        // ACES approximation with pre-exposure cancellation
        k0 = 0.6f * 0.6f * 2.51f;
        k1 = 0.6f * 0.03f;
        k2 = 0.0f;
        k3 = 0.6f * 0.6f * 2.43f;
        k4 = 0.6f * 0.59f;
        k5 = 0.14f;
    }
    else if (toneMapper == FLIPToneMapperType::Hable)
    {
        // Hable tone mapper with white scale
        const float A = 0.15f;
        const float B = 0.50f;
        const float C = 0.10f;
        const float D = 0.20f;
        const float E = 0.02f;
        const float F = 0.30f;
        const float W = 11.2f;
        const float nom = k0 * pow(W, 2.0f) + k1 * W + k2;
        const float denom = k3 * pow(W, 2.0f) + k4 * W + k5;
        const float whiteScale = denom / nom;
        k0 = 4.0f * k0 * whiteScale;
        k1 = 2.0f * k1 * whiteScale;
        k2 = k2 * whiteScale;
        k3 = 4.0f * k3;
        k4 = 2.0f * k4;
    }
    else if (toneMapper == FLIPToneMapperType::Reinhard)
    {
        // Reinhard tone mapper
        float3 luminanceCoefficients = float3(0.2126f, 0.7152f, 0.0722f);
        float Y = dot(col, luminanceCoefficients);
        return clamp(col / (Y + 1.0f), 0.0f, 1.0f);
    }

    float3 colSq = col * col;
    float3 nom = k0 * colSq + k1 * col + k2;
    float3 denom = k3 * colSq + k4 * col + k5;
    denom = select(isinf(denom), 1.0f, denom);
    float3 toneMappedCol = nom / denom;
    return clamp(toneMappedCol, 0.0f, 1.0f);
}
```

### FLIP Common Definitions

**File**: [`flip.hlsli`](Source/RenderPasses/FLIPPass/flip.hlsli:50)

**Purpose**: Common definitions for FLIP computation including color maps and constants.

**Color Maps**:
- `MagmaMap[256]` - Magma colormap for FLIP error visualization
- `ViridisMap[256]` - Viridis colormap for HDR-FLIP exposure map

**Constants**:
- `gqc = 0.7f` - Color difference exponent
- `gpc = 0.4f` - Perceptual cutoff coefficient
- `gpt = 0.95f` - Perceptual threshold
- `gw = 0.082f` - Feature detection sigma
- `gqf = 0.5f` - Feature difference exponent

## Technical Details

### FLIP Algorithm Overview

FLIP (Framework for Learning Image Priors) is a perceptual error metric that correlates better with human perception than traditional metrics like MSE or PSNR. The algorithm consists of two parallel pipelines:

1. **Color Pipeline**: Computes color differences using CSF (Contrast Sensitivity Function) filtering and Hunt color space
2. **Feature Pipeline**: Detects edges and points to compute feature differences

The two pipelines are combined through error redistribution based on perceptual thresholds.

### LDR-FLIP Computation

LDR-FLIP computes perceptual error for low dynamic range images:

**Steps**:
1. Compute pixels per degree (PPD) from monitor information
2. Convert RGB to YCxCz color space
3. Apply CSF filtering using Gaussian weights
4. Convert filtered colors to CIELab color space
5. Apply Hunt transformation
6. Compute HyAB color difference
7. Detect edges and points using feature detection
8. Compute feature difference
9. Redistribute errors based on perceptual cutoff
10. Map result to colormap (Magma or grayscale)

**CSF Filtering**:
- Uses three CSF filters: A (luminance), RG (red-green), BY (blue-yellow)
- Filter parameters: `{a1, a2, b1, b2}`
- Radius computed based on PPD: `radius = ceil(3.0f * sqrt(0.04f / (2.0f * M_PI * M_PI)) * pixelsPerDegree)`
- Weight computation: `weight = a1 * sqrt(M_PI * b1_inv) * exp(b1_inv * dist2) + a2 * sqrt(M_PI * b2_inv) * exp(b2_inv * dist2)`

**Feature Detection**:
- Uses Gaussian kernel with sigma: `sigmaFeatures = 0.5f * gw * pixelsPerDegree`
- Computes point gradients: `(x² + y²) / sigma² - 1) * g`
- Computes edge gradients: `-(x, y) * g`
- Separates positive and negative weights for normalization

**Error Redistribution**:
```cpp
float redistributeErrors(float colorDifference, float featureDifference)
{
    float error = pow(colorDifference, gqc);
    float perceptualCutoff = gpc * MaxDistance;

    if (error < perceptualCutoff)
    {
        error *= (gpt / perceptualCutoff);
    }
    else
    {
        error = gpt + ((error - perceptualCutoff) / (MaxDistance - perceptualCutoff)) * (1.0f - gpt);
    }

    error = pow(error, (1.0f - featureDifference));
    return error;
}
```

### HDR-FLIP Computation

HDR-FLIP extends LDR-FLIP for high dynamic range images:

**Steps**:
1. Compute luminance of reference image
2. Compute median and maximum luminance
3. Solve for exposure parameters based on tone mapper
4. For each exposure in range:
   - Apply exposure compensation
   - Apply tone mapping
   - Compute LDR-FLIP
5. Take maximum FLIP value across all exposures
6. Store exposure map showing which exposure produced maximum error

**Exposure Parameter Computation**:
```cpp
void computeExposureParameters(const float Ymedian, const float Ymax)
{
    // Get tone mapper coefficients
    const float t = 0.85f;
    const float a = tmCoefficients[0] - t * tmCoefficients[3];
    const float b = tmCoefficients[1] - t * tmCoefficients[4];
    const float c = tmCoefficients[2] - t * tmCoefficients[5];

    // Solve a * x² + b * x + c = 0
    float xMin = 0.0f;
    float xMax = 0.0f;
    solveSecondDegree(a, b, c, xMin, xMax);

    mStartExposure = std::log2(xMax / Ymax);
    float stopExposure = std::log2(xMax / Ymedian);
    mNumExposures = uint32_t(std::max(2.0f, std::ceil(stopExposure - mStartExposure)));
    mExposureDelta = (stopExposure - mStartExposure) / (mNumExposures - 1.0f);
}
```

**HDR-FLIP Loop**:
```cpp
float HDRFLIP(uint2 pixel)
{
    float exposure;
    float ldrflip;
    float hdrflip = 0.0f;
    uint exposureMapIndex = 0u;

    for (uint i = 0; i < gNumExposures; i++)
    {
        exposure = gStartExposure + i * gExposureDelta;
        ldrflip = LDRFLIP(pixel, exposure);
        if (ldrflip > hdrflip)
        {
            hdrflip = ldrflip;
            exposureMapIndex = i;
        }
    }

    // Store exposure map
    float exposureMapFloatIndex = float(exposureMapIndex) / (gNumExposures - 1.0f);
    gExposureMapDisplay[pixel] = float4(sRGBToLinear(ViridisMap[int(exposureMapFloatIndex * 255.0f + 0.5f)]), 1.0f);

    return hdrflip;
}
```

### Pixels Per Degree (PPD) Calculation

PPD is computed from monitor information:
```cpp
float ppd(void)
{
    return gMonitorDistance * (gMonitorWidthPixels / gMonitorWidthMeters) * (M_PI / 180.0f);
}
```

This ensures accurate spatial filtering based on viewing conditions.

### Color Space Conversions

**RGB to YCxCz**:
- Used for CSF filtering
- Separates luminance (Y) from chromaticity (Cx, Cz)
- Optimized for perceptual filtering

**RGB to CIELab**:
- Used for color difference computation
- Perceptually uniform color space
- Better correlates with human perception

**Hunt Transformation**:
- Adapts colors for viewing conditions
- Scales chromatic components by luminance
- Formula: `Hunt(color) = (Y, 0.01*Y*a, 0.01*Y*b)`

### Color Maps

**Magma Map**:
- 256-entry colormap for FLIP error visualization
- Black (low error) to white (high error)
- Perceptually uniform

**Viridis Map**:
- 256-entry colormap for HDR-FLIP exposure map
- Blue (low exposure) to yellow (high exposure)
- Perceptually uniform

### Monitor Information

The pass uses monitor information for accurate PPD calculation:

**Default Values**:
- Monitor width pixels: 3840
- Monitor width meters: 0.7f
- Monitor distance meters: 0.7f

**Real Monitor Info**:
- When `mUseRealMonitorInfo = true`, queries OS for actual monitor information
- Uses first monitor's resolution and physical size
- Overrides default values

### Parallel Reduction

The pass uses [`ParallelReduction`](Source/Falcor/Utils/Algorithm/ParallelReduction.h:1) for computing pooled FLIP values:

**Operations**:
- `Type::Sum` - Sum all FLIP values for mean computation
- `Type::MinMax` - Find minimum and maximum FLIP values

**Metrics**:
- Mean FLIP: `sum.a / (width * height)`
- Min FLIP: `minmax[0].a`
- Max FLIP: `minmax[1].a`

### Render Graph Integration

**Input Channels**:
- `testImage` (required): Test image to compare
- `referenceImage` (required): Reference image for comparison

**Output Channels**:
- `errorMap` (required): High-precision FLIP error map (RGBA32Float)
- `errorMapDisplay` (required): Low-precision FLIP error map for display (RGBA8UnormSrgb)
- `exposureMapDisplay` (required): HDR-FLIP exposure map (RGBA8UnormSrgb)

**Reflection**:
```cpp
RenderPassReflection reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    reflector.addInput(kTestImageInput, "Test image").bindFlags(Falcor::ResourceBindFlags::ShaderResource).texture2D(0, 0);
    reflector.addInput(kReferenceImageInput, "Reference image").bindFlags(Falcor::ResourceBindFlags::ShaderResource).texture2D(0, 0);
    reflector.addOutput(kErrorMapOutput, "FLIP error map for computations")
        .format(ResourceFormat::RGBA32Float)
        .bindFlags(Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::ShaderResource)
        .texture2D(0, 0);
    reflector.addOutput(kErrorMapDisplayOutput, "FLIP error map for display")
        .format(ResourceFormat::RGBA8UnormSrgb)
        .bindFlags(Falcor::ResourceBindFlags::RenderTarget)
        .texture2D(0, 0);
    reflector.addOutput(kExposureMapDisplayOutput, "HDR-FLIP exposure map for display")
        .format(ResourceFormat::RGBA8UnormSrgb)
        .bindFlags(Falcor::ResourceBindFlags::RenderTarget)
        .texture2D(0, 0);
    return reflector;
}
```

### Property Serialization

The pass supports property serialization for configuration:

```cpp
Properties getProperties() const
{
    Properties props;
    props[kEnabled] = mEnabled;
    props[kUseMagma] = mUseMagma;
    props[kClampInput] = mClampInput;
    props[kIsHDR] = mIsHDR;
    props[kToneMapper] = mToneMapper;
    props[kUseCustomExposureParameters] = mUseCustomExposureParameters;
    props[kStartExposure] = mStartExposure;
    props[kStopExposure] = mStopExposure;
    props[kNumExposures] = mNumExposures;
    props[kMonitorWidthPixels] = mMonitorWidthPixels;
    props[kMonitorWidthMeters] = mMonitorWidthMeters;
    props[kMonitorDistance] = mMonitorDistanceMeters;
    props[kComputePooledFLIPValues] = mComputePooledFLIPValues;
    props[kUseRealMonitorInfo] = mUseRealMonitorInfo;
    return props;
}
```

## Architecture Patterns

### Render Pass Pattern

- Inherits from [`RenderPass`](Source/Falcor/RenderGraph/RenderPass.h:1) base class
- Implements standard render pass interface:
  - `reflect()` - Defines input/output resources
  - `execute()` - Performs FLIP computation
  - `renderUI()` - Provides UI controls
  - `getProperties()` - Serializes configuration

### Compute Pass Pattern

- Uses [`ComputePass`](Source/Falcor/Core/Program/ComputePass.h:1) for shader execution
- Two compute passes:
  - `mpFLIPPass` - Main FLIP computation shader
  - `mpComputeLuminancePass` - Luminance computation shader
- Shader compilation with defines for tone mapper selection

### Parallel Reduction Pattern

- Uses [`ParallelReduction`](Source/Falcor/Utils/Algorithm/ParallelReduction.h:1) for efficient aggregation
- Type: `ParallelReduction::Type::Sum` and `Type::MinMax`
- GPU-accelerated reduction algorithm
- Returns `float4` containing aggregated values

### Property Pattern

- Implements property-based configuration
- Supports serialization/deserialization via `Properties` class
- Enables configuration through render graph and scripting

### Dual Pipeline Pattern

- Implements two parallel pipelines:
  - Color pipeline: CSF filtering, color difference computation
  - Feature pipeline: Edge/point detection, feature difference computation
- Pipelines combined through error redistribution

### Tone Mapper Pattern

- Supports multiple tone mappers via enumeration
- Tone mapper selected at compile time via defines
- Three tone mappers: ACES, Hable, Reinhard

## Code Patterns

### LDR-FLIP Execution

```cpp
float LDRFLIP(uint2 pixel, float exposure = 0.0f)
{
    // Compute pixels per degree
    const float pixelsPerDegree = ppd();
    const float dx = 1.0f / ppd();

    // Variables for CSF filtering
    const float4 abValuesA = { 1.0f, 0.0f, 0.0047f, 1.0e-5f };
    const float4 abValuesRG = { 1.0f, 0.0f, 0.0053f, 1.0e-5f };
    const float4 abValuesBY = { 34.1f, 13.5f, 0.04f, 0.025f };
    float3 colorWeight = { 0.0f, 0.0f };
    float3 csfKernelSum = float3(0.0f, 0.0f, 0.0f);
    float3 referenceColorSum = float3(0.0f, 0.0f, 0.0f);
    float3 testColorSum = float3(0.0f, 0.0f, 0.0f);

    // Variables for feature detection
    float sigmaFeatures = 0.5f * gw * pixelsPerDegree;
    float sigmaFeaturesSquared = sigmaFeatures * sigmaFeatures;
    float positiveKernelSum = 0.0f;
    float negativeKernelSum = 0.0f;
    float edgeKernelSum = 0.0f;
    float2 referenceEdgeGradient = float2(0.0f, 0.0f);
    float2 referencePointGradient = float2(0.0f, 0.0f);
    float2 testEdgeGradient = float2(0.0f, 0.0f);
    float2 testPointGradient = float2(0.0f, 0.0f);

    int radius = int(ceil(3.0f * sqrt(0.04f / (2.0f * M_PI * M_PI)) * pixelsPerDegree));

    // Prepare point and edge kernel sums for feature detection
    for (int y = -radius; y <= radius; y++)
    {
        for (int x = -radius; x <= radius; x++)
        {
            int2 pixelNeighbor = pixel + int2(x, y);
            pixelNeighbor = min(max(int2(0, 0), pixelNeighbor), gResolution - 1);

            float g = exp(-(x * x + y * y) / (2.0f * sigmaFeaturesSquared));

            float2 pointWeight = (float2(x * x, y * y) / sigmaFeaturesSquared - 1) * g;
            positiveKernelSum += (pointWeight.x >= 0.0f ? pointWeight.x : 0.0f);
            negativeKernelSum += (pointWeight.x < 0.0f ? -pointWeight.x : 0.0f);

            float2 edgeWeight = -float2(x, y) * g;
            edgeKernelSum += (edgeWeight.x >= 0.0f ? edgeWeight.x : 0.0f);
        }
    }

    // Main computation loop
    for (int y = -radius; y <= radius; y++)
    {
        for (int x = -radius; x <= radius; x++)
        {
            // Load pixel colors
            int2 pixelNeighbor = pixel + int2(x, y);
            pixelNeighbor = min(max(int2(0, 0), pixelNeighbor), gResolution - 1);
            float3 referenceColor = getPixel(gReferenceImage, pixelNeighbor, exposure);
            float3 testColor = getPixel(gTestImage, pixelNeighbor, exposure);

            // Color pipeline
            float2 p = float2(x, y) * dx;
            float dist2 = -(p.x * p.x + p.y * p.y) * M_PI * M_PI;
            float weightA = calculateWeight(dist2, abValuesA);
            float weightRG = calculateWeight(dist2, abValuesRG);
            float weightBY = calculateWeight(dist2, abValuesBY);
            colorWeight = float3(weightA, weightRG, weightBY);

            csfKernelSum += colorWeight;
            referenceColorSum += colorWeight * referenceColor;
            testColorSum += colorWeight * testColor;

            // Feature pipeline
            float g = exp(-(x * x + y * y) / (2.0f * sigmaFeaturesSquared));

            float2 pointWeight = (float2(x * x, y * y) / sigmaFeaturesSquared - 1) * g;
            float2 pointNormalization = float2(1.0f, 1.0f) / float2(
                pointWeight.x >= 0.0f ? positiveKernelSum : negativeKernelSum,
                pointWeight.y >= 0.0f ? positiveKernelSum : negativeKernelSum
            );
            float2 edgeWeight = -float2(x, y) * g;
            float2 edgeNormalization = float2(1.0f, 1.0f) / float2(edgeKernelSum, edgeKernelSum);

            float referenceLuminance = (referenceColor.x + 16.0f) / 116.0f;
            referencePointGradient += referenceLuminance * pointWeight * pointNormalization;
            referenceEdgeGradient += referenceLuminance * edgeWeight * edgeNormalization;

            float testLuminance = (testColor.x + 16.0f) / 116.0f;
            testPointGradient += testLuminance * pointWeight * pointNormalization;
            testEdgeGradient += testLuminance * edgeWeight * edgeNormalization;
        }
    }

    // Color pipeline
    float3 spatialFilteredReference = referenceColorSum / csfKernelSum;
    float3 spatialFilteredTest = testColorSum / csfKernelSum;
    spatialFilteredReference = clamp(YCxCzToLinearRGB(spatialFilteredReference), 0.0f, 1.0f);
    spatialFilteredTest = clamp(YCxCzToLinearRGB(spatialFilteredTest), 0.0f, 1.0f);
    float colorDiff = HyAB(Hunt(linearRGBToCIELab(spatialFilteredReference)), Hunt(linearRGBToCIELab(spatialFilteredTest)));

    // Feature pipeline
    float edgeDifference = abs(length(referenceEdgeGradient) - length(testEdgeGradient));
    float pointDifference = abs(length(referencePointGradient) - length(testPointGradient));
    float featureDiff = pow(max(pointDifference, edgeDifference) * M_SQRT1_2, gqf);

    return redistributeErrors(colorDiff, featureDiff);
}
```

### Exposure Parameter Computation

```cpp
void computeExposureParameters(const float Ymedian, const float Ymax)
{
    std::vector<float> tmCoefficients;
    if (mToneMapper == FLIPToneMapperType::Reinhard)
    {
        tmCoefficients = {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f};
    }
    else if (mToneMapper == FLIPToneMapperType::ACES)
    {
        tmCoefficients = {0.6f * 0.6f * 2.51f, 0.6f * 0.03f, 0.0f, 0.6f * 0.6f * 2.43f, 0.6f * 0.59f, 0.14f};
    }
    else if (mToneMapper == FLIPToneMapperType::Hable)
    {
        tmCoefficients = {0.231683f, 0.013791f, 0.0f, 0.18f, 0.3f, 0.018f};
    }

    const float t = 0.85f;
    const float a = tmCoefficients[0] - t * tmCoefficients[3];
    const float b = tmCoefficients[1] - t * tmCoefficients[4];
    const float c = tmCoefficients[2] - t * tmCoefficients[5];

    float xMin = 0.0f;
    float xMax = 0.0f;
    solveSecondDegree(a, b, c, xMin, xMax);

    mStartExposure = std::log2(xMax / Ymax);
    float stopExposure = std::log2(xMax / Ymedian);

    mNumExposures = uint32_t(std::max(2.0f, std::ceil(stopExposure - mStartExposure)));
    mExposureDelta = (stopExposure - mStartExposure) / (mNumExposures - 1.0f);
}
```

### Median and Max Computation

```cpp
static void computeMedianMax(const float* values, const uint32_t numValues, float& median, float& max)
{
    std::vector<float> sortedValues(values, values + numValues);
    std::sort(sortedValues.begin(), sortedValues.end());
    if (numValues & 1) // Odd number of values.
    {
        median = sortedValues[numValues / 2];
    }
    else // Even number of values.
    {
        uint32_t medianLocation = numValues / 2 - 1;
        median = (sortedValues[medianLocation] + sortedValues[medianLocation + 1]) * 0.5f;
    }
    max = sortedValues[numValues - 1];
}
```

### Program Update

```cpp
void updatePrograms()
{
    if (mRecompile == false)
    {
        return;
    }

    DefineList defines;
    defines.add("TONE_MAPPER", std::to_string((uint32_t)mToneMapper));

    mpFLIPPass->getProgram()->addDefines(defines);

    mRecompile = false;
}
```

## Integration Points

### Render Graph Integration

The pass integrates with the render graph system:
- Input channels: testImage, referenceImage
- Output channels: errorMap, errorMapDisplay, exposureMapDisplay
- Reflects input/output resources via `reflect()` method
- Executes FLIP computation in `execute()` method
- Supports property-based configuration

### Color Utilities Integration

The pass uses color utilities from Utils/Color:
- `linearRGBToYCxCz()` - RGB to YCxCz conversion
- `YCxCzToLinearRGB()` - YCxCz to RGB conversion
- `linearRGBToCIELab()` - RGB to CIELab conversion
- `luminance()` - Luminance computation
- `sRGBToLinear()` - sRGB to linear RGB conversion
- `sRGBToLinear(ViridisMap[...])` - Color map lookup

### Monitor Info Integration

The pass integrates with monitor information from Core/Platform:
- `MonitorInfo::getMonitorDescs()` - Query monitor information
- Uses first monitor's resolution and physical size
- Enables accurate PPD calculation

### Parallel Reduction Integration

The pass uses parallel reduction for efficient aggregation:
- `ParallelReduction::Type::Sum` - Sum all FLIP values
- `ParallelReduction::Type::MinMax` - Find min/max FLIP values
- GPU-accelerated reduction algorithm

### Scripting Integration

The pass supports scripting through:
- Property serialization via `getProperties()` method
- Configurable parameters: enabled, useMagma, clampInput, isHDR, toneMapper, etc.
- Can be instantiated and configured from Python scripts

### UI Integration

The pass provides comprehensive UI:
- Checkboxes for configuration options
- Dropdown for tone mapper selection
- Sliders for exposure parameters
- Display of pooled FLIP values (mean, min, max)
- Monitor information configuration

## Use Cases

### Render Quality Comparison

Compare different rendering techniques:
- Path tracing vs. rasterization
- Different sampling strategies
- Denoiser quality evaluation
- Anti-aliasing effectiveness

### Perceptual Error Analysis

Analyze perceptual errors:
- Identify regions with high perceptual error
- Compare with traditional metrics (MSE, PSNR)
- Understand human perception of errors
- Evaluate tone mapper effectiveness

### HDR Rendering Evaluation

Evaluate HDR rendering:
- Compare HDR tone mappers
- Analyze exposure parameter impact
- Evaluate HDR-FLIP vs. LDR-FLIP
- Assess perceptual quality of HDR images

### Algorithm Validation

Validate rendering algorithms:
- Verify correctness of new techniques
- Compare against ground truth references
- Measure perceptual quality
- Evaluate approximation errors

### Research and Development

Research applications:
- Evaluate new rendering techniques
- Compare perceptual error metrics
- Study FLIP algorithm behavior
- Validate theoretical predictions

### Educational Use

Educational applications:
- Demonstrate perceptual error metrics
- Show FLIP vs. traditional metrics
- Explain color space conversions
- Visualize error distributions

## Performance Considerations

### GPU Computation

- FLIP computation is GPU-accelerated
- Compute shader with 32x32 thread groups
- Spatial filtering with variable radius based on PPD
- Feature detection with Gaussian kernels
- Memory bandwidth limited by texture reads

### Memory Usage

- FLIP error map: RGBA32Float (16 bytes per pixel)
- FLIP error map display: RGBA32Float (16 bytes per pixel)
- Exposure map display: RGBA32Float (16 bytes per pixel)
- Luminance buffer: 4 bytes per pixel
- Total memory: O(width × height) bytes

### Computational Complexity

- LDR-FLIP: O(N × R²) where N = width × height, R = filter radius
- HDR-FLIP: O(N × R² × E) where E = number of exposures
- Radius depends on PPD: R = O(PPD)
- Overall: O(N × PPD² × E)

### Parallel Reduction Overhead

- Parallel reduction for sum: O(N) with logarithmic depth
- Parallel reduction for min/max: O(N) with logarithmic depth
- Minimal overhead compared to FLIP computation

### Luminance Computation Overhead

- Luminance computation: O(N)
- Required for HDR-FLIP exposure parameter computation
- CPU readback for median/max computation
- Adds synchronization point

### Tone Mapper Overhead

- Tone mapping: O(N) per exposure
- Three tone mappers supported
- ACES and Hable use rational polynomials
- Reinhard uses simple division

## Limitations

### Tone Mapper Limitations

- Only three tone mappers supported (ACES, Hable, Reinhard)
- Tone mapper selected at compile time
- No custom tone mapper support
- Tone mapper coefficients hardcoded

### Monitor Information Limitations

- Assumes single monitor setup
- Uses first monitor's information
- Physical size may not be available
- No multi-monitor support

### Exposure Parameter Limitations

- Exposure range computed automatically for HDR-FLIP
- Custom exposure parameters may not be optimal
- Number of exposures limited to 2-20
- Linear spacing between exposures

### Color Map Limitations

- Only Magma and Viridis color maps supported
- Color maps hardcoded in shader
- No custom color map support
- 256-entry lookup tables

### PPD Calculation Limitations

- Assumes fixed viewing distance
- No head tracking support
- No variable viewing conditions
- Static monitor information

### Error Value Limitations

- FLIP values clamped to [0, 1]
- NaN/Inf values displayed as red
- No negative error values
- No error magnitude scaling

## Best Practices

### Tone Mapper Selection

- Use ACES for cinematic HDR rendering
- Use Hable for game-like HDR rendering
- Use Reinhard for simple HDR rendering
- Select tone mapper based on use case
- Recompile shader when changing tone mapper

### Monitor Configuration

- Use real monitor info when available
- Measure actual monitor dimensions
- Set accurate viewing distance
- Update monitor info when hardware changes
- Verify PPD calculation

### Exposure Parameter Configuration

- Use automatic exposure computation for HDR-FLIP
- Adjust exposure range if needed
- Increase number of exposures for wide dynamic range
- Verify exposure parameters with visual inspection
- Monitor exposure map for correctness

### Color Map Selection

- Use Magma for FLIP error visualization
- Use grayscale for quantitative analysis
- Magma provides perceptually uniform mapping
- Color maps aid visual inspection
- Consider color blindness accessibility

### Performance Optimization

- Reduce number of exposures for HDR-FLIP
- Disable pooled FLIP values if not needed
- Use lower resolution for preview
- Optimize PPD calculation
- Consider async readback for luminance

### Error Analysis

- Compare FLIP with traditional metrics
- Analyze error distribution
- Identify high-error regions
- Use exposure map for HDR-FLIP
- Monitor mean/min/max over time

## Related Components

### Utils/Algorithm/ParallelReduction

Parallel reduction utility for sum/min/max computation:
- Type: `ParallelReduction::Type::Sum` and `Type::MinMax`
- GPU-accelerated reduction algorithm
- Returns `float4` containing aggregated values

### Utils/Color/ColorHelpers

Color utility functions:
- `linearRGBToYCxCz()` - RGB to YCxCz conversion
- `YCxCzToLinearRGB()` - YCxCz to RGB conversion
- `linearRGBToCIELab()` - RGB to CIELab conversion
- `luminance()` - Luminance computation
- `sRGBToLinear()` - sRGB to linear RGB conversion

### Core/Platform/MonitorInfo

Monitor information utility:
- `MonitorInfo::getMonitorDescs()` - Query monitor information
- Returns resolution and physical size
- Used for accurate PPD calculation

## References

**FLIP Papers**:
- "FLIP: A Difference Evaluator for Alternating Images", High Performance Graphics 2020
- "Visualizing Errors in Rendered High Dynamic Range Images", Eurographics 2021
- "Visualizing and Communicating Errors in Rendered Images", Ray Tracing Gems II, 2021

**FLIP Implementation**:
- FLIP GitHub repository: https://github.com/NVlabs/flip
- Original implementation by Pontus Andersson, Jim Nilsson, and Tomas Akenine-Moller

**Tone Mappers**:
- ACES approximation: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
- Hable tone mapper: https://64.github.io/tonemapping/
- Reinhard tone mapper: Classic photographic tone mapping

**Color Spaces**:
- YCxCz color space for CSF filtering
- CIELab color space for color difference
- Hunt color space transformation for viewing conditions

## Future Enhancements

### Additional Tone Mappers

- More tone mapper options
- Custom tone mapper support
- Runtime tone mapper selection
- Tone mapper parameter tuning

### Adaptive Exposure Parameters

- Adaptive exposure range computation
- Dynamic number of exposures
- Exposure parameter optimization
- Per-pixel exposure selection

### Advanced Color Maps

- More color map options
- Custom color map support
- Perceptually optimized color maps
- Color map interpolation

### Improved Monitor Information

- Multi-monitor support
- Head tracking integration
- Variable viewing conditions
- Real-time PPD updates

### Performance Optimization

- Reduced precision computation
- Progressive FLIP computation
- Sparse error computation
- Async luminance readback

### Advanced Error Analysis

- Error distribution statistics
- Temporal error tracking
- Error clustering analysis
- Perceptual error thresholds
