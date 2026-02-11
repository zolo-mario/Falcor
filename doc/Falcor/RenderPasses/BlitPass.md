# BlitPass - Image Blitting

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **BlitPass** - Image blitting pass (no sub-modules)

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Texture, Formats)
- **Core/Object** - Base object class
- **RenderGraph** - Render graph system (RenderPass, RenderPassReflection)
- **Utils/UI** - UI utilities (Gui)

## Module Overview

BlitPass is a simple render pass that blits (copies) an input texture to an output texture. This pass is primarily useful for format conversion between different texture formats. It supports configurable filtering modes (Linear or Point) and allows specification of the output format. The pass uses the RenderContext's blit functionality to perform the copy operation.

## Component Specifications

### BlitPass Class

**Files**:
- [`BlitPass.h`](Source/RenderPasses/BlitPass/BlitPass.h:1) - Blit pass header
- [`BlitPass.cpp`](Source/RenderPasses/BlitPass/BlitPass.cpp:1) - Blit pass implementation

**Purpose**: Render pass that blits an input texture to an output texture, useful for format conversion.

**Key Features**:
- Direct texture-to-texture blitting
- Configurable filtering mode (Linear/Point)
- Output format specification
- Simple and efficient implementation
- No custom shaders required

**Public Interface**:

```cpp
class BlitPass : public RenderPass
{
public:
    static ref<BlitPass> create(ref<Device> pDevice, const Properties& props);
    
    // RenderPass overrides
    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    
    // Scripting functions
    TextureFilteringMode getFilter() const;
    void setFilter(TextureFilteringMode filter);
};
```

**Private Members**:

```cpp
private:
    void parseProperties(const Properties& props);
    
    TextureFilteringMode mFilter = TextureFilteringMode::Linear;  ///< Filtering mode
    ResourceFormat mOutputFormat = ResourceFormat::Unknown;       ///< Output format
};
```

## Technical Details

### Blit Operation

The pass uses `RenderContext::blit()` to perform the copy operation:

```cpp
pRenderContext->blit(
    pSrcTex->getSRV(),           // Source shader resource view
    pDstTex->getRTV(),           // Destination render target view
    RenderContext::kMaxRect,      // Source rectangle (full texture)
    RenderContext::kMaxRect,      // Destination rectangle (full texture)
    mFilter                       // Filtering mode
);
```

### Filtering Modes

**Linear Filtering**:
- Bilinear interpolation
- Smoother results
- Better for upscaling
- Default mode

**Point Filtering**:
- Nearest neighbor sampling
- Sharp results
- Better for pixelated effects
- Preserves exact texel values

### Format Conversion

The pass supports format conversion by:
1. Specifying output format via `mOutputFormat`
2. Using RenderContext's automatic format conversion during blit
3. Supporting any format that the GPU can convert between

### Input/Output Specification

**RenderPassReflection**:

```cpp
RenderPassReflection r;
r.addOutput(kDst, "The destination texture").format(mOutputFormat);
r.addInput(kSrc, "The source texture");
```

- **Input**: "src" - Source texture to copy from
- **Output**: "dst" - Destination texture to copy to

### Property Serialization

**Serialized Properties**:
- `filter` - Filtering mode (Linear/Point)
- `outputFormat` - Output format (ResourceFormat::Unknown for default)

### Error Handling

The pass handles missing resources gracefully:
- Logs warning if input or output is missing
- Continues execution without crashing
- Allows render graph to continue even if this pass fails

## Integration Points

### Render Graph Integration

**Input/Output**:
- Input: "src" - Source texture
- Output: "dst" - Destination texture

**Bind Flags**:
- Input: ShaderResource (via getSRV())
- Output: RenderTarget (via getRTV())

**Format**:
- Input: Any format supported by GPU
- Output: Configurable via mOutputFormat, defaults to input format

**Size**:
- Source and destination can have different sizes
- Blit operation handles scaling automatically

### Scripting Integration

```python
# Python bindings
pass.filter = "Linear"  # or "Point"
```

The Python bindings use `enumToString()` and `stringToEnum()` for filter mode conversion.

### UI Integration

```cpp
void renderUI(Gui::Widgets& widget)
{
    if (auto filter = mFilter; widget.dropdown("Filter", filter))
        setFilter(filter);
}
```

Simple dropdown UI for selecting filtering mode.

## Architecture Patterns

### Simple Pass Pattern
- Minimal implementation
- No custom shaders
- Direct use of RenderContext functionality
- Single operation per frame

### Property Pattern
- Property serialization/deserialization
- UI integration
- Scripting bindings
- Configurable behavior

## Code Patterns

### Blit Execution

```cpp
void BlitPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    const auto& pSrcTex = renderData.getTexture(kSrc);
    const auto& pDstTex = renderData.getTexture(kDst);

    if (pSrcTex && pDstTex)
    {
        pRenderContext->blit(
            pSrcTex->getSRV(),
            pDstTex->getRTV(),
            RenderContext::kMaxRect,
            RenderContext::kMaxRect,
            mFilter
        );
    }
    else
    {
        logWarning("BlitPass::execute() - missing an input or output resource");
    }
}
```

### Property Parsing

```cpp
void BlitPass::parseProperties(const Properties& props)
{
    for (const auto& [key, value] : props)
    {
        if (key == kFilter)
            setFilter(value);
        if (key == kOutputFormat)
            mOutputFormat = value;
        else
            logWarning("Unknown property '{}' in a BlitPass properties.", key);
    }
}
```

### Property Serialization

```cpp
Properties BlitPass::getProperties() const
{
    Properties props;
    props[kFilter] = mFilter;
    if (mOutputFormat != ResourceFormat::Unknown)
        props[kOutputFormat] = mOutputFormat;
    return props;
}
```

## Use Cases

1. **Format Conversion**: Convert between different texture formats (e.g., RGBA8 to RGBA16Float)
2. **Texture Copy**: Copy textures between different render targets
3. **Scaling**: Resize textures with filtering (upscaling or downscaling)
4. **Format Standardization**: Ensure all textures in a pipeline use the same format
5. **Debugging**: Copy intermediate results for visualization
6. **Post-Processing**: Prepare textures for display or further processing

## Performance Considerations

- **GPU Operation**: Blit is performed entirely on GPU, no CPU-GPU synchronization needed
- **Hardware Accelerated**: Uses GPU's built-in blit functionality
- **Linear vs Point**: Linear filtering is slightly more expensive than point filtering
- **Format Conversion**: Some format conversions may be slower than others
- **Memory Bandwidth**: Blit operation is memory-bandwidth bound
- **No Shader Overhead**: No custom shaders, minimal driver overhead

## Limitations

- **Format Compatibility**: Not all format combinations are supported by GPU
- **Rectangular Regions**: Only supports full texture blitting (kMaxRect)
- **No Custom Shaders**: Cannot perform custom transformations during blit
- **Single Operation**: Only one blit per pass execution
- **No Mipmap Control**: Always blits mip level 0 and array slice 0
- **No Depth/Stencil**: Does not handle depth-stencil formats specially

## Comparison with Alternatives

**vs. Custom Shader Pass**:
- Simpler to implement
- Less flexible
- Better performance for simple copies
- No custom logic

**vs. CopyTexture**:
- More flexible (format conversion)
- Higher level API
- May have slightly more overhead

**vs. Compute Shader**:
- Simpler
- Less control
- Better for simple copies
- Worse for complex operations

## Best Practices

1. **Use Linear Filtering** for upscaling and smooth results
2. **Use Point Filtering** for pixelated effects and exact texel preservation
3. **Specify Output Format** when format conversion is needed
4. **Check Resource Availability** before blitting (handled by pass)
5. **Use for Format Conversion** rather than custom shaders when possible
6. **Avoid Unnecessary Blits** to minimize memory bandwidth usage

## Progress Log

- **2026-01-07T21:07:04Z**: BlitPass analysis completed. Analyzed BlitPass class (simple texture blitting with configurable filtering), BlitPass implementation (RenderContext::blit usage), filtering modes (Linear, Point), format conversion support, input/output specification (src/dst), property serialization (filter, outputFormat), error handling (missing resources), integration points (render graph, scripting, UI), architecture patterns (simple pass, property), code patterns (blit execution, property parsing, serialization), use cases (format conversion, texture copy, scaling), performance considerations (GPU operation, hardware accelerated, memory bandwidth), limitations (format compatibility, no custom shaders), comparison with alternatives, and best practices. Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
