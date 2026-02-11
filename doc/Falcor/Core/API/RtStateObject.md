# RtStateObject

## Overview
[`RtStateObject`](Source/Falcor/Core/API/RtStateObject.h:57) is a raytracing pipeline state object that encapsulates all state required for raytracing operations. It provides a cross-platform abstraction for raytracing pipeline states using the Slang gfx::IPipelineState interface, supporting both D3D12 and Vulkan backends.

## Source Files
- Header: [`Source/Falcor/Core/API/RtStateObject.h`](Source/Falcor/Core/API/RtStateObject.h) (77 lines)
- Implementation: [`Source/Falcor/Core/API/RtStateObject.cpp`](Source/Falcor/Core/API/RtStateObject.cpp) (88 lines)

## Class Hierarchy
```
Object (base class)
  └── RtStateObject
```

## Dependencies

### Direct Dependencies
- [`fwd.h`](Source/Falcor/Core/API/fwd.h) - Forward declarations
- [`Handles.h`](Source/Falcor/Core/API/Handles.h) - Handle types
- [`Raytracing.h`](Source/Falcor/Core/API/Raytracing.h) - Raytracing types
- [`Core/Macros.h`](Source/Falcor/Core/Macros.h) - Macros
- [`Core/Object.h`](Source/Falcor/Core/Object.h) - Base class
- [`Core/Program/ProgramVersion.h`](Source/Falcor/Core/Program/ProgramVersion.h) - Program version
- [`Device.h`](Source/Falcor/Core/API/Device.h) - Device (implementation)
- [`GFXAPI.h`](Source/Falcor/Core/API/GFXAPI.h) - GFX API (implementation)
- [`Core/Program/Program.h`](Source/Falcor/Core/Program/Program.h) - Program (implementation)

### Indirect Dependencies
- Object base class (from Core/Object.h)
- ProgramKernels (from Core/Program/ProgramVersion.h)
- EntryPointGroupKernels (from Core/Program/ProgramVersion.h)
- EntryPointKernel (from Core/Program/ProgramVersion.h)
- ShaderType enum (from Core/Program/ProgramVersion.h)
- Slang gfx interfaces (gfx::IPipelineState, gfx::RayTracingPipelineStateDesc, gfx::HitGroupDesc, gfx::RayTracingPipelineFlags)
- RtPipelineFlags enum (from Raytracing.h)

## Known Issues and Limitations

1. **Thread Safety**: Not thread-safe for concurrent access
2. **No Caching**: RtStateObject instances are not cached; each creation constructs new gfx objects
3. **No Validation**: No validation of max trace recursion depth value
4. **Vector Access**: `getShaderIdentifier()` performs unchecked vector access (no bounds checking)
5. **Dynamic Allocations**: Multiple dynamic allocations during construction (vectors and strings)
6. **No Static Default Objects**: No static default state objects

## Usage Example

```cpp
// Create a raytracing state object
RtStateObjectDesc desc = {};
desc.pProgramKernels = programKernels;
desc.maxTraceRecursionDepth = 2;
desc.pipelineFlags = RtPipelineFlags::None;

ref<RtStateObject> rtStateObject = RtStateObject::create(pDevice, desc);

// Use the raytracing state object
gfx::IPipelineState* pGfxPSO = rtStateObject->getGfxPipelineState();
const ref<const ProgramKernels>& kernels = rtStateObject->getKernels();
uint32_t maxRecursionDepth = rtStateObject->getMaxTraceRecursionDepth();
const void* shaderIdentifier = rtStateObject->getShaderIdentifier(0);
const RtStateObjectDesc& rtDesc = rtStateObject->getDesc();
```

## Comparison with Other State Objects

### Similarities with GraphicsStateObject
- Both use gfx::IPipelineState for cross-platform support
- Both encapsulate pipeline state in a single object
- Both have descriptor structs with equality operators
- Both are immutable after construction
- Both are used in rendering operations

### Differences from GraphicsStateObject
- RtStateObject is for raytracing, GraphicsStateObject is for graphics
- RtStateObject has simpler descriptor (3 fields vs 8 fields)
- RtStateObject has shader identifiers (entry point group names)
- RtStateObject has hit groups with optional shaders
- RtStateObject has max trace recursion depth
- RtStateObject has pipeline flags specific to raytracing

### Similarities with ComputeStateObject
- Both use gfx::IPipelineState for cross-platform support
- Both encapsulate pipeline state in a single object
- Both have descriptor structs
- Both are immutable after construction
- Both are used in compute-like operations

### Differences from ComputeStateObject
- RtStateObject is for raytracing, ComputeStateObject is for compute
- RtStateObject has more complex descriptor (3 fields vs 1 field)
- RtStateObject has shader identifiers (entry point group names)
- RtStateObject has hit groups with optional shaders
- RtStateObject has max trace recursion depth
- RtStateObject has pipeline flags specific to raytracing

## Conclusion

RtStateObject provides a comprehensive cross-platform abstraction for raytracing pipeline states. The implementation is straightforward but has several performance and thread safety limitations.

**Strengths**:
- Clean cross-platform abstraction using Slang gfx interfaces
- Comprehensive state encapsulation (program kernels, recursion depth, pipeline flags)
- Automatic resource management using smart pointers
- Support for hit groups with optional shaders
- Shader identifier management for shader table creation
- Immutable after construction (thread-safe for reads)

**Weaknesses**:
- Not thread-safe for concurrent access
- Poor cache locality (members scattered across cache lines)
- No caching of RtStateObject instances
- Heavyweight construction (multiple dynamic allocations)
- No validation of max trace recursion depth
- Unchecked vector access in `getShaderIdentifier()`
- No static default state objects

**Recommendations**:
1. Add bounds checking to `getShaderIdentifier()`
2. Add validation for max trace recursion depth value
3. Consider caching RtStateObject instances based on descriptor
4. Reorder members to improve cache locality
5. Consider pre-allocating vector capacity to avoid reallocations
6. Add thread safety for concurrent access if needed
7. Consider adding static default state objects for common configurations
