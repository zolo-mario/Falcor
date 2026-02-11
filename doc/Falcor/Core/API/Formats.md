# Formats

## Overview
[`Formats`](Source/Falcor/Core/API/Formats.h) provides comprehensive GPU resource format definitions and utilities for format conversion, querying, and cross-platform mapping (D3D12 and Vulkan).

## Source Files
- Header: [`Source/Falcor/Core/API/Formats.h`](Source/Falcor/Core/API/Formats.h) (516 lines)
- Implementation: [`Source/Falcor/Core/API/Formats.cpp`](Source/Falcor/Core/API/Formats.cpp) (269 lines)

## Type Definitions

### Enums

#### TextureChannelFlags
```cpp
enum class TextureChannelFlags : uint32_t
{
    None = 0x0,
    Red = 0x1,
    Green = 0x2,
    Blue = 0x4,
    Alpha = 0x8,
    RGB = 0x7,
    RGBA = 0xf,
};
```
**Memory Layout**: 4 bytes (uint32_t underlying type)

**Cache Locality**: **Excellent** - Fits in a single cache line

**Usage**: Flags for enumerating texture color channels

#### ResourceBindFlags
```cpp
enum class ResourceBindFlags : uint32_t
{
    None = 0x0,
    Vertex = 0x1,
    Index = 0x2,
    Constant = 0x4,
    StreamOutput = 0x8,
    ShaderResource = 0x10,
    UnorderedAccess = 0x20,
    RenderTarget = 0x40,
    DepthStencil = 0x80,
    IndirectArg = 0x100,
    Shared = 0x200,
    AccelerationStructure = 0x80000000,
    AllColorViews = ShaderResource | UnorderedAccess | RenderTarget,
    AllDepthViews = ShaderResource | DepthStencil
};
```
**Memory Layout**: 4 bytes (uint32_t underlying type)

**Cache Locality**: **Excellent** - Fits in a single cache line

**Usage**: Flags indicating which pipeline stages a resource will be bound to

#### ResourceFormat
```cpp
enum class ResourceFormat : uint32_t
{
    Unknown,
    // Unorm formats (13)
    R8Unorm, R8Snorm, R16Unorm, R16Snorm, RG8Unorm, RG8Snorm, RG16Unorm, RG16Snorm,
    RGB5A1Unorm, RGBA8Unorm, RGBA8Snorm, RGB10A2Unorm, RGBA16Unorm, RGBA16Snorm,
    // UnormSrgb formats (6)
    RGBA8UnormSrgb, BGRA8UnormSrgb, BGRX8UnormSrgb, BC1UnormSrgb, BC2UnormSrgb,
    BC3UnormSrgb, BC7UnormSrgb,
    // Float formats (10)
    R16Float, RG16Float, RGBA16Float, R32Float, RG32Float, RGB32Float, RGBA32Float,
    R11G11B10Float, RGB9E5Float, BC6HS16, BC6HU16,
    // Uint formats (12)
    R8Uint, R16Uint, R32Uint, RG8Uint, RG16Uint, RG32Uint, RGB32Uint, RGBA8Uint,
    RGBA16Uint, RGBA32Uint, RGB10A2Uint, BGRA4Unorm,
    // Sint formats (11)
    R8Int, R16Int, R32Int, RG8Int, RG16Int, RG32Int, RGB32Int, RGBA8Int,
    RGBA16Int, RGBA32Int, RGBA8Snorm, RG16Snorm, BC4Snorm, BC5Snorm,
    // BGRA formats (4)
    BGRA4Unorm, BGRA8Unorm, BGRA8UnormSrgb, BGRX8Unorm, BGRX8UnormSrgb,
    // Depth-stencil formats (3)
    D32Float, D32FloatS8Uint, D16Unorm,
    // Compressed formats (14)
    BC1Unorm, BC2Unorm, BC3Unorm, BC4Unorm, BC4Snorm, BC5Unorm, BC5Snorm,
    BC6HS16, BC6HU16, BC7Unorm, BC7UnormSrgb,
    Count
};
```
**Memory Layout**: 4 bytes (uint32_t underlying type)

**Cache Locality**: **Excellent** - Fits in a single cache line

**Usage**: GPU resource format enumeration (64 total formats)

#### FormatType
```cpp
enum class FormatType
{
    Unknown,
    Float,
    Unorm,
    UnormSrgb,
    Snorm,
    Uint,
    Sint
};
```
**Memory Layout**: 4 bytes (enum, likely 4 bytes)

**Cache Locality**: **Excellent** - Fits in a single cache line

**Usage**: Format type classification

### Structs

#### FormatDesc
```cpp
struct FormatDesc
{
    ResourceFormat format;
    const std::string name;
    uint32_t bytesPerBlock;
    uint32_t channelCount;
    FormatType Type;
    struct
    {
        bool isDepth;
        bool isStencil;
        bool isCompressed;
    };
    struct
    {
        uint32_t width;
        uint32_t height;
    } compressionRatio;
    int numChannelBits[4];
};
```
**Memory Layout**: ~40-48 bytes (excluding string allocation)

**Member Layout**:
- `format`: 4 bytes
- `name`: ~24-32 bytes (std::string, small string optimization)
- `bytesPerBlock`: 4 bytes
- `channelCount`: 4 bytes
- `Type`: 4 bytes
- `{isDepth, isStencil, isCompressed}`: 3 bytes + 1 byte padding
- `compressionRatio`: 8 bytes (2 × uint32_t)
- `numChannelBits[4]`: 16 bytes (4 × int)
- Total: ~47-55 bytes (excluding string allocation)

**Cache Locality**: **Good** - Fits in a single cache line (64 bytes)

**Usage**: Format description structure

#### NativeFormatDesc
```cpp
struct NativeFormatDesc
{
    ResourceFormat falcorFormat;
    DXGI_FORMAT dxgiFormat;
    VkFormat vkFormat;
};
```
**Memory Layout**: ~12-16 bytes (3 × 4-byte enums)

**Cache Locativity**: **Excellent** - Fits in a single cache line

**Usage**: Cross-platform format mapping (D3D12 and Vulkan)

## Global Data Structures

### kFormatDesc Array
**Size**: 64 entries (ResourceFormat::Count)

**Memory Layout**: ~2560-3072 bytes (64 × 40-48 bytes per entry)

**Cache Locality**: **Poor for random access, Good for sequential access**
- Large array may span multiple cache lines
- Sequential access patterns benefit from spatial locality
- Random access may cause cache misses

**Content**: Format descriptions for all 64 ResourceFormat values

### kNativeFormatDescs Array
**Size**: 64 entries (ResourceFormat::Count)

**Memory Layout**: ~768-1024 bytes (64 × 12-16 bytes per entry)

**Cache Locality**: **Poor for random access, Good for sequential access**
- Large array may span multiple cache lines
- Sequential access patterns benefit from spatial locality
- Random access may cause cache misses

**Content**: Cross-platform format mappings (DXGI and Vulkan)

## Threading Model

**Thread Safety**: **Thread-Safe (Read-Only Data)**

**Analysis**:
- All global data structures are const (kFormatDesc, kNativeFormatDescs)
- No mutable state in any functions
- All functions are pure (no side effects)
- Safe for concurrent read access from multiple threads
- No synchronization primitives needed

**Thread Safety Characteristics**:
1. **Read-Only Data**: All global arrays are const
2. **Pure Functions**: All inline functions have no side effects
3. **No Mutable State**: No global or static mutable variables
4. **No External Dependencies**: Functions only read from const arrays
5. **Safe for Concurrent Access**: Multiple threads can safely call any function simultaneously

**Note**: The format conversion functions (getResourceFormat) perform linear search through arrays, but this is safe for concurrent access as they only read const data.

## Cache Locality Analysis

### Access Patterns

#### Format Query Functions (O(1))
- [`getFormatBytesPerBlock()`](Source/Falcor/Core/API/Formats.h:207): Direct array lookup
- [`getFormatPixelsPerBlock()`](Source/Falcor/Core/API/Formats.h:213): Direct array lookup
- [`isDepthFormat()`](Source/Falcor/Core/API/Formats.h:222): Direct array lookup
- [`isStencilFormat()`](Source/Falcor/Core/API/Formats.h:231): Direct array lookup
- [`isDepthStencilFormat()`](Source/Falcor/Core/API/Formats.h:240): Direct array lookup
- [`isCompressedFormat()`](Source/Falcor/Core/API/Formats.h:248): Direct array lookup
- [`getFormatWidthCompressionRatio()`](Source/Falcor/Core/API/Formats.h:257): Direct array lookup
- [`getFormatHeightCompressionRatio()`](Source/Falcor/Core/API/Formats.h:266): Direct array lookup
- [`getFormatChannelCount()`](Source/Falcor/Core/API/Formats.h:275): Direct array lookup
- [`getFormatType()`](Source/Falcor/Core/API/Formats.h:284): Direct array lookup
- [`isIntegerFormat()`](Source/Falcor/Core/API/Formats.h:293): Direct array lookup
- [`getNumChannelBits()`](Source/Falcor/Core/API/Formats.h:302): Direct array lookup
- [`getChannelMask()`](Source/Falcor/Core/API/Formats.h:327): Direct array lookup
- [`getFormatRowPitch()`](Source/Falcor/Core/API/Formats.h:344): Direct array lookup
- [`isSrgbFormat()`](Source/Falcor/Core/API/Formats.h:353): Direct array lookup
- [`doesFormatHaveAlpha()`](Source/Falcor/Core/API/Formats.h:425): Direct array lookup

**Cache Locality**: **Good to Excellent**
- Direct array access (O(1))
- Single cache line access for most queries
- Excellent for repeated queries to the same format

#### Format Conversion Functions (O(N))
- [`srgbToLinearFormat()`](Source/Falcor/Core/API/Formats.h:361): Switch statement (O(1))
- [`linearToSrgbFormat()`](Source/Falcor/Core/API/Formats.h:388): Switch statement (O(1))
- [`depthToColorFormat()`](Source/Falcor/Core/API/Formats.h:411): Switch statement (O(1))
- [`getDxgiFormat()`](Source/Falcor/Core/API/Formats.cpp:235): Direct array lookup (O(1))
- [`getVulkanFormat()`](Source/Falcor/Core/API/Formats.cpp:252): Direct array lookup (O(1))
- [`getResourceFormat(DXGI_FORMAT)`](Source/Falcor/Core/API/Formats.cpp:241): Linear search (O(N))
- [`getResourceFormat(VkFormat)`](Source/Falcor/Core/API/Formats.cpp:258): Linear search (O(N))

**Cache Locality**: **Poor for reverse lookups**
- Linear search through 64 entries
- May cause multiple cache misses
- Poor performance for frequent reverse lookups

#### String Conversion Functions
- [`to_string(ResourceFormat)`](Source/Falcor/Core/API/Formats.h:442): Direct array lookup (O(1))
- [`to_string(FormatType)`](Source/Falcor/Core/API/Formats.h:448): Switch statement (O(1))
- [`to_string(ResourceBindFlags)`](Source/Falcor/Core/API/Formats.h:469): String concatenation (O(1))

**Cache Locality**: **Good**
- Direct array access for ResourceFormat
- Switch statement for FormatType
- String operations for ResourceBindFlags

### Cache Line Analysis

#### kFormatDesc Array
- **Total Size**: ~2560-3072 bytes
- **Cache Lines**: ~40-48 cache lines (64 bytes each)
- **Access Pattern**: Direct array index lookup
- **Cache Locality**: **Good for sequential access, Poor for random access**

#### kNativeFormatDescs Array
- **Total Size**: ~768-1024 bytes
- **Cache Lines**: ~12-16 cache lines (64 bytes each)
- **Access Pattern**: Direct array index lookup or linear search
- **Cache Locality**: **Good for sequential access, Poor for random access**

### Optimization Opportunities

1. **Reverse Lookup Optimization**: Consider using hash maps for reverse lookups (getResourceFormat) to improve cache locality and performance
2. **Format Cache**: Consider caching frequently queried formats to reduce array lookups
3. **Format Grouping**: Consider grouping related formats together to improve spatial locality
4. **Bit Packing**: Consider packing boolean flags into bit fields to reduce memory footprint

## Algorithmic Complexity Analysis

### Format Query Functions

#### getFormatBytesPerBlock
**Time Complexity**: **O(1)**
- Array lookup: O(1)

**Space Complexity**: **O(1)**

#### getFormatPixelsPerBlock
**Time Complexity**: **O(1)**
- Array lookup: O(1)
- Multiplication: O(1)

**Space Complexity**: **O(1)**

#### isDepthFormat
**Time Complexity**: **O(1)**
- Array lookup: O(1)

**Space Complexity**: **O(1)**

#### isStencilFormat
**Time Complexity**: **O(1)**
- Array lookup: O(1)

**Space Complexity**: **O(1)**

#### isDepthStencilFormat
**Time Complexity**: **O(1)**
- Two array lookups: O(1)
- Logical OR: O(1)

**Space Complexity**: **O(1)**

#### isCompressedFormat
**Time Complexity**: **O(1)**
- Array lookup: O(1)

**Space Complexity**: **O(1)**

#### getFormatWidthCompressionRatio
**Time Complexity**: **O(1)**
- Array lookup: O(1)

**Space Complexity**: **O(1)**

#### getFormatHeightCompressionRatio
**Time Complexity**: **O(1)**
- Array lookup: O(1)

**Space Complexity**: **O(1)**

#### getFormatChannelCount
**Time Complexity**: **O(1)**
- Array lookup: O(1)

**Space Complexity**: **O(1)**

#### getFormatType
**Time Complexity**: **O(1)**
- Array lookup: O(1)

**Space Complexity**: **O(1)**

#### isIntegerFormat
**Time Complexity**: **O(1)**
- Array lookup: O(1)
- Comparison: O(1)

**Space Complexity**: **O(1)**

#### getNumChannelBits (single channel)
**Time Complexity**: **O(1)**
- Array lookup: O(1)

**Space Complexity**: **O(1)**

#### getNumChannelBits (mask)
**Time Complexity**: **O(1)**
- Up to 4 array lookups: O(1)
- Bit operations: O(1)

**Space Complexity**: **O(1)**

#### getChannelMask
**Time Complexity**: **O(1)**
- Array lookup: O(1)
- Up to 4 bit operations: O(1)

**Space Complexity**: **O(1)**

#### getFormatRowPitch
**Time Complexity**: **O(1)**
- Two array lookups: O(1)
- Division and multiplication: O(1)

**Space Complexity**: **O(1)**

#### isSrgbFormat
**Time Complexity**: **O(1)**
- Array lookup: O(1)
- Comparison: O(1)

**Space Complexity**: **O(1)**

#### doesFormatHaveAlpha
**Time Complexity**: **O(1)**
- Array lookup: O(1)
- Switch statement: O(1)

**Space Complexity**: **O(1)**

### Format Conversion Functions

#### srgbToLinearFormat
**Time Complexity**: **O(1)**
- Switch statement: O(1)

**Space Complexity**: **O(1)**

#### linearToSrgbFormat
**Time Complexity**: **O(1)**
- Switch statement: O(1)

**Space Complexity**: **O(1)**

#### depthToColorFormat
**Time Complexity**: **O(1)**
- Switch statement: O(1)

**Space Complexity**: **O(1)**

#### getDxgiFormat
**Time Complexity**: **O(1)**
- Array lookup: O(1)

**Space Complexity**: **O(1)**

#### getVulkanFormat
**Time Complexity**: **O(1)**
- Array lookup: O(1)

**Space Complexity**: **O(1)**

#### getResourceFormat (DXGI_FORMAT)
**Time Complexity**: **O(N)** where N = 64
- Linear search through array: O(N)
- Average case: O(N/2) = O(32)
- Worst case: O(N) = O(64)

**Space Complexity**: **O(1)**

**Performance Note**: This function performs a linear search through 64 entries, which is inefficient for frequent calls. Consider using a hash map or binary search for better performance.

#### getResourceFormat (VkFormat)
**Time Complexity**: **O(N)** where N = 64
- Linear search through array: O(N)
- Average case: O(N/2) = O(32)
- Worst case: O(N) = O(64)

**Space Complexity**: **O(1)**

**Performance Note**: This function performs a linear search through 64 entries, which is inefficient for frequent calls. Consider using a hash map or binary search for better performance.

### String Conversion Functions

#### to_string (ResourceFormat)
**Time Complexity**: **O(1)**
- Array lookup: O(1)

**Space Complexity**: **O(1)**

#### to_string (FormatType)
**Time Complexity**: **O(1)**
- Switch statement: O(1)

**Space Complexity**: **O(1)**

#### to_string (ResourceBindFlags)
**Time Complexity**: **O(1)**
- String concatenation: O(1) (max 10 flags)
- Bit operations: O(1)

**Space Complexity**: **O(1)**

## Key Technical Features

### Comprehensive Format Support
- **64 Resource Formats**: Including color, depth-stencil, and compressed formats
- **6 Format Types**: Unknown, Float, Unorm, UnormSrgb, Snorm, Uint, Sint
- **14 Compressed Formats**: BC1-BC7 (DXT1-DXT5, RGTC, BPTC)
- **3 Depth-Stencil Formats**: D16Unorm, D32Float, D32FloatS8Uint

### Cross-Platform Support
- **DXGI Format Mapping**: Direct mapping to D3D12 DXGI_FORMAT
- **Vulkan Format Mapping**: Direct mapping to Vulkan VkFormat
- **Bidirectional Conversion**: Convert between Falcor and native formats

### Format Query Functions
- **Bytes Per Block**: Query format memory size
- **Pixels Per Block**: Query compression ratio
- **Channel Count**: Query number of color channels
- **Channel Bits**: Query bits per channel
- **Channel Mask**: Query enabled color channels
- **Format Type**: Query format type (Float, Unorm, etc.)
- **Depth/Stencil**: Query depth-stencil formats
- **Compressed**: Query compressed formats
- **sRGB**: Query sRGB formats
- **Alpha**: Query alpha channel presence

### Format Conversion Functions
- **sRGB to Linear**: Convert sRGB formats to linear
- **Linear to sRGB**: Convert linear formats to sRGB
- **Depth to Color**: Convert depth formats to color formats

### Resource Bind Flags
- **12 Bind Flags**: Vertex, Index, Constant, StreamOutput, ShaderResource, UnorderedAccess, RenderTarget, DepthStencil, IndirectArg, Shared, AccelerationStructure
- **Combined Flags**: AllColorViews, AllDepthViews

### Texture Channel Flags
- **6 Channel Flags**: None, Red, Green, Blue, Alpha, RGB, RGBA

### Python Bindings
- **ResourceBindFlags Enum**: Python enum for resource bind flags
- **ResourceFormat Enum**: Python enum for resource formats
- **TextureChannelFlags Enum**: Python enum for texture channel flags
- **Binary Operators**: Support for bitwise operations on flags

## Performance Characteristics

### Format Query Performance
- **Direct Array Lookup**: O(1) with excellent cache locality
- **Single Cache Line Access**: Most queries access only one cache line
- **No Memory Allocations**: All functions are allocation-free
- **Thread-Safe**: Safe for concurrent access from multiple threads

### Format Conversion Performance
- **Switch Statement**: O(1) for forward conversions (sRGB to Linear, Linear to sRGB, Depth to Color)
- **Direct Array Lookup**: O(1) for native format conversions (Falcor to DXGI/Vulkan)
- **Linear Search**: O(N) for reverse conversions (DXGI/Vulkan to Falcor) - inefficient

### String Conversion Performance
- **Direct Array Lookup**: O(1) for ResourceFormat
- **Switch Statement**: O(1) for FormatType
- **String Concatenation**: O(1) for ResourceBindFlags (max 10 flags)

### Memory Footprint
- **kFormatDesc Array**: ~2560-3072 bytes (64 entries)
- **kNativeFormatDescs Array**: ~768-1024 bytes (64 entries)
- **Total Static Data**: ~3328-4096 bytes

### Cache Locality
- **Good for Sequential Access**: Arrays are laid out sequentially
- **Poor for Random Access**: Large arrays may span multiple cache lines
- **Excellent for Enum Types**: All enums fit in single cache lines
- **Good for Struct Types**: Most structs fit in single cache lines

## Dependencies

### Direct Dependencies
- [`Core/Macros.h`](Source/Falcor/Core/Macros.h) - Macros (FALCOR_ASSERT, FALCOR_UNREACHABLE)
- [`Core/Error.h`](Source/Falcor/Core/Error.h) - Error handling
- [`Core/Enum.h`](Source/Falcor/Core/Enum.h) - Enum utilities (FALCOR_ENUM_CLASS_OPERATORS, FALCOR_ENUM_REGISTER)
- [`string`](Source/Falcor/Core/API/string_view) - String type (std::string)
- [`vector`](Source/Falcor/Core/API/vector) - Vector type (std::vector)
- [`cstdint`](Source/Falcor/Core/API/cstdint) - Integer types (uint32_t, int32_t)
- [`NativeFormats.h`](Source/Falcor/Core/API/NativeFormats.h) - Native format types (DXGI_FORMAT, VkFormat)
- [`Device.h`](Source/Falcor/Core/API/Device.h) - Device type (forward declaration)
- [`GFXHelpers.h`](Source/Falcor/Core/API/GFXHelpers.h) - GFX helper functions
- [`GFXAPI.h`](Source/Falcor/Core/API/GFXAPI.h) - GFX API types
- [`Utils/Scripting/ScriptBindings.h`](Source/Falcor/Utils/Scripting/ScriptBindings.h) - Python bindings

### Indirect Dependencies
- Enum utilities (from Core/Enum.h)
- String utilities (from std::string)
- Vector utilities (from std::vector)
- Native format types (from NativeFormats.h)
- Python bindings (from ScriptBindings.h)

## Known Issues and Limitations

1. **Reverse Lookup Performance**: Linear search (O(N)) for getResourceFormat(DXGI_FORMAT) and getResourceFormat(VkFormat) is inefficient
2. **No Format Validation**: No validation of format compatibility with specific GPU features
3. **No Format Support Query**: No function to query which formats are supported by the current GPU
4. **No Format Conversion**: No pixel format conversion functions (e.g., RGB to BGR)
5. **No Format Swizzling**: No format swizzling functions (e.g., RGBA to BGRA)
6. **No Format Packing**: No format packing functions (e.g., packing multiple formats into one)
7. **No Format Unpacking**: No format unpacking functions (e.g., unpacking multiple formats from one)
8. **No Format Interpolation**: No format interpolation functions (e.g., bilinear filtering)
9. **No Format Compression**: No format compression functions (e.g., compressing to BC7)
10. **No Format Decompression**: No format decompression functions (e.g., decompressing from BC7)

## Usage Example

```cpp
// Query format properties
ResourceFormat format = ResourceFormat::RGBA8Unorm;
uint32_t bytesPerBlock = getFormatBytesPerBlock(format); // 4 bytes
uint32_t channelCount = getFormatChannelCount(format); // 4 channels
FormatType type = getFormatType(format); // FormatType::Unorm
bool hasAlpha = doesFormatHaveAlpha(format); // true

// Query channel bits
uint32_t redBits = getNumChannelBits(format, 0); // 8 bits
uint32_t greenBits = getNumChannelBits(format, 1); // 8 bits
uint32_t blueBits = getNumChannelBits(format, 2); // 8 bits
uint32_t alphaBits = getNumChannelBits(format, 3); // 8 bits

// Query channel mask
TextureChannelFlags mask = getChannelMask(format); // TextureChannelFlags::RGBA

// Query depth-stencil formats
bool isDepth = isDepthFormat(ResourceFormat::D32Float); // true
bool isStencil = isStencilFormat(ResourceFormat::D32FloatS8Uint); // true

// Query compressed formats
bool isCompressed = isCompressedFormat(ResourceFormat::BC7Unorm); // true
uint32_t compressionWidth = getFormatWidthCompressionRatio(ResourceFormat::BC7Unorm); // 4
uint32_t compressionHeight = getFormatHeightCompressionRatio(ResourceFormat::BC7Unorm); // 4

// Query sRGB formats
bool isSrgb = isSrgbFormat(ResourceFormat::RGBA8UnormSrgb); // true

// Convert formats
ResourceFormat linear = srgbToLinearFormat(ResourceFormat::RGBA8UnormSrgb); // ResourceFormat::RGBA8Unorm
ResourceFormat srgb = linearToSrgbFormat(ResourceFormat::RGBA8Unorm); // ResourceFormat::RGBA8UnormSrgb
ResourceFormat color = depthToColorFormat(ResourceFormat::D32Float); // ResourceFormat::R32Float

// Cross-platform format conversion
DXGI_FORMAT dxgiFormat = getDxgiFormat(ResourceFormat::RGBA8Unorm); // DXGI_FORMAT_R8G8B8A8_UNORM
VkFormat vkFormat = getVulkanFormat(ResourceFormat::RGBA8Unorm); // VK_FORMAT_R8G8B8A8_UNORM
ResourceFormat falcorFormat = getResourceFormat(DXGI_FORMAT_R8G8B8A8_UNORM); // ResourceFormat::RGBA8Unorm

// String conversion
std::string formatName = to_string(ResourceFormat::RGBA8Unorm); // "RGBA8Unorm"
std::string typeName = to_string(FormatType::Unorm); // "Unorm"
std::string flagsName = to_string(ResourceBindFlags::ShaderResource | ResourceBindFlags::RenderTarget); // "ShaderResource | RenderTarget"

// Calculate row pitch
uint32_t width = 1920;
uint32_t rowPitch = getFormatRowPitch(ResourceFormat::RGBA8Unorm, width); // 1920 * 4 = 7680 bytes
```

## Conclusion

Formats provides a comprehensive and efficient format management system for GPU resources. The implementation is clean with excellent cache locality for most operations, but reverse lookups suffer from poor performance due to linear search.

**Strengths**:
- Comprehensive format support (64 formats)
- Cross-platform support (DXGI and Vulkan)
- Excellent cache locality for format queries (O(1) direct array lookup)
- Thread-safe (read-only data)
- No memory allocations (all functions are allocation-free)
- Extensive format query functions
- Format conversion functions
- Python bindings
- Clean and simple API

**Weaknesses**:
- Poor reverse lookup performance (O(N) linear search)
- No format validation
- No format support query
- No format conversion (pixel format conversion)
- No format swizzling
- No format packing/unpacking
- No format interpolation
- No format compression/decompression
- Large static data footprint (~3328-4096 bytes)

**Recommendations**:
1. Consider using hash maps for reverse lookups to improve performance
2. Add format validation functions
3. Add format support query functions
4. Consider adding pixel format conversion functions
5. Consider adding format swizzling functions
6. Consider adding format packing/unpacking functions
7. Consider adding format interpolation functions
8. Consider adding format compression/decompression functions
9. Consider reducing static data footprint by using more compact representations
10. Consider adding format compatibility checks with GPU features
