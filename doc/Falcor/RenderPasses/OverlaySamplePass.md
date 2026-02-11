# OverlaySamplePass - Overlay Sample Pass

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **OverlaySamplePass** - Overlay sample demonstration pass
  - [x] **OverlaySamplePass.h** - Overlay sample pass header
  - [x] **OverlaySamplePass.cpp** - Overlay sample pass implementation

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Texture)
- **Core/Object** - Base object class
- **RenderGraph** - Render graph system (RenderPass, RenderPassReflection, RenderPassHelpers)
- **ImGui** - Immediate mode GUI library (imgui.h)

## Module Overview

The OverlaySamplePass is a demonstration pass that shows how to use the `renderOverlayUI` callback in a render pass to draw simple shapes to the screen. The pass uses ImGui directly to draw a 5x3 grid of rectangles, each containing different primitive types. This is an educational/example pass that demonstrates overlay UI capabilities in Falcor's render graph system. The pass copies input to output and draws overlay UI on top of the rendered output.

## Component Specifications

### OverlaySamplePass Class

**File**: [`OverlaySamplePass.h`](Source/RenderPasses/OverlaySamplePass/OverlaySamplePass.h:35)

**Purpose**: Demonstrate how to use `renderOverlayUI` callback to draw simple shapes to the screen.

**Public Interface**:

```cpp
class OverlaySamplePass : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(
        OverlaySamplePass,
        "OverlaySamplePass",
        "Demonstrates how to use the renderOverlayUI callback in a renderpass to draw simple shapes to screen."
    );

    static ref<OverlaySamplePass> create(ref<Device> pDevice, const Properties& props)
    {
        return make_ref<OverlaySamplePass>(pDevice, props);
    }

    OverlaySamplePass(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderOverlayUI(RenderContext* pRenderContext) override;
    static void registerBindings(pybind11::module& m);

private:
    void setFrameDim(const uint2 frameDim);

    uint2 mFrameDim;
    uint32_t mFrameCount = 0;
};
```

**Public Members**: None (all members are private)

**Private Members**:
- `uint2 mFrameDim` - Frame dimensions (default: {0, 0})
- `uint32_t mFrameCount` - Frame counter (default: 0)

**Private Methods**:
- `void setFrameDim(const uint2 frameDim)` - Set frame dimensions

## Technical Details

### Render Graph Integration

**Input Channels**:
- `input` (required): Input buffer (RGBA32Float)

**Output Channels**:
- `output` (required): Output buffer of solution (RGBA32Float)

**Reflection**:
```cpp
RenderPassReflection reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;

    // Define our input/output channels.
    addRenderPassInputs(reflector, kInputChannels);
    addRenderPassOutputs(reflector, kOutputChannels);

    return reflector;
}
```

**Input Channels Definition**:
```cpp
const ChannelList kInputChannels = {
    {"input", "", "Input buffer", true, ResourceFormat::RGBA32Float},
};
```

**Output Channels Definition**:
```cpp
const ChannelList kOutputChannels = {
    {"output", "", "Output buffer of solution", false, ResourceFormat::RGBA32Float},
};
```

### Execute

**Implementation**:
```cpp
void execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // Just copy input to output.
    auto src = renderData.getTexture(kInputChannels[0].name);
    auto dst = renderData.getTexture(kOutputChannels[0].name);
    mFrameDim.x = dst->getWidth();
    mFrameDim.y = dst->getHeight();
    if (src)
    {
        pRenderContext->blit(src->getSRV(), dst->getRTV());
    }
    mFrameCount++;
}
```

**Features**:
- Simple blit from input to output
- Frame dimension tracking
- Frame counter for statistics

### Overlay UI Rendering

**Implementation**:
```cpp
void renderOverlayUI(RenderContext* pRenderContext)
{
    // This callback occurs after "renderUI" callback, and will be triggered even when dropdown is closed.
    // Rather than drawing to a widget, we work directly with ImGui draw list.

    float margin = 50.f;

    float2 frameMin = float2(0.0) + margin;
    float2 frameMax = float2(mFrameDim) - margin;
    float2 frameSize = frameMax - frameMin;

    // Get background draw list
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    // Create a rectangular frame with a red border.
    drawList->AddRect(frameMin, frameMax, ImColor(float4(1.0, 0.0, 0.0, 0.0)));

    int primType = 0;

    // Inside that rectangle, create a 5x3 grid of rectangles.
    for (int j = 0; j < 3; j++)
    {
        for (int i = 0; i < 5; i++)
        {
            float2 rectMin = frameMin + frameSize * float2(float(i + 0) / 5.0f, float(j + 0) / 3.0f);
            float2 rectMax = frameMin + frameSize * float2(float(i + 1) / 5.0f, float(j + 1) / 3.0f);
            rectMin += margin;
            rectMax -= margin;
            drawList->AddRect(rectMin, rectMax, ImColor(float4(1.0f, 1.0f, 1.0f, 1.0f)));

            // Draw one of the primitive types in this rectangle.

            // draw a line
            if (primType == 0)
            {
                float2 p0 = rectMin;
                float2 p1 = rectMax;
                drawList->AddLine(p0 + margin, p1 - margin, ImColor(float4(1.0f, 1.0f, 1.0f, 1.0f)));
            }

            // draw a filled rectangle
            if (primType == 1)
            {
                drawList->AddRect(rectMin + margin, rectMax - margin, ImColor(float4(1.0f, 1.0f, 1.0f, 1.0f)));
            }

            // Draw a filled multicolor rectangle
            if (primType == 2)
            {
                drawList->AddRectFilledMultiColor(
                    rectMin + margin,
                    rectMax - margin,
                    ImColor(float4(1.0f, 1.0f, 1.0f, 1.0f)),
                    ImColor(float4(0.0f, 0.0f, 1.0f, 1.0f)),
                    ImColor(float4(0.0f, 1.0f, 0.0f, 1.0f)),
                    ImColor(float4(1.0f, 0.0f, 0.0f, 1.0f))
                );
            }

            // Draw a diamond using a quad function
            if (primType == 3)
            {
                float2 center = (rectMin + rectMax) / 2.0f;
                float2 size = ((rectMax - rectMin) / 2.0f) - 2.f * float2(margin, margin);
                float2 p1 = center + float2(0.0f, size.y);
                float2 p2 = center + float2(size.x, 0.0f);
                float2 p3 = center + float2(0.0f, -size.y);
                float2 p4 = center + float2(-size.x, 0.0f);
                drawList->AddQuad(p1, p2, p3, p4, ImColor(float4(1.0f, 1.0f, 1.0f, 1.0f)));
            }

            // Draw a filled diamond using a quad function
            if (primType == 4)
            {
                float2 center = (rectMin + rectMax) / 2.0f;
                float2 size = ((rectMax - rectMin) / 2.0f) - 2.f * float2(margin, margin);
                float2 p1 = center + float2(0.0f, size.y);
                float2 p2 = center + float2(size.x, 0.0f);
                float2 p3 = center + float2(0.0f, -size.y);
                float2 p4 = center + float2(-size.x, 0.0f);
                drawList->AddQuadFilled(p1, p2, p3, p4, ImColor(float4(1.0f, 1.0f, 1.0f, 1.0f)));
            }

            // Draw a triangle within the rectangle
            if (primType == 5)
            {
                // Create three tri points between 0 and 1
                float2 p1 = (rectMin + margin) + float2(0.5f, 0.0f) * ((rectMax - margin) - (rectMin + margin));
                float2 p2 = (rectMin + margin) + float2(1.0f, 1.0f) * ((rectMax - margin) - (rectMin + margin));
                float2 p3 = (rectMin + margin) + float2(0.0f, 1.0f) * ((rectMax - margin) - (rectMin + margin));
                drawList->AddTriangle(p1, p2, p3, ImColor(float4(1.0f, 1.0f, 1.0f, 1.0f)));
            }

            // Draw a filled triangle within the rectangle
            if (primType == 6)
            {
                float2 p1 = (rectMin + margin) + float2(0.5f, 0.0f) * ((rectMax - margin) - (rectMin + margin));
                float2 p2 = (rectMin + margin) + float2(1.0f, 1.0f) * ((rectMax - margin) - (rectMin + margin));
                float2 p3 = (rectMin + margin) + float2(0.0f, 1.0f) * ((rectMax - margin) - (rectMin + margin));
                drawList->AddTriangleFilled(p1, p2, p3, ImColor(float4(1.0f, 1.0f, 1.0f, 1.0f)));
            }

            // Draw a circle within the rectangle
            if (primType == 7)
            {
                float2 center = ((rectMin + margin) + (rectMax - margin)) / 2.0f;
                float2 size = (rectMax - margin) - (rectMin + margin);
                float radius = std::min(size.x, size.y) / 2.0f;
                drawList->AddCircle(center, radius, ImColor(float4(1.0f, 1.0f, 1.0f, 1.0f)));
            }

            // Draw a filled circle within the rectangle
            if (primType == 8)
            {
                float2 center = ((rectMin + margin) + (rectMax - margin)) / 2.0f;
                float2 size = (rectMax - margin) - (rectMin + margin);
                float radius = std::min(size.x, size.y) / 2.0f;
                drawList->AddCircleFilled(center, radius, ImColor(float4(1.0f, 1.0f, 1.0f, 1.0f)));
            }

            // Draw an ngon
            if (primType == 9)
            {
                float2 center = ((rectMin + margin) + (rectMax - margin)) / 2.0f;
                float2 size = (rectMax - margin) - (rectMin + margin);
                float radius = std::min(size.x, size.y) / 2.0f;
                drawList->AddNgon(center, radius, 5, ImColor(float4(1.0f, 1.0f, 1.0f, 1.0f)));
            }

            // Draw a filled ngon
            if (primType == 10)
            {
                float2 center = ((rectMin + margin) + (rectMax - margin)) / 2.0f;
                float2 size = (rectMax - margin) - (rectMin + margin);
                float radius = std::min(size.x, size.y) / 2.0f;
                drawList->AddNgonFilled(center, radius, 5, ImColor(float4(1.0f, 1.0f, 1.0f, 1.0f)));
            }

            // Draw a text string
            if (primType == 11)
            {
                std::string text = "Hello, world!";
                drawList->AddText(rectMin + margin, ImColor(float4(1.0f, 1.0f, 1.0f, 1.0f)), text.c_str());
            }

            // Draw an example polyline inside the area of the rectangle in the form of a star
            if (primType == 12)
            {
                std::vector<ImVec2> points;
                float2 center = ((rectMin + margin) + (rectMax - margin)) / 2.0f;
                float2 size = (rectMax - margin) - (rectMin + margin);
                float radius = std::min(size.x, size.y) / 2.0f;
                for (int k = 0; k < 12; k++)
                {
                    float angle = 2.0f * 3.14159f * float(k) / 12.0f;
                    float2 point = center + radius * (((k % 2) == 0) ? .5f : 1.f) * float2(std::cos(angle), std::sin(angle));
                    points.push_back(point);
                }
                drawList->AddPolyline(points.data(), points.size(), ImColor(float4(1.0f, 1.0f, 1.0f, 1.0f)), ImDrawFlags_None, 1.0f);
            }

            // Same as above, but makes a filled convex polygon
            if (primType == 13)
            {
                std::vector<ImVec2> points;
                float2 center = ((rectMin + margin) + (rectMax - margin)) / 2.0f;
                float2 size = (rectMax - margin) - (rectMin + margin);
                float radius = std::min(size.x, size.y) / 2.0f;
                for (int k = 0; k < 12; k++)
                {
                    float angle = 2.0f * 3.14159f * float(k) / 12.0f;
                    float2 point = center + radius * (((k % 2) == 0) ? .5f : 1.f) * float2(std::cos(angle), std::sin(angle));
                    points.push_back(point);
                }
                drawList->AddConvexPolyFilled(points.data(), points.size(), ImColor(float4(1.0f, 1.0f, 1.0f, 1.0f)));
            }

            // Now draw a bezier cubic curve in the next rectangle
            if (primType == 14)
            {
                float2 p0 = rectMin + margin;
                float2 p1 = rectMin + float2(0.25f, 0.75f) * (rectMax - rectMin);
                float2 p2 = rectMin + float2(0.75f, 0.25f) * (rectMax - rectMin);
                float2 p3 = rectMax - margin;
                drawList->AddBezierCubic(p0, p1, p2, p3, ImColor(float4(1.0f, 1.0f, 1.0f, 1.0f)), 1.0f);
            }

            primType++;
        }
    }
}
```

**Features**:
- Direct ImGui draw list usage
- Background draw list retrieval
- 5x3 grid of rectangles
- 15 different primitive types
- Red border frame
- Margin-based positioning

### Primitive Types

The pass demonstrates 15 different primitive types:

1. **Line** (primType == 0): Simple line from rectMin to rectMax
2. **Filled Rectangle** (primType == 1): Filled rectangle
3. **Multicolor Rectangle** (primType == 2): Rectangle with 4 colors
4. **Diamond** (primType == 3): Diamond using quad function
5. **Filled Diamond** (primType == 4): Filled diamond using quad function
6. **Triangle** (primType == 5): Triangle with 3 points
7. **Filled Triangle** (primType == 6): Filled triangle
8. **Circle** (primType == 7): Circle with radius
9. **Filled Circle** (primType == 8): Filled circle
10. **Ngon** (primType == 9): 5-sided polygon
11. **Filled Ngon** (primType == 10): Filled 5-sided polygon
12. **Polyline** (primType == 11): Star-shaped polyline with 12 points
13. **Convex Polygon** (primType == 12): Filled star-shaped convex polygon
14. **Bezier Cubic** (primType == 13): Cubic Bezier curve
15. **Text** (primType == 14): Text string "Hello, world!"

### Property Serialization

The pass supports property serialization for configuration:

```cpp
Properties getProperties() const
{
    Properties props;
    return props;
}
```

**Note**: The pass does not expose any configurable properties.

### Scripting Integration

The pass supports scripting through:
- Static registration via `registerBindings()` function
- No configurable parameters
- Can be instantiated from Python scripts

```cpp
static void registerBindings(pybind11::module& m) {}
```

### Plugin Registration

The pass registers itself as a plugin:

```cpp
extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, OverlaySamplePass>();
    ScriptBindings::registerBinding(OverlaySamplePass::registerBindings);
}
```

## Architecture Patterns

### Render Pass Pattern

- Inherits from [`RenderPass`](Source/Falcor/RenderGraph/RenderPass.h:1) base class
- Implements standard render pass interface:
  - `reflect()` - Defines input/output resources
  - `execute()` - Performs blit operation
  - `renderOverlayUI()` - Draws overlay UI
  - `getProperties()` - Serializes configuration

### Overlay UI Pattern

- Uses `renderOverlayUI()` callback for overlay rendering
- Direct ImGui draw list usage
- Background draw list retrieval
- Primitive drawing with ImGui API

### Simple Pass Pattern

- Simple blit from input to output
- Frame dimension tracking
- Frame counter for statistics
- No complex processing or computation

### Educational Pattern

- Demonstrates overlay UI capabilities
- Shows various primitive types
- Example code for custom overlays
- Grid-based layout for multiple examples

## Code Patterns

### Property Serialization

```cpp
Properties OverlaySamplePass::getProperties() const
{
    Properties props;
    return props;
}
```

### Execute Implementation

```cpp
void OverlaySamplePass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // Just copy input to output.
    auto src = renderData.getTexture(kInputChannels[0].name);
    auto dst = renderData.getTexture(kOutputChannels[0].name);
    mFrameDim.x = dst->getWidth();
    mFrameDim.y = dst->getHeight();
    if (src)
    {
        pRenderContext->blit(src->getSRV(), dst->getRTV());
    }
    mFrameCount++;
}
```

### Frame Dimension Setting

```cpp
void OverlaySamplePass::setFrameDim(const uint2 frameDim)
{
    mFrameDim = frameDim;
}
```

### Plugin Registration

```cpp
extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, OverlaySamplePass>();
    ScriptBindings::registerBinding(OverlaySamplePass::registerBindings);
}
```

### Scripting Registration

```cpp
static void registerBindings(pybind11::module& m) {}
```

## Integration Points

### Render Graph Integration

The pass integrates with render graph system:
- Input channels: 1 required (input)
- Output channels: 1 required (output)
- Reflects input/output resources via `reflect()` method
- Executes blit in `execute()` method
- Draws overlay UI via `renderOverlayUI()` method

### ImGui Integration

The pass integrates with ImGui:
- Direct ImGui draw list usage
- Background draw list retrieval
- Primitive drawing functions
- Color specification with ImColor
- Coordinate system with float2

### Scripting Integration

The pass supports scripting through:
- Static registration via `registerBindings()` function
- Plugin registration via `registerPlugin()` function
- Can be instantiated from Python scripts
- No configurable parameters

## Use Cases

### Educational Use

Educational applications:
- Demonstrate overlay UI capabilities
- Show various primitive types
- Example code for custom overlays
- Teach ImGui drawing techniques
- Explain render pass concepts

### Development Use

Development applications:
- Test overlay UI functionality
- Debug rendering pipeline
- Verify ImGui integration
- Prototype custom overlays
- Test primitive drawing

### UI Development

UI development applications:
- Test ImGui draw lists
- Verify primitive rendering
- Test color specifications
- Validate coordinate systems
- Test overlay positioning

### Debugging

Debugging applications:
- Visualize frame dimensions
- Test frame counter
- Verify blit operation
- Test overlay rendering
- Debug ImGui integration

### Example Code

Example code:
- Show how to use renderOverlayUI callback
- Demonstrate ImGui drawing
- Show primitive types
- Example coordinate calculations
- Example color usage

## Performance Considerations

### GPU Computation

- Simple blit operation: O(width × height)
- No complex shaders or compute passes
- Minimal GPU computation
- Memory bandwidth limited operation

### Memory Usage

- Input texture: RGBA32Float (16 bytes per pixel)
- Output texture: RGBA32Float (16 bytes per pixel)
- Total memory: O(width × height × 16 × 2) bytes
- No additional buffers or resources

### Computational Complexity

- Per-pixel computation: O(1) for blit
- Overall: O(width × height) per frame
- No complex algorithms or loops
- Simple copy operation

### Overlay Rendering Overhead

- ImGui draw list creation: O(1)
- Primitive drawing: O(15) per frame
- Grid calculation: O(15) per frame
- Total overhead: Minimal

### Frame Counter Overhead

- Simple increment operation: O(1) per frame
- No complex calculations
- Minimal overhead

## Limitations

### Feature Limitations

- No configurable parameters
- No property serialization
- No dynamic primitive selection
- Fixed 5x3 grid layout
- Fixed primitive types
- No color customization
- No size customization
- No position customization

### UI Limitations

- No interactive controls
- No user input
- No configuration options
- Fixed layout and colors
- No customization options
- No preset management

### Performance Limitations

- No performance optimization
- No adaptive quality
- No performance profiling
- No benchmarking tools
- No statistics display

### Functional Limitations

- Simple blit operation only
- No image processing
- No filtering or effects
- No advanced features
- Educational/example only

### Integration Limitations

- Limited to render graph system
- No external dependencies
- No scene integration
- No camera integration
- No lighting integration

## Best Practices

### Educational Use

- Use as reference for overlay UI
- Study ImGui drawing techniques
- Learn primitive types
- Understand coordinate systems
- Study color specifications

### Development Use

- Use as template for custom overlays
- Modify for specific needs
- Add configurable parameters
- Extend with new primitives
- Integrate with render graphs

### UI Development

- Study ImGui draw list usage
- Learn primitive drawing functions
- Understand coordinate systems
- Test color specifications
- Validate overlay positioning

### Debugging

- Test with different frame sizes
- Verify blit operation
- Test overlay rendering
- Monitor frame counter
- Validate ImGui integration

### Example Code

- Use as starting point
- Modify for specific requirements
- Add new primitive types
- Customize layout and colors
- Integrate with existing passes

## Related Components

### RenderGraph/RenderPass

Render pass base class:
- `RenderPass::reflect()` - Define input/output resources
- `RenderPass::execute()` - Execute render pass
- `RenderPass::renderOverlayUI()` - Render overlay UI
- `RenderPass::getProperties()` - Serialize configuration

### RenderGraph/RenderPassHelpers

Render pass helper utilities:
- `addRenderPassInputs()` - Add input channels
- `addRenderPassOutputs()` - Add output channels

### ImGui

Immediate mode GUI library:
- `ImGui::GetBackgroundDrawList()` - Get background draw list
- `ImDrawList::AddRect()` - Add rectangle
- `ImDrawList::AddLine()` - Add line
- `ImDrawList::AddRectFilled()` - Add filled rectangle
- `ImDrawList::AddRectFilledMultiColor()` - Add multicolor rectangle
- `ImDrawList::AddQuad()` - Add quad
- `ImDrawList::AddQuadFilled()` - Add filled quad
- `ImDrawList::AddTriangle()` - Add triangle
- `ImDrawList::AddTriangleFilled()` - Add filled triangle
- `ImDrawList::AddCircle()` - Add circle
- `ImDrawList::AddCircleFilled()` - Add filled circle
- `ImDrawList::AddNgon()` - Add ngon
- `ImDrawList::AddNgonFilled()` - Add filled ngon
- `ImDrawList::AddText()` - Add text
- `ImDrawList::AddPolyline()` - Add polyline
- `ImDrawList::AddConvexPolyFilled()` - Add filled convex polygon
- `ImDrawList::AddBezierCubic()` - Add cubic Bezier curve
- `ImColor` - Color specification

### Falcor/ScriptBindings

Scripting utilities:
- `ScriptBindings::registerBinding()` - Register Python bindings
