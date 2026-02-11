# ResourceViews - Resource View Management

## File Location
- Header: [`Source/Falcor/Core/API/ResourceViews.h`](Source/Falcor/Core/API/ResourceViews.h:1)
- Implementation: [`Source/Falcor/Core/API/ResourceViews.cpp`](Source/Falcor/Core/API/ResourceViews.cpp:1)

## State Machine Status
**Analysis Phase**: Complete
**Last Updated**: 2026-01-07T16:25:08.580Z
**Parent Node**: API
**Current Node**: ResourceViews

## Module Overview

The ResourceViews module provides resource view management for GPU resources. It abstracts the underlying graphics API's resource view creation and management, supporting multiple view types: shader resource views (SRV), depth-stencil views (DSV), unordered access views (UAV), and render target views (RTV).

### Key Classes

1. **[`ResourceViewInfo`](Source/Falcor/Core/API/ResourceViews.h:44)** - Resource view information structure
2. **[`ResourceView`](Source/Falcor/Core/API/ResourceViews.h:76)** - Base resource view class
3. **[`ShaderResourceView`](Source/Falcor/Core/API/ResourceViews.h:138)** - Shader resource view (SRV)
4. **[`DepthStencilView`](Source/Falcor/Core/API/ResourceViews.h:178)** - Depth-stencil view (DSV)
5. **[`UnorderedAccessView`](Source/Falcor/Core/API/ResourceViews.h:203)** - Unordered access view (UAV)
6. **[`RenderTargetView`](Source/Falcor/Core/API/ResourceViews.h:239)** - Render target view (RTV)

## ResourceViewInfo Structure

### Memory Layout

```cpp
struct ResourceViewInfo {
    // Textures
    uint32_t mostDetailedMip = 0;         // 4 bytes
    uint32_t mipCount = kMaxPossible;       // 4 bytes
    uint32_t firstArraySlice = 0;          // 4 bytes
    uint32_t arraySize = kMaxPossible;        // 4 bytes

    // Buffers
    uint64_t offset = 0;                    // 8 bytes
    uint64_t size = kEntireBuffer;           // 8 bytes
};
// Total: 32 bytes (union between texture and buffer fields)
```

### Alignment Analysis

**Total Size**: 32 bytes

**Alignment Characteristics**:
- **Natural Alignment**: All members are naturally aligned (4-byte integers, 8-byte integers)
- **Cache Line Alignment**: NOT cache-line aligned (64 bytes)
- **Padding Issues**: Potential padding between members due to alignment requirements

**Cache Locality**: Good (fits in single cache line)

## ResourceView Base Class

### Memory Layout

```cpp
class ResourceView : public Object {
protected:
    Device* mpDevice;                                    // 8 bytes (ptr)
    Slang::ComPtr<gfx::IResourceView> mGfxResourceView;  // 8 bytes (ptr)
    ResourceViewInfo mViewInfo;                           // 32 bytes
    Resource* mpResource;                                 // 8 bytes (ptr)
};
// Total: ~56 bytes (excluding virtual table and base class)
```

### Alignment Analysis

**Total Estimated Size**: ~56-64 bytes (excluding virtual table and base class)

**Alignment Characteristics**:
- **Natural Alignment**: All members are naturally aligned (8-byte pointers, 4-byte integers)
- **Cache Line Alignment**: NOT cache-line aligned (64 bytes)
- **Padding Issues**: Potential padding between members due to alignment requirements

**Hot Path Members**:
- [`mpDevice`](Source/Falcor/Core/API/ResourceViews.h:132): Accessed for GPU operations
- [`mGfxResourceView`](Source/Falcor/Core/API/ResourceViews.h:133): Accessed for GPU resource view operations
- [`mViewInfo`](Source/Falcor/Core/API/ResourceViews.h:134): Accessed for view information queries
- [`mpResource`](Source/Falcor/Core/API/ResourceViews.h:135): Accessed for resource operations

### Cache Locality Assessment

**Good Cache Locality**:
- All members fit in single cache line (~56-64 bytes < 64 bytes)
- Hot path members are grouped together
- Good for single-threaded scenarios

**Optimization Opportunities**:
1. Align to cache line boundary for multi-threaded scenarios
2. Separate hot path from cold path (device pointer could be separate allocation)

### Threading Model

**NOT Thread-Safe**: The [`ResourceView`](Source/Falcor/Core/API/ResourceViews.h:76) class is not thread-safe.

### Concurrent Access Patterns

**Single-Threaded Design**:
- Resource view operations assume single-threaded access
- View creation is not thread-safe
- Resource access is not thread-safe

### Mutable State

**Mutable Members**:
- [`mGfxResourceView`](Source/Falcor/Core/API/ResourceViews.h:133): Mutable and can be modified from const methods
- [`mViewInfo`](Source/Falcor/Core/API/ResourceViews.h:134): Mutable and can be modified from const methods
- [`mpResource`](Source/Falcor/Core/API/ResourceViews.h:135): Mutable and can be modified from const methods

**Thread Safety Implications**:
- No synchronization primitives protect these mutable members
- **Race Conditions**: Multiple threads calling view operations will corrupt state

### Key Operations

**Get Native Handle**: [`getNativeHandle()`](Source/Falcor/Core/API/ResourceViews.cpp:41)
```cpp
NativeHandle ResourceView::getNativeHandle() const
```
- **Thread Safety**: Not safe for concurrent calls
- **Time Complexity**: O(1) - Single native handle retrieval
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: O(1) - Native handle retrieval

**Get View Info**: [`getViewInfo()`](Source/Falcor/Core/API/ResourceViews.h:120)
```cpp
const ResourceViewInfo& getViewInfo() const
```
- **Thread Safety**: Not safe for concurrent calls
- **Time Complexity**: O(1) - Single view info retrieval
- **Space Complexity**: O(1) - No additional allocation

**Get Resource**: [`getResource()`](Source/Falcor/Core/API/ResourceViews.h:125)
```cpp
Resource* getResource() const
```
- **Thread Safety**: Not safe for concurrent calls
- **Time Complexity**: O(1) - Single resource retrieval
- **Space Complexity**: O(1) - No additional allocation

**Invalidate**: [`invalidate()`](Source/Falcor/Core/API/ResourceViews.h:130)
```cpp
void ResourceView::invalidate()
```
- **Thread Safety**: Not safe for concurrent calls
- **Time Complexity**: O(1) - Single invalidation
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: O(1) - Resource view release

## ShaderResourceView Class

### Memory Layout

```cpp
class ShaderResourceView : public ResourceView {
private:
    // Inherits all members from ResourceView
    // No additional members
};
// Total: ~56-64 bytes (excluding virtual table and base class)
```

### Alignment Analysis

**Total Estimated Size**: ~56-64 bytes (excluding virtual table and base class)

**Alignment Characteristics**:
- **Natural Alignment**: All members are naturally aligned (inherited from ResourceView)
- **Cache Line Alignment**: NOT cache-line aligned (64 bytes)
- **Padding Issues**: Inherited from ResourceView

### Threading Model

**NOT Thread-Safe**: The [`ShaderResourceView`](Source/Falcor/Core/API/ResourceViews.h:138) class is not thread-safe.

### Key Operations

**Create from Texture**: [`create()`](Source/Falcor/Core/API/ResourceViews.cpp:89)
```cpp
static ref<ShaderResourceView> ShaderResourceView::create(
    Device* pDevice,
    Texture* pTexture,
    uint32_t mostDetailedMip,
    uint32_t mipCount,
    uint32_t firstArraySlice,
    uint32_t arraySize
)
```
- **Thread Safety**: Not safe for concurrent create calls
- **Time Complexity**: O(1) - Single SRV creation
- **Space Complexity**: O(1) - No additional allocation (SRV is cached)
- **GPU Work**: O(1) - SRV creation

**Create from Buffer**: [`create()`](Source/Falcor/Core/API/ResourceViews.cpp:112)
```cpp
static ref<ShaderResourceView> ShaderResourceView::create(Device* pDevice, Buffer* pBuffer, uint64_t offset, uint64_t size)
```
- **Thread Safety**: Not safe for concurrent create calls
- **Time Complexity**: O(1) - Single SRV creation
- **Space Complexity**: O(1) - No additional allocation (SRV is cached)
- **GPU Work**: O(1) - SRV creation

**Create Null View**: [`create()`](Source/Falcor/Core/API/ResourceViews.cpp:125)
```cpp
static ref<ShaderResourceView> ShaderResourceView::create(Device* pDevice, Dimension dimension)
```
- **Thread Safety**: Not safe for concurrent create calls
- **Time Complexity**: O(1) - Single null SRV creation
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: O(1) - Null SRV creation

## DepthStencilView Class

### Memory Layout

```cpp
class DepthStencilView : public ResourceView {
private:
    // Inherits all members from ResourceView
    // No additional members
};
// Total: ~56-64 bytes (excluding virtual table and base class)
```

### Alignment Analysis

**Total Estimated Size**: ~56-64 bytes (excluding virtual table and base class)

**Alignment Characteristics**:
- **Natural Alignment**: All members are naturally aligned (inherited from ResourceView)
- **Cache Line Alignment**: NOT cache-line aligned (64 bytes)
- **Padding Issues**: Inherited from ResourceView

### Threading Model

**NOT Thread-Safe**: The [`DepthStencilView`](Source/Falcor/Core/API/ResourceViews.h:178) class is not thread-safe.

### Key Operations

**Create from Texture**: [`create()`](Source/Falcor/Core/API/ResourceViews.cpp:131)
```cpp
static ref<DepthStencilView> DepthStencilView::create(
    Device* pDevice,
    Texture* pTexture,
    uint32_t mipLevel,
    uint32_t firstArraySlice,
    uint32_t arraySize
)
```
- **Thread Safety**: Not safe for concurrent create calls
- **Time Complexity**: O(1) - Single DSV creation
- **Space Complexity**: O(1) - No additional allocation (DSV is cached)
- **GPU Work**: O(1) - DSV creation

**Create Null View**: [`create()`](Source/Falcor/Core/API/ResourceViews.cpp:154)
```cpp
static ref<DepthStencilView> DepthStencilView::create(Device* pDevice, Dimension dimension)
```
- **Thread Safety**: Not safe for concurrent create calls
- **Time Complexity**: O(1) - Single null DSV creation
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: O(1) - Null DSV creation

## UnorderedAccessView Class

### Memory Layout

```cpp
class UnorderedAccessView : public ResourceView {
private:
    // Inherits all members from ResourceView
    // No additional members
};
// Total: ~56-64 bytes (excluding virtual table and base class)
```

### Alignment Analysis

**Total Estimated Size**: ~56-64 bytes (excluding virtual table and base class)

**Alignment Characteristics**:
- **Natural Alignment**: All members are naturally aligned (inherited from ResourceView)
- **Cache Line Alignment**: NOT cache-line aligned (64 bytes)
- **Padding Issues**: Inherited from ResourceView

### Threading Model

**NOT Thread-Safe**: The [`UnorderedAccessView`](Source/Falcor/Core/API/ResourceViews.h:203) class is not thread-safe.

### Key Operations

**Create from Texture**: [`create()`](Source/Falcor/Core/API/ResourceViews.cpp:159)
```cpp
static ref<UnorderedAccessView> UnorderedAccessView::create(
    Device* pDevice,
    Texture* pTexture,
    uint32_t mipLevel,
    uint32_t firstArraySlice,
    uint32_t arraySize
)
```
- **Thread Safety**: Not safe for concurrent create calls
- **Time Complexity**: O(1) - Single UAV creation
- **Space Complexity**: O(1) - No additional allocation (UAV is cached)
- **GPU Work**: O(1) - UAV creation

**Create from Buffer**: [`create()`](Source/Falcor/Core/API/ResourceViews.cpp:180)
```cpp
static ref<UnorderedAccessView> UnorderedAccessView::create(Device* pDevice, Buffer* pBuffer, uint64_t offset, uint64_t size)
```
- **Thread Safety**: Not safe for concurrent create calls
- **Time Complexity**: O(1) - Single UAV creation
- **Space Complexity**: O(1) - No additional allocation (UAV is cached)
- **GPU Work**: O(1) - UAV creation

**Create Null View**: [`create()`](Source/Falcor/Core/API/ResourceViews.cpp:197)
```cpp
static ref<UnorderedAccessView> UnorderedAccessView::create(Device* pDevice, Dimension dimension)
```
- **Thread Safety**: Not safe for concurrent create calls
- **Time Complexity**: O(1) - Single null UAV creation
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: O(1) - Null UAV creation

## RenderTargetView Class

### Memory Layout

```cpp
class RenderTargetView : public ResourceView {
private:
    // Inherits all members from ResourceView
    // No additional members
};
// Total: ~56-64 bytes (excluding virtual table and base class)
```

### Alignment Analysis

**Total Estimated Size**: ~56-64 bytes (excluding virtual table and base class)

**Alignment Characteristics**:
- **Natural Alignment**: All members are naturally aligned (inherited from ResourceView)
- **Cache Line Alignment**: NOT cache-line aligned (64 bytes)
- **Padding Issues**: Inherited from ResourceView

### Threading Model

**NOT Thread-Safe**: The [`RenderTargetView`](Source/Falcor/Core/API/ResourceViews.h:239) class is not thread-safe.

### Key Operations

**Create from Texture**: [`create()`](Source/Falcor/Core/API/ResourceViews.cpp:202)
```cpp
static ref<RenderTargetView> RenderTargetView::create(
    Device* pDevice,
    Texture* pTexture,
    uint32_t mipLevel,
    uint32_t firstArraySlice,
    uint32_t arraySize
)
```
- **Thread Safety**: Not safe for concurrent create calls
- **Time Complexity**: O(1) - Single RTV creation
- **Space Complexity**: O(1) - No additional allocation (RTV is cached)
- **GPU Work**: O(1) - RTV creation

**Create Null View**: [`create()`](Source/Falcor/Core/API/ResourceViews.cpp:250)
```cpp
static ref<RenderTargetView> RenderTargetView::create(Device* pDevice, Dimension dimension)
```
- **Thread Safety**: Not safe for concurrent create calls
- **Time Complexity**: O(1) - Single null RTV creation
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: O(1) - Null RTV creation

## Performance Characteristics

### Memory Allocation Patterns

**View Creation**:
- Resource view allocation
- All views are cached and reused
- No per-call allocation (views are cached)

**View Access**:
- No per-call allocation (views are cached)
- Minimal memory overhead per operation

### Hot Path Analysis

**View Creation**:
1. SRV creation: O(1), 1 cache line access
2. DSV creation: O(1), 1 cache line access
3. UAV creation: O(1), 1 cache line access
4. RTV creation: O(1), 1 cache line access
5. **Optimization**: View caching reduces GPU overhead

**View Access**:
1. [`getNativeHandle()`](Source/Falcor/Core/API/ResourceViews.cpp:41): O(1), 1 cache line access
2. [`getViewInfo()`](Source/Falcor/Core/API/ResourceViews.h:120): O(1), 1 cache line access
3. [`getResource()`](Source/Falcor/Core/API/ResourceViews.h:125): O(1), 1 cache line access
4. **Optimization**: Direct access to cached members

### Memory Bandwidth

**View Creation**:
- GPU work: O(1) - View creation
- Memory bandwidth: Minimal (views are cached)

**View Access**:
- GPU work: O(1) - Native handle retrieval
- Memory bandwidth: Minimal (native handle is cached)

## Critical Path Analysis

### Hot Paths

1. **View Creation**: [`ShaderResourceView::create()`](Source/Falcor/Core/API/ResourceViews.cpp:89), [`DepthStencilView::create()`](Source/Falcor/Core/API/ResourceViews.cpp:131), [`UnorderedAccessView::create()`](Source/Falcor/Core/API/ResourceViews.cpp:159), [`RenderTargetView::create()`](Source/Falcor/Core/API/ResourceViews.cpp:202)
   - Called frequently for resource view creation
   - O(1) complexity
   - **Optimization**: View caching reduces GPU overhead

2. **View Access**: [`getNativeHandle()`](Source/Falcor/Core/API/ResourceViews.cpp:41), [`getViewInfo()`](Source/Falcor/Core/API/ResourceViews.h:120), [`getResource()`](Source/Falcor/Core/API/ResourceViews.h:125)
   - Called frequently for resource view access
   - O(1) complexity
   - **Optimization**: Direct access to cached members

3. **View Invalidation**: [`invalidate()`](Source/Falcor/Core/API/ResourceViews.h:130)
   - Called frequently for resource view invalidation
   - O(1) complexity
   - **Optimization**: Direct release of GPU resource view

### Bottlenecks

1. **No Thread Safety**: Mutable state not protected
   - **Mitigation**: Add atomic operations or mutexes

2. **View Creation Overhead**: View creation on first access
   - **Mitigation**: Pre-create views during initialization

3. **No View Pooling**: No support for view pooling
   - **Mitigation**: Use view pooling to reduce view creation overhead

## Platform-Specific Considerations

### DirectX 12

**Resource Views**:
- D3D12 shader resource views, depth-stencil views, unordered access views, and render target views
- Direct mapping to D3D12 views
- Native handle: D3D12_CPU_DESCRIPTOR_HANDLE

**Buffer Views**:
- D3D12 buffer views for typed and untyped buffers
- Direct mapping to D3D12 buffer views

**Texture Views**:
- D3D12 texture views for various texture types
- Direct mapping to D3D12 texture views

### Vulkan

**Resource Views**:
- Vulkan shader resource views, depth-stencil views, unordered access views, and render target views
- Direct mapping to Vulkan views
- Native handle: VkImageView for texture views, VkBufferView for typed buffer views, VkBuffer for untyped buffer views

**Buffer Views**:
- Vulkan buffer views for typed and untyped buffers
- Direct mapping to Vulkan buffer views

**Texture Views**:
- Vulkan image views for various texture types
- Direct mapping to Vulkan image views

## Summary

### Strengths

1. **Simple Design**: Minimal member footprint (~56-64 bytes per view)
2. **Efficient Caching**: Views are cached and reused
3. **Multiple View Types**: Support for SRV, DSV, UAV, and RTV
4. **Flexible View Creation**: Support for texture and buffer views
5. **Null View Support**: Support for null views
6. **Cross-Platform**: Unified interface for D3D12 and Vulkan
7. **Good Cache Locality**: All members fit in single cache line
8. **Lazy Initialization**: Views created on-demand

### Weaknesses

1. **Not Thread-Safe**: No synchronization on mutable state
2. **View Creation Overhead**: View creation on first access
3. **No View Pooling**: No support for view pooling
4. **Limited Validation**: Minimal validation for view operations
5. **No Batch Operations**: No support for batch view creation

### Optimization Recommendations

1. **Add Thread Safety**: Protect mutable state with atomic operations or mutexes
2. **Pre-create Views**: Pre-create views during initialization
3. **View Pooling**: Use view pooling to reduce view creation overhead
4. **Add Validation**: Add validation for view operations
5. **Batch View Creation**: Support batch view creation to reduce overhead
6. **Improve Cache Locality**: Align to cache line boundary for multi-threaded scenarios

---

*This technical specification is derived solely from static analysis of provided source code files.*
