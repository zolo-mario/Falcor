# Image - Image Processing Utilities

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **AsyncTextureLoader** - Asynchronous texture loading
- [x] **Bitmap** - Bitmap image format
- [x] **ImageIO** - Image I/O operations
- [x] **ImageProcessing** - Image processing operations
- [x] **TextureAnalyzer** - Texture analysis
- [x] **TextureManager** - Texture management system

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Buffer, Texture, Formats)
- **Core/Pass** - Compute pass (ComputePass)
- **Core/Platform** - Platform utilities (OS)
- **Core/Program** - Shader program management (ShaderVar)
- **Scene/Material** - Material system (TextureHandle)

## Module Overview

The Image module provides comprehensive image processing and texture management for the Falcor rendering framework. It includes bitmap image format support, DDS file I/O, GPU-based texture analysis, multi-threaded texture management with asynchronous loading, UDIM support, and image processing utilities. These utilities are essential for texture loading, analysis, and management in rendering applications.

## Component Specifications

### Bitmap

**Files**:
- [`Bitmap.h`](Source/Falcor/Utils/Image/Bitmap.h:1) - Bitmap header
- [`Bitmap.cpp`](Source/Falcor/Utils/Image/Bitmap.cpp:1) - Bitmap implementation

**Purpose**: Memory bitmap class for image data.

**Key Features**:
- Multiple file format support (PNG, JPEG, TGA, BMP, PFM, EXR, DDS)
- Import/export flags for flexible control
- Top-down and bottom-up memory layout support
- File dialog integration
- Format conversion utilities

**Core Methods**:

**Creation**:
- [`create(uint32_t width, uint32_t height, ResourceFormat format, const uint8_t* pData)`](Source/Falcor/Utils/Image/Bitmap.h:83) - Create from memory
  - Parameters:
    - `width` - Width in pixels
    - `height` - Height in pixels
    - `format` - Resource format
    - `pData` - Pointer to data (copied internally)

- [`createFromFile(const std::filesystem::path& path, bool isTopDown, ImportFlags importFlags = ImportFlags::None)`](Source/Falcor/Utils/Image/Bitmap.h:93) - Create from file
  - Parameters:
    - `path` - File path (absolute or relative to working directory)
    - `isTopDown` - Memory layout (true = top-left first, false = bottom-left first)
    - `importFlags` - Import flags
  - Returns: New bitmap object or nullptr if loading failed

**Export**:
- [`saveImage(const std::filesystem::path& path, uint32_t width, uint32_t height, FileFormat fileFormat, ExportFlags exportFlags, ResourceFormat resourceFormat, bool isTopDown, void* pData)`](Source/Falcor/Utils/Image/Bitmap.h:107) - Save memory buffer to file
  - Parameters:
    - `path` - File path to write to
    - `width` - Image width
    - `height` - Image height
    - `fileFormat` - Destination file format
    - `exportFlags` - Export flags
    - `resourceFormat` - Resource format of data
    - `isTopDown` - Memory layout (true = top-left first)
    - `pData` - Pointer to image data

- [`saveImageDialog(Texture* pTexture)`](Source/Falcor/Utils/Image/Bitmap.h:122) - Open dialog to save texture to file

**Accessors**:
- [`getData() const`](Source/Falcor/Utils/Image/Bitmap.h:125) - Get pointer to bitmap's data store
- [`getWidth() const`](Source/Falcor/Utils/Image/Bitmap.h:128) - Get bitmap width
- [`getHeight() const`](Source/Falcor/Utils/Image/Bitmap.h:131) - Get bitmap height
- [`getFormat() const`](Source/Falcor/Utils/Image/Bitmap.h:134) - Get data format
- [`getRowPitch() const`](Source/Falcor/Utils/Image/Bitmap.h:137) - Get row pitch in bytes
- [`getSize() const`](Source/Falcor/Utils/Image/Bitmap.h:140) - Get data size in bytes

**Utilities**:
- [`getFileDialogFilters(ResourceFormat format = ResourceFormat::Unknown)`](Source/Falcor/Utils/Image/Bitmap.h:147) - Get file dialog filter vector
  - If format is Unknown, returns all supported formats
  - Otherwise, returns only formats supporting the specified format

- [`getFileExtFromResourceFormat(ResourceFormat format)`](Source/Falcor/Utils/Image/Bitmap.h:152) - Get file extension from resource format

- [`getFormatFromFileExtension(const std::string& ext)`](Source/Falcor/Utils/Image/Bitmap.h:158) - Get file format from extension

**Data Structures**:

**Bitmap Class**:
- [`mpData`](Source/Falcor/Utils/Image/Bitmap.h:165) - Unique pointer to data
- [`mWidth`](Source/Falcor/Utils/Image/Bitmap.h:166) - Width in pixels
- [`mHeight`](Source/Falcor/Utils/Image/Bitmap.h:167) - Height in pixels
- [`mRowPitch`](Source/Falcor/Utils/Image/Bitmap.h:168) - Row pitch in bytes
- [`mSize`](Source/Falcor/Utils/Image/Bitmap.h:169) - Total size in bytes
- [`mFormat`](Source/Falcor/Utils/Image/Bitmap.h:170) - Resource format

**ExportFlags Enum**:
- [`None`](Source/Falcor/Utils/Image/Bitmap.h:47) - Default
- [`ExportAlpha`](Source/Falcor/Utils/Image/Bitmap.h:48) - Save alpha channel
- [`Lossy`](Source/Falcor/Utils/Image/Bitmap.h:49) - Try to store in lossy format
- [`Uncompressed`](Source/Falcor/Utils/Image/Bitmap.h:50) - Prefer faster load to more compact file size
- [`ExrFloat16`](Source/Falcor/Utils/Image/Bitmap.h:51) - Use half-float instead of float for EXR

**ImportFlags Enum**:
- [`None`](Source/Falcor/Utils/Image/Bitmap.h:56) - Default
- [`ConvertToFloat16`](Source/Falcor/Utils/Image/Bitmap.h:57) - Convert HDR images to 16-bit float per channel

**FileFormat Enum**:
- [`PngFile`](Source/Falcor/Utils/Image/Bitmap.h:62) - PNG file (lossless compressed 8-bit with optional alpha)
- [`JpegFile`](Source/Falcor/Utils/Image/Bitmap.h:63) - JPEG file (lossy compressed 8-bit without alpha)
- [`TgaFile`](Source/Falcor/Utils/Image/Bitmap.h:64) - TGA file (lossless uncompressed 8-bit with optional alpha)
- [`BmpFile`](Source/Falcor/Utils/Image/Bitmap.h:65) - BMP file (lossless uncompressed 8-bit with optional alpha)
- [`PfmFile`](Source/Falcor/Utils/Image/Bitmap.h:66) - PFM file (floating point HDR 32-bit float per channel)
- [`ExrFile`](Source/Falcor/Utils/Image/Bitmap.h:67) - EXR file (floating point HDR 16/32-bit float per channel)
- [`DdsFile`](Source/Falcor/Utils/Image/Bitmap.h:68) - DDS file (GPU resource formats including block compression)

**Technical Details**:

**Memory Layout**:
- Top-down: top-left pixel is first in buffer
- Bottom-up: bottom-left pixel is first in buffer
- Row pitch: bytes per row (for compressed formats, this is blocks not pixels)

**File Format Support**:
- PNG: Lossless, 8-bit, optional alpha
- JPEG: Lossy, 8-bit, no alpha
- TGA: Lossless, uncompressed, 8-bit, optional alpha
- BMP: Lossless, uncompressed, 8-bit, optional alpha
- PFM: HDR, 32-bit float per channel
- EXR: HDR, 16/32-bit float per channel
- DDS: GPU formats, block compression supported

**Use Cases**:
- Image file loading
- Image file saving
- Format conversion
- Texture data management
- Image I/O operations

### ImageIO

**Files**:
- [`ImageIO.h`](Source/Falcor/Utils/Image/ImageIO.h:1) - Image I/O header
- [`ImageIO.cpp`](Source/Falcor/Utils/Image/ImageIO.cpp:1) - Image I/O implementation

**Purpose**: DDS file I/O operations with compression support.

**Key Features**:
- DDS file loading to Bitmap
- DDS file loading to Texture
- DDS file saving from Bitmap
- DDS file saving from Texture
- Block compression support (BC1-BC7)
- Mipmap chain support

**Core Methods**:

**Loading**:
- [`loadBitmapFromDDS(const std::filesystem::path& path)`](Source/Falcor/Utils/Image/ImageIO.h:81) - Load DDS to Bitmap
  - Parameters:
    - `path` - File path to load
  - Returns: Bitmap object or nullptr if loading failed
  - Throws exception if DDS file is malformed
  - Only loads first image if file contains array/mips

- [`loadTextureFromDDS(ref<Device> pDevice, const std::filesystem::path& path, bool loadAsSrgb)`](Source/Falcor/Utils/Image/ImageIO.h:91) - Load DDS to Texture
  - Parameters:
    - `pDevice` - GPU device
    - `path` - File path to load
    - `loadAsSrgb` - Convert to sRGB format if available
  - Returns: Texture object or nullptr if loading failed
  - Throws exception if DDS file is malformed

**Saving**:
- [`saveToDDS(const std::filesystem::path& path, const Bitmap& bitmap, CompressionMode mode = CompressionMode::None, bool generateMips = false)`](Source/Falcor/Utils/Image/ImageIO.h:101) - Save Bitmap to DDS
  - Parameters:
    - `path` - File path to save to
    - `bitmap` - Bitmap object to save
    - `mode` - Block compression mode (default: save as-is)
    - `generateMips` - Generate full mipmap chain (requires COM initialization)
  - Throws exception if path invalid or save fails

- [`saveToDDS(CopyContext* pContext, const std::filesystem::path& path, const ref<Texture>& pTexture, CompressionMode mode = CompressionMode::None, bool generateMips = false)`](Source/Falcor/Utils/Image/ImageIO.h:120) - Save Texture to DDS
  - Parameters:
    - `pContext` - Copy context for reading texture data from GPU
    - `path` - File path to save to
    - `pTexture` - Texture object to save
    - `mode` - Block compression mode (default: save as-is)
    - `generateMips` - Generate full mipmap chain (requires COM initialization)
  - Saves all mips and array images
  - Throws exception if path invalid or save fails

**Data Structures**:

**CompressionMode Enum**:
- [`BC1`](Source/Falcor/Utils/Image/ImageIO.h:45) - RGB with 1-bit alpha (8 bytes per block)
- [`BC2`](Source/Falcor/Utils/Image/ImageIO.h:49) - RGBA combining BC1 for RGB with 4 bits of alpha (16 bytes per block)
- [`BC3`](Source/Falcor/Utils/Image/ImageIO.h:53) - RGBA combining BC1 for RGB and BC4 for alpha (16 bytes per block)
- [`BC4`](Source/Falcor/Utils/Image/ImageIO.h:57) - Single grayscale channel (8 bytes per block)
- [`BC5`](Source/Falcor/Utils/Image/ImageIO.h:61) - Two channels using BC4 for each channel (16 bytes per block)
- [`BC6`](Source/Falcor/Utils/Image/ImageIO.h:65) - RGB 16-bit floating point (16 bytes per block)
- [`BC7`](Source/Falcor/Utils/Image/ImageIO.h:69) - 8-bit RGB or RGBA (16 bytes per block)
- [`None`](Source/Falcor/Utils/Image/ImageIO.h:72) - No compression mode specified

**Technical Details**:

**DDS Format**:
- DirectDraw Surface format
- Supports GPU resource formats
- Block compression formats (BC1-BC7)
- Mipmap chains
- Texture arrays

**Block Compression**:
- BC1: 1-bit alpha, 8 bytes/block
- BC2: 4-bit alpha, 16 bytes/block
- BC3: Separate alpha, 16 bytes/block
- BC4: Grayscale, 8 bytes/block
- BC5: Two channels, 16 bytes/block
- BC6: HDR RGB, 16 bytes/block
- BC7: 8-bit RGB/RGBA, 16 bytes/block

**Use Cases**:
- DDS file loading
- DDS file saving
- Block compression
- Mipmap generation
- Texture format conversion

### TextureAnalyzer

**Files**:
- [`TextureAnalyzer.h`](Source/Falcor/Utils/Image/TextureAnalyzer.h:1) - Texture analyzer header
- [`TextureAnalyzer.cpp`](Source/Falcor/Utils/Image/TextureAnalyzer.cpp:1) - Texture analyzer implementation
- [`TextureAnalyzer.cs.slang`](Source/Falcor/Utils/Image/TextureAnalyzer.cs.slang:1) - Texture analyzer shader

**Purpose**: GPU-based texture analysis for constant color detection and range analysis.

**Key Features**:
- Constant color detection
- Min/max value computation
- Channel variation analysis
- Range flags (positive, negative, inf, NaN)
- Batch analysis support
- GPU-based computation

**Core Methods**:

**Constructor**:
- [`TextureAnalyzer(ref<Device> pDevice)`](Source/Falcor/Utils/Image/TextureAnalyzer.h:97) - Constructor
  - Parameters:
    - `pDevice` - GPU device
  - Throws exception if creation failed

**Analysis**:
- [`analyze(RenderContext* pRenderContext, const ref<Texture> pInput, uint32_t mipLevel, uint32_t arraySlice, ref<Buffer> pResult, uint64_t resultOffset = 0, bool clearResult = true)`](Source/Falcor/Utils/Image/TextureAnalyzer.h:112) - Analyze single texture
  - Parameters:
    - `pRenderContext` - Render context
    - `pInput` - Input texture (2D non-MSAA floating-point)
    - `mipLevel` - Mip level
    - `arraySlice` - Array slice
    - `pResult` - Result buffer (UAV, cleared to zero)
    - `resultOffset` - Offset into result buffer
    - `clearResult` - Clear result buffer first
  - Throws exception if format/dimension unsupported
  - Result written as 64B Result struct

- [`analyze(RenderContext* pRenderContext, const std::vector<ref<Texture>>& inputs, ref<Buffer> pResult, bool clearResult = true)`](Source/Falcor/Utils/Image/TextureAnalyzer.h:132) - Batch analyze textures
  - Parameters:
    - `pRenderContext` - Render context
    - `inputs` - Array of input textures (2D non-MSAA floating-point)
    - `pResult` - Result buffer (UAV)
    - `clearResult` - Clear result buffer first
  - More efficient than repeated analyze() calls
  - Result written as array of 64B Result structs

**Utilities**:
- [`clear(RenderContext* pRenderContext, ref<Buffer> pResult, uint64_t resultOffset, size_t resultCount) const`](Source/Falcor/Utils/Image/TextureAnalyzer.h:141) - Clear results buffer
  - Parameters:
    - `pRenderContext` - Render context
    - `pResult` - Result buffer (UAV)
    - `resultOffset` - Offset into result buffer
    - `resultCount` - Number of result structs

- [`getResultSize()`](Source/Falcor/Utils/Image/TextureAnalyzer.h:146) - Get result size for one texture in bytes

**Data Structures**:

**Result Struct**:
- [`mask`](Source/Falcor/Utils/Image/TextureAnalyzer.h:51) - Bits 0-3: varying channels (0=constant, 1=varying), Bits 4-19: numerical range (4 bits/channel), Bits 20-31: reserved
- [`value`](Source/Falcor/Utils/Image/TextureAnalyzer.h:55) - Constant color value (RGBA fp32, valid for constant channels)
- [`minValue`](Source/Falcor/Utils/Image/TextureAnalyzer.h:56) - Minimum color value (RGBA fp32, clamped to zero)
- [`maxValue`](Source/Falcor/Utils/Image/TextureAnalyzer.h:57) - Maximum color value (RGBA fp32, clamped to zero)

**RangeFlags Enum**:
- [`Pos`](Source/Falcor/Utils/Image/TextureAnalyzer.h:61) - Texture channel has positive values > 0
- [`Neg`](Source/Falcor/Utils/Image/TextureAnalyzer.h:62) - Texture channel has negative values < 0
- [`Inf`](Source/Falcor/Utils/Image/TextureAnalyzer.h:63) - Texture channel has +/-inf values
- [`NaN`](Source/Falcor/Utils/Image/TextureAnalyzer.h:64) - Texture channel has NaN values

**Result Methods**:
- [`isConstant(uint32_t channelMask) const`](Source/Falcor/Utils/Image/TextureAnalyzer.h:67) - Check if channels are constant
- [`isPos(TextureChannelFlags channelMask) const`](Source/Falcor/Utils/Image/TextureAnalyzer.h:70) - Check if channels have positive values
- [`isNeg(TextureChannelFlags channelMask) const`](Source/Falcor/Utils/Image/TextureAnalyzer.h:71) - Check if channels have negative values
- [`isInf(TextureChannelFlags channelMask) const`](Source/Falcor/Utils/Image/TextureAnalyzer.h:72) - Check if channels have inf values
- [`isNaN(TextureChannelFlags channelMask) const`](Source/Falcor/Utils/Image/TextureAnalyzer.h:73) - Check if channels have NaN values
- [`getRange(TextureChannelFlags channelMask) const`](Source/Falcor/Utils/Image/TextureAnalyzer.h:80) - Get numerical range of channels

**Technical Details**:

**Analysis Requirements**:
- 2D textures only
- Non-MSAA textures only
- Floating-point formats only
- GPU-based computation

**Result Format**:
- 64 bytes per texture
- Bit-packed flags for efficiency
- Float4 values for min/max/constant
- Per-channel analysis

**Range Encoding**:
- 4 bits per channel in mask (bits 4-19)
- Channel 0: bits 4-7
- Channel 1: bits 8-11
- Channel 2: bits 12-15
- Channel 3: bits 16-19

**Use Cases**:
- Constant texture detection
- Texture optimization
- Range analysis
- Quality control
- Texture validation

### TextureManager

**Files**:
- [`TextureManager.h`](Source/Falcor/Utils/Image/TextureManager.h:1) - Texture manager header
- [`TextureManager.cpp`](Source/Falcor/Utils/Image/TextureManager.cpp:1) - Texture manager implementation

**Purpose**: Multi-threaded texture management with asynchronous loading and UDIM support.

**Key Features**:
- Multi-threaded texture loading
- Asynchronous texture loading
- UDIM (UDIM) texture support
- Texture handle system
- Shader descriptor array binding
- Thread-safe operations
- Deferred loading support
- Owner-based texture removal
- Statistics tracking

**Core Methods**:

**Constructor**:
- [`TextureManager(ref<Device> pDevice, size_t maxTextureCount, size_t threadCount = std::thread::hardware_concurrency())`](Source/Falcor/Utils/Image/TextureManager.h:147) - Constructor
  - Parameters:
    - `pDevice` - GPU device
    - `maxTextureCount` - Maximum number of textures to manage
    - `threadCount` - Number of worker threads (default: hardware concurrency)

**Texture Management**:
- [`addTexture(const ref<Texture>& pTexture)`](Source/Falcor/Utils/Image/TextureManager.h:157) - Add texture to manager
  - Parameters:
    - `pTexture` - Texture resource
  - Returns: Unique handle to texture
  - Returns existing handle if texture already managed

- [`removeTexture(const CpuTextureHandle& handle)`](Source/Falcor/Utils/Image/TextureManager.h:212) - Remove texture
  - Parameters:
    - `handle` - Texture handle

- [`removeTextures(const Object* object)`](Source/Falcor/Utils/Image/TextureManager.h:218) - Remove all textures loaded by object
  - Parameters:
    - `object` - Owner object

**Texture Loading**:
- [`loadTexture(const std::filesystem::path& path, bool generateMipLevels, bool loadAsSRGB, ResourceBindFlags bindFlags = ResourceBindFlags::ShaderResource, bool async = true, Bitmap::ImportFlags importFlags = Bitmap::ImportFlags::None, const AssetResolver* assetResolver = nullptr, size_t* loadedTextureCount = nullptr, const Object* owner = nullptr)`](Source/Falcor/Utils/Image/TextureManager.h:174) - Request texture loading
  - Parameters:
    - `path` - File path (full or relative to data directory)
    - `generateMipLevels` - Generate full mip-chain
    - `loadAsSRGB` - Load as sRGB format
    - `bindFlags` - Bind flags for texture resource
    - `async` - Load asynchronously (default: true)
    - `importFlags` - File import flags
    - `assetResolver` - Optional asset resolver
    - `loadedTextureCount` - Output: number of loaded textures (2+ for UDIMs)
    - `owner` - Owner object
  - Returns: Unique handle to texture, or invalid handle if not found
  - Adds texture to managed set
  - Returns handle immediately, data available later if async

- [`waitForTextureLoading(const CpuTextureHandle& handle)`](Source/Falcor/Utils/Image/TextureManager.h:191) - Wait for texture to load
  - Parameters:
    - `handle` - Texture handle
  - Blocks until texture is loaded or failed

- [`waitForAllTexturesLoading()`](Source/Falcor/Utils/Image/TextureManager.h:196) - Wait for all textures to load

**Deferred Loading**:
- [`beginDeferredLoading()`](Source/Falcor/Utils/Image/TextureManager.h:205) - Begin deferred loading section
  - Puts subsequent loadTexture() calls on deferred list
  - WARNING: Dangerous operation, only use from main thread

- [`endDeferredLoading()`](Source/Falcor/Utils/Image/TextureManager.h:206) - End deferred loading section
  - Loads all queued textures in parallel

**Texture Access**:
- [`getTexture(const CpuTextureHandle& handle) const`](Source/Falcor/Utils/Image/TextureManager.h:226) - Get loaded texture
  - Parameters:
    - `handle` - Texture handle
  - Returns: Texture if loaded, nullptr if invalid or not yet loaded

- [`getTexture(const CpuTextureHandle& handle, const float2& uv) const`](Source/Falcor/Utils/Image/TextureManager.h:227) - Get texture with UV coordinate (UDIM support)

- [`getTexture(const CpuTextureHandle& handle, const uint32_t udimID) const`](Source/Falcor/Utils/Image/TextureManager.h:228) - Get texture with UDIM ID

- [`getTextureDesc(const CpuTextureHandle& handle) const`](Source/Falcor/Utils/Image/TextureManager.h:236) - Get texture descriptor

- [`getUdimIDs(const CpuTextureHandle& handle) const`](Source/Falcor/Utils/Image/TextureManager.h:250) - Get list of UDIM IDs

- [`getTextureDescCount() const`](Source/Falcor/Utils/Image/TextureManager.h:256) - Get number of texture descriptors

**Shader Binding**:
- [`bindShaderData(const ShaderVar& texturesVar, const size_t descCount, const ShaderVar& udimsVar) const`](Source/Falcor/Utils/Image/TextureManager.h:275) - Bind all textures to shader var
  - Parameters:
    - `texturesVar` - Shader var for descriptor array
    - `descCount` - Size of descriptor array
    - `udimsVar` - Shader var for UDIM indirection
  - Binds all managed textures to shader descriptor array
  - Array must be large enough (throws exception otherwise)

**Statistics**:
- [`getStats() const`](Source/Falcor/Utils/Image/TextureManager.h:280) - Get texture statistics

**Data Structures**:

**TextureState Enum**:
- [`Invalid`](Source/Falcor/Utils/Image/TextureManager.h:64) - Invalid/unknown texture
- [`Referenced`](Source/Falcor/Utils/Image/TextureManager.h:65) - Referenced but not yet loaded
- [`Loaded`](Source/Falcor/Utils/Image/TextureManager.h:66) - Finished loading

**Stats Struct**:
- [`textureCount`](Source/Falcor/Utils/Image/TextureManager.h:71) - Number of unique textures
- [`textureCompressedCount`](Source/Falcor/Utils/Image/TextureManager.h:72) - Number of compressed textures
- [`textureTexelCount`](Source/Falcor/Utils/Image/TextureManager.h:73) - Total number of texels
- [`textureTexelChannelCount`](Source/Falcor/Utils/Image/TextureManager.h:74) - Total number of texel channels
- [`textureMemoryInBytes`](Source/Falcor/Utils/Image/TextureManager.h:75) - Total memory in bytes

**CpuTextureHandle Class**:
- [`mID`](Source/Falcor/Utils/Image/TextureManager.h:128) - Texture ID
- [`mIsUdim`](Source/Falcor/Utils/Image/TextureManager.h:129) - UDIM enabled flag
- [`kInvalidID`](Source/Falcor/Utils/Image/TextureManager.h:84) - Invalid ID constant
- [`isValid() const`](Source/Falcor/Utils/Image/TextureManager.h:101) - Check if handle is valid
- [`getID() const`](Source/Falcor/Utils/Image/TextureManager.h:104) - Get texture ID
- [`isUdim() const`](Source/Falcor/Utils/Image/TextureManager.h:105) - Check if UDIM enabled
- [`toGpuHandle() const`](Source/Falcor/Utils/Image/TextureManager.h:110) - Convert to GPU handle

**TextureDesc Struct**:
- [`state`](Source/Falcor/Utils/Image/TextureManager.h:135) - Current texture state
- [`pTexture`](Source/Falcor/Utils/Image/TextureManager.h:136) - Valid texture object when Loaded, or nullptr
- [`isValid() const`](Source/Falcor/Utils/Image/TextureManager.h:138) - Check if descriptor is valid

**TextureKey Struct**:
- [`fullPaths`](Source/Falcor/Utils/Image/TextureManager.h:311) - Vector of file paths
- [`generateMipLevels`](Source/Falcor/Utils/Image/TextureManager.h:312) - Generate mip levels flag
- [`loadAsSRGB`](Source/Falcor/Utils/Image/TextureManager.h:313) - Load as sRGB flag
- [`bindFlags`](Source/Falcor/Utils/Image/TextureManager.h:314) - Bind flags
- [`importFlags`](Source/Falcor/Utils/Image/TextureManager.h:315) - Import flags

**Technical Details**:

**Handle System**:
- Unique handle per texture
- Handles are valid for entire lifetime
- Invalid handle: std::numeric_limits<uint32_t>::max()
- Supports UDIM textures

**UDIM Support**:
- UDIM (UDIM) texture indirection
- UV coordinate-based UDIM resolution
- Sparse UDIM indirection table
- Automatic UDIM detection in loadTexture()

**Thread Safety**:
- Mutex-protected shared resources
- Condition variable for loading synchronization
- Multi-threaded loading
- Async texture loading

**Deferred Loading**:
- Queue load requests during begin/end
- Parallel loading of queued textures
- Main thread only (not thread-safe)

**Shader Integration**:
- Descriptor array binding
- Fixed-size descriptor arrays
- Texture handles map to shader indices
- UDIM indirection buffer

**Statistics**:
- Texture count tracking
- Compressed texture count
- Texel count (total and per channel)
- Memory usage in bytes

**Use Cases**:
- Texture loading and management
- Asynchronous texture loading
- UDIM texture support
- Shader texture binding
- Texture statistics
- Memory management

## Architecture Patterns

### Manager Pattern
- Resource lifecycle management
- Handle-based access
- Thread-safe operations
- Statistics tracking

### Async Loading Pattern
- Multi-threaded loading
- Handle-based deferred access
- Condition variable synchronization
- Progress tracking

### UDIM Pattern
- UV coordinate-based indirection
- Sparse indirection table
- Automatic detection
- Shader integration

## Technical Details

### Bitmap Implementation

**File Format Support**:
- Multiple formats via external libraries
- Format-specific handling
- Import/export flags for control
- Memory layout flexibility

**Data Management**:
- Unique pointer for data ownership
- Row pitch for efficient access
- Format information
- Size tracking

### ImageIO Implementation

**DDS Format**:
- GPU-oriented format
- Block compression support
- Mipmap chains
- Texture arrays

**Compression Modes**:
- BC1-BC7 block compression
- Variable block sizes (8-16 bytes)
- Format-specific compression
- Optional decompression

### TextureAnalyzer Implementation

**GPU-Based Analysis**:
- Compute shader for analysis
- Efficient parallel processing
- Batch analysis support
- Result buffer output

**Analysis Results**:
- Bit-packed flags for efficiency
- Per-channel analysis
- Range detection (positive, negative, inf, NaN)
- Constant color detection

### TextureManager Implementation

**Handle System**:
- Unique IDs for textures
- Handle-based API
- UDIM support in handles
- GPU handle conversion

**Thread Safety**:
- Mutex for critical sections
- Condition variable for waiting
- Atomic operations where possible
- Multi-threaded loading

**UDIM System**:
- UV coordinate to UDIM ID mapping
- Sparse indirection table
- Efficient lookup
- Shader-side indirection

**Memory Management**:
- Fixed-size texture pool
- Free list for reuse
- UDIM range allocation
- Statistics tracking

## Progress Log

- **2026-01-07T19:12:20Z**: Image sub-module analysis completed. Analyzed Bitmap, ImageIO, TextureAnalyzer, and TextureManager classes. Documented bitmap image format support with multiple file formats, DDS file I/O with compression support, GPU-based texture analysis for constant color detection, and multi-threaded texture management with asynchronous loading and UDIM support. Created comprehensive technical specification with detailed code patterns, data structures, and use cases.

## Next Steps

Proceed to analyze Math sub-module to understand mathematical utilities (AABB, Vector, Matrix, Quaternion, Ray, Rectangle, etc.).
