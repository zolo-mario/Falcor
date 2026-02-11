# CopyContext - Resource Copy Operations Context

## File Location
- Header: [`Source/Falcor/Core/API/CopyContext.h`](Source/Falcor/Core/API/CopyContext.h:1)
- Implementation: [`Source/Falcor/Core/API/CopyContext.cpp`](Source/Falcor/Core/API/CopyContext.cpp:1)

## State Machine Status
**Analysis Phase**: Complete
**Last Updated**: 2026-01-07T16:16:46.165Z
**Parent Node**: API
**Current Node**: CopyContext

## Class Overview

The [`CopyContext`](Source/Falcor/Core/API/CopyContext.h:49) class provides resource copying operations between textures, buffers, and subresources. It serves as a specialized context for copy operations, extending the base functionality provided by [`CopyContext`](Source/Falcor/Core/API/CopyContext.h:49).

### Inheritance Hierarchy
```
Object (ref-counted base)
  └── CopyContext
        ├── ComputeContext (base)
        └── RenderContext (base)
```

### Key Responsibilities
- **Texture Copying**: Copy entire textures or subresources
- **Buffer Copying**: Copy buffer regions or entire buffers
- **Subresource Copying**: Copy specific subresources with index control
- **Region Copying**: Copy texture regions with offset and size control
- **Texture Data Updates**: Update texture subresource data directly
- **Buffer Data Updates**: Update buffer data directly
- **Async Texture Read**: Asynchronous texture reading with fence synchronization
- **Fence Management**: Signal and wait operations for synchronization
- **Descriptor Management**: Bind descriptor heaps for direct GPU access
- **Aftermath Markers**: Add profiling markers to command lists

## Memory Layout and Alignment

### Class Member Layout

```cpp
class CopyContext : public CopyContext {
private:
    const Profiler* mpProfiler;                    // 8 bytes (ptr)
    Device* mpDevice;                           // 8 bytes (ptr)
    std::unique_ptr<LowLevelContextData> mpLowLevelData;  // 8 bytes (ptr)
    bool mCommandsPending = false;              // 1 byte
};
```

**Note**: Inherits all members from [`CopyContext`](Source/Falcor/Core/API/CopyContext.h:49) and [`ComputeContext`](Source/Falcor/Core/API/ComputeContext.h:43) (not shown here)

### Memory Alignment Analysis

**Total Estimated Size**: ~25 bytes (excluding virtual table, base classes, and low-level context)

**Alignment Characteristics**:
- **Natural Alignment**: All members are naturally aligned (8-byte pointers, 1-byte bool)
- **Cache Line Alignment**: NOT cache-line aligned (64 bytes)
- **Padding Issues**: None (all members naturally aligned)

**Hot Path Members**:
- [`mpProfiler`](Source/Falcor/Core/API/CopyContext.h:92): Accessed for profiling operations
- [`mpDevice`](Source/Falcor/Core/API/CopyContext.h:272): Accessed for GPU operations
- [`mpLowLevelData`](Source/Falcor/Core/API/CopyContext.h:273): Accessed for low-level context operations
- [`mCommandsPending`](Source/Falcor/Core/API/CopyContext.h:274): Accessed for command tracking

### Cache Locality Assessment

**Good Cache Locality**:
- All members fit in single cache line (25 bytes < 64 bytes)
- Hot path members are grouped together
- Good for single-threaded scenarios

**Optimization Opportunities**:
1. Align to cache line boundary for multi-threaded scenarios
2. Separate hot path from cold path (profiler could be separate allocation)

## Threading Model

### Thread Safety Analysis

**NOT Thread-Safe**: The [`CopyContext`](Source/Falcor/Core/API/CopyContext.h:49) class is not thread-safe.

### Concurrent Access Patterns

**Single-Threaded Design**:
- Copy operations assume single-threaded access
- Fence operations are not thread-safe
- Async read operations are not thread-safe

### Mutable State

**Mutable Members**:
```cpp
bool mCommandsPending = false;
```

**Thread Safety Implications**:
- [`mCommandsPending`](Source/Falcor/Core/API/CopyContext.h:274) is mutable and can be modified from const methods
- No synchronization primitives protect this mutable member
- **Race Conditions**: Multiple threads calling copy operations will corrupt state

### Fence Management

**Signal**: [`signal()`](Source/Falcor/Core/API/CopyContext.h:106)
```cpp
uint64_t CopyContext::signal(Fence* pFence, uint64_t value)
```
- **Thread Safety**: Not safe for concurrent signal calls
- **GPU Synchronization**: Updates fence value and executes command buffers
- **Side Effect**: Sets [`mCommandsPending`](Source/Falcor/Core/API/CopyContext.h:274) to true

**Wait**: [`wait()`](Source/Falcor/Core/API/CopyContext.h:115)
```cpp
void CopyContext::wait(Fence* pFence, uint64_t value)
```
- **Thread Safety**: Not safe for concurrent wait calls
- **GPU Synchronization**: Waits on device queue until fence reaches value
- **Side Effect**: Sets [`mCommandsPending`](Source/Falcor/Core/API/CopyContext.h:274) to false

### Threading Model Summary

| Operation | Thread Safety | Synchronization |
|-----------|--------------|----------------|
| Resource Copying | Single-threaded | None |
| Buffer Copying | Single-threaded | None |
| Subresource Copying | Single-threaded | None |
| Region Copying | Single-threaded | None |
| Texture Data Updates | Single-threaded | None |
| Buffer Data Updates | Single-threaded | None |
| Async Texture Read | Single-threaded | None |
| Fence Management | Single-threaded | Fence-based |
| Command Submission | Single-threaded | Fence-based |

## Algorithmic Complexity

### Resource Copying Operations

**Copy Resource**: [`copyResource()`](Source/Falcor/Core/API/CopyContext.cpp:488)
```cpp
void CopyContext::copyResource(const Resource* pDst, const Resource* pSrc)
```
- **Time Complexity**: O(1) - Single copy call
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: O(n) where n = resource size in bytes (GPU-side copy)

**Copy Subresource**: [`copySubresource()`](Source/Falcor/Core/API/CopyContext.cpp:156)
```cpp
void CopyContext::copySubresource(const Texture* pDst, uint32_t dstSubresourceIdx, const Texture* pSrc, uint32_t srcSubresourceIdx)
```
- **Time Complexity**: O(1) - Single copy call
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: O(n) where n = subresource size in bytes (GPU-side copy)

**Copy Region**: [`copySubresourceRegion()`](Source/Falcor/Core/API/CopyContext.cpp:589)
```cpp
void CopyContext::copySubresourceRegion(
    const Texture* pDst,
    uint32_t dstSubresourceIdx,
    const Texture* pSrc,
    uint32_t srcSubresourceIdx,
    const uint3& dstOffset,
    const uint3& srcOffset,
    const uint3& size
)
```
- **Time Complexity**: O(1) - Single copy call
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: O(width × height) where width, height from size (GPU-side copy)

### Buffer Copying Operations

**Copy Buffer**: [`copyBuffer()`](Source/Falcor/Core/API/CopyContext.cpp:549)
```cpp
void CopyContext::copyBuffer(const Buffer* pDst, const Buffer* pSrc)
```
- **Time Complexity**: O(1) - Single copy call
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: O(n) where n = buffer size in bytes (GPU-side copy)

**Copy Buffer Region**: [`copyBufferRegion()`](Source/Falcor/Core/API/CopyContext.cpp:577)
```cpp
void CopyContext::copyBufferRegion(const Buffer* pDst, uint64_t dstOffset, const Buffer* pSrc, uint64_t srcOffset, uint64_t numBytes)
```
- **Time Complexity**: O(1) - Single copy call
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: O(n) where n = numBytes (GPU-side copy)

### Texture Data Updates

**Update Texture Data**: [`updateTextureData()`](Source/Falcor/Core/API/CopyContext.cpp:199)
```cpp
void CopyContext::updateTextureData(const Texture* pTexture, const void* pData, const uint3& offset, const uint3& size)
```
- **Time Complexity**: O(1) - Single update call
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: O(n) where n = size in bytes (GPU-side update)

**Update Buffer Data**: [`updateBuffer()`](Source/Falcor/Core/API/CopyContext.cpp:532)
```cpp
void CopyContext::updateBuffer(const Buffer* pBuffer, const void* pData, size_t offset, size_t numBytes)
```
- **Time Complexity**: O(1) - Single update call
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: O(n) where n = numBytes (GPU-side update)

### Async Texture Read Operations

**Read Texture**: [`ReadTextureTask`](Source/Falcor/Core/API/CopyContext.h:52)
```cpp
class ReadTextureTask {
    using SharedPtr = std::shared_ptr<ReadTextureTask>;
    static SharedPtr create(CopyContext* pCtx, const Texture* pTexture, uint32_t subresourceIndex);
    void getData(void* pData, size_t size) const;
    std::vector<uint8_t> getData() const;
};
```

**Async Read Subresource**: [`asyncReadTextureSubresource()`](Source/Falcor/Core/API/CopyContext.cpp:226)
```cpp
ReadTextureTask::SharedPtr CopyContext::asyncReadTextureSubresource(const Texture* pTexture, uint32_t subresourceIndex)
```
- **Time Complexity**: O(1) - Single task creation
- **Space Complexity**: O(1) - No additional allocation (uses shared_ptr)
- **GPU Work**: O(n) where n = subresource size in bytes (GPU-side copy)
- **Synchronization**: Uses fence for completion notification

**Read Texture Data**: [`readTextureSubresource()`](Source/Falcor/Core/API/CopyContext.cpp:221)
```cpp
std::vector<uint8_t> CopyContext::readTextureSubresource(const Texture* pTexture, uint32_t subresourceIndex)
```
- **Time Complexity**: O(n) where n = subresource size in bytes
- **Space Complexity**: O(n) - Vector allocation
- **GPU Work**: O(n) where n = subresource size in bytes (GPU-side read)

### Fence Management

**Signal**: [`signal()`](Source/Falcor/Core/API/CopyContext.cpp:92)
```cpp
uint64_t CopyContext::signal(Fence* pFence, uint64_t value)
```
- **Time Complexity**: O(1) - Single signal call
- **Space Complexity**: O(1) - No additional allocation
- **GPU Synchronization**: Updates fence value and executes command buffers

**Wait**: [`wait()`](Source/Falcor/Core/API/CopyContext.cpp:101)
```cpp
void CopyContext::wait(Fence* pFence, uint64_t value)
```
- **Time Complexity**: O(1) - Single wait call
- **Space Complexity**: O(1) - No additional allocation
- **GPU Synchronization**: Waits on device queue until fence reaches value

### Descriptor Management

**Bind Descriptor Heaps**: [`bindDescriptorHeaps()`](Source/Falcor/Core/API/CopyContext.cpp:235)
```cpp
void CopyContext::bindDescriptorHeaps()
```
- **Time Complexity**: O(1) - Single bind call
- **Space Complexity**: O(1) - No additional allocation
- **GPU Synchronization**: Binds descriptor heaps for direct GPU access

**Bind Custom GPU Descriptor Pool**: [`bindCustomGPUDescriptorPool()`](Source/Falcor/Core/API/CopyContext.cpp:243)
```cpp
void CopyContext::bindCustomGPUDescriptorPool()
```
- **Time Complexity**: O(1) - Single bind call
- **Space Complexity**: O(1) - No additional allocation
- **GPU Synchronization**: Binds custom GPU descriptor pool for direct GPU access

**Unbind Custom GPU Descriptor Pool**: [`unbindCustomGPUDescriptorPool()`](Source/Falcor/Core/API/CopyContext.cpp:245)
```cpp
void CopyContext::unbindCustomGPUDescriptorPool()
```
- **Time Complexity**: O(1) - Single unbind call
- **Space Complexity**: O(1) - No additional allocation
- **GPU Synchronization**: Unbinds custom GPU descriptor pool

### Command Submission

**Submit**: [`submit()`](Source/Falcor/Core/API/CopyContext.cpp:70)
```cpp
void CopyContext::submit(bool wait)
```
- **Time Complexity**: O(1) - Single submit call
- **Space Complexity**: O(1) - No additional allocation
- **GPU Synchronization**: Waits for GPU completion if `wait` is true

### Aftermath Markers

**Add Marker**: [`addAftermathMarker()`](Source/Falcor/Core/API/CopyContext.cpp:638)
```cpp
void CopyContext::addAftermathMarker(std::string_view name)
```
- **Time Complexity**: O(1) - Single marker addition
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: Minimal (marker metadata only)

## Performance Characteristics

### Memory Allocation Patterns

**Command Recording**:
- No per-call allocation (all operations use stack-allocated structures)
- Encoder objects managed by low-level context
- Minimal memory overhead per operation

**Task-Based Operations**:
- [`ReadTextureTask`](Source/Falcor/Core/API/CopyContext.h:52): Uses shared_ptr for task lifecycle
- Allocation: O(1) per task
- No dynamic allocation during operation

### Hot Path Analysis

**Resource Copying**:
1. [`copyResource()`](Source/Falcor/Core/API/CopyContext.cpp:488): O(1), 1 cache line access
2. GPU work: O(n) where n = resource size
3. **Optimization**: Direct GPU-to-GPU copy

**Buffer Copying**:
1. [`copyBuffer()`](Source/Falcor/Core/API/CopyContext.cpp:549): O(1), 1 cache line access
2. GPU work: O(n) where n = buffer size
3. **Optimization**: Direct GPU-to-GPU copy

**Texture Data Updates**:
1. [`updateTextureData()`](Source/Falcor/Core/API/CopyContext.cpp:199): O(1), 1 cache line access
2. GPU work: O(n) where n = size in bytes

**Async Operations**:
1. [`asyncReadTextureSubresource()`](Source/Falcor/Core/API/CopyContext.cpp:226): O(1), shared_ptr overhead
2. GPU work: O(n) where n = subresource size
3. Fence synchronization for completion notification

**Fence Management**:
1. [`signal()`](Source/Falcor/Core/API/CopyContext.cpp:92): O(1), 1 cache line access
2. GPU synchronization via fence
3. [`wait()`](Source/Falcor/Core/API/CopyContext.cpp:101): O(1), 1 cache line access
4. Device queue wait for completion

### Memory Bandwidth

**Resource Copying**:
- GPU work: O(n) where n = resource size in bytes
- Memory bandwidth: Linear in resource size

**Buffer Copying**:
- GPU work: O(n) where n = buffer size in bytes
- Memory bandwidth: Linear in buffer size

**Texture Data Updates**:
- GPU work: O(n) where n = size in bytes
- Memory bandwidth: Linear in size

**Async Texture Read**:
- GPU work: O(n) where n = subresource size in bytes
- Memory bandwidth: O(n) for GPU copy + O(n) for CPU read

## Critical Path Analysis

### Hot Paths

1. **Resource Copying**: [`copyResource()`](Source/Falcor/Core/API/CopyContext.cpp:488), [`copySubresource()`](Source/Falcor/Core/API/CopyContext.cpp:156)
   - Called frequently for texture and buffer copying
   - O(1) complexity
   - **Optimization**: Direct GPU-to-GPU copy

2. **Buffer Copying**: [`copyBuffer()`](Source/Falcor/Core/API/CopyContext.cpp:549), [`copyBufferRegion()`](Source/Falcor/Core/API/CopyContext.cpp:577)
   - Called for buffer copying operations
   - O(1) complexity
   - **Optimization**: Direct GPU-to-GPU copy

3. **Texture Data Updates**: [`updateTextureData()`](Source/Falcor/Core/API/CopyContext.cpp:199), [`updateBuffer()`](Source/Falcor/Core/API/CopyContext.cpp:532)
   - Called for data updates
   - O(1) complexity
   - GPU work: O(n) where n = size in bytes

4. **Async Operations**: [`asyncReadTextureSubresource()`](Source/Falcor/Core/API/CopyContext.cpp:226), [`readTextureSubresource()`](Source/Falcor/Core/API/CopyContext.cpp:221)
   - Called for asynchronous texture reading
   - O(1) complexity
   - Fence synchronization for completion

5. **Fence Management**: [`signal()`](Source/Falcor/Core/API/CopyContext.cpp:92), [`wait()`](Source/Falcor/Core/API/CopyContext.cpp:101)
   - Called for synchronization
   - O(1) complexity
   - Device queue wait for completion

### Bottlenecks

1. **No Thread Safety**: Mutable state not protected
   - **Mitigation**: Add atomic operations or mutexes

2. **Async Overhead**: Task creation and shared_ptr management
   - **Mitigation**: Use object pools or custom allocators

3. **Buffer Copying**: No batched operations
   - **Mitigation**: Support batch buffer copies

4. **Region Copying**: Complex offset and size calculations
   - **Mitigation**: Pre-calculate offsets and sizes

## Memory Management

### Reference Counting

**Smart Pointer Usage**:
- All resources use `ref<T>` smart pointers
- Automatic reference counting via [`Object`](Source/Falcor/Core/Object.h:1) base class
- No manual memory management required

### Task Lifecycle

**ReadTextureTask**: [`ReadTextureTask`](Source/Falcor/Core/API/CopyContext.h:52)
- Uses `std::shared_ptr` for task lifecycle
- Automatic cleanup when task completes
- No reference counting overhead during operation

## Platform-Specific Considerations

### DirectX 12

**Command List Recording**:
- Uses low-level context encoder
- Direct mapping to D3D12 command list
- No additional abstraction overhead

**Resource Barriers**:
- Maps to D3D12 resource barriers
- Supports UAV barriers for optimized synchronization

**Descriptor Heaps**:
- D3D12 descriptor heap binding
- Custom GPU descriptor pool support

### Vulkan

**Command List Recording**:
- Uses low-level context encoder
- Direct mapping to Vulkan command buffer
- No additional abstraction overhead

**Resource Barriers**:
- Maps to Vulkan pipeline barriers
- Supports UAV barriers for optimized synchronization

**Descriptor Heaps**:
- Vulkan descriptor heap binding
- Custom GPU descriptor pool support

## Summary

### Strengths

1. **Simple Design**: Minimal member footprint (25 bytes)
2. **Efficient Resource Copying**: Direct GPU-to-GPU copies
3. **Async Operations**: Task-based asynchronous texture reading
4. **Fence Management**: Explicit fence-based synchronization
5. **Descriptor Management**: Direct GPU descriptor access support
6. **Aftermath Integration**: Profiling marker support
7. **Good Cache Locality**: All members fit in single cache line
8. **Cross-Platform**: Unified interface for D3D12 and Vulkan

### Weaknesses

1. **Not Thread-Safe**: No synchronization on mutable state
2. **No Command Batching**: Each operation submits immediately
3. **Async Overhead**: Task creation and shared_ptr management
4. **Region Copying Complexity**: Complex offset and size calculations
5. **Limited Validation**: Minimal validation for copy operations

### Optimization Recommendations

1. **Add Thread Safety**: Protect mutable state with atomic operations or mutexes
2. **Command List Batching**: Support command list batching to reduce submissions
3. **Object Pooling**: Use object pools for task management
4. **Pre-calculate Offsets**: Cache frequently used offset and size calculations
5. **Batch Resource Copies**: Support batch buffer copies
6. **Descriptor Set Caching**: Cache descriptor sets across operations

---

*This technical specification is derived solely from static analysis of provided source code files.*
