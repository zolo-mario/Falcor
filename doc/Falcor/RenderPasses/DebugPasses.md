# DebugPasses - Debug Visualization Passes

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **ComparisonPass** - Base comparison pass
  - [x] **ComparisonPass.h** - Comparison pass header (75 lines)
  - [x] **ComparisonPass.cpp** - Comparison pass implementation (136 lines)
  - [x] **Comparison.ps.slang** - Comparison shader (86 lines)
- [x] **SplitScreenPass** - Split screen comparison
  - [x] **SplitScreenPass.h** - Split screen pass header (68 lines)
  - [x] **SplitScreenPass.cpp** - Split screen pass implementation (141 lines)
  - [x] **SplitScreen.ps.slang** - Split screen shader (45 lines)
- [x] **ColorMapPass** - Color mapping pass
  - [x] **ColorMapPass.h** - Color map pass header (79 lines)
  - [x] **ColorMapPass.cpp** - Color map pass implementation (229 lines)
  - [x] **ColorMapPass.ps.slang** - Color map shader (87 lines)
  - [x] **ColorMapParams.slang** - Color map parameters (61 lines)
- [x] **SideBySidePass** - Side-by-side comparison
  - [x] **SideBySidePass.h** - Side-by-side pass header
  - [x] **SideBySidePass.cpp** - Side-by-side pass implementation
  - [x] **SideBySide.ps.slang** - Side-by-side shader
- [x] **InvalidPixelDetectionPass** - Invalid pixel detection
  - [x] **InvalidPixelDetectionPass.h** - Invalid pixel detection header
  - [x] **InvalidPixelDetectionPass.cpp** - Invalid pixel detection implementation
  - [x] **InvalidPixelDetection.ps.slang** - Invalid pixel detection shader

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Texture, Fbo)
- **Core/Object** - Base object class
- **Core/Pass** - FullScreenPass base class
- **RenderGraph** - Render graph system (RenderPass, RenderPassReflection)
- **Utils/UI** - UI utilities (Gui, TextRenderer)
- **Utils/Algorithm** - Algorithm utilities (ParallelReduction)
- **Utils/Color** - Color utilities (ColorMap)
- **Utils/Timing** - Timing utilities (CpuTimer)

## Module Overview

DebugPasses module provides debug visualization passes for rendering analysis and comparison. These passes enable side-by-side image comparison, split screen comparison, color mapping visualization, and invalid pixel detection. Each pass is designed to be used within the render graph system and can be combined to create complex debugging and analysis pipelines for rendering quality assessment.

## Component Specifications

### ComparisonPass

**File**: [`ComparisonPass.h`](Source/RenderPasses/DebugPasses/ComparisonPass.h:36)

**Purpose**: Base class for comparison passes that display two images side-by-side with a movable divider.

**Key Features**:
- Split screen comparison with movable divider
- Text labels for left and right images
- Swap sides functionality
- Configurable divider size
- Divider color highlighting on hover

**Public Interface**:

```cpp
class ComparisonPass : public RenderPass
{
public:
    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;

protected:
    ComparisonPass(ref<Device> pDevice);
    virtual void createProgram() = 0;
    bool parseKeyValuePair(const std::string key, const Properties::ConstValue& val);

    ref<FullScreenPass> mpSplitShader;
    ref<Texture> pLeftSrcTex;
    ref<Texture> pRightSrcTex;
    ref<Fbo> pDstFbo;
    std::unique_ptr<TextRenderer> mpTextRenderer;

    // Screen parameters
    bool mSwapSides = false;

    // Divider parameters
    float mSplitLoc = -1.0f;
    uint32_t mDividerSize = 2;

    // Label Parameters
    bool mShowLabels = false;
    std::string mLeftLabel = "Left side";
    std::string mRightLabel = "Right side";
};
```

**Protected Members**:
- `ref<FullScreenPass> mpSplitShader` - Split screen shader
- `ref<Texture> pLeftSrcTex` - Left source texture
- `ref<Texture> pRightSrcTex` - Right source texture
- `ref<Fbo> pDstFbo` - Destination FBO
- `std::unique_ptr<TextRenderer> mpTextRenderer` - Text renderer for labels
- `bool mSwapSides` - Swap left and right sides
- `float mSplitLoc` - Split location (fraction of screen width, < 0 means initialize to 0.5)
- `uint32_t mDividerSize` - Divider size in pixels (2*mDividerSize+1)
- `bool mShowLabels` - Show text labels
- `std::string mLeftLabel` - Left label text
- `std::string mRightLabel` - Right label text

**Protected Methods**:
- `ComparisonPass(ref<Device> pDevice)` - Constructor
- `virtual void createProgram() = 0` - Create shader program (abstract)
- `bool parseKeyValuePair(const std::string key, const Properties::ConstValue& val)` - Parse properties

**Public Methods**:
- `Properties getProperties() const override` - Get properties
- `RenderPassReflection reflect(const CompileData& compileData) override` - Reflect input/output
- `void execute(RenderContext* pRenderContext, const RenderData& renderData) override` - Execute pass
- `void renderUI(Gui::Widgets& widget) override` - Render UI

### SplitScreenPass

**File**: [`SplitScreenPass.h`](Source/RenderPasses/DebugPasses/SplitScreenPass/SplitScreenPass.h:35)

**Purpose**: Split screen comparison pass with interactive divider that can be dragged with mouse.

**Key Features**:
- Interactive split screen with draggable divider
- Arrow indicators on divider hover
- Double-click to reset to center
- Visual feedback on divider selection
- Configurable arrow display
- Mouse event handling

**Public Interface**:

```cpp
class SplitScreenPass : public ComparisonPass
{
public:
    FALCOR_PLUGIN_CLASS(SplitScreenPass, "SplitScreenPass", "Allows to user to split screen between two inputs.");

    static ref<SplitScreenPass> create(ref<Device> pDevice, const Properties& props)
    {
        return make_ref<SplitScreenPass>(pDevice, props);
    }

    SplitScreenPass(ref<Device> pDevice, const Properties& props);

    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) override;
    virtual void renderUI(Gui::Widgets& widget) override;

private:
    virtual void createProgram() override;

    ref<Texture> mpArrowTex;

    // Mouse parameters
    bool mMouseOverDivider = false;
    int2 mMousePos = int2(0, 0);
    bool mDividerGrabbed = false;

    bool mDrawArrows = false;

    CpuTimer::TimePoint mTimeOfLastClick{};
};
```

**Private Members**:
- `ref<Texture> mpArrowTex` - Arrow texture (16x16 grayscale)
- `bool mMouseOverDivider` - Mouse over divider flag
- `int2 mMousePos` - Mouse position
- `bool mDividerGrabbed` - Divider grabbed flag
- `bool mDrawArrows` - Draw arrows flag
- `CpuTimer::TimePoint mTimeOfLastClick` - Time of last click (double-click detection)

**Private Methods**:
- `virtual void createProgram() override` - Create split screen shader

**Public Methods**:
- `void execute(RenderContext* pRenderContext, const RenderData& renderData) override` - Execute pass
- `bool onMouseEvent(const MouseEvent& mouseEvent) override` - Mouse event handler
- `void renderUI(Gui::Widgets& widget) override` - Render UI

### ColorMapPass

**File**: [`ColorMapPass.h`](Source/RenderPasses/DebugPasses/ColorMapPass/ColorMapPass.h:37)

**Purpose**: Applies color mapping to input texture for visualization of scalar values.

**Key Features**:
- Multiple color map types (Grey, Jet, Viridis, Plasma, Magma, Inferno)
- Channel selection (0-3 for RGBA)
- Auto range detection with adaptive smoothing
- Manual range configuration
- Format-aware (Float, Uint, Sint)
- Parallel reduction for min/max computation

**Public Interface**:

```cpp
class ColorMapPass : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(ColorMapPass, "ColorMapPass", "Pass that applies a color map to the input.");

    static ref<ColorMapPass> create(ref<Device> pDevice, const Properties& props)
    {
        return make_ref<ColorMapPass>(pDevice, props);
    }

    ColorMapPass(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;

private:
    ColorMap mColorMap = ColorMap::Jet;
    uint32_t mChannel = 0;
    bool mAutoRange = true;
    float mMinValue = 0.f;
    float mMaxValue = 1.f;

    ref<FullScreenPass> mpColorMapPass;
    ref<Fbo> mpFbo;
    bool mRecompile = true;

    class AutoRanging
    {
    public:
        AutoRanging(ref<Device> pDevice);

        std::optional<std::pair<double, double>> getMinMax(
            RenderContext* pRenderContext,
            const ref<Texture>& texture,
            uint32_t channel
        );

    private:
        std::unique_ptr<ParallelReduction> mpParallelReduction;
        ref<Buffer> mpReductionResult;
        ref<Fence> mpFence;
        bool mReductionAvailable = false;
    };

    std::unique_ptr<AutoRanging> mpAutoRanging;
    double mAutoMinValue;
    double mAutoMaxValue;
};
```

**Private Members**:
- `ColorMap mColorMap` - Color map type (default Jet)
- `uint32_t mChannel` - Channel selection (0-3, default 0)
- `bool mAutoRange` - Auto range flag (default true)
- `float mMinValue` - Minimum value (default 0.f)
- `float mMaxValue` - Maximum value (default 1.f)
- `ref<FullScreenPass> mpColorMapPass` - Color map shader
- `ref<Fbo> mpFbo` - Destination FBO
- `bool mRecompile` - Recompile flag
- `std::unique_ptr<AutoRanging> mpAutoRanging` - Auto ranging instance
- `double mAutoMinValue` - Auto min value
- `double mAutoMaxValue` - Auto max value

**Private Methods**:
- None (all methods are public)

**Public Methods**:
- `Properties getProperties() const override` - Get properties
- `RenderPassReflection reflect(const CompileData& compileData) override` - Reflect input/output
- `void execute(RenderContext* pRenderContext, const RenderData& renderData) override` - Execute pass
- `void renderUI(Gui::Widgets& widget) override` - Render UI

### ColorMapParams

**File**: [`ColorMapParams.slang`](Source/RenderPasses/DebugPasses/ColorMapPass/ColorMapParams.slang:32)

**Purpose**: Color map parameter structures and enums.

**Features**:
- Color map enum (Grey, Jet, Viridis, Plasma, Magma, Inferno)
- Color map parameters structure
- Enum registration with FALCOR_ENUM_INFO

**Enum Definition**:
```cpp
enum class ColorMap : uint32_t
{
    Grey,
    Jet,
    Viridis,
    Plasma,
    Magma,
    Inferno,
};
```

**Parameter Structure**:
```cpp
struct ColorMapParams
{
    float minValue = 0.f;
    float maxValue = 1.f;
};
```

## Technical Details

### Comparison.ps.slang

**Purpose**: Comparison shader that combines two images with a movable divider.

**Features**:
- Split screen comparison
- Divider overlay
- Arrow indicators
- Color mapping interface

**Constant Buffer**:
```cpp
cbuffer GlobalCB
{
    int gSplitLocation;   ///< X-value of current split location to display
    uint gDividerSize;    ///< How wide should divider be?
    float4 gDividerColor; ///< What color should divider be this frame?
    int2 gMousePosition;  ///< What is current position of mouse? (Used to position arrows)
    bool gDrawArrows;     ///< Should we draw arrows to left & right of divider?
    uint gLeftBound;      ///< How many pixels from left side should comparison window start?
};
```

**Textures**:
- `Texture2D gLeftInput` - Left input texture
- `Texture2D gRightInput` - Right input texture
- `Texture2D<float> gArrowTex` - Arrow texture (16x16 grayscale)

**Interface**:
```cpp
interface ICalcPixelColor
{
    float2x4 calcColors(uint2 pixelPos);
};
```

**Comparison Function**:
```cpp
float4 compare<C : ICalcPixelColor>(float2 texC, float4 pos, C calc)
{
    // Get two sides of image
    uint2 pixelPos = (uint2)pos.xy;
    float2x4 colors = calc.calcColors(pixelPos);
    float4 leftColor = colors[0];
    float4 rightColor = colors[1];

    // Combine two images, depending on which side of split we're on.  Clamp inputs to minimum 0.
    float4 color = max((pixelPos.x < gSplitLocation) ? leftColor : rightColor, float4(0, 0, 0, 0));

    // Overlay divider onto image
    if (abs(int(pixelPos.x) - gSplitLocation) < gDividerSize)
    {
        color = gDividerColor;
    }

    // Draw arrows
    if (gDrawArrows)
    {
        uint2 dims;
        gArrowTex.GetDimensions(dims.x, dims.y);
        int2 arrow1Coord = int2(pixelPos.x - 5 - gSplitLocation, pixelPos.y - gMousePosition.y + 8);
        if (all(clamp(arrow1Coord, int2(0, 0), int2(dims)) == arrow1Coord))
        {
            float arrowAlpha = gArrowTex[arrow1Coord];
            color = gDividerColor * arrowAlpha + ((arrowAlpha < 1.0) ? color * (1 - arrowAlpha) : float4(0, 0, 0, 0));
        }

        int2 arrow2Coord = int2(pixelPos.x + 21 - gSplitLocation, pixelPos.y - gMousePosition.y + 8);
        if (all(clamp(arrow2Coord, int2(0, 0), int2(dims)) == arrow2Coord))
        {
            arrow2Coord.x = abs(arrow2Coord.x - 15);
            float arrowAlpha = gArrowTex[arrow2Coord];
            color = gDividerColor * arrowAlpha + ((arrowAlpha < 1.0) ? color * (1 - arrowAlpha) : float4(0, 0, 0, 0));
        }
    }

    return color;
}
```

### SplitScreen.ps.slang

**Purpose**: Split screen shader implementation using Comparison.ps.slang.

**Features**:
- Interactive image comparison
- Simple color extraction from both inputs

**Implementation**:
```cpp
struct InteractiveImageComparison : ICalcPixelColor
{
    float2x4 calcColors(uint2 pixelPos)
    {
        float2x4 colors;
        colors[0] = gLeftInput[pixelPos];
        colors[1] = gRightInput[pixelPos];
        return colors;
    }
};

float4 main(float2 texC: TEXCOORD, float4 pos: SV_Position) : SV_Target0
{
    InteractiveImageComparison comp = {};
    return compare(texC, pos, comp);
}
```

### ColorMapPass.ps.slang

**Purpose**: Color mapping shader that applies color maps to scalar values.

**Features**:
- Multiple color map types
- Channel selection
- Value normalization
- Format-aware texture loading

**Format Defines**:
```cpp
#define FORMAT_FLOAT 0
#define FORMAT_UINT 1
#define FORMAT_SINT 2
```

**Texture Loading**:
```cpp
#if _FORMAT == FORMAT_FLOAT
    Texture2D<float4> gTexture;
#elif _FORMAT == FORMAT_UINT
    Texture2D<uint4> gTexture;
#elif _FORMAT == FORMAT_SINT
    Texture2D<int4> gTexture;
#else
    #error "Invalid format"
#endif
```

**Color Map Application**:
```cpp
float3 applyColorMap(float value)
{
    switch (kColorMap)
    {
    case ColorMap::Grey:
        return colormapGray(value);
    case ColorMap::Jet:
        return colormapJet(value);
    case ColorMap::Viridis:
        return colormapViridis(value);
    case ColorMap::Plasma:
        return colormapPlasma(value);
    case ColorMap::Magma:
        return colormapMagma(value);
    case ColorMap::Inferno:
        return colormapInferno(value);
    default:
        return 0.f;
    }
}
```

**Main Function**:
```cpp
float4 main(float2 texC: TEXCOORD, float4 pos: SV_Position) : SV_Target0
{
    int2 pixelPos = int2(pos.xy);

    // Read raw value from selected channel.
    float value = gTexture.Load(int3(pixelPos, 0))[_CHANNEL];

    // Normalize and colorize value.
    float normalized =
        gParams.minValue == gParams.maxValue ? 0.f : saturate((value - gParams.minValue) / (gParams.maxValue - gParams.minValue));
    float3 colorized = applyColorMap(normalized);

    return float4(colorized, 1.f);
}
```

### AutoRanging

**Purpose**: Automatic range detection using parallel reduction.

**Features**:
- Min/max computation using parallel reduction
- Format-aware reduction (Float, Uint, Sint)
- Async reduction with fence synchronization
- Adaptive range smoothing

**Implementation**:
```cpp
std::optional<std::pair<double, double>> ColorMapPass::AutoRanging::getMinMax(
    RenderContext* pRenderContext,
    const ref<Texture>& texture,
    uint32_t channel
)
{
    FALCOR_ASSERT(pRenderContext);
    FALCOR_ASSERT(texture);
    FALCOR_ASSERT(channel < 4);

    std::optional<std::pair<double, double>> result;

    FormatType formatType = getFormatType(texture->getFormat());

    if (mReductionAvailable)
    {
        mpFence->wait();

        const void* values = mpReductionResult->map();

        switch (formatType)
        {
        case FormatType::Uint:
            result = {reinterpret_cast<const uint4*>(values)[0][channel], reinterpret_cast<const uint4*>(values)[1][channel]};
            break;
        case FormatType::Sint:
            result = {reinterpret_cast<const int4*>(values)[0][channel], reinterpret_cast<const int4*>(values)[1][channel]};
            break;
        default:
            result = {reinterpret_cast<const float4*>(values)[0][channel], reinterpret_cast<const float4*>(values)[1][channel]};
            break;
        }

        mpReductionResult->unmap();

        mReductionAvailable = false;
    }

    switch (formatType)
    {
    case FormatType::Uint:
        mpParallelReduction->execute<uint4>(pRenderContext, texture, ParallelReduction::Type::MinMax, nullptr, mpReductionResult);
        break;
    case FormatType::Sint:
        mpParallelReduction->execute<int4>(pRenderContext, texture, ParallelReduction::Type::MinMax, nullptr, mpReductionResult);
        break;
    default:
        mpParallelReduction->execute<float4>(pRenderContext, texture, ParallelReduction::Type::MinMax, nullptr, mpReductionResult);
        break;
    }

    pRenderContext->signal(mpFence.get());
    mReductionAvailable = true;

    return result;
}
```

### Arrow Texture

**Purpose**: 16x16 grayscale arrow texture for split screen divider.

**Format**:
- 256 bytes (16x16 grayscale)
- R8Unorm format
- Single mipmap level
- Max possible array size

**Arrow Array**:
```cpp
const unsigned char kArrowArray[256] = {
    // 16x16 grayscale arrow bitmap
    // ... (full array in SplitScreenPass.cpp)
};
```

### Mouse Event Handling

**SplitScreenPass Mouse Events**:
- **ButtonDown**: Grab divider on left mouse button
- **ButtonUp**: Release divider
- **Move**: Move divider if grabbed
- **Double-click**: Reset to center (detected via CpuTimer)

**Mouse Position Tracking**:
- Clamped to screen bounds
- Divider grab detection
- Mouse over divider detection
- Minimum landing size: 13 pixels (2*6+1)

**Divider Color**:
- Unselected: `float4(0, 0, 0, 1)` (black)
- Selected: `float4(1, 1, 1, 1)` (white)

## Architecture Patterns

### Render Pass Pattern
- Inherits from RenderPass base class
- Implements render() method
- Specifies input/output resources
- Manages shader compilation
- Parameter reflection

### Full Screen Pass Pattern
- Uses FullScreenPass base class
- Pixel shader execution
- Screen-space rendering
- Texture sampling

### Comparison Pattern
- Base class for comparison passes
- Split screen functionality
- Text labeling
- Swap sides

### Interactive Pattern
- Mouse event handling
- Drag-and-drop divider
- Visual feedback
- Double-click reset

### AutoRanging Pattern
- Parallel reduction for min/max
- Async computation with fence
- Adaptive range smoothing
- Format-aware reduction

## Code Patterns

### Property Parsing
```cpp
bool ComparisonPass::parseKeyValuePair(const std::string key, const Properties::ConstValue& val)
{
    if (key == kSplitLocation)
    {
        mSplitLoc = val;
        return true;
    }
    else if (key == kShowTextLabels)
    {
        mShowLabels = val;
        return true;
    }
    else if (key == kLeftLabel)
    {
        std::string str = val;
        mLeftLabel = str;
        return true;
    }
    else if (key == kRightLabel)
    {
        std::string str = val;
        mRightLabel = str;
        return true;
    }
    else
        return false;
}
```

### Property Serialization
```cpp
Properties ComparisonPass::getProperties() const
{
    Properties props;
    props[kSplitLocation] = mSplitLoc;
    props[kShowTextLabels] = mShowLabels;
    props[kLeftLabel] = mLeftLabel;
    props[kRightLabel] = mRightLabel;
    return props;
}
```

### Execute Implementation
```cpp
void ComparisonPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // Get references to our input, output, and temporary accumulation texture
    pLeftSrcTex = renderData.getTexture(kLeftInput);
    pRightSrcTex = renderData.getTexture(kRightInput);
    pDstFbo = Fbo::create(mpDevice, {renderData.getTexture(kOutput)});

    // If we haven't initialized split location, split screen in half by default
    if (mSplitLoc < 0)
        mSplitLoc = 0.5f;

    // Set shader parameters
    auto var = mpSplitShader->getRootVar();
    var["GlobalCB"]["gSplitLocation"] = int32_t(mSplitLoc * renderData.getDefaultTextureDims().x);
    var["GlobalCB"]["gDividerSize"] = mDividerSize;
    var["gLeftInput"] = mSwapSides ? pRightSrcTex : pLeftSrcTex;
    var["gRightInput"] = mSwapSides ? pLeftSrcTex : pRightSrcTex;

    // Execute the accumulation shader
    mpSplitShader->execute(pRenderContext, pDstFbo);

    // Render some labels
    if (mShowLabels)
    {
        const int32_t screenLocX = int32_t(mSplitLoc * renderData.getDefaultTextureDims().x);
        const int32_t screenLocY = int32_t(renderData.getDefaultTextureDims().y - 32);

        // Draw text labeling right side image
        std::string rightSide = mSwapSides ? mLeftLabel : mRightLabel;
        mpTextRenderer->render(pRenderContext, rightSide, pDstFbo, float2(screenLocX + 16, screenLocY));

        // Draw text labeling left side image
        std::string leftSide = mSwapSides ? mRightLabel : mLeftLabel;
        uint32_t leftLength = uint32_t(leftSide.length()) * 9;
        mpTextRenderer->render(pRenderContext, leftSide, pDstFbo, float2(screenLocX - 16 - leftLength, screenLocY));
    }
}
```

### Mouse Event Handling
```cpp
bool SplitScreenPass::onMouseEvent(const MouseEvent& mouseEvent)
{
    // If we have divider grabbed, claim *all* mouse movements for ourself
    bool handled = mDividerGrabbed;

    // Find out where on the screen we are
    mMousePos = int2(mouseEvent.screenPos.x, mouseEvent.screenPos.y);

    // If we're outside of window, stop.
    mMousePos = clamp(mMousePos, int2(0, 0), int2(pDstFbo->getWidth() - 1, pDstFbo->getHeight() - 1));

    // Actually process our events
    if (mMouseOverDivider && mouseEvent.type == MouseEvent::Type::ButtonDown && mouseEvent.button == Input::MouseButton::Left)
    {
        mDividerGrabbed = true;
        handled = true;

        if (CpuTimer::calcDuration(mTimeOfLastClick, CpuTimer::getCurrentTimePoint()) < 100.0)
            mSplitLoc = 0.5f;
        else
            mTimeOfLastClick = CpuTimer::getCurrentTimePoint();
    }
    else if (mDividerGrabbed)
    {
        if (mouseEvent.type == MouseEvent::Type::ButtonUp && mouseEvent.button == Input::MouseButton::Left)
        {
            mDividerGrabbed = false;
            handled = true;
        }
        else if (mouseEvent.type == MouseEvent::Type::Move)
        {
            mSplitLoc = (float)mMousePos.x / (float)pDstFbo->getWidth();
            handled = true;
        }
    }

    // Update whether mouse if over the divider.  To ensure selecting the slider isn't a pain,
    // have a minimum landing size (13 pixels, 2*6+1) that counts as hovering over the slider.
    mMouseOverDivider = (std::abs(int32_t(mSplitLoc * pDstFbo->getWidth()) - mMousePos.x) < std::max(6, int32_t(mDividerSize)));

    return handled;
}
```

### ColorMap Execution
```cpp
void ColorMapPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    const auto& inputTexture = renderData.getTexture(kInput);
    const auto& outputTexture = renderData.getTexture(kOutput);

    FormatType inputType = inputTexture ? getFormatType(inputTexture->getFormat()) : FormatType::Float;

    if (mAutoRange && inputTexture)
    {
        if (!mpAutoRanging)
            mpAutoRanging = std::make_unique<AutoRanging>(mpDevice);

        if (auto minMax = mpAutoRanging->getMinMax(pRenderContext, inputTexture, mChannel))
        {
            auto [minValue, maxValue] = *minMax;

            // Immediately grow auto range to include range of input.
            mAutoMinValue = std::min(mAutoMinValue, minValue);
            mAutoMaxValue = std::max(mAutoMaxValue, maxValue);

            // Smoothly shrink auto range when the range of input shrinks.
            double alpha = 0.01;
            mAutoMinValue = math::lerp(mAutoMinValue, minValue, alpha);
            mAutoMaxValue = math::lerp(mAutoMaxValue, maxValue, alpha);

            mMinValue = (float)mAutoMinValue;
            mMaxValue = (float)mAutoMaxValue;
        }
        else
        {
            mAutoMinValue = mMinValue;
            mAutoMaxValue = mMaxValue;
        }
    }
    else
    {
        mpAutoRanging.reset();
    }

    DefineList defines;
    defines.add("_COLOR_MAP", std::to_string((uint32_t)mColorMap));
    defines.add("_CHANNEL", std::to_string(mChannel));

    switch (inputType)
    {
    case FormatType::Uint:
        defines.add("_FORMAT", "FORMAT_UINT");
        break;
    case FormatType::Sint:
        defines.add("_FORMAT", "FORMAT_SINT");
        break;
    default:
        defines.add("_FORMAT", "FORMAT_FLOAT");
        break;
    }

    if (!mpColorMapPass || mRecompile)
    {
        mpColorMapPass = FullScreenPass::create(mpDevice, kShaderFile, defines);
        mRecompile = false;
    }

    ColorMapParams params;
    params.minValue = mMinValue;
    params.maxValue = mMaxValue;

    auto var = mpColorMapPass->getRootVar();
    var["gTexture"] = inputTexture;
    var["StaticCB"]["gParams"].setBlob(params);
    mpFbo->attachColorTarget(outputTexture, 0);
    mpColorMapPass->getState()->setFbo(mpFbo);
    mpColorMapPass->execute(pRenderContext, mpFbo);
}
```

## Integration Points

### Render Graph Integration
- Input/output specification via reflect()
- Resource binding in execute()
- Property serialization
- Scene-independent passes

### UI Integration
- Gui::Widgets for controls
- TextRenderer for labels
- Mouse event handling
- Interactive controls

### Text Rendering Integration
- TextRenderer for label rendering
- Font-based text rendering
- Screen-space positioning
- Label customization

### Algorithm Integration
- ParallelReduction for min/max computation
- Format-aware reduction
- Async computation with fence
- Adaptive range smoothing

### Color Utilities Integration
- ColorMap enum
- Color map functions (colormapGray, colormapJet, colormapViridis, colormapPlasma, colormapMagma, colormapInferno)
- Color map application

## Use Cases

### Rendering Quality Comparison
- Side-by-side comparison of rendered images
- Split screen comparison with movable divider
- Visual quality assessment
- Algorithm validation

### Debugging and Analysis
- Color mapping for data visualization
- Invalid pixel detection
- Material parameter visualization
- Lighting analysis

### Research and Development
- A/B testing of rendering techniques
- Parameter tuning visualization
- Performance comparison
- Algorithm development

### Educational Use
- Demonstrate rendering concepts
- Show comparison techniques
- Explain color mapping
- Teach debugging methods

## Performance Considerations

### GPU Computation
- Comparison pass: O(width × height) per frame
- Color mapping: O(width × height) per frame
- Parallel reduction: O(width × height) per frame
- Full screen quad rendering

### Memory Usage
- Input textures: O(width × height × sizeof(float4)) bytes
- Output texture: O(width × height × sizeof(float4)) bytes
- Arrow texture: 256 bytes (16x16 grayscale)
- Reduction result: 32 bytes (min/max for uint4/int4/float4)

### Computational Complexity
- Comparison: O(1) per pixel (simple texture fetch and comparison)
- Color mapping: O(1) per pixel (texture fetch, normalization, color map lookup)
- Parallel reduction: O(width × height) per frame (min/max computation)
- Overall: O(width × height) per frame

### Performance Optimization
- Full screen quad rendering (efficient)
- Simple pixel shader operations
- Minimal texture fetches
- Async parallel reduction
- Adaptive range smoothing (low overhead)

## Limitations

### Feature Limitations
- ComparisonPass: No configurable parameters beyond split location and labels
- SplitScreenPass: Limited to two input images, fixed divider size
- ColorMapPass: Limited to 2D textures, single channel selection
- No multi-sample anti-aliasing
- No advanced filtering

### UI Limitations
- ComparisonPass: No preset management, no undo/redo
- SplitScreenPass: No keyboard shortcuts, limited to mouse interaction
- ColorMapPass: No interactive range visualization, no preset management

### Performance Limitations
- No performance optimization for large textures
- No adaptive quality settings
- No caching of color maps
- No GPU compute optimization

### Integration Limitations
- Limited to Falcor's render graph system
- Requires specific input formats
- No external data sources
- Limited to Falcor's UI system

## Best Practices

### Comparison Pass Configuration
- Use appropriate split location for comparison
- Enable labels for clarity
- Use descriptive labels
- Swap sides for different perspectives

### Split Screen Pass Configuration
- Use appropriate divider size for visibility
- Enable arrows for visual feedback
- Use double-click for quick reset
- Test with different image sizes

### Color Map Pass Configuration
- Select appropriate color map for data type
- Use auto range for adaptive visualization
- Select correct channel for data
- Test with different color maps

### Debugging Best Practices
- Use side-by-side comparison for A/B testing
- Use color mapping for scalar data visualization
- Use split screen for interactive comparison
- Test with different rendering techniques

### Performance Optimization
- Use appropriate texture formats
- Minimize texture fetches
- Use parallel reduction for large textures
- Enable auto range for adaptive visualization

### Research and Development
- Use comparison passes for algorithm validation
- Use color mapping for data analysis
- Test with different rendering techniques
- Compare different parameter settings

### Educational Use
- Demonstrate comparison techniques
- Show color mapping concepts
- Explain debugging methods
- Visualize rendering concepts
