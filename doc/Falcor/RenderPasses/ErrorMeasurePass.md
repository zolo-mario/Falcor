# ErrorMeasurePass - Error Measurement Pass

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **ErrorMeasurePass** - Error measurement pass implementation
  - [x] **ErrorMeasurePass.h** - Error measure pass header
  - [x] **ErrorMeasurePass.cpp** - Error measure pass implementation
  - [x] **ErrorMeasurer.cs.slang** - Error measurer compute shader

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Texture, Formats)
- **Core/Object** - Base object class
- **Core/Program** - Shader program management
- **RenderGraph** - Render graph system (RenderPass, RenderPassReflection)
- **Utils/Algorithm** - ParallelReduction for sum computation
- **Utils/UI** - UI utilities (Gui)
- **Core/AssetResolver** - Asset path resolution

## Module Overview

The ErrorMeasurePass provides comprehensive error measurement capabilities for comparing rendered images against reference images. It supports multiple error metrics (L1, L2/MSE), background pixel exclusion, running error computation with exponential moving average, and CSV file output for measurement tracking. The pass can operate with reference images loaded from files or provided through render graph input channels.

## Component Specifications

### ErrorMeasurePass Class

**File**: [`ErrorMeasurePass.h`](Source/RenderPasses/ErrorMeasurePass/ErrorMeasurePass.h:36)

**Purpose**: Measures error between source and reference images with configurable metrics and visualization options.

**Public Interface**:

```cpp
class ErrorMeasurePass : public RenderPass
{
public:
    // Output mode enumeration
    enum class OutputId { Source, Reference, Difference, Count };

    // Factory method
    static ref<ErrorMeasurePass> create(ref<Device> pDevice, const Properties& props);

    // RenderPass overrides
    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual bool onKeyEvent(const KeyboardEvent& keyEvent) override;
};
```

**Public Members**:
- `OutputId mSelectedOutputId` - Currently selected output mode (default: Source)

**Private Members**:
- `ref<ComputePass> mpErrorMeasurerPass` - Compute pass for error computation
- `std::unique_ptr<ParallelReduction> mpParallelReduction` - Parallel reduction for sum computation
- `ref<Texture> mpReferenceTexture` - Loaded reference texture
- `ref<Texture> mpDifferenceTexture` - Difference image texture
- `std::ofstream mMeasurementsFile` - CSV output file stream
- `std::filesystem::path mReferenceImagePath` - Path to reference image file
- `std::filesystem::path mMeasurementsFilePath` - Path to CSV measurements file
- `bool mIgnoreBackground` - Exclude background pixels (default: true)
- `bool mComputeSquaredDifference` - Compute L2 error (default: true)
- `bool mComputeAverage` - Compute RGB average (default: false)
- `bool mUseLoadedReference` - Use loaded reference image (default: false)
- `bool mReportRunningError` - Use EMA for error (default: true)
- `float mRunningErrorSigma` - EMA coefficient (default: 0.995f)
- `float3 mRunningError` - Running RGB error
- `float mRunningAvgError` - Running average error (-1 = invalid)
- `struct { float3 error; float avgError; bool valid; } mMeasurements` - Current frame measurements

**Static Members**:
- `static const Gui::RadioButtonGroup sOutputSelectionButtons` - Output selection buttons (Source, Reference, Difference)
- `static const Gui::RadioButtonGroup sOutputSelectionButtonsSourceOnly` - Output selection buttons (Source only)

### ErrorMeasurer Compute Shader

**File**: [`ErrorMeasurer.cs.slang`](Source/RenderPasses/ErrorMeasurePass/ErrorMeasurer.cs.slang:28)

**Purpose**: Computes pixel-wise differences between source and reference images.

**Shader Resources**:
- `Texture2D<float4> gReference` - Reference image texture
- `Texture2D<float4> gSource` - Source image texture
- `Texture2D<float4> gWorldPosition` - World position texture (for background detection)
- `RWTexture2D<float4> gResult` - Output difference texture

**Constant Buffer** (`PerFrameCB`):
- `uint2 gResolution` - Image resolution
- `uint gIgnoreBackground` - Enable background exclusion
- `uint gComputeDiffSqr` - Compute squared differences (L2)
- `uint gComputeAverage` - Compute RGB average

**Thread Group Size**: 16x16x1

**Main Function** (`main`):
```cpp
[numthreads(16, 16, 1)]
void main(uint3 DTid: SV_DispatchThreadID)
{
    const uint2 pixel = DTid.xy;
    if (any(pixel >= gResolution))
        return;

    // Determine if we should include this pixel or not.
    const float4 worldPos = gWorldPosition[pixel];
    const bool isForeground = worldPos.w != 0.0f; // w-component identifies valid pixels

    const bool isPixelValid = !gIgnoreBackground || isForeground;

    // Compute error based on the current options.
    float3 diff = isPixelValid ? abs(gSource[pixel].rgb - gReference[pixel].rgb) : float3(0.0f);
    if (gComputeDiffSqr)
        diff *= diff;
    if (gComputeAverage)
        diff.rgb = (diff.r + diff.g + diff.b) / 3.f;

    gResult[pixel] = float4(diff, 0.0f);
}
```

## Technical Details

### Error Computation

The pass supports two error metrics:

1. **L1 Error (Absolute Difference)**:
   - Computes absolute difference: `|source - reference|`
   - Used when `mComputeSquaredDifference = false`

2. **L2 Error (Mean Squared Error)**:
   - Computes squared difference: `(source - reference)²`
   - Used when `mComputeSquaredDifference = true`
   - Provides higher weight to larger errors

**RGB Average Option**:
- When `mComputeAverage = true`, computes average over RGB components
- For L1: `(|r| + |g| + |b|) / 3`
- For L2: `(r² + g² + b²) / 3` (averaged after squaring)

### Background Pixel Exclusion

Background pixels are identified using the world position texture:
- Background pixels have `worldPos.w == 0.0f`
- Foreground pixels have `worldPos.w != 0.0f`
- When `mIgnoreBackground = true` and world position texture is bound, background pixels contribute zero error

### Running Error (Exponential Moving Average)

The pass maintains running error values using exponential moving average (EMA):
- Formula: `runningError = sigma * runningError + (1 - sigma) * currentError`
- `sigma = mRunningErrorSigma` (default: 0.995f)
- Larger sigma values mean slower response to changes
- Running error is reset to invalid (-1) when:
  - Report running error checkbox is enabled
  - Reference image is loaded

### Parallel Reduction

Error aggregation uses [`ParallelReduction`](Source/Falcor/Utils/Algorithm/ParallelReduction.h:1) from Utils/Algorithm:
- Type: `ParallelReduction::Type::Sum`
- Sums all pixel values in difference texture
- Returns `float4` containing sum of RGBA components
- Average computed by dividing by pixel count: `error = sum / (width * height)`

### Output Modes

Three output modes available via `OutputId` enumeration:

1. **Source**: Displays the source image
2. **Reference**: Displays the reference image
3. **Difference**: Displays the computed difference image

Output selection can be changed via:
- UI radio buttons
- Keyboard shortcut: 'O' key (Shift+O reverses direction)

### Reference Image Sources

Two methods for providing reference images:

1. **Loaded Reference**:
   - Loaded from file using `Texture::createFromFile()`
   - Supported formats: EXR, PFM
   - Enabled via `mUseLoadedReference = true`
   - Path specified via `mReferenceImagePath`

2. **Input Channel**:
   - Provided through render graph input channel "Reference"
   - Optional input (can be unbound)
   - Used when `mUseLoadedReference = false`

### CSV Measurements Output

Measurements can be saved to CSV file:
- Path specified via `mMeasurementsFilePath`
- File opened in truncation mode (overwrites existing)
- Header row written on file creation:
  - L2: `avg_L2_error,red_L2_error,green_L2_error,blue_L2_error`
  - L1: `avg_L1_error,red_L1_error,green_L1_error,blue_L1_error`
- One row per frame with scientific notation
- Values: `avgError, error.r, error.g, error.b`

### Render Graph Integration

**Input Channels**:
- `Source` (required): Source image to measure
- `Reference` (optional): Reference image for comparison
- `WorldPosition` (optional): World position for background detection

**Output Channels**:
- `Output` (required): Output image (format: RGBA32Float)

**Reflection**:
```cpp
RenderPassReflection reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    reflector.addInput(kInputChannelSourceImage, "Source image");
    reflector.addInput(kInputChannelReferenceImage, "Reference image (optional)").flags(RenderPassReflection::Field::Flags::Optional);
    reflector.addInput(kInputChannelWorldPosition, "World-space position").flags(RenderPassReflection::Field::Flags::Optional);
    reflector.addOutput(kOutputChannelImage, "Output image").format(ResourceFormat::RGBA32Float);
    return reflector;
}
```

### Property Serialization

The pass supports property serialization for configuration:

```cpp
Properties props;
props[kReferenceImagePath] = mReferenceImagePath;
props[kMeasurementsFilePath] = mMeasurementsFilePath;
props[kIgnoreBackground] = mIgnoreBackground;
props[kComputeSquaredDifference] = mComputeSquaredDifference;
props[kComputeAverage] = mComputeAverage;
props[kUseLoadedReference] = mUseLoadedReference;
props[kReportRunningError] = mReportRunningError;
props[kRunningErrorSigma] = mRunningErrorSigma;
props[kSelectedOutputId] = mSelectedOutputId;
```

## Architecture Patterns

### Render Pass Pattern

- Inherits from [`RenderPass`](Source/Falcor/RenderGraph/RenderPass.h:1) base class
- Implements standard render pass interface:
  - `reflect()` - Defines input/output resources
  - `execute()` - Performs error computation
  - `renderUI()` - Provides UI controls
  - `onKeyEvent()` - Handles keyboard shortcuts
  - `getProperties()` - Serializes configuration

### Compute Pass Pattern

- Uses [`ComputePass`](Source/Falcor/Core/Program/ComputePass.h:1) for shader execution
- Shader file: `RenderPasses/ErrorMeasurePass/ErrorMeasurer.cs.slang`
- Constant buffer name: `PerFrameCB`
- Thread group size: 16x16x1

### Parallel Reduction Pattern

- Uses [`ParallelReduction`](Source/Falcor/Utils/Algorithm/ParallelReduction.h:1) for efficient sum computation
- Type: `ParallelReduction::Type::Sum`
- Reduces difference texture to single `float4` value
- GPU-accelerated reduction algorithm

### Property Pattern

- Implements property-based configuration
- Supports serialization/deserialization via `Properties` class
- Enables configuration through render graph and scripting

### UI Pattern

- Provides comprehensive UI via `renderUI()` method
- Uses [`Gui::Widgets`](Source/Falcor/Utils/UI/Gui.h:1) for UI controls
- Supports file dialogs for reference and measurements file selection
- Displays error measurements with scientific notation

## Code Patterns

### Difference Pass Execution

```cpp
void runDifferencePass(RenderContext* pRenderContext, const RenderData& renderData)
{
    // Bind textures.
    ref<Texture> pSourceTexture = renderData.getTexture(kInputChannelSourceImage);
    ref<Texture> pWorldPositionTexture = renderData.getTexture(kInputChannelWorldPosition);
    auto var = mpErrorMeasurerPass->getRootVar();
    var["gReference"] = getReference(renderData);
    var["gSource"] = pSourceTexture;
    var["gWorldPosition"] = pWorldPositionTexture;
    var["gResult"] = mpDifferenceTexture;

    // Set constant buffer parameters.
    const uint2 resolution = uint2(pSourceTexture->getWidth(), pSourceTexture->getHeight());
    var[kConstantBufferName]["gResolution"] = resolution;
    var[kConstantBufferName]["gIgnoreBackground"] = (uint32_t)(mIgnoreBackground && pWorldPositionTexture);
    var[kConstantBufferName]["gComputeDiffSqr"] = (uint32_t)mComputeSquaredDifference;
    var[kConstantBufferName]["gComputeAverage"] = (uint32_t)mComputeAverage;

    // Run the compute shader.
    mpErrorMeasurerPass->execute(pRenderContext, resolution.x, resolution.y);
}
```

### Reduction Pass Execution

```cpp
void runReductionPasses(RenderContext* pRenderContext, const RenderData& renderData)
{
    float4 error;
    mpParallelReduction->execute(pRenderContext, mpDifferenceTexture, ParallelReduction::Type::Sum, &error);

    const float pixelCountf = static_cast<float>(mpDifferenceTexture->getWidth() * mpDifferenceTexture->getHeight());
    mMeasurements.error = error.xyz() / pixelCountf;
    mMeasurements.avgError = (mMeasurements.error.x + mMeasurements.error.y + mMeasurements.error.z) / 3.f;
    mMeasurements.valid = true;

    if (mRunningAvgError < 0)
    {
        // The running error values are invalid. Start them off with the current frame's error.
        mRunningError = mMeasurements.error;
        mRunningAvgError = mMeasurements.avgError;
    }
    else
    {
        mRunningError = mRunningErrorSigma * mRunningError + (1 - mRunningErrorSigma) * mMeasurements.error;
        mRunningAvgError = mRunningErrorSigma * mRunningAvgError + (1 - mRunningErrorSigma) * mMeasurements.avgError;
    }
}
```

### Reference Image Loading

```cpp
bool loadReference()
{
    if (mReferenceImagePath.empty())
        return false;

    std::filesystem::path resolvedPath = AssetResolver::getDefaultResolver().resolvePath(mReferenceImagePath);
    mpReferenceTexture = Texture::createFromFile(mpDevice, resolvedPath, false /* no MIPs */, false /* linear color */);
    if (!mpReferenceTexture)
    {
        logWarning("Failed to load texture from '{}'", mReferenceImagePath);
        mReferenceImagePath.clear();
        return false;
    }

    mUseLoadedReference = mpReferenceTexture != nullptr;
    mRunningAvgError = -1.f; // Mark running error values as invalid.
    return true;
}
```

### Measurements File Output

```cpp
void saveMeasurementsToFile()
{
    if (!mMeasurementsFile)
        return;

    FALCOR_ASSERT(mMeasurements.valid);
    mMeasurementsFile << mMeasurements.avgError << ",";
    mMeasurementsFile << mMeasurements.error.r << ',' << mMeasurements.error.g << ',' << mMeasurements.error.b;
    mMeasurementsFile << std::endl;
}
```

### Keyboard Event Handling

```cpp
bool onKeyEvent(const KeyboardEvent& keyEvent)
{
    if (keyEvent.type == KeyboardEvent::Type::KeyPressed && keyEvent.key == Input::Key::O)
    {
        int32_t ofs = keyEvent.hasModifier(Input::Modifier::Shift) ? -1 : 1;
        int32_t index = (int32_t)mSelectedOutputId;
        index = (index + ofs + (int32_t)OutputId::Count) % (int32_t)OutputId::Count;
        mSelectedOutputId = (OutputId)index;
        return true;
    }

    return false;
}
```

## Integration Points

### Render Graph Integration

The pass integrates with the render graph system:
- Input channels: Source, Reference (optional), WorldPosition (optional)
- Output channel: Output (RGBA32Float format)
- Reflects input/output resources via `reflect()` method
- Executes error computation in `execute()` method
- Supports property-based configuration

### Scene Integration

The pass can work with scene data:
- Uses world position texture for background detection
- Background pixels identified by `worldPos.w == 0.0f`
- Enables error measurement on foreground pixels only

### Scripting Integration

The pass supports scripting through:
- Property serialization via `getProperties()` method
- Configurable parameters: reference path, measurements path, error metrics
- Can be instantiated and configured from Python scripts

### UI Integration

The pass provides comprehensive UI:
- File dialogs for reference and measurements file selection
- Radio buttons for output mode selection
- Checkboxes for configuration options
- Error display with scientific notation
- Keyboard shortcuts for output mode cycling

### File I/O Integration

The pass integrates with file I/O:
- Reference image loading via `Texture::createFromFile()`
- Asset path resolution via `AssetResolver`
- CSV file output via `std::ofstream`
- Support for EXR and PFM image formats

## Use Cases

### Render Quality Comparison

Compare different rendering techniques:
- Path tracing vs. rasterization
- Different sampling strategies
- Denoiser quality evaluation
- Anti-aliasing effectiveness

### Algorithm Validation

Validate rendering algorithms:
- Verify correctness of new techniques
- Compare against ground truth references
- Measure convergence of iterative methods
- Evaluate approximation errors

### Performance Analysis

Analyze rendering performance:
- Measure error vs. render time trade-offs
- Evaluate quality degradation for optimizations
- Compare different quality presets
- Analyze temporal stability

### Regression Testing

Automated regression testing:
- Compare against reference images
- Detect visual regressions
- Track error metrics over time
- Generate test reports

### Research and Development

Research applications:
- Evaluate new rendering techniques
- Compare different error metrics
- Study perceptual error characteristics
- Validate theoretical predictions

### Educational Use

Educational applications:
- Demonstrate error metrics (L1 vs. L2)
- Show impact of background exclusion
- Visualize difference images
- Explain running error computation

## Performance Considerations

### GPU Computation

- Error computation is GPU-accelerated
- Compute shader with 16x16 thread groups
- Parallel reduction for efficient sum computation
- Memory bandwidth limited by texture reads

### Memory Usage

- Difference texture: RGBA32Float (16 bytes per pixel)
- Reference texture: loaded from file (if using loaded reference)
- Parallel reduction requires temporary buffers
- Total memory: O(width × height) bytes

### Computational Complexity

- Difference pass: O(N) where N = width × height
- Reduction pass: O(N) with logarithmic reduction depth
- Overall: O(N) per frame

### Running Error Overhead

- Minimal overhead for EMA computation
- Simple arithmetic operations
- No additional GPU passes required

### CSV Output Overhead

- File I/O occurs on CPU
- Minimal impact on frame time
- Can be disabled by not setting measurements file path

## Limitations

### Reference Image Requirements

- Reference image must match source image resolution
- Format compatibility: EXR, PFM supported
- No automatic resolution scaling
- No automatic format conversion

### Background Detection

- Requires world position texture for background exclusion
- Background detection based on `worldPos.w == 0.0f`
- May not work correctly for all scene setups
- World position texture must be provided by upstream passes

### Error Metrics

- Limited to L1 and L2 error metrics
- No perceptual error metrics (e.g., SSIM, FLIP)
- No color space conversion support
- No gamma correction

### Output Modes

- Only three output modes (Source, Reference, Difference)
- No visualization of error heatmaps
- No color mapping of error values
- No per-channel error visualization

### Running Error

- Simple EMA implementation
- No adaptive sigma adjustment
- No temporal variance tracking
- Running error reset on reference load

## Best Practices

### Reference Image Management

- Use reference images with matching resolution
- Store reference images in asset directories
- Use asset resolver for path resolution
- Consider using input channel for dynamic references

### Error Metric Selection

- Use L1 error for robust error measurement
- Use L2 error for penalizing large errors
- Enable RGB average for scalar error metric
- Disable RGB average for per-channel analysis

### Background Exclusion

- Enable background exclusion for scenes with backgrounds
- Ensure world position texture is provided
- Verify background detection works correctly
- Consider impact on error measurements

### Running Error Configuration

- Adjust sigma based on temporal stability needs
- Higher sigma (0.99+) for stable scenes
- Lower sigma (0.95-0.98) for dynamic scenes
- Reset running error when reference changes

### CSV Output

- Set measurements file path for automated testing
- Use CSV format for data analysis
- Consider file size for long-running sessions
- Disable output if not needed

### Output Mode Selection

- Use Source mode for debugging
- Use Reference mode for comparison
- Use Difference mode for error visualization
- Cycle through modes using 'O' key

## Related Components

### Utils/Algorithm/ParallelReduction

Parallel reduction utility for sum computation:
- Type: `ParallelReduction::Type::Sum`
- GPU-accelerated reduction algorithm
- Returns `float4` containing sum of RGBA components

### Core/AssetResolver

Asset path resolution:
- Resolves relative paths to absolute paths
- Supports asset directory configuration
- Used for reference image loading

### Utils/UI/Gui

UI framework for rendering controls:
- Provides radio buttons, checkboxes, buttons
- Supports file dialogs
- Displays error measurements

## Future Enhancements

### Additional Error Metrics

- SSIM (Structural Similarity Index)
- PSNR (Peak Signal-to-Noise Ratio)
- FLIP perceptual error metric
- Color difference metrics (Delta E)

### Advanced Visualization

- Error heatmap visualization
- Color mapping of error values
- Per-channel error display
- Error distribution histograms

### Temporal Analysis

- Temporal error tracking
- Error variance over time
- Convergence detection
- Stability metrics

### Adaptive Configuration

- Automatic sigma adjustment
- Adaptive background detection
- Dynamic error metric selection
- Automatic reference matching

### Performance Optimization

- Async file I/O for CSV output
- Reduced precision computation
- Progressive error estimation
- Sparse error computation

## References

- **ErrorMeasurer.cs.slang**: Compute shader for pixel-wise error computation
- **ErrorMeasurePass.h**: Error measure pass header
- **ErrorMeasurePass.cpp**: Error measure pass implementation
- **ParallelReduction**: Utility for GPU-accelerated reduction
- **RenderPass**: Base class for render passes
