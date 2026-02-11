# FBO - Framebuffer Object

## File Location
- Header: [`Source/Falcor/Core/API/FBO.h`](Source/Falcor/Core/API/FBO.h:1)
- Implementation: [`Source/Falcor/Core/API/FBO.cpp`](Source/Falcor/Core/API/FBO.cpp:1)

## State Machine Status
**Analysis Phase**: Complete
**Last Updated**: 2026-01-07T16:22:55.354Z
**Parent Node**: API
**Current Node**: FBO

## Class Overview

The [`Fbo`](Source/Falcor/Core/API/FBO.h:45) class provides framebuffer object management for rendering operations. It abstracts the underlying graphics API's framebuffer creation and management, supporting multiple color targets, depth-stencil targets, and various texture types.

### Key Responsibilities
- **Framebuffer Management**: Create and manage framebuffer objects
- **Attachment Management**: Attach color and depth-stencil textures
- **View Management**: Create and manage render target and depth-stencil views
- **Sample Position Management**: Configure sample positions for multi-sampled buffers
- **Descriptor Caching**: Cache framebuffer descriptors for performance
- **Validation**: Validate framebuffer attachments and properties

## Memory Layout and Alignment

### Class Member Layout

```cpp
class Fbo : public Object {
private:
    mutable BreakableReference<Device> mpDevice;              // 8 bytes (ptr)

    std::vector<SamplePosition> mSamplePositions;            // 24 bytes (vector overhead)
    uint32_t mSamplePositionsPixelCount = 0;              // 4 bytes

    mutable std::vector<Attachment> mColorAttachments;         // 24 bytes (vector overhead)
    mutable Attachment mDepthStencil;                         // 32 bytes (5 members)

    mutable Desc mTempDesc;                                 // 56 bytes (Desc struct)
    mutable const Desc* mpDesc = nullptr;                     // 8 bytes (ptr)
    mutable uint32_t mWidth = (uint32_t)-1;              // 4 bytes
    mutable uint32_t mHeight = (uint32_t)-1;             // 4 bytes
    mutable uint32_t mDepth = (uint32_t)-1;               // 4 bytes
    mutable bool mHasDepthAttachment = false;                 // 1 byte
    mutable bool mIsLayered = false;                        // 1 byte
    mutable bool mIsZeroAttachment = false;                  // 1 byte

    mutable Slang::ComPtr<gfx::IFramebuffer> mGfxFramebuffer;  // 8 bytes (ptr)
    mutable bool mHandleDirty = true;                        // 1 byte
};
```

### Attachment Struct Layout

```cpp
struct Attachment {
    ref<Texture> pTexture;                 // 8 bytes (ptr)
    ref<ResourceView> pNullView;           // 8 bytes (ptr)
    uint32_t mipLevel = 0;                // 4 bytes
    uint32_t arraySize = 1;                // 4 bytes
    uint32_t firstArraySlice = 0;           // 4 bytes
};
// Total: 28 bytes (with potential padding to 32 bytes)
```

### Desc Struct Layout

```cpp
class Desc {
private:
    struct TargetDesc {
        ResourceFormat format = ResourceFormat::Unknown;  // 4 bytes
        bool allowUav = false;                         // 1 byte
        // Padding: 3 bytes
    };
    // Total: 8 bytes per TargetDesc

    std::vector<TargetDesc> mColorTargets;  // 24 bytes (vector overhead)
    TargetDesc mDepthStencilTarget;             // 8 bytes
    uint32_t mSampleCount = 1;                // 4 bytes
    // Total: ~36 bytes (excluding vector contents)
};
```

### Memory Alignment Analysis

**Total Estimated Size**: ~200-250 bytes (excluding virtual table and base class)

**Alignment Characteristics**:
- **Natural Alignment**: Most members are naturally aligned (8-byte pointers, 4-byte integers)
- **Cache Line Alignment**: NOT cache-line aligned (64 bytes)
- **Padding Issues**: Potential padding between members due to alignment requirements

**Hot Path Members**:
- [`mpDevice`](Source/Falcor/Core/API/FBO.h:371): Accessed for GPU operations
- [`mColorAttachments`](Source/Falcor/Core/API/FBO.h:376): Accessed for color attachment operations
- [`mDepthStencil`](Source/Falcor/Core/API/FBO.h:377): Accessed for depth-stencil operations
- [`mpDesc`](Source/Falcor/Core/API/FBO.h:380): Accessed for descriptor operations
- [`mWidth`](Source/Falcor/Core/API/FBO.h:381), [`mHeight`](Source/Falcor/Core/API/FBO.h:382), [`mDepth`](Source/Falcor/Core/API/FBO.h:383): Accessed for size queries
- [`mGfxFramebuffer`](Source/Falcor/Core/API/FBO.h:388): Accessed for GPU framebuffer operations
- [`mHandleDirty`](Source/Falcor/Core/API/FBO.h:389): Accessed for framebuffer dirty tracking

### Cache Locality Assessment

**Poor Cache Locality**:
- Members scattered across multiple cache lines (~200-250 bytes / 64 bytes = ~4 cache lines)
- Hot path members not grouped together
- Poor for multi-threaded scenarios

**Optimization Opportunities**:
1. Align to cache line boundary for multi-threaded scenarios
2. Group hot path members together
3. Separate hot path from cold path (sample positions could be separate allocation)
4. Use struct-of-arrays instead of array-of-structs for attachments

**Cache Line Analysis**:
```
Cache Line 1 (bytes 0-63):
- mpDevice (8 bytes)
- mSamplePositions (24 bytes, vector overhead)
- mSamplePositionsPixelCount (4 bytes)
- Padding (28 bytes)

Cache Line 2 (bytes 64-127):
- mColorAttachments (24 bytes, vector overhead)
- mDepthStencil (32 bytes)
- Padding (8 bytes)

Cache Line 3 (bytes 128-191):
- mTempDesc (56 bytes, Desc struct)
- mpDesc (8 bytes)
- mWidth (4 bytes)
- mHeight (4 bytes)
- mDepth (4 bytes)
- mHasDepthAttachment (1 byte)
- mIsLayered (1 byte)
- mIsZeroAttachment (1 byte)
- Padding (15 bytes)

Cache Line 4 (bytes 192-255):
- mGfxFramebuffer (8 bytes)
- mHandleDirty (1 byte)
- Padding (55 bytes)
```

## Threading Model

### Thread Safety Analysis

**NOT Thread-Safe**: The [`Fbo`](Source/Falcor/Core/API/FBO.h:45) class is not thread-safe.

### Concurrent Access Patterns

**Single-Threaded Design**:
- Framebuffer operations assume single-threaded access
- Attachment management is not thread-safe
- View management is not thread-safe
- Descriptor caching is not thread-safe

### Mutable State

**Mutable Members**:
```cpp
mutable std::vector<Attachment> mColorAttachments;
mutable Attachment mDepthStencil;
mutable Desc mTempDesc;
mutable const Desc* mpDesc = nullptr;
mutable uint32_t mWidth = (uint32_t)-1;
mutable uint32_t mHeight = (uint32_t)-1;
mutable uint32_t mDepth = (uint32_t)-1;
mutable bool mHasDepthAttachment = false;
mutable bool mIsLayered = false;
mutable bool mIsZeroAttachment = false;
mutable Slang::ComPtr<gfx::IFramebuffer> mGfxFramebuffer;
mutable bool mHandleDirty = true;
```

**Thread Safety Implications**:
- All mutable members can be modified from const methods
- No synchronization primitives protect these mutable members
- **Race Conditions**: Multiple threads calling attachment or view operations will corrupt state

### Attachment Management

**Attach Color Target**: [`attachColorTarget()`](Source/Falcor/Core/API/FBO.cpp:376)
```cpp
void Fbo::attachColorTarget(const ref<Texture>& pTexture, uint32_t rtIndex, uint32_t mipLevel, uint32_t firstArraySlice, uint32_t arraySize)
```
- **Thread Safety**: Not safe for concurrent attach calls
- **Side Effect**: Modifies [`mColorAttachments`](Source/Falcor/Core/API/FBO.h:376), [`mTempDesc`](Source/Falcor/Core/API/FBO.h:379), [`mpDesc`](Source/Falcor/Core/API/FBO.h:380)
- **GPU Work**: Creates render target views

**Attach Depth-Stencil Target**: [`attachDepthStencilTarget()`](Source/Falcor/Core/API/FBO.cpp:350)
```cpp
void Fbo::attachDepthStencilTarget(const ref<Texture>& pDepthStencil, uint32_t mipLevel, uint32_t firstArraySlice, uint32_t arraySize)
```
- **Thread Safety**: Not safe for concurrent attach calls
- **Side Effect**: Modifies [`mDepthStencil`](Source/Falcor/Core/API/FBO.h:377), [`mTempDesc`](Source/Falcor/Core/API/FBO.h:379), [`mpDesc`](Source/Falcor/Core/API/FBO.h:380)
- **GPU Work**: Creates depth-stencil views

### View Management

**Get Render Target View**: [`getRenderTargetView()`](Source/Falcor/Core/API/FBO.cpp:312)
```cpp
ref<RenderTargetView> Fbo::getRenderTargetView(uint32_t rtIndex) const
```
- **Thread Safety**: Not safe for concurrent calls
- **Side Effect**: Creates null view if texture not attached
- **GPU Work**: Creates render target view

**Get Depth-Stencil View**: [`getDepthStencilView()`](Source/Falcor/Core/API/FBO.cpp:331)
```cpp
ref<DepthStencilView> Fbo::getDepthStencilView() const
```
- **Thread Safety**: Not safe for concurrent calls
- **Side Effect**: Creates null view if texture not attached
- **GPU Work**: Creates depth-stencil view

### Framebuffer Management

**Get Framebuffer**: [`getGfxFramebuffer()`](Source/Falcor/Core/API/FBO.cpp:233)
```cpp
gfx::IFramebuffer* Fbo::getGfxFramebuffer() const
```
- **Thread Safety**: Not safe for concurrent calls
- **Side Effect**: Initializes framebuffer if dirty
- **GPU Work**: Creates framebuffer object

**Finalize**: [`finalize()`](Source/Falcor/Core/API/FBO.cpp:493)
```cpp
void Fbo::finalize() const
```
- **Thread Safety**: Not safe for concurrent calls
- **Side Effect**: Calculates properties and initializes framebuffer
- **GPU Work**: Creates framebuffer layout and framebuffer object

### Threading Model Summary

| Operation | Thread Safety | Synchronization |
|-----------|--------------|----------------|
| Attachment Management | Single-threaded | None |
| View Management | Single-threaded | None |
| Framebuffer Management | Single-threaded | None |
| Sample Position Management | Single-threaded | None |
| Descriptor Caching | Single-threaded | None |

## Algorithmic Complexity

### Attachment Management

**Attach Color Target**: [`attachColorTarget()`](Source/Falcor/Core/API/FBO.cpp:376)
```cpp
void Fbo::attachColorTarget(const ref<Texture>& pTexture, uint32_t rtIndex, uint32_t mipLevel, uint32_t firstArraySlice, uint32_t arraySize)
```
- **Time Complexity**: O(1) - Single attachment operation
- **Space Complexity**: O(1) - No additional allocation (attachment is cached)
- **GPU Work**: O(1) - Render target view creation

**Attach Depth-Stencil Target**: [`attachDepthStencilTarget()`](Source/Falcor/Core/API/FBO.cpp:350)
```cpp
void Fbo::attachDepthStencilTarget(const ref<Texture>& pDepthStencil, uint32_t mipLevel, uint32_t firstArraySlice, uint32_t arraySize)
```
- **Time Complexity**: O(1) - Single attachment operation
- **Space Complexity**: O(1) - No additional allocation (attachment is cached)
- **GPU Work**: O(1) - Depth-stencil view creation

### View Management

**Get Render Target View**: [`getRenderTargetView()`](Source/Falcor/Core/API/FBO.cpp:312)
```cpp
ref<RenderTargetView> Fbo::getRenderTargetView(uint32_t rtIndex) const
```
- **Time Complexity**: O(1) - Single view retrieval or creation
- **Space Complexity**: O(1) - No additional allocation (view is cached)
- **GPU Work**: O(1) - Render target view creation (only if not exists)

**Get Depth-Stencil View**: [`getDepthStencilView()`](Source/Falcor/Core/API/FBO.cpp:331)
```cpp
ref<DepthStencilView> Fbo::getDepthStencilView() const
```
- **Time Complexity**: O(1) - Single view retrieval or creation
- **Space Complexity**: O(1) - No additional allocation (view is cached)
- **GPU Work**: O(1) - Depth-stencil view creation (only if not exists)

### Framebuffer Management

**Get Framebuffer**: [`getGfxFramebuffer()`](Source/Falcor/Core/API/FBO.cpp:233)
```cpp
gfx::IFramebuffer* Fbo::getGfxFramebuffer() const
```
- **Time Complexity**: O(1) - Single framebuffer retrieval or creation
- **Space Complexity**: O(1) - No additional allocation (framebuffer is cached)
- **GPU Work**: O(1) - Framebuffer creation (only if dirty)

**Finalize**: [`finalize()`](Source/Falcor/Core/API/FBO.cpp:493)
```cpp
void Fbo::finalize() const
```
- **Time Complexity**: O(n) where n = number of attachments
- **Space Complexity**: O(1) - No additional allocation (properties are cached)
- **GPU Work**: O(1) - Framebuffer layout and framebuffer creation

**Initialize Framebuffer**: [`initFramebuffer()`](Source/Falcor/Core/API/FBO.cpp:259)
```cpp
void Fbo::initFramebuffer() const
```
- **Time Complexity**: O(n) where n = number of attachments
- **Space Complexity**: O(n) - Vector allocation for target layouts
- **GPU Work**: O(1) - Framebuffer layout and framebuffer creation

### Validation

**Validate Attachment**: [`validateAttachment()`](Source/Falcor/Core/API/FBO.cpp:409)
```cpp
void Fbo::validateAttachment(const Attachment& attachment) const
```
- **Time Complexity**: O(1) - Single attachment validation
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: None

**Calculate and Validate Properties**: [`calcAndValidateProperties()`](Source/Falcor/Core/API/FBO.cpp:446)
```cpp
void Fbo::calcAndValidateProperties() const
```
- **Time Complexity**: O(n) where n = number of attachments
- **Space Complexity**: O(1) - No additional allocation (properties are cached)
- **GPU Work**: None

### Sample Position Management

**Set Sample Positions**: [`setSamplePositions()`](Source/Falcor/Core/API/FBO.cpp:502)
```cpp
void Fbo::setSamplePositions(uint32_t samplesPerPixel, uint32_t pixelCount, const SamplePosition positions[])
```
- **Time Complexity**: O(n) where n = samplesPerPixel × pixelCount
- **Space Complexity**: O(n) - Vector allocation for sample positions
- **GPU Work**: None

### Framebuffer Creation

**Create 2D FBO**: [`create2D()`](Source/Falcor/Core/API/FBO.cpp:516)
```cpp
ref<Fbo> Fbo::create2D(ref<Device> pDevice, uint32_t width, uint32_t height, const Fbo::Desc& fboDesc, uint32_t arraySize, uint32_t mipLevels)
```
- **Time Complexity**: O(n) where n = number of color targets
- **Space Complexity**: O(n) - Texture allocation for color and depth targets
- **GPU Work**: O(n) - Texture creation and view creation

**Create Cubemap FBO**: [`createCubemap()`](Source/Falcor/Core/API/FBO.cpp:558)
```cpp
ref<Fbo> Fbo::createCubemap(ref<Device> pDevice, uint32_t width, uint32_t height, const Desc& fboDesc, uint32_t arraySize, uint32_t mipLevels)
```
- **Time Complexity**: O(n) where n = number of color targets
- **Space Complexity**: O(n) - Texture allocation for color and depth targets
- **GPU Work**: O(n) - Texture creation and view creation

**Create FBO from Textures**: [`create()`](Source/Falcor/Core/API/FBO.cpp:208)
```cpp
ref<Fbo> Fbo::create(ref<Device> pDevice, const std::vector<ref<Texture>>& colors, const ref<Texture>& pDepth)
```
- **Time Complexity**: O(n) where n = number of color textures
- **Space Complexity**: O(n) - Attachment allocation
- **GPU Work**: O(n) - View creation

## Performance Characteristics

### Memory Allocation Patterns

**Initialization**:
- Framebuffer object allocation
- Attachment vector allocation (8 color attachments max)
- Sample position vector allocation
- All resources are cached and reused

**Attachment Management**:
- No per-call allocation (attachments are cached)
- Minimal memory overhead per operation

**View Management**:
- View creation on-demand and cached
- No per-call allocation (views are cached)
- Minimal memory overhead per operation

**Framebuffer Management**:
- Framebuffer creation on-demand and cached
- No per-call allocation (framebuffer is cached)
- Minimal memory overhead per operation

### Hot Path Analysis

**Attachment Management**:
1. [`attachColorTarget()`](Source/Falcor/Core/API/FBO.cpp:376): O(1), 2 cache line accesses
2. [`attachDepthStencilTarget()`](Source/Falcor/Core/API/FBO.cpp:350): O(1), 2 cache line accesses
3. **Optimization**: Attachment caching reduces GPU overhead

**View Management**:
1. [`getRenderTargetView()`](Source/Falcor/Core/API/FBO.cpp:312): O(1), 1 cache line access
2. [`getDepthStencilView()`](Source/Falcor/Core/API/FBO.cpp:331): O(1), 1 cache line access
3. **Optimization**: View caching reduces GPU overhead

**Framebuffer Management**:
1. [`getGfxFramebuffer()`](Source/Falcor/Core/API/FBO.cpp:233): O(1), 1 cache line access
2. [`finalize()`](Source/Falcor/Core/API/FBO.cpp:493): O(n), 3 cache line accesses
3. [`initFramebuffer()`](Source/Falcor/Core/API/FBO.cpp:259): O(n), 3 cache line accesses
4. **Optimization**: Framebuffer caching reduces GPU overhead

**Validation**:
1. [`validateAttachment()`](Source/Falcor/Core/API/FBO.cpp:409): O(1), 1 cache line access
2. [`calcAndValidateProperties()`](Source/Falcor/Core/API/FBO.cpp:446): O(n), 3 cache line accesses
3. **Optimization**: Property caching reduces validation overhead

### Memory Bandwidth

**Attachment Management**:
- GPU work: O(1) - View creation
- Memory bandwidth: Minimal (views are cached)

**View Management**:
- GPU work: O(1) - View creation (only if not exists)
- Memory bandwidth: Minimal (views are cached)

**Framebuffer Management**:
- GPU work: O(1) - Framebuffer creation (only if dirty)
- Memory bandwidth: Minimal (framebuffer is cached)

## Critical Path Analysis

### Hot Paths

1. **Attachment Management**: [`attachColorTarget()`](Source/Falcor/Core/API/FBO.cpp:376), [`attachDepthStencilTarget()`](Source/Falcor/Core/API/FBO.cpp:350)
   - Called frequently for framebuffer operations
   - O(1) complexity
   - **Optimization**: Attachment caching reduces GPU overhead

2. **View Management**: [`getRenderTargetView()`](Source/Falcor/Core/API/FBO.cpp:312), [`getDepthStencilView()`](Source/Falcor/Core/API/FBO.cpp:331)
   - Called frequently for rendering operations
   - O(1) complexity
   - **Optimization**: View caching reduces GPU overhead

3. **Framebuffer Management**: [`getGfxFramebuffer()`](Source/Falcor/Core/API/FBO.cpp:233), [`finalize()`](Source/Falcor/Core/API/FBO.cpp:493), [`initFramebuffer()`](Source/Falcor/Core/API/FBO.cpp:259)
   - Called frequently for rendering operations
   - O(n) complexity for finalize and initFramebuffer
   - **Optimization**: Framebuffer caching reduces GPU overhead

4. **Validation**: [`validateAttachment()`](Source/Falcor/Core/API/FBO.cpp:409), [`calcAndValidateProperties()`](Source/Falcor/Core/API/FBO.cpp:446)
   - Called frequently for framebuffer operations
   - O(n) complexity for calcAndValidateProperties
   - **Optimization**: Property caching reduces validation overhead

### Bottlenecks

1. **No Thread Safety**: Mutable state not protected
   - **Mitigation**: Add atomic operations or mutexes

2. **Poor Cache Locality**: Members scattered across multiple cache lines
   - **Mitigation**: Group hot path members together and align to cache line boundary

3. **Validation Overhead**: Property calculation and validation
   - **Mitigation**: Cache properties and only validate when changed

4. **Framebuffer Initialization Overhead**: Framebuffer layout and framebuffer creation
   - **Mitigation**: Cache framebuffer and only recreate when dirty

5. **View Creation Overhead**: View creation on first access
   - **Mitigation**: Pre-create views during initialization

## Memory Management

### Reference Counting

**Smart Pointer Usage**:
- All resources use `ref<T>` smart pointers
- Automatic reference counting via [`Object`](Source/Falcor/Core/Object.h:1) base class
- No manual memory management required

### Attachment Lifecycle

**Color Attachments**:
- Created during attachment
- Cached and reused for all operations
- No per-call allocation

**Depth-Stencil Attachments**:
- Created during attachment
- Cached and reused for all operations
- No per-call allocation

### View Lifecycle

**Render Target Views**:
- Created on-demand and cached
- Cached and reused for all operations
- No per-call allocation

**Depth-Stencil Views**:
- Created on-demand and cached
- Cached and reused for all operations
- No per-call allocation

### Framebuffer Lifecycle

**Framebuffer Object**:
- Created on-demand and cached
- Cached and reused for all operations
- No per-call allocation
- Released when attachments are released

## Platform-Specific Considerations

### DirectX 12

**Framebuffer Management**:
- Uses low-level framebuffer object
- Direct mapping to D3D12 framebuffer
- No additional abstraction overhead

**Resource Views**:
- D3D12 render target views and depth-stencil views
- Direct mapping to D3D12 views

**Descriptor Caching**:
- D3D12 descriptor heap management
- Direct mapping to D3D12 descriptors

### Vulkan

**Framebuffer Management**:
- Uses low-level framebuffer object
- Direct mapping to Vulkan framebuffer
- No additional abstraction overhead

**Resource Views**:
- Vulkan render target views and depth-stencil views
- Direct mapping to Vulkan views

**Descriptor Caching**:
- Vulkan descriptor set management
- Direct mapping to Vulkan descriptors

## Summary

### Strengths

1. **Flexible Design**: Support for multiple color targets and depth-stencil targets
2. **Efficient Caching**: Attachments, views, and framebuffers are cached
3. **Multiple Texture Types**: Support for 2D, cubemap, and array textures
4. **Sample Position Management**: Support for custom sample positions
5. **Descriptor Caching**: Global descriptor cache for performance
6. **Cross-Platform**: Unified interface for D3D12 and Vulkan
7. **Lazy Initialization**: Framebuffer and views created on-demand
8. **Dirty Tracking**: Efficient dirty tracking for framebuffer updates

### Weaknesses

1. **Not Thread-Safe**: No synchronization on mutable state
2. **Poor Cache Locality**: Members scattered across multiple cache lines
3. **Validation Overhead**: Property calculation and validation
4. **Framebuffer Initialization Overhead**: Framebuffer layout and framebuffer creation
5. **View Creation Overhead**: View creation on first access
6. **Limited Validation**: Minimal validation for attachment operations
7. **No Batch Operations**: No support for batch attachment operations

### Optimization Recommendations

1. **Add Thread Safety**: Protect mutable state with atomic operations or mutexes
2. **Improve Cache Locality**: Group hot path members together and align to cache line boundary
3. **Cache Properties**: Cache properties and only validate when changed
4. **Pre-create Views**: Pre-create views during initialization
5. **Batch Attachment Operations**: Support batch attachment operations to reduce overhead
6. **Add Validation**: Add validation for attachment operations
7. **View Pooling**: Use view pooling to reduce view creation overhead
8. **Framebuffer Pooling**: Use framebuffer pooling to reduce framebuffer creation overhead

---

*This technical specification is derived solely from static analysis of provided source code files.*
