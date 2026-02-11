# GraphicsStateObject

## Overview
[`GraphicsStateObject`](Source/Falcor/Core/API/GraphicsStateObject.h:82) is a graphics pipeline state object that encapsulates all state required for graphics rendering operations. It provides a cross-platform abstraction for graphics pipeline states using the Slang gfx::IPipelineState interface, supporting both D3D12 and Vulkan backends.

## Source Files
- Header: [`Source/Falcor/Core/API/GraphicsStateObject.h`](Source/Falcor/Core/API/GraphicsStateObject.h) (111 lines)
- Implementation: [`Source/Falcor/Core/API/GraphicsStateObject.cpp`](Source/Falcor/Core/API/GraphicsStateObject.cpp) (426 lines)

## Class Hierarchy
```
Object (base class)
  └── GraphicsStateObject
```

## Dependencies

### Direct Dependencies
- [`Handles.h`](Source/Falcor/Core/API/Handles.h) - Handle types
- [`Core/Macros.h`](Source/Falcor/Core/Macros.h) - Macros
- [`Core/Object.h`](Source/Falcor/Core/Object.h) - Base class
- [`Core/API/VertexLayout.h`](Source/Falcor/Core/API/VertexLayout.h) - Vertex layout
- [`Core/API/FBO.h`](Source/Falcor/Core/API/FBO.h) - Framebuffer object
- [`Core/API/RasterizerState.h`](Source/Falcor/Core/API/RasterizerState.h) - Rasterizer state
- [`Core/API/DepthStencilState.h`](Source/Falcor/Core/API/DepthStencilState.h) - Depth-stencil state
- [`Core/API/BlendState.h`](Source/Falcor/Core/API/BlendState.h) - Blend state
- [`Core/Program/ProgramVersion.h`](Source/Falcor/Core/Program/ProgramVersion.h) - Program version
- [`Device.h`](Source/Falcor/Core/API/Device.h) - Device (implementation)
- [`GFXHelpers.h`](Source/Falcor/Core/API/GFXHelpers.h) - GFX helpers (implementation)
- [`GFXAPI.h`](Source/Falcor/Core/API/GFXAPI.h) - GFX API (implementation)

### Indirect Dependencies
- Slang gfx interfaces (gfx::IPipelineState, gfx::IInputLayout, gfx::IFramebufferLayout, gfx::IRenderPassLayout)
- Device implementation (D3D12Device or VulkanDevice)

## Known Issues and Limitations

1. **Thread Safety**: Not thread-safe for concurrent access
2. **Static Initialization**: Default state objects initialized without synchronization
3. **Cache Locality**: Poor cache locality due to scattered member layout
4. **Global State**: Static default state objects marked for removal (TODO: REMOVEGLOBAL)
5. **No Caching**: GraphicsStateObject instances are not cached; each creation constructs new gfx objects

## Usage Example

```cpp
// Create a graphics state object
GraphicsStateObjectDesc desc = {};
desc.fboDesc = Fbo::Desc();
desc.pVertexLayout = vertexLayout;
desc.pProgramKernels = programKernels;
desc.pRasterizerState = RasterizerState::create(rasterizerDesc);
desc.pDepthStencilState = DepthStencilState::create(depthStencilDesc);
desc.pBlendState = BlendState::create(blendDesc);
desc.sampleMask = GraphicsStateObjectDesc::kSampleMaskAll;
desc.primitiveType = GraphicsStateObjectDesc::PrimitiveType::Triangle;

ref<GraphicsStateObject> pso = GraphicsStateObject::create(pDevice, desc);

// Use the graphics state object
gfx::IPipelineState* pGfxPSO = pso->getGfxPipelineState();
const GraphicsStateObjectDesc& psoDesc = pso->getDesc();
gfx::IRenderPassLayout* pRenderPassLayout = pso->getGFXRenderPassLayout();
```

## Conclusion

GraphicsStateObject provides a comprehensive cross-platform abstraction for graphics pipeline states. The implementation is straightforward but has several performance and thread safety limitations:

**Strengths**:
- Clean cross-platform abstraction using Slang gfx interfaces
- Comprehensive state encapsulation (blend, depth-stencil, rasterizer, vertex layout, program)
- Automatic resource management using smart pointers
- BreakableReference pattern to avoid circular references

**Weaknesses**:
- Not thread-safe (no synchronization primitives)
- Poor cache locality (members scattered across cache lines)
- Static initialization without synchronization (potential race condition)
- No caching of GraphicsStateObject instances
- Heavyweight construction (creates multiple gfx objects)

**Recommendations**:
1. Add synchronization for static default state initialization
2. Reorder members to improve cache locality
3. Consider caching GraphicsStateObject instances based on descriptor
4. Remove global default state objects (as indicated by TODO comments)
5. Add thread safety for concurrent access if needed
