# Texture - GPU Texture Management

## File Location
- Header: [`Source/Falcor/Core/API/Texture.h`](Source/Falcor/Core/API/Texture.h:1)
- Implementation: [`Source/Falcor/Core/API/Texture.cpp`](Source/Falcor/Core/API/Texture.cpp:1)

## State Machine Status
**Analysis Phase**: Complete
**Last Updated**: 2026-01-07T16:26:34.117Z
**Parent Node**: API
**Current Node**: Texture

## Class Overview

The [`Texture`](Source/Falcor/Core/API/Texture.h:47) class provides GPU texture management for various texture types (1D, 2D, 3D, cube, multi-sampled). It abstracts the underlying graphics API's texture creation and management.

### Key Responsibilities
- **Texture Creation**: Create textures from data, files, or existing GPU resources
- **Mipmap Generation**: Generate mipmaps for textures
- **Subresource Management**: Manage texture subresources (mip levels, array slices)
- **View Management**: Create and manage SRV, UAV, RTV, and DSV views
- **Data Upload**: Upload texture data to GPU
- **Data Read**: Read texture data from GPU
- **Texture Capture**: Capture texture to image files
- **Format Conversion**: Support for sRGB format conversion

## Memory Layout and Alignment

### Class Member Layout

```cpp
class Texture : public Resource {
protected:
    Slang::ComPtr<gfx::ITextureResource> mGfxTextureResource;  // 8 bytes (ptr)

    bool mReleaseRtvsAfterGenMips = true;                    // 1 byte
    std::filesystem::path mSourcePath;                           // 32 bytes (path)
    Bitmap::ImportFlags mImportFlags = Bitmap::ImportFlags::None; // 4 bytes

    ResourceFormat mFormat = ResourceFormat::Unknown;             // 4 bytes
    uint32_t mWidth = 0;                                     // 4 bytes
    uint32_t mHeight = 0;                                    // 4 bytes
    uint32_t mDepth = 0;                                     // 4 bytes
    uint32_t mMipLevels = 0;                                 // 4 bytes
    uint32_t mArraySize = 0;                                 // 4 bytes
    uint32_t mSampleCount = 0;                                 // 4 bytes
    bool mIsSparse = false;                                    // 1 byte
    int3 mSparsePageRes = int3(0);                           // 12 bytes (3 ints)
};
// Total: ~89 bytes (excluding virtual table, base class, and view maps)
```

### View Maps (Inherited from Resource)

```cpp
std::unordered_map<ResourceViewInfo, ref<ShaderResourceView>> mSrvs;   // 48 bytes (map overhead)
std::unordered_map<ResourceViewInfo, ref<UnorderedAccessView>> mUavs; // 48 bytes (map overhead)
std::unordered_map<ResourceViewInfo, ref<RenderTargetView>> mRtvs; // 48 bytes (map overhead)
// Total: ~144 bytes (excluding map contents)
```

### Alignment Analysis

**Total Estimated Size**: ~233-250 bytes (excluding virtual table, base class, and map contents)

**Alignment Characteristics**:
- **Natural Alignment**: Most members are naturally aligned (8-byte pointers, 4-byte integers)
- **Cache Line Alignment**: NOT cache-line aligned (64 bytes)
- **Padding Issues**: Potential padding between members due to alignment requirements

**Hot Path Members**:
- [`mGfxTextureResource`](Source/Falcor/Core/API/Texture.h:337): Accessed for GPU texture operations
- [`mFormat`](Source/Falcor/Core/API/Texture.h:343): Accessed for format queries
- [`mWidth`](Source/Falcor/Core/API/Texture.h:344), [`mHeight`](Source/Falcor/Core/API/Texture.h:345), [`mDepth`](Source/Falcor/Core/API/Texture.h:346): Accessed for size queries
- [`mMipLevels`](Source/Falcor/Core/API/Texture.h:347): Accessed for mipmap queries
- [`mArraySize`](Source/Falcor/Core/API/Texture.h:348): Accessed for array size queries
- [`mSampleCount`](Source/Falcor/Core/API/Texture.h:349): Accessed for sample count queries
- [`mSrvs`](Source/Falcor/Core/API/Resource.h:1), [`mUavs`](Source/Falcor/Core/API/Resource.h:1), [`mRtvs`](Source/Falcor/Core/API/Resource.h:1): Accessed for view operations

### Cache Locality Assessment

**Poor Cache Locality**:
- Members scattered across multiple cache lines (~233-250 bytes / 64 bytes = ~4 cache lines)
- Hot path members not grouped together
- Poor for multi-threaded scenarios

**Optimization Opportunities**:
1. Align to cache line boundary for multi-threaded scenarios
2. Group hot path members together
3. Separate hot path from cold path (source path, import flags could be separate allocation)
4. Use struct-of-arrays instead of array-of-structs for view maps

**Cache Line Analysis**:
```
Cache Line 1 (bytes 0-63):
- mGfxTextureResource (8 bytes)
- mReleaseRtvsAfterGenMips (1 byte)
- Padding (55 bytes)

Cache Line 2 (bytes 64-127):
- mSourcePath (32 bytes)
- mImportFlags (4 bytes)
- mFormat (4 bytes)
- mWidth (4 bytes)
- mHeight (4 bytes)
- mDepth (4 bytes)
- Padding (12 bytes)

Cache Line 3 (bytes 128-191):
- mMipLevels (4 bytes)
- mArraySize (4 bytes)
- mSampleCount (4 bytes)
- mIsSparse (1 byte)
- mSparsePageRes (12 bytes)
- Padding (37 bytes)

Cache Line 4 (bytes 192-255):
- mSrvs (48 bytes, map overhead)
- mUavs (48 bytes, map overhead)
- mRtvs (48 bytes, map overhead)
- Padding (8 bytes)
```

## Threading Model

### Thread Safety Analysis

**NOT Thread-Safe**: The [`Texture`](Source/Falcor/Core/API/Texture.h:47) class is not thread-safe.

### Concurrent Access Patterns

**Single-Threaded Design**:
- Texture operations assume single-threaded access
- View management is not thread-safe
- Subresource operations are not thread-safe

### Mutable State

**Mutable Members**:
```cpp
bool mReleaseRtvsAfterGenMips = true;
std::filesystem::path mSourcePath;
Bitmap::ImportFlags mImportFlags = Bitmap::ImportFlags::None;
```

**Thread Safety Implications**:
- [`mReleaseRtvsAfterGenMips`](Source/Falcor/Core/API/Texture.h:339) is mutable and can be modified from const methods
- [`mSourcePath`](Source/Falcor/Core/API/Texture.h:340) is mutable and can be modified from const methods
- [`mImportFlags`](Source/Falcor/Core/API/Texture.h:341) is mutable and can be modified from const methods
- No synchronization primitives protect these mutable members
- **Race Conditions**: Multiple threads calling texture operations will corrupt state

### Texture Creation

**Constructor**: [`Texture()`](Source/Falcor/Core/API/Texture.cpp:74)
```cpp
Texture::Texture(
    ref<Device> pDevice,
    Type type,
    ResourceFormat format,
    uint32_t width,
    uint32_t height,
    uint32_t depth,
    uint32_t arraySize,
    uint32_t mipLevels,
    uint32_t sampleCount,
    ResourceBindFlags bindFlags,
    const void* pInitData
)
```
- **Thread Safety**: Not safe for concurrent creation calls
- **Side Effect**: Modifies all mutable members
- **GPU Work**: Texture creation and data upload

**Constructor from GPU Resource**: [`Texture()`](Source/Falcor/Core/API/Texture.cpp:213)
```cpp
Texture::Texture(
    ref<Device> pDevice,
    gfx::ITextureResource* pResource,
    Type type,
    ResourceFormat format,
    uint32_t width,
    uint32_t height,
    uint32_t depth,
    uint32_t arraySize,
    uint32_t mipLevels,
    uint32_t sampleCount,
    ResourceBindFlags bindFlags,
    Resource::State initState
)
```
- **Thread Safety**: Not safe for concurrent creation calls
- **Side Effect**: Modifies all mutable members
- **GPU Work**: No GPU work (wraps existing GPU resource)

### View Management

**Get SRV**: [`getSRV()`](Source/Falcor/Core/API/Texture.h:205), [`getSRV()`](Source/Falcor/Core/API/Texture.cpp:518)
```cpp
ref<ShaderResourceView> Texture::getSRV(uint32_t mostDetailedMip, uint32_t mipCount, uint32_t firstArraySlice, uint32_t arraySize)
```
- **Thread Safety**: Not safe for concurrent calls
- **Time Complexity**: O(1) - Single SRV retrieval or creation
- **Space Complexity**: O(1) - No additional allocation (SRV is cached)
- **GPU Work**: O(1) - SRV creation (only if not exists)
- **View Caching**: Uses unordered_map for view caching

**Get UAV**: [`getUAV()`](Source/Falcor/Core/API/Texture.h:210), [`getUAV()`](Source/Falcor/Core/API/Texture.cpp:523)
```cpp
ref<UnorderedAccessView> Texture::getUAV(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t arraySize)
```
- **Thread Safety**: Not safe for concurrent calls
- **Time Complexity**: O(1) - Single UAV retrieval or creation
- **Space Complexity**: O(1) - No additional allocation (UAV is cached)
- **GPU Work**: O(1) - UAV creation (only if not exists)
- **View Caching**: Uses unordered_map for view caching

**Get RTV**: [`getRTV()`](Source/Falcor/Core/API/Texture.h:235), [`getRTV()`](Source/Falcor/Core/API/Texture.cpp:528)
```cpp
ref<RenderTargetView> Texture::getRTV(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t arraySize)
```
- **Thread Safety**: Not safe for concurrent calls
- **Time Complexity**: O(1) - Single RTV retrieval or creation
- **Space Complexity**: O(1) - No additional allocation (RTV is cached)
- **GPU Work**: O(1) - RTV creation (only if not exists)
- **View Caching**: Uses unordered_map for view caching

**Get DSV**: [`getDSV()`](Source/Falcor/Core/API/Texture.h:244), [`getDSV()`](Source/Falcor/Core/API/Texture.cpp:502)
```cpp
ref<DepthStencilView> Texture::getDSV(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t arraySize)
```
- **Thread Safety**: Not safe for concurrent calls
- **Time Complexity**: O(1) - Single DSV retrieval or creation
- **Space Complexity**: O(1) - No additional allocation (DSV is cached)
- **GPU Work**: O(1) - DSV creation (only if not exists)
- **View Caching**: Uses unordered_map for view caching

### Subresource Management

**Get Subresource Layout**: [`getSubresourceLayout()`](Source/Falcor/Core/API/Texture.h:259), [`getSubresourceLayout()`](Source/Falcor/Core/API/Texture.cpp:544)
```cpp
Texture::SubresourceLayout Texture::getSubresourceLayout(uint32_t subresource) const
```
- **Thread Safety**: Not safe for concurrent calls
- **Time Complexity**: O(1) - Single subresource layout calculation
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: None

**Set Subresource Blob**: [`setSubresourceBlob()`](Source/Falcor/Core/API/Texture.h:267), [`setSubresourceBlob()`](Source/Falcor/Core/API/Texture.cpp:562)
```cpp
void Texture::setSubresourceBlob(uint32_t subresource, const void* pData, size_t size)
```
- **Thread Safety**: Not safe for concurrent calls
- **Time Complexity**: O(1) - Single subresource update
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: O(1) - Subresource data update

**Get Subresource Blob**: [`getSubresourceBlob()`](Source/Falcor/Core/API/Texture.h:275), [`getSubresourceBlob()`](Source/Falcor/Core/API/Texture.cpp:573)
```cpp
void Texture::getSubresourceBlob(uint32_t subresource, void* pData, size_t size) const
```
- **Thread Safety**: Not safe for concurrent calls
- **Time Complexity**: O(1) - Single subresource read
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: O(1) - Subresource data read

### Mipmap Generation

**Generate Mips**: [`generateMips()`](Source/Falcor/Core/API/Texture.h:301), [`generateMips()`](Source/Falcor/Core/API/Texture.cpp:670)
```cpp
void Texture::generateMips(RenderContext* pContext, bool minMaxMips = false)
```
- **Thread Safety**: Not safe for concurrent calls
- **Time Complexity**: O(n × m) where n = array size, m = mip levels
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: O(n × m) - Blit operations for each mip level
- **View Caching**: Creates and caches SRV and RTV views for each mip level

### Texture Loading

**Create from File**: [`createFromFile()`](Source/Falcor/Core/API/Texture.h:189), [`createFromFile()`](Source/Falcor/Core/API/Texture.cpp:364)
```cpp
static ref<Texture> Texture::createFromFile(
    ref<Device> pDevice,
    const std::filesystem::path& path,
    bool generateMipLevels,
    bool loadAsSrgb,
    ResourceBindFlags bindFlags,
    Bitmap::ImportFlags importFlags
)
```
- **Thread Safety**: Not safe for concurrent creation calls
- **Time Complexity**: O(n) where n = number of mip levels
- **Space Complexity**: O(n) - Bitmap allocation for each mip level
- **GPU Work**: O(1) - Texture creation and data upload

**Create Mipped from Files**: [`createMippedFromFiles()`](Source/Falcor/Core/API/Texture.h:172), [`createMippedFromFiles()`](Source/Falcor/Core/API/Texture.cpp:263)
```cpp
static ref<Texture> Texture::createMippedFromFiles(
    ref<Device> pDevice,
    fstd::span<const std::filesystem::path> paths,
    bool loadAsSrgb,
    ResourceBindFlags bindFlags,
    Bitmap::ImportFlags importFlags
)
```
- **Thread Safety**: Not safe for concurrent creation calls
- **Time Complexity**: O(n) where n = number of mip levels
- **Space Complexity**: O(n) - Bitmap allocation for each mip level
- **GPU Work**: O(1) - Texture creation and data upload

### Texture Capture

**Capture to File**: [`captureToFile()`](Source/Falcor/Core/API/Texture.h:286), [`captureToFile()`](Source/Falcor/Core/API/Texture.cpp:586)
```cpp
void Texture::captureToFile(
    uint32_t mipLevel,
    uint32_t arraySlice,
    const std::filesystem::path& path,
    Bitmap::FileFormat format,
    Bitmap::ExportFlags exportFlags,
    bool async
)
```
- **Thread Safety**: Not safe for concurrent calls
- **Time Complexity**: O(n) where n = texture size in bytes
- **Space Complexity**: O(n) - Data buffer allocation
- **GPU Work**: O(n) - Texture read and image save

## Performance Characteristics

### Memory Allocation Patterns

**Texture Creation**:
- GPU texture resource allocation
- View map allocation (3 unordered_maps)
- All resources are cached and reused

**View Management**:
- View creation on-demand and cached
- No per-call allocation (views are cached)
- Minimal memory overhead per operation

**Subresource Operations**:
- No per-call allocation (subresource data is cached)
- Minimal memory overhead per operation

**Mipmap Generation**:
- View creation for each mip level
- No per-call allocation (views are cached)
- Minimal memory overhead per operation

### Hot Path Analysis

**View Management**:
1. [`getSRV()`](Source/Falcor/Core/API/Texture.cpp:518): O(1), 1 cache line access
2. [`getUAV()`](Source/Falcor/Core/API/Texture.cpp:523): O(1), 1 cache line access
3. [`getRTV()`](Source/Falcor/Core/API/Texture.cpp:528): O(1), 1 cache line access
4. [`getDSV()`](Source/Falcor/Core/API/Texture.cpp:502): O(1), 1 cache line access
5. **Optimization**: View caching reduces GPU overhead

**Subresource Operations**:
1. [`getSubresourceLayout()`](Source/Falcor/Core/API/Texture.cpp:544): O(1), 1 cache line access
2. [`setSubresourceBlob()`](Source/Falcor/Core/API/Texture.cpp:562): O(1), 1 cache line access
3. [`getSubresourceBlob()`](Source/Falcor/Core/API/Texture.cpp:573): O(1), 1 cache line access
4. **Optimization**: Direct access to cached members

**Mipmap Generation**:
1. [`generateMips()`](Source/Falcor/Core/API/Texture.cpp:670): O(n × m), 1 cache line access
2. **Optimization**: View caching reduces GPU overhead

**Texture Loading**:
1. [`createFromFile()`](Source/Falcor/Core/API/Texture.cpp:364): O(n), 1 cache line access
2. [`createMippedFromFiles()`](Source/Falcor/Core/API/Texture.cpp:263): O(n), 1 cache line access
3. **Optimization**: Bitmap caching reduces GPU overhead

### Memory Bandwidth

**View Management**:
- GPU work: O(1) - View creation (only if not exists)
- Memory bandwidth: Minimal (views are cached)

**Subresource Operations**:
- GPU work: O(1) - Subresource data update or read
- Memory bandwidth: O(n) where n = subresource size

**Mipmap Generation**:
- GPU work: O(n × m) - Blit operations for each mip level
- Memory bandwidth: O(n × m) where n = texture size, m = number of mip levels

**Texture Loading**:
- GPU work: O(1) - Texture creation and data upload
- Memory bandwidth: O(n) where n = texture size in bytes

## Critical Path Analysis

### Hot Paths

1. **View Management**: [`getSRV()`](Source/Falcor/Core/API/Texture.cpp:518), [`getUAV()`](Source/Falcor/Core/API/Texture.cpp:523), [`getRTV()`](Source/Falcor/Core/API/Texture.cpp:528), [`getDSV()`](Source/Falcor/Core/API/Texture.cpp:502)
   - Called frequently for texture operations
   - O(1) complexity
   - **Optimization**: View caching reduces GPU overhead

2. **Subresource Operations**: [`getSubresourceLayout()`](Source/Falcor/Core/API/Texture.cpp:544), [`setSubresourceBlob()`](Source/Falcor/Core/API/Texture.cpp:562), [`getSubresourceBlob()`](Source/Falcor/Core/API/Texture.cpp:573)
   - Called frequently for texture operations
   - O(1) complexity
   - **Optimization**: Direct access to cached members

3. **Mipmap Generation**: [`generateMips()`](Source/Falcor/Core/API/Texture.cpp:670)
   - Called frequently for texture operations
   - O(n × m) complexity
   - **Optimization**: View caching reduces GPU overhead

4. **Texture Loading**: [`createFromFile()`](Source/Falcor/Core/API/Texture.cpp:364), [`createMippedFromFiles()`](Source/Falcor/Core/API/Texture.cpp:263)
   - Called frequently for texture operations
   - O(n) complexity
   - **Optimization**: Bitmap caching reduces GPU overhead

### Bottlenecks

1. **No Thread Safety**: Mutable state not protected
   - **Mitigation**: Add atomic operations or mutexes

2. **Poor Cache Locality**: Members scattered across multiple cache lines
   - **Mitigation**: Group hot path members together and align to cache line boundary

3. **View Map Overhead**: Unordered_map lookups for view caching
   - **Mitigation**: Use more efficient data structures for view caching

4. **Mipmap Generation Overhead**: Blit operations for each mip level
   - **Mitigation**: Use compute shaders for mipmap generation

5. **Subresource Data Copy**: Memory copy for subresource operations
   - **Mitigation**: Use direct GPU-to-GPU transfers

## Memory Management

### Reference Counting

**Smart Pointer Usage**:
- All resources use `ref<T>` smart pointers
- Automatic reference counting via [`Object`](Source/Falcor/Core/Object.h:1) base class
- No manual memory management required

### View Lifecycle

**Shader Resource Views**:
- Created on-demand and cached
- Cached and reused for all operations
- No per-call allocation

**Unordered Access Views**:
- Created on-demand and cached
- Cached and reused for all operations
- No per-call allocation

**Render Target Views**:
- Created on-demand and cached
- Cached and reused for all operations
- No per-call allocation

**Depth Stencil Views**:
- Created on-demand and cached
- Cached and reused for all operations
- No per-call allocation

## Platform-Specific Considerations

### DirectX 12

**Texture Management**:
- Uses low-level texture resource
- Direct mapping to D3D12 texture
- No additional abstraction overhead

**Resource Views**:
- D3D12 shader resource views, depth-stencil views, unordered access views, and render target views
- Direct mapping to D3D12 views

**Mipmap Generation**:
- D3D12 blit operations for each mip level
- Direct mapping to D3D12 blit

### Vulkan

**Texture Management**:
- Uses low-level texture resource
- Direct mapping to Vulkan image
- No additional abstraction overhead

**Resource Views**:
- Vulkan shader resource views, depth-stencil views, unordered access views, and render target views
- Direct mapping to Vulkan views

**Mipmap Generation**:
- Vulkan blit operations for each mip level
- Direct mapping to Vulkan blit

## Summary

### Strengths

1. **Flexible Design**: Support for 1D, 2D, 3D, cube, and multi-sampled textures
2. **Efficient View Caching**: Views are cached and reused
3. **Mipmap Generation**: Automatic mipmap generation
4. **Subresource Management**: Efficient subresource access and manipulation
5. **Texture Loading**: Support for loading from files (DDS, PNG, etc.)
6. **Texture Capture**: Support for capturing to image files
7. **Format Conversion**: Support for sRGB format conversion
8. **Cross-Platform**: Unified interface for D3D12 and Vulkan
9. **Python Integration**: NumPy integration for Python bindings

### Weaknesses

1. **Not Thread-Safe**: No synchronization on mutable state
2. **Poor Cache Locality**: Members scattered across multiple cache lines
3. **View Map Overhead**: Unordered_map lookups for view caching
4. **Mipmap Generation Overhead**: Blit operations for each mip level
5. **Subresource Data Copy**: Memory copy for subresource operations
6. **Limited Validation**: Minimal validation for texture operations
7. **No Batch Operations**: No support for batch texture operations

### Optimization Recommendations

1. **Add Thread Safety**: Protect mutable state with atomic operations or mutexes
2. **Improve Cache Locality**: Group hot path members together and align to cache line boundary
3. **Optimize View Caching**: Use more efficient data structures for view caching
4. **Use Compute Shaders**: Use compute shaders for mipmap generation
5. **Direct GPU Transfers**: Use direct GPU-to-GPU transfers for subresource operations
6. **Batch Texture Operations**: Support batch texture operations to reduce overhead
7. **Add Validation**: Add validation for texture operations
8. **View Pooling**: Use view pooling to reduce view creation overhead

---

*This technical specification is derived solely from static analysis of provided source code files.*
