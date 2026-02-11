# BlitContext - Blit Operations Context

## File Location
- Header: [`Source/Falcor/Core/API/BlitContext.h`](Source/Falcor/Core/API/BlitContext.h:1)
- Implementation: [`Source/Falcor/Core/API/BlitContext.cpp`](Source/Falcor/Core/API/BlitContext.cpp:1)

## State Machine Status
**Analysis Phase**: Complete
**Last Updated**: 2026-01-07T16:21:40.250Z
**Parent Node**: API
**Current Node**: BlitContext

## Struct Overview

The [`BlitContext`](Source/Falcor/Core/API/BlitContext.h:40) struct provides blit operations using full-screen passes. It manages shader-based blitting with support for different filtering modes (linear, point) and reduction modes (standard, min, max).

### Key Responsibilities
- **Shader-Based Blitting**: Use full-screen passes for blitting operations
- **Sampler Management**: Manage multiple sampler states for different filtering and reduction modes
- **Parameter Block Management**: Manage shader parameter blocks for blit operations
- **Component Transformation**: Support complex blit with component transformation
- **FBO Management**: Manage framebuffer objects for blit operations

## Memory Layout and Alignment

### Struct Member Layout

```cpp
struct BlitContext {
    ref<FullScreenPass> pPass;                          // 8 bytes (ptr)
    ref<Fbo> pFbo;                                     // 8 bytes (ptr)

    ref<Sampler> pLinearSampler;                       // 8 bytes (ptr)
    ref<Sampler> pPointSampler;                        // 8 bytes (ptr)
    ref<Sampler> pLinearMinSampler;                     // 8 bytes (ptr)
    ref<Sampler> pPointMinSampler;                     // 8 bytes (ptr)
    ref<Sampler> pLinearMaxSampler;                     // 8 bytes (ptr)
    ref<Sampler> pPointMaxSampler;                     // 8 bytes (ptr)

    ref<ParameterBlock> pBlitParamsBuffer;             // 8 bytes (ptr)
    float2 prevSrcRectOffset = float2(0, 0);            // 8 bytes (2 floats)
    float2 prevSrcReftScale = float2(0, 0);            // 8 bytes (2 floats)

    // Variable offsets in constant buffer
    TypedShaderVarOffset offsetVarOffset;               // 4 bytes (uint32_t)
    TypedShaderVarOffset scaleVarOffset;                // 4 bytes (uint32_t)
    ProgramReflection::BindLocation texBindLoc;         // 4 bytes (uint32_t)

    // Parameters for complex blit
    float4 prevComponentsTransform[4];                 // 64 bytes (4 × 16 bytes)
    TypedShaderVarOffset compTransVarOffset[4];         // 16 bytes (4 × 4 bytes)
};
```

### Memory Alignment Analysis

**Total Estimated Size**: ~160 bytes

**Alignment Characteristics**:
- **Natural Alignment**: All members are naturally aligned (8-byte pointers, 4-byte offsets)
- **Cache Line Alignment**: NOT cache-line aligned (64 bytes)
- **Padding Issues**: Potential padding between members due to alignment requirements

**Hot Path Members**:
- [`pPass`](Source/Falcor/Core/API/BlitContext.h:42): Accessed for blit operations
- [`pFbo`](Source/Falcor/Core/API/BlitContext.h:43): Accessed for framebuffer operations
- [`pBlitParamsBuffer`](Source/Falcor/Core/API/BlitContext.h:52): Accessed for parameter updates
- [`offsetVarOffset`](Source/Falcor/Core/API/BlitContext.h:57): Accessed for offset updates
- [`scaleVarOffset`](Source/Falcor/Core/API/BlitContext.h:58): Accessed for scale updates
- [`texBindLoc`](Source/Falcor/Core/API/BlitContext.h:59): Accessed for texture binding
- [`prevSrcRectOffset`](Source/Falcor/Core/API/BlitContext.h:53): Accessed for offset caching
- [`prevSrcReftScale`](Source/Falcor/Core/API/BlitContext.h:54): Accessed for scale caching
- [`prevComponentsTransform`](Source/Falcor/Core/API/BlitContext.h:62): Accessed for component transformation caching
- [`compTransVarOffset`](Source/Falcor/Core/API/BlitContext.h:63): Accessed for component transformation updates

### Cache Locality Assessment

**Poor Cache Locality**:
- Members scattered across multiple cache lines (~160 bytes / 64 bytes = ~3 cache lines)
- Hot path members not grouped together
- Poor for multi-threaded scenarios

**Optimization Opportunities**:
1. Align to cache line boundary for multi-threaded scenarios
2. Group hot path members together
3. Separate hot path from cold path (samplers could be separate allocation)

**Cache Line Analysis**:
```
Cache Line 1 (bytes 0-63):
- pPass (8 bytes)
- pFbo (8 bytes)
- pLinearSampler (8 bytes)
- pPointSampler (8 bytes)
- pLinearMinSampler (8 bytes)
- pPointMinSampler (8 bytes)
- pLinearMaxSampler (8 bytes)
- pPointMaxSampler (8 bytes)

Cache Line 2 (bytes 64-127):
- pBlitParamsBuffer (8 bytes)
- prevSrcRectOffset (8 bytes)
- prevSrcReftScale (8 bytes)
- offsetVarOffset (4 bytes)
- scaleVarOffset (4 bytes)
- texBindLoc (4 bytes)
- Padding (4 bytes)
- prevComponentsTransform[0] (16 bytes)
- prevComponentsTransform[1] (16 bytes)

Cache Line 3 (bytes 128-191):
- prevComponentsTransform[2] (16 bytes)
- prevComponentsTransform[3] (16 bytes)
- compTransVarOffset[0] (4 bytes)
- compTransVarOffset[1] (4 bytes)
- compTransVarOffset[2] (4 bytes)
- compTransVarOffset[3] (4 bytes)
- Padding (112 bytes)
```

## Threading Model

### Thread Safety Analysis

**NOT Thread-Safe**: The [`BlitContext`](Source/Falcor/Core/API/BlitContext.h:40) struct is not thread-safe.

### Concurrent Access Patterns

**Single-Threaded Design**:
- Blit operations assume single-threaded access
- Parameter updates are not thread-safe
- Sampler management is not thread-safe

### Mutable State

**Mutable Members**:
```cpp
float2 prevSrcRectOffset = float2(0, 0);
float2 prevSrcReftScale = float2(0, 0);
float4 prevComponentsTransform[4] = {float4(0), float4(0), float4(0), float4(0)};
```

**Thread Safety Implications**:
- [`prevSrcRectOffset`](Source/Falcor/Core/API/BlitContext.h:53) is mutable and can be modified from const methods
- [`prevSrcReftScale`](Source/Falcor/Core/API/BlitContext.h:54) is mutable and can be modified from const methods
- [`prevComponentsTransform`](Source/Falcor/Core/API/BlitContext.h:62) is mutable and can be modified from const methods
- No synchronization primitives protect these mutable members
- **Race Conditions**: Multiple threads calling blit operations will corrupt state

### Initialization

**Constructor**: [`BlitContext()`](Source/Falcor/Core/API/BlitContext.cpp:36)
```cpp
BlitContext::BlitContext(Device* pDevice)
```
- **Thread Safety**: Not safe for concurrent initialization
- **Side Effect**: Creates full-screen pass, FBO, samplers, and parameter block
- **GPU Work**: Creates shader resources and samplers

### Threading Model Summary

| Operation | Thread Safety | Synchronization |
|-----------|--------------|----------------|
| Initialization | Single-threaded | None |
| Blit Operations | Single-threaded | None |
| Parameter Updates | Single-threaded | None |
| Sampler Management | Single-threaded | None |
| FBO Management | Single-threaded | None |

## Algorithmic Complexity

### Initialization

**Constructor**: [`BlitContext()`](Source/Falcor/Core/API/BlitContext.cpp:36)
```cpp
BlitContext::BlitContext(Device* pDevice)
```
- **Time Complexity**: O(1) - Constant time initialization
- **Space Complexity**: O(1) - No additional allocation (all resources are cached)
- **GPU Work**: O(1) - Shader compilation and resource creation

**Initialization Steps**:
1. Create full-screen pass with shader program
2. Create FBO for rendering
3. Get parameter block for blit parameters
4. Get variable offsets for offset and scale parameters
5. Create 6 samplers (linear, point, linear-min, point-min, linear-max, point-max)
6. Get texture binding location
7. Get component transformation variable offsets
8. Set initial component transformation values

### Parameter Updates

**Offset Update**: [`offsetVarOffset`](Source/Falcor/Core/API/BlitContext.h:57)
- **Time Complexity**: O(1) - Single offset update
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: O(1) - Parameter buffer update

**Scale Update**: [`scaleVarOffset`](Source/Falcor/Core/API/BlitContext.h:58)
- **Time Complexity**: O(1) - Single scale update
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: O(1) - Parameter buffer update

**Component Transformation Update**: [`compTransVarOffset`](Source/Falcor/Core/API/BlitContext.h:63)
- **Time Complexity**: O(1) - Single component transformation update
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: O(1) - Parameter buffer update

### Sampler Management

**Sampler Creation**: [`pLinearSampler`](Source/Falcor/Core/API/BlitContext.h:45), [`pPointSampler`](Source/Falcor/Core/API/BlitContext.h:46), [`pLinearMinSampler`](Source/Falcor/Core/API/BlitContext.h:47), [`pPointMinSampler`](Source/Falcor/Core/API/BlitContext.h:48), [`pLinearMaxSampler`](Source/Falcor/Core/API/BlitContext.h:49), [`pPointMaxSampler`](Source/Falcor/Core/API/BlitContext.h:50)
- **Time Complexity**: O(1) - Single sampler creation
- **Space Complexity**: O(1) - No additional allocation (sampler is cached)
- **GPU Work**: O(1) - Sampler state creation

**Sampler Types**:
1. **Linear Sampler**: Linear filtering for both min and mag
2. **Point Sampler**: Point filtering for both min and mag
3. **Linear Min Sampler**: Linear filtering with min reduction
4. **Point Min Sampler**: Point filtering with min reduction
5. **Linear Max Sampler**: Linear filtering with max reduction
6. **Point Max Sampler**: Point filtering with max reduction

### Parameter Block Management

**Parameter Block Creation**: [`pBlitParamsBuffer`](Source/Falcor/Core/API/BlitContext.h:52)
- **Time Complexity**: O(1) - Single parameter block creation
- **Space Complexity**: O(1) - No additional allocation (parameter block is cached)
- **GPU Work**: O(1) - Parameter buffer creation

**Variable Offset Retrieval**: [`offsetVarOffset`](Source/Falcor/Core/API/BlitContext.h:57), [`scaleVarOffset`](Source/Falcor/Core/API/BlitContext.h:58), [`compTransVarOffset`](Source/Falcor/Core/API/BlitContext.h:63)
- **Time Complexity**: O(1) - Single variable offset retrieval
- **Space Complexity**: O(1) - No additional allocation
- **GPU Work**: O(1) - No GPU work

### FBO Management

**FBO Creation**: [`pFbo`](Source/Falcor/Core/API/BlitContext.h:43)
- **Time Complexity**: O(1) - Single FBO creation
- **Space Complexity**: O(1) - No additional allocation (FBO is cached)
- **GPU Work**: O(1) - FBO creation

## Performance Characteristics

### Memory Allocation Patterns

**Initialization**:
- Full-screen pass allocation (shader compilation)
- FBO allocation (framebuffer object)
- 6 sampler allocations (sampler states)
- Parameter block allocation (constant buffer)
- All resources are cached and reused

**Blit Operations**:
- No per-call allocation (all resources are cached)
- Minimal memory overhead per operation

### Hot Path Analysis

**Initialization**:
1. [`BlitContext()`](Source/Falcor/Core/API/BlitContext.cpp:36): O(1), 3 cache line accesses
2. **Optimization**: All resources are cached and reused

**Parameter Updates**:
1. Offset update: O(1), 1 cache line access
2. Scale update: O(1), 1 cache line access
3. Component transformation update: O(1), 1 cache line access
4. **Optimization**: Parameter caching reduces GPU overhead

**Sampler Management**:
1. Sampler creation: O(1), 1 cache line access
2. **Optimization**: Samplers are cached and reused

**FBO Management**:
1. FBO creation: O(1), 1 cache line access
2. **Optimization**: FBO is cached and reused

### Memory Bandwidth

**Initialization**:
- GPU work: O(1) - Shader compilation and resource creation
- Memory bandwidth: Minimal (resources are cached)

**Blit Operations**:
- GPU work: O(n) where n = blit operation complexity
- Memory bandwidth: O(n) where n = blit operation complexity

**Parameter Updates**:
- GPU work: O(1) - Parameter buffer update
- Memory bandwidth: O(1) - Parameter buffer update

## Critical Path Analysis

### Hot Paths

1. **Initialization**: [`BlitContext()`](Source/Falcor/Core/API/BlitContext.cpp:36)
   - Called once during context creation
   - O(1) complexity
   - **Optimization**: All resources are cached and reused

2. **Parameter Updates**: [`offsetVarOffset`](Source/Falcor/Core/API/BlitContext.h:57), [`scaleVarOffset`](Source/Falcor/Core/API/BlitContext.h:58), [`compTransVarOffset`](Source/Falcor/Core/API/BlitContext.h:63)
   - Called frequently for blit operations
   - O(1) complexity
   - **Optimization**: Parameter caching reduces GPU overhead

3. **Sampler Management**: [`pLinearSampler`](Source/Falcor/Core/API/BlitContext.h:45), [`pPointSampler`](Source/Falcor/Core/API/BlitContext.h:46), [`pLinearMinSampler`](Source/Falcor/Core/API/BlitContext.h:47), [`pPointMinSampler`](Source/Falcor/Core/API/BlitContext.h:48), [`pLinearMaxSampler`](Source/Falcor/Core/API/BlitContext.h:49), [`pPointMaxSampler`](Source/Falcor/Core/API/BlitContext.h:50)
   - Called frequently for blit operations
   - O(1) complexity
   - **Optimization**: Samplers are cached and reused

4. **FBO Management**: [`pFbo`](Source/Falcor/Core/API/BlitContext.h:43)
   - Called frequently for blit operations
   - O(1) complexity
   - **Optimization**: FBO is cached and reused

### Bottlenecks

1. **No Thread Safety**: Mutable state not protected
   - **Mitigation**: Add atomic operations or mutexes

2. **Poor Cache Locality**: Members scattered across multiple cache lines
   - **Mitigation**: Group hot path members together and align to cache line boundary

3. **Initialization Overhead**: Shader compilation and resource creation
   - **Mitigation**: Pre-compile shaders and pre-create resources

4. **Parameter Update Overhead**: Parameter buffer updates
   - **Mitigation**: Cache parameter values and only update when changed

## Memory Management

### Reference Counting

**Smart Pointer Usage**:
- All resources use `ref<T>` smart pointers
- Automatic reference counting via [`Object`](Source/Falcor/Core/Object.h:1) base class
- No manual memory management required

### Resource Lifecycle

**Full-Screen Pass**:
- Created during initialization
- Cached and reused for all blit operations
- No per-call allocation

**FBO**:
- Created during initialization
- Cached and reused for all blit operations
- No per-call allocation

**Samplers**:
- 6 samplers created during initialization
- Cached and reused for all blit operations
- No per-call allocation

**Parameter Block**:
- Created during initialization
- Cached and reused for all blit operations
- No per-call allocation

## Platform-Specific Considerations

### DirectX 12

**Shader-Based Blitting**:
- Uses full-screen pass with HLSL shader
- Direct mapping to D3D12 shader compilation
- No additional abstraction overhead

**Sampler States**:
- D3D12 sampler state objects
- Direct mapping to D3D12 sampler states

**Parameter Blocks**:
- D3D12 constant buffers
- Direct mapping to D3D12 constant buffers

### Vulkan

**Shader-Based Blitting**:
- Uses full-screen pass with SPIR-V shader
- Direct mapping to Vulkan shader compilation
- No additional abstraction overhead

**Sampler States**:
- Vulkan sampler objects
- Direct mapping to Vulkan samplers

**Parameter Blocks**:
- Vulkan uniform buffers
- Direct mapping to Vulkan uniform buffers

## Summary

### Strengths

1. **Simple Design**: Minimal member footprint (~160 bytes)
2. **Efficient Resource Management**: All resources are cached and reused
3. **Multiple Sampler Modes**: Support for 6 different sampler modes
4. **Component Transformation**: Support for complex blit with component transformation
5. **Parameter Caching**: Parameter caching reduces GPU overhead
6. **Cross-Platform**: Unified interface for D3D12 and Vulkan
7. **Shader-Based Blitting**: Flexible shader-based blitting operations

### Weaknesses

1. **Not Thread-Safe**: No synchronization on mutable state
2. **Poor Cache Locality**: Members scattered across multiple cache lines
3. **Initialization Overhead**: Shader compilation and resource creation
4. **Parameter Update Overhead**: Parameter buffer updates
5. **Limited Validation**: Minimal validation for blit operations
6. **No Batch Operations**: No support for batch blit operations

### Optimization Recommendations

1. **Add Thread Safety**: Protect mutable state with atomic operations or mutexes
2. **Improve Cache Locality**: Group hot path members together and align to cache line boundary
3. **Pre-compile Shaders**: Pre-compile shaders and pre-create resources
4. **Cache Parameter Values**: Cache parameter values and only update when changed
5. **Batch Blit Operations**: Support batch blit operations to reduce overhead
6. **Add Validation**: Add validation for blit operations
7. **Sampler Pooling**: Use sampler pooling to reduce sampler creation overhead

---

*This technical specification is derived solely from static analysis of provided source code files.*
