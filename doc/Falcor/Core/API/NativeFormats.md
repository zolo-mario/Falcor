# NativeFormats

## Overview
[`NativeFormats`](Source/Falcor/Core/API/NativeFormats.h) provides type definitions for native graphics API formats and function declarations for format conversion. This module defines DXGI_FORMAT and VkFormat enums (if not available from system headers) and declares conversion functions between Falcor formats and native formats.

## Source Files
- Header: [`Source/Falcor/Core/API/NativeFormats.h`](Source/Falcor/Core/API/NativeFormats.h) (370 lines)
- Implementation: [`Source/Falcor/Core/API/Formats.cpp`](Source/Falcor/Core/API/Formats.cpp) (lines 235-267)

## Type Definitions

### DXGI_FORMAT Enum
```cpp
#if __has_include(<dxgiformat.h>)
#include <dxgiformat.h>
#else
enum DXGI_FORMAT
{
    DXGI_FORMAT_UNKNOWN = 0,
    DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
    DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
    DXGI_FORMAT_R32G32B32A32_UINT = 3,
    DXGI_FORMAT_R32G32B32A32_SINT = 4,
    // ... (133 total formats)
    DXGI_FORMAT_FORCE_UINT = 0xffffffff
};
#endif
```
**Purpose**: Direct3D 12 format enumeration

**Memory Layout**: 4 bytes (enum, likely 4 bytes)

**Cache Locativity**: **Excellent** - Fits in a single cache line

**Usage**: Represents Direct3D 12 texture and resource formats

**Platform**: Windows only (D3D12)

**Total Formats**: 133 formats

### VkFormat Enum
```cpp
#if __has_include(<vulkan/vulkan.h>)
#include <vulkan/vulkan.h>
#else
typedef enum VkFormat
{
    VK_FORMAT_UNDEFINED = 0,
    VK_FORMAT_R4G4_UNORM_PACK8 = 1,
    VK_FORMAT_R4G4B4A4_UNORM_PACK16 = 2,
    // ... (184 total formats)
    VK_FORMAT_ASTC_12x12_SRGB_BLOCK = 184,
} VkFormat;
#endif
```
**Purpose**: Vulkan format enumeration

**Memory Layout**: 4 bytes (enum, likely 4 bytes)

**Cache Locativity**: **Excellent** - Fits in a single cache line

**Usage**: Represents Vulkan texture and resource formats

**Platform**: Cross-platform (Vulkan)

**Total Formats**: 184 formats

## Functions

### getDxgiFormat
```cpp
DXGI_FORMAT FALCOR_API getDxgiFormat(ResourceFormat format);
```
**Purpose**: Convert Falcor ResourceFormat to DXGI format

**Parameters**:
- `format`: Falcor ResourceFormat enum value

**Return**: DXGI_FORMAT enum value

**Memory Layout**: N/A (function call)

**Cache Locativity**: **Excellent**
- Direct array lookup (O(1))
- Single cache line access for most cases
- Excellent for repeated conversions

**Algorithmic Complexity**: **O(1)**
- Array lookup: O(1)

**Implementation**: Located in [`Formats.cpp`](Source/Falcor/Core/API/Formats.cpp:235) (lines 235-239)

### getResourceFormat (DXGI_FORMAT)
```cpp
ResourceFormat FALCOR_API getResourceFormat(DXGI_FORMAT format);
```
**Purpose**: Convert DXGI format to Falcor ResourceFormat

**Parameters**:
- `format`: DXGI_FORMAT enum value

**Return**: Falcor ResourceFormat enum value

**Memory Layout**: N/A (function call)

**Cache Locativity**: **Poor**
- Linear search through 64 entries
- May cause multiple cache misses
- Poor for repeated conversions

**Algorithmic Complexity**: **O(N)** where N = 64
- Linear search through array: O(N)
- Average case: O(N/2) = O(32)
- Worst case: O(N) = O(64)

**Implementation**: Located in [`Formats.cpp`](Source/Falcor/Core/API/Formats.cpp:241) (lines 241-250)

**Performance Note**: This function performs a linear search through 64 entries, which is inefficient for frequent calls. Consider using a hash map or binary search for better performance.

### getVulkanFormat
```cpp
VkFormat FALCOR_API getVulkanFormat(ResourceFormat format);
```
**Purpose**: Convert Falcor ResourceFormat to Vulkan format

**Parameters**:
- `format`: Falcor ResourceFormat enum value

**Return**: VkFormat enum value

**Memory Layout**: N/A (function call)

**Cache Locativity**: **Excellent**
- Direct array lookup (O(1))
- Single cache line access for most cases
- Excellent for repeated conversions

**Algorithmic Complexity**: **O(1)**
- Array lookup: O(1)

**Implementation**: Located in [`Formats.cpp`](Source/Falcor/Core/API/Formats.cpp:252) (lines 252-256)

### getResourceFormat (VkFormat)
```cpp
ResourceFormat FALCOR_API getResourceFormat(VkFormat format);
```
**Purpose**: Convert Vulkan format to Falcor ResourceFormat

**Parameters**:
- `format`: VkFormat enum value

**Return**: Falcor ResourceFormat enum value

**Memory Layout**: N/A (function call)

**Cache Locativity**: **Poor**
- Linear search through 64 entries
- May cause multiple cache misses
- Poor for repeated conversions

**Algorithmic Complexity**: **O(N)** where N = 64
- Linear search through array: O(N)
- Average case: O(N/2) = O(32)
- Worst case: O(N) = O(64)

**Implementation**: Located in [`Formats.cpp`](Source/Falcor/Core/API/Formats.cpp:258) (lines 258-267)

**Performance Note**: This function performs a linear search through 64 entries, which is inefficient for frequent calls. Consider using a hash map or binary search for better performance.

## Dependencies

### Direct Dependencies
- [`Formats.h`](Source/Falcor/Core/API/Formats.h) - Format types (ResourceFormat)
- [`Core/Macros.h`](Source/Falcor/Core/Macros.h) - Macros (FALCOR_API)
- [`dxgiformat.h`](Source/Falcor/Core/API/dxgiformat.h) - DXGI format types (conditional)
- [`vulkan/vulkan.h`](Source/Falcor/Core/API/vulkan/vulkan.h) - Vulkan format types (conditional)

### Indirect Dependencies
- ResourceFormat enum (from Formats.h)
- DXGI_FORMAT enum (from dxgiformat.h or local definition)
- VkFormat enum (from vulkan/vulkan.h or local definition)

## Threading Model

**Thread Safety**: **Thread-Safe (No Mutable State)**

**Analysis**:
- No global or static mutable variables
- All functions are pure (no side effects)
- Safe for concurrent access from multiple threads
- No synchronization primitives needed

**Thread Safety Characteristics**:
1. **No Mutable State**: All functions are pure
2. **Pure Functions**: No side effects beyond return values
3. **Safe for Concurrent Access**: Multiple threads can safely call any function simultaneously
4. **No External Dependencies**: Only depends on input parameters and const data arrays

**Note**: The functions are designed for single-threaded or multi-threaded use. Multiple threads can safely call these functions simultaneously without any synchronization.

## Cache Locality Analysis

### Access Patterns

#### Forward Conversions (Falcor to Native)
- **Direct Array Lookup**: Direct array lookup for forward conversions
- **Single Cache Line**: Most cases access single cache line
- **Excellent for Repeated Conversions**: Excellent cache locality for repeated conversions to same format

**Cache Locativity**: **Excellent**
- Direct array lookup (O(1))
- Single cache line access for most cases
- Excellent for repeated conversions

#### Reverse Conversions (Native to Falcor)
- **Linear Search**: Linear search through 64 entries for reverse conversions
- **Multiple Cache Lines**: Each search may cause multiple cache misses
- **Poor for Repeated Conversions**: Poor cache locality for repeated conversions

**Cache Locativity**: **Poor**
- Linear search (O(N))
- Multiple cache line accesses
- Poor for repeated conversions

### Cache Line Analysis

#### Format Conversion Tables
- **kNativeFormatDescs Array**: ~768-1024 bytes (64 entries × 12-16 bytes)
- **Cache Lines**: ~12-16 cache lines (64 bytes each)
- **Access Pattern**: Direct array lookup or linear search
- **Cache Locativity**: **Good for sequential access, Poor for random access**

## Algorithmic Complexity Analysis

### Format Conversion Functions

#### getDxgiFormat
**Time Complexity**: **O(1)**
- Array lookup: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 4 bytes (enum)

#### getResourceFormat (DXGI_FORMAT)
**Time Complexity**: **O(N)** where N = 64
- Linear search through array: O(N)
- Average case: O(N/2) = O(32)
- Worst case: O(N) = O(64)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 4 bytes (enum)

#### getVulkanFormat
**Time Complexity**: **O(1)**
- Array lookup: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 4 bytes (enum)

#### getResourceFormat (VkFormat)
**Time Complexity**: **O(N)** where N = 64
- Linear search through array: O(N)
- Average case: O(N/2) = O(32)
- Worst case: O(N) = O(64)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 4 bytes (enum)

## Key Technical Features

### Native Format Enumerations
- **DXGI_FORMAT**: Direct3D 12 format enumeration (133 formats)
- **VkFormat**: Vulkan format enumeration (184 formats)
- **Conditional Compilation**: Use system headers if available, otherwise define locally
- **Cross-Platform**: Support for both D3D12 and Vulkan

### Format Conversion Functions
- **Forward Conversions**: Convert from Falcor to native formats (O(1))
- **Reverse Conversions**: Convert from native formats to Falcor (O(N))
- **Bidirectional Support**: Support for both directions of conversion
- **Cross-Platform**: Works with both D3D12 and Vulkan

### Conditional Compilation
- **System Headers**: Use system headers if available
- **Fallback Definitions**: Provide fallback definitions if system headers not available
- **Platform-Specific**: DXGI_FORMAT only available on Windows

### Format Mapping Table
- **64 Entries**: Format mapping table for all ResourceFormat values
- **Direct Mapping**: Direct mapping for forward conversions
- **Linear Search**: Linear search for reverse conversions
- **Cross-Platform**: Same table works for both D3D12 and Vulkan

## Performance Characteristics

### Forward Conversion Performance
- **O(1) Direct Lookup**: Direct array lookup for forward conversions
- **Excellent Cache Locality**: Single cache line access for most cases
- **No Memory Allocations**: Zero dynamic memory allocation
- **Thread-Safe**: Safe for concurrent access from multiple threads

### Reverse Conversion Performance
- **O(N) Linear Search**: Linear search through 64 entries for reverse conversions
- **Poor Cache Locality**: Multiple cache line accesses
- **No Memory Allocations**: Zero dynamic memory allocation
- **Thread-Safe**: Safe for concurrent access from multiple threads

### Overall Performance
- **Fast for Forward Conversions**: O(1) direct lookup
- **Slow for Reverse Conversions**: O(N) linear search
- **Cache-Friendly for Forward Conversions**: Excellent cache locality
- **Cache-Unfriendly for Reverse Conversions**: Poor cache locality
- **Thread-Safe**: Safe for concurrent access

## Known Issues and Limitations

1. **Reverse Lookup Performance**: Linear search (O(N)) for getResourceFormat(DXGI_FORMAT) and getResourceFormat(VkFormat) is inefficient
2. **No Format Validation**: No validation of format compatibility with specific GPU features
3. **No Format Support Query**: No function to query which formats are supported by current GPU
4. **No Format Conversion**: No pixel format conversion functions (e.g., RGB to BGR)
5. **No Format Swizzling**: No format swizzling functions (e.g., RGBA to BGRA)
6. **No Format Packing**: No format packing functions (e.g., packing multiple formats into one)
7. **No Format Unpacking**: No format unpacking functions (e.g., unpacking multiple formats from one)
8. **No Format Interpolation**: No format interpolation functions (e.g., bilinear filtering)
9. **No Format Compression**: No format compression functions (e.g., compressing to BC7)
10. **No Format Decompression**: No format decompression functions (e.g., decompressing from BC7)

## Usage Example

```cpp
// Convert Falcor format to DXGI format (Windows only)
ResourceFormat falcorFormat = ResourceFormat::RGBA8Unorm;
DXGI_FORMAT dxgiFormat = getDxgiFormat(falcorFormat); // DXGI_FORMAT_R8G8B8A8_UNORM

// Convert DXGI format to Falcor format (Windows only)
DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
ResourceFormat falcorFormat = getResourceFormat(dxgiFormat); // ResourceFormat::RGBA8Unorm

// Convert Falcor format to Vulkan format
ResourceFormat falcorFormat = ResourceFormat::RGBA8Unorm;
VkFormat vkFormat = getVulkanFormat(falcorFormat); // VK_FORMAT_R8G8B8A8_UNORM

// Convert Vulkan format to Falcor format
VkFormat vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
ResourceFormat falcorFormat = getResourceFormat(vkFormat); // ResourceFormat::RGBA8Unorm
```

## Conclusion

NativeFormats provides a comprehensive format conversion system between Falcor formats and native graphics API formats (DXGI and Vulkan). The implementation is simple with excellent cache locality for forward conversions, but reverse lookups suffer from poor performance due to linear search.

**Strengths**:
- Clean and simple API for format conversion
- Complete support for all Falcor formats (64 formats)
- O(1) direct lookup for forward conversions
- Excellent cache locality for forward conversions
- Zero dynamic memory allocation
- Thread-safe (no mutable state)
- Cross-platform support (D3D12 and Vulkan)
- Conditional compilation for system headers
- Fallback definitions for missing headers
- Bidirectional format conversion

**Weaknesses**:
- Poor reverse lookup performance (O(N) linear search)
- No format validation
- No format support query
- No format conversion (pixel format conversion)
- No format swizzling
- No format packing/unpacking
- No format interpolation
- No format compression/decompression
- Large static data footprint (~768-1024 bytes)
- Platform-specific DXGI_FORMAT (Windows only)

**Recommendations**:
1. Consider using hash maps for reverse lookups to improve performance
2. Consider adding format validation functions
3. Consider adding format support query functions
4. Consider adding pixel format conversion functions
5. Consider adding format swizzling functions
6. Consider adding format packing/unpacking functions
7. Consider adding format interpolation functions
8. Consider adding format compression/decompression functions
9. Consider reducing static data footprint by using more compact representations
10. Consider adding format compatibility checks with GPU features
