# ImageLoader - Image Loading Pass

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **ImageLoader** - Image loading pass
  - [x] **ImageLoader.h** - Image loader header
  - [x] **ImageLoader.cpp** - Image loader implementation

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Texture, Formats)
- **Core/Object** - Base object class
- **Core/AssetResolver** - Asset path resolution
- **RenderGraph** - Render graph system (RenderPass, RenderPassReflection, RenderPassHelpers)
- **Utils/UI** - UI utilities (Gui)

## Module Overview

The ImageLoader provides a simple render pass for loading images from files and outputting them as textures. It supports various image formats, mipmap generation, sRGB color space loading, array slice selection, and mip level selection. The pass integrates with the render graph system and provides UI controls for loading images, configuring output size, and selecting mip levels and array slices.

## Component Specifications

### ImageLoader Class

**File**: [`ImageLoader.h`](Source/RenderPasses/ImageLoader/ImageLoader.h:35)

**Purpose**: Load images from files and output as textures.

**Public Interface**:

```cpp
class ImageLoader : public RenderPass
{
public:
    static ref<ImageLoader> create(ref<Device> pDevice, const Properties& props);
    ImageLoader(ref<Device> pDevice, const Properties& props);

    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual Properties getProperties() const override;
};
```

**Public Members**: None (all members are private)

**Private Members**:
- `ref<Texture> mpTex` - Loaded texture
- `std::filesystem::path mImagePath` - Path to image file
- `uint32_t mArraySlice` - Array slice index (default: 0)
- `uint32_t mMipLevel` - Mip level index (default: 0)
- `bool mGenerateMips` - Generate mipmaps (default: false)
- `bool mLoadSRGB` - Load as sRGB (default: true)
- `RenderPassHelpers::IOSize mOutputSizeSelection` - Output size selection (default: Default)
- `ResourceFormat mOutputFormat` - Output resource format (default: Unknown)
- `uint2 mOutputSize` - Current output size in pixels

**Private Methods**:
- `bool loadImage(const std::filesystem::path& path)` - Load image from file

## Technical Details

### Image Loading

The pass loads images using [`Texture::createFromFile()`](Source/Falcor/Core/API/Texture.h:1):

**Function**:
```cpp
bool loadImage(const std::filesystem::path& path)
{
    if (path.empty())
        return false;

    std::filesystem::path resolvedPath = AssetResolver::getDefaultResolver().resolvePath(path);
    if (std::filesystem::exists(resolvedPath))
    {
        mImagePath = path;
        mpTex = Texture::createFromFile(mpDevice, resolvedPath, mGenerateMips, mLoadSRGB);
        return mpTex != nullptr;
    }
    else
    {
        return false;
    }
}
```

**Parameters**:
- `path` - Path to image file
- `mGenerateMips` - Generate mipmaps flag
- `mLoadSRGB` - Load as sRGB flag

**Supported Formats**: Depends on [`Texture::createFromFile()`](Source/Falcor/Core/API/Texture.h:1) implementation (typically PNG, JPG, EXR, HDR, DDS, etc.)

### Output Size Configuration

The pass supports three output size modes via [`RenderPassHelpers::IOSize`](Source/Falcor/RenderGraph/RenderPassHelpers.h:1):

**Modes**:
1. **Default** - Output size based on requirements of connected passes
2. **Fixed** - Output size always at image's native size
3. **Custom** - User-specified output size

**Size Calculation**:
```cpp
uint2 fixedSize = mpTex ? uint2(mpTex->getWidth(), mpTex->getHeight()) : uint2(0);
const uint2 sz = RenderPassHelpers::calculateIOSize(mOutputSizeSelection, fixedSize, compileData.defaultTexDims);
```

**Rescaling**: If output size differs from native image resolution, image is rescaled bilinearly during blit.

### Render Graph Integration

**Output Channels**:
- `dst` (required): Destination texture output

**Reflection**:
```cpp
RenderPassReflection reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    uint2 fixedSize = mpTex ? uint2(mpTex->getWidth(), mpTex->getHeight()) : uint2(0);
    const uint2 sz = RenderPassHelpers::calculateIOSize(mOutputSizeSelection, fixedSize, compileData.defaultTexDims);

    reflector.addOutput(kDst, "Destination texture").format(mOutputFormat).texture2D(sz.x, sz.y);
    return reflector;
}
```

**Output Format**: Matches the format of the destination texture connected in the render graph.

### Property Serialization

The pass supports property serialization for configuration:

```cpp
Properties getProperties() const
{
    Properties props;
    props[kOutputSize] = mOutputSizeSelection;
    if (mOutputFormat != ResourceFormat::Unknown)
        props[kOutputFormat] = mOutputFormat;
    props[kImage] = mImagePath;
    props[kMips] = mGenerateMips;
    props[kSrgb] = mLoadSRGB;
    props[kArraySlice] = mArraySlice;
    props[kMipLevel] = mMipLevel;
    return props;
}
```

**Property Keys**:
- `outputSize` - Output size selection
- `outputFormat` - Output resource format
- `filename` - Image file path
- `mips` - Generate mipmaps flag
- `srgb` - Load as sRGB flag
- `arrayIndex` - Array slice index
- `mipLevel` - Mip level index

### Mip Level Selection

The pass supports selecting specific mip levels for output:

**Constraints**:
- `mMipLevel` must be less than `mpTex->getMipCount()`
- Clamped in `execute()`: `mMipLevel = std::min(mMipLevel, mpTex->getMipCount() - 1)`

**UI Control**:
```cpp
if (mpTex->getMipCount() > 1)
    widget.slider("Mip Level", mMipLevel, 0u, mpTex->getMipCount() - 1);
```

**Blit with Mip Level**:
```cpp
pRenderContext->blit(mpTex->getSRV(mMipLevel, 1, mArraySlice, 1), pDstTex->getRTV());
```

### Array Slice Selection

The pass supports selecting specific array slices for texture arrays:

**Constraints**:
- `mArraySlice` must be less than `mpTex->getArraySize()`
- Clamped in `execute()`: `mArraySlice = std::min(mArraySlice, mpTex->getArraySize() - 1)`

**UI Control**:
```cpp
if (mpTex->getArraySize() > 1)
    widget.slider("Array Slice", mArraySlice, 0u, mpTex->getArraySize() - 1);
```

**Blit with Array Slice**:
```cpp
pRenderContext->blit(mpTex->getSRV(mMipLevel, 1, mArraySlice, 1), pDstTex->getRTV());
```

### Color Space Loading

The pass supports loading images in different color spaces:

**sRGB Loading**:
- When `mLoadSRGB = true`, image is loaded as sRGB
- When `mLoadSRGB = false`, image is loaded as linear RGB
- Default: `true`

**UI Control**:
```cpp
widget.checkbox("Load As SRGB", mLoadSRGB);
```

### Mipmap Generation

The pass supports automatic mipmap generation:

**Generation**:
- When `mGenerateMips = true`, mipmaps are generated during image loading
- When `mGenerateMips = false`, only base level is loaded
- Default: `false`

**UI Control**:
```cpp
widget.checkbox("Generate Mipmaps", mGenerateMips);
```

### Asset Path Resolution

The pass uses [`AssetResolver`](Source/Falcor/Core/AssetResolver.h:1) for path resolution:

**Resolution**:
```cpp
std::filesystem::path resolvedPath = AssetResolver::getDefaultResolver().resolvePath(path);
```

**Purpose**:
- Resolves relative paths to absolute paths
- Supports asset directory configuration
- Enables flexible image file location

### Compile and Execute

**Compile Phase**:
```cpp
void compile(RenderContext* pRenderContext, const CompileData& compileData)
{
    FALCOR_CHECK(mpTex, "ImageLoader: No image loaded");
}
```

**Execute Phase**:
```cpp
void execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    const auto& pDstTex = renderData.getTexture(kDst);
    FALCOR_ASSERT(pDstTex);
    mOutputFormat = pDstTex->getFormat();
    mOutputSize = {pDstTex->getWidth(), pDstTex->getHeight()};

    if (!mpTex)
    {
        pRenderContext->clearRtv(pDstTex->getRTV().get(), float4(0, 0, 0, 0));
        return;
    }

    mMipLevel = std::min(mMipLevel, mpTex->getMipCount() - 1);
    mArraySlice = std::min(mArraySlice, mpTex->getArraySize() - 1);
    pRenderContext->blit(mpTex->getSRV(mMipLevel, 1, mArraySlice, 1), pDstTex->getRTV());
}
```

**Clear on No Image**: If no image is loaded, destination texture is cleared to black.

**Blit**: Uses [`RenderContext::blit()`](Source/Falcor/Core/API/RenderContext.h:1) for texture copy with optional rescaling.

### UI Controls

The pass provides comprehensive UI controls:

**Output Size Dropdown**:
```cpp
if (widget.dropdown("Output size", mOutputSizeSelection))
    requestRecompile();
```

**Image File Display**:
```cpp
widget.text("Image File: " + mImagePath.string());
```

**Load Options**:
```cpp
bool reloadImage = false;
reloadImage |= widget.checkbox("Load As SRGB", mLoadSRGB);
reloadImage |= widget.checkbox("Generate Mipmaps", mGenerateMips);
```

**Load File Button**:
```cpp
if (widget.button("Load File"))
{
    reloadImage |= openFileDialog({}, mImagePath);
}
```

**Mip Level Slider**:
```cpp
if (mpTex->getMipCount() > 1)
    widget.slider("Mip Level", mMipLevel, 0u, mpTex->getMipCount() - 1);
```

**Array Slice Slider**:
```cpp
if (mpTex->getArraySize() > 1)
    widget.slider("Array Slice", mArraySlice, 0u, mpTex->getArraySize() - 1);
```

**Image Preview**:
```cpp
widget.image(mImagePath.string().c_str(), mpTex.get(), {320, 320});
```

**Image Information Display**:
```cpp
widget.text("Image format: " + to_string(mpTex->getFormat()));
widget.text("Image size: (" + std::to_string(mpTex->getWidth()) + ", " + std::to_string(mpTex->getHeight()) + ")");
widget.text("Output format: " + to_string(mOutputFormat));
widget.text("Output size: (" + std::to_string(mOutputSize.x) + ", " + std::to_string(mOutputSize.y) + ")");
```

**Image Reload**:
```cpp
if (reloadImage && !mImagePath.empty())
{
    uint2 prevSize = {};
    if (mpTex)
        prevSize = {mpTex->getWidth(), mpTex->getHeight()};

    if (!loadImage(mImagePath))
    {
        msgBox("Error", fmt::format("Failed to load image from '{}'", mImagePath), MsgBoxType::Ok, MsgBoxIcon::Warning);
    }

    // Trigger graph recompile if output is set to native size and image dimensions have changed
    if (mOutputSizeSelection == RenderPassHelpers::IOSize::Fixed && mpTex != nullptr &&
        (mpTex->getWidth() != prevSize.x || mpTex->getHeight() != prevSize.y))
    {
        requestRecompile();
    }
}
```

## Architecture Patterns

### Render Pass Pattern

- Inherits from [`RenderPass`](Source/Falcor/RenderGraph/RenderPass.h:1) base class
- Implements standard render pass interface:
  - `reflect()` - Defines output resources
  - `compile()` - Validates loaded image
  - `execute()` - Performs texture blit
  - `renderUI()` - Provides UI controls
  - `getProperties()` - Serializes configuration

### Property Pattern

- Implements property-based configuration
- Supports serialization/deserialization via `Properties` class
- Enables configuration through render graph and scripting

### Asset Loading Pattern

- Uses [`Texture::createFromFile()`](Source/Falcor/Core/API/Texture.h:1) for image loading
- Supports various image formats
- Handles mipmap generation and color space conversion
- Uses [`AssetResolver`](Source/Falcor/Core/AssetResolver.h:1) for path resolution

### Blit Pattern

- Uses [`RenderContext::blit()`](Source/Falcor/Core/API/RenderContext.h:1) for texture copy
- Supports rescaling when output size differs from source
- Handles mip level and array slice selection
- GPU-accelerated texture copy

### UI Pattern

- Provides comprehensive UI via `renderUI()` method
- Uses [`Gui::Widgets`](Source/Falcor/Utils/UI/Gui.h:1) for UI controls
- Supports file dialogs for image selection
- Displays image preview and information
- Triggers graph recompile when needed

## Code Patterns

### Image Loading

```cpp
bool loadImage(const std::filesystem::path& path)
{
    if (path.empty())
        return false;

    std::filesystem::path resolvedPath = AssetResolver::getDefaultResolver().resolvePath(path);
    if (std::filesystem::exists(resolvedPath))
    {
        mImagePath = path;
        mpTex = Texture::createFromFile(mpDevice, resolvedPath, mGenerateMips, mLoadSRGB);
        return mpTex != nullptr;
    }
    else
    {
        return false;
    }
}
```

### Execute with Blit

```cpp
void execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    const auto& pDstTex = renderData.getTexture(kDst);
    FALCOR_ASSERT(pDstTex);
    mOutputFormat = pDstTex->getFormat();
    mOutputSize = {pDstTex->getWidth(), pDstTex->getHeight()};

    if (!mpTex)
    {
        pRenderContext->clearRtv(pDstTex->getRTV().get(), float4(0, 0, 0, 0));
        return;
    }

    mMipLevel = std::min(mMipLevel, mpTex->getMipCount() - 1);
    mArraySlice = std::min(mArraySlice, mpTex->getArraySize() - 1);
    pRenderContext->blit(mpTex->getSRV(mMipLevel, 1, mArraySlice, 1), pDstTex->getRTV());
}
```

### UI with Reload

```cpp
void renderUI(Gui::Widgets& widget)
{
    if (widget.dropdown("Output size", mOutputSizeSelection))
        requestRecompile();

    widget.text("Image File: " + mImagePath.string());
    bool reloadImage = false;
    reloadImage |= widget.checkbox("Load As SRGB", mLoadSRGB);
    reloadImage |= widget.checkbox("Generate Mipmaps", mGenerateMips);

    if (widget.button("Load File"))
    {
        reloadImage |= openFileDialog({}, mImagePath);
    }

    if (mpTex)
    {
        if (mpTex->getMipCount() > 1)
            widget.slider("Mip Level", mMipLevel, 0u, mpTex->getMipCount() - 1);
        if (mpTex->getArraySize() > 1)
            widget.slider("Array Slice", mArraySlice, 0u, mpTex->getArraySize() - 1);

        widget.image(mImagePath.string().c_str(), mpTex.get(), {320, 320});
        widget.text("Image format: " + to_string(mpTex->getFormat()));
        widget.text("Image size: (" + std::to_string(mpTex->getWidth()) + ", " + std::to_string(mpTex->getHeight()) + ")");
        widget.text("Output format: " + to_string(mOutputFormat));
        widget.text("Output size: (" + std::to_string(mOutputSize.x) + ", " + std::to_string(mOutputSize.y) + ")");
    }

    if (reloadImage && !mImagePath.empty())
    {
        uint2 prevSize = {};
        if (mpTex)
            prevSize = {mpTex->getWidth(), mpTex->getHeight()};

        if (!loadImage(mImagePath))
        {
            msgBox("Error", fmt::format("Failed to load image from '{}'", mImagePath), MsgBoxType::Ok, MsgBoxIcon::Warning);
        }

        if (mOutputSizeSelection == RenderPassHelpers::IOSize::Fixed && mpTex != nullptr &&
            (mpTex->getWidth() != prevSize.x || mpTex->getHeight() != prevSize.y))
        {
            requestRecompile();
        }
    }
}
```

### Property Parsing

```cpp
ImageLoader::ImageLoader(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    for (const auto& [key, value] : props)
    {
        if (key == kOutputSize)
            mOutputSizeSelection = value;
        else if (key == kOutputFormat)
            mOutputFormat = value;
        else if (key == kImage)
            mImagePath = value.operator std::filesystem::path();
        else if (key == kSrgb)
            mLoadSRGB = value;
        else if (key == kMips)
            mGenerateMips = value;
        else if (key == kArraySlice)
            mArraySlice = value;
        else if (key == kMipLevel)
            mMipLevel = value;
        else
            logWarning("Unknown property '{}' in a ImageLoader properties.", key);
    }

    if (!mImagePath.empty())
    {
        if (!loadImage(mImagePath))
        {
            FALCOR_THROW("ImageLoader: Failed to load image from '{}'", mImagePath);
        }
    }
}
```

## Integration Points

### Render Graph Integration

The pass integrates with render graph system:
- Output channel: dst
- Reflects output resources via `reflect()` method
- Executes texture blit in `execute()` method
- Supports property-based configuration
- Triggers graph recompile when output size changes

### Asset Resolver Integration

The pass uses asset resolver for path resolution:
- `AssetResolver::getDefaultResolver().resolvePath(path)` - Resolve relative paths
- Supports asset directory configuration
- Enables flexible image file location

### Texture API Integration

The pass uses texture API for image loading:
- `Texture::createFromFile()` - Load image from file
- `Texture::getSRV()` - Get shader resource view with mip level and array slice
- `Texture::getFormat()` - Get texture format
- `Texture::getWidth()` - Get texture width
- `Texture::getHeight()` - Get texture height
- `Texture::getMipCount()` - Get number of mip levels
- `Texture::getArraySize()` - Get array size

### Render Context Integration

The pass uses render context for GPU operations:
- `RenderContext::blit()` - Copy texture with optional rescaling
- `RenderContext::clearRtv()` - Clear render target to black

### Scripting Integration

The pass supports scripting through:
- Property serialization via `getProperties()` method
- Configurable parameters: outputSize, outputFormat, filename, mips, srgb, arrayIndex, mipLevel
- Can be instantiated and configured from Python scripts

### UI Integration

The pass provides comprehensive UI:
- File dialog for image selection
- Dropdown for output size selection
- Checkboxes for load options
- Sliders for mip level and array slice
- Image preview and information display

## Use Cases

### Reference Image Loading

Load reference images for comparison:
- Error measurement passes
- FLIP perceptual error metric
- Render quality evaluation
- Ground truth comparison

### Environment Map Loading

Load environment maps for rendering:
- Skybox environment maps
- IBL (Image-Based Lighting)
- Reflection probes
- Ambient occlusion

### Texture Atlas Loading

Load texture atlases:
- Sprite sheets
- Font textures
- UI element atlases
- Material textures

### Background Image Loading

Load background images:
- Scene backgrounds
- UI backgrounds
- Menu backgrounds
- Loading screens

### Test Image Loading

Load test images for debugging:
- Render pass testing
- Shader debugging
- Format testing
- Performance testing

### Array Texture Loading

Load texture arrays:
- Cubemap faces
- Texture array slices
- Volume texture slices
- Multi-view textures

### Mipmap Inspection

Inspect different mip levels:
- Mipmap quality evaluation
- LOD (Level of Detail) testing
- Texture streaming verification
- Memory optimization analysis

## Performance Considerations

### GPU Computation

- Texture blit is GPU-accelerated
- Uses hardware-accelerated texture copy
- Supports bilinear filtering for rescaling
- Memory bandwidth limited operation

### Memory Usage

- Source texture: Depends on image format and resolution
- Destination texture: Matches render graph requirements
- Total memory: O(width × height × formatSize) bytes

### Computational Complexity

- Blit operation: O(N) where N = width × height
- Mipmap generation: O(N) if enabled (during load)
- Rescaling: O(N) when output size differs from source
- Overall: O(N) per frame

### File I/O Overhead

- Image loading occurs during initialization or UI interaction
- Synchronous file I/O operation
- May block rendering thread
- Minimal impact on frame time

### Asset Resolution Overhead

- Path resolution occurs during image loading
- Simple string manipulation
- File system check for existence
- Minimal overhead

## Limitations

### Format Limitations

- Supported formats depend on [`Texture::createFromFile()`](Source/Falcor/Core/API/Texture.h:1) implementation
- No custom format conversion
- No format-specific options
- Limited to common image formats

### Mipmap Limitations

- Mipmap generation quality depends on implementation
- No custom mipmap generation parameters
- No mipmap filtering options
- Fixed filtering algorithm

### Array Texture Limitations

- Only single array slice can be selected
- No support for multiple slices
- No array texture iteration
- Manual slice selection required

### Color Space Limitations

- Only sRGB and linear RGB supported
- No other color spaces (HDR, scRGB, etc.)
- No color space conversion options
- sRGB flag affects entire image

### Output Size Limitations

- No custom output size specification
- Only Default, Fixed, and Custom modes
- Custom mode uses render graph default dimensions
- No explicit width/height controls

### File Loading Limitations

- No support for animated images (GIF, APNG)
- No support for volume textures
- No support for compressed texture formats
- Synchronous loading may block

### UI Limitations

- No image editing capabilities
- No format conversion in UI
- No image processing options
- Limited to loading and display

## Best Practices

### Image Format Selection

- Use appropriate format for use case (sRGB for display, linear for computation)
- Consider memory requirements (compressed vs. uncompressed)
- Use formats supported by hardware
- Test format compatibility before deployment
- Document format requirements

### Mipmap Configuration

- Enable mipmap generation for distant views
- Disable mipmap generation for pixel-perfect rendering
- Consider memory vs. quality trade-off
- Test mipmap quality visually
- Use mip level slider for debugging

### Array Slice Selection

- Verify array slice index is valid
- Use slice selection for cubemap faces
- Document slice ordering convention
- Test all slices for correctness
- Consider slice selection for UI

### Output Size Configuration

- Use Default mode for flexibility
- Use Fixed mode for consistency
- Use Custom mode for specific requirements
- Consider rescaling quality impact
- Test output size with downstream passes

### Asset Path Management

- Use relative paths for portability
- Configure asset directories correctly
- Verify path resolution works
- Document asset location requirements
- Test on different systems

### Performance Optimization

- Pre-load images during initialization
- Use mipmaps for distant views
- Avoid unnecessary rescaling
- Minimize format conversions
- Consider texture streaming for large images

### Error Handling

- Check image loading success
- Handle missing files gracefully
- Display error messages to user
- Provide fallback options
- Log warnings for debugging

## Related Components

### Core/API/Texture

Texture management API:
- `Texture::createFromFile()` - Load image from file
- `Texture::getSRV()` - Get shader resource view
- `Texture::getFormat()` - Get texture format
- `Texture::getWidth()` - Get texture width
- `Texture::getHeight()` - Get texture height
- `Texture::getMipCount()` - Get number of mip levels
- `Texture::getArraySize()` - Get array size

### Core/AssetResolver

Asset path resolution utility:
- `AssetResolver::getDefaultResolver().resolvePath(path)` - Resolve relative paths
- Supports asset directory configuration
- Enables flexible image file location

### RenderGraph/RenderPassHelpers

Render pass helper utilities:
- `RenderPassHelpers::calculateIOSize()` - Calculate output size
- `RenderPassHelpers::IOSize` - Output size enumeration
- Supports Default, Fixed, and Custom modes

### Utils/UI/Gui

UI framework for rendering controls:
- Provides dropdowns, checkboxes, sliders, buttons
- Supports file dialogs
- Displays images and text
- Handles user input

## Future Enhancements

### Additional Format Support

- More image format options
- Custom format conversion
- Format-specific parameters
- Compressed texture support
- Volume texture support

### Advanced Mipmap Options

- Custom mipmap generation parameters
- Multiple mipmap filtering algorithms
- Per-channel mipmap generation
- Mipmap quality presets
- Mipmap generation preview

### Array Texture Improvements

- Multiple slice selection
- Array texture iteration
- Slice range selection
- Automatic slice cycling
- Cubemap face selection

### Color Space Enhancements

- More color space options
- Runtime color space conversion
- Per-channel color space selection
- HDR color space support
- Color space visualization

### Output Size Enhancements

- Custom width/height controls
- Aspect ratio preservation
- Multiple output size presets
- Output size constraints
- Resolution scaling factors

### Image Processing

- Basic image editing capabilities
- Format conversion in UI
- Image cropping and resizing
- Color adjustment controls
- Image filtering options

### Performance Improvements

- Async image loading
- Texture streaming support
- Progressive loading
- Background loading
- Caching for frequently used images

### UI Improvements

- Image editing UI
- Format conversion UI
- Mipmap preview
- Array slice visualization
- Drag-and-drop support
