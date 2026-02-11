# DepthStencilState

## Overview
[`DepthStencilState`](Source/Falcor/Core/API/DepthStencilState.h:39) is a graphics pipeline state object that encapsulates depth testing and stencil testing configuration for rendering operations. It provides a builder pattern for configuring depth and stencil state with support for front-facing and back-facing primitives.

## Source Files
- Header: [`Source/Falcor/Core/API/DepthStencilState.h`](Source/Falcor/Core/API/DepthStencilState.h) (223 lines)
- Implementation: [`Source/Falcor/Core/API/DepthStencilState.cpp`](Source/Falcor/Core/API/DepthStencilState.cpp) (98 lines)

## Class Hierarchy
```
Object (base class)
  └── DepthStencilState
```

## Dependencies

### Direct Dependencies
- [`Types.h`](Source/Falcor/Core/API/Types.h) - ComparisonFunc enum
- [`Handles.h`](Source/Falcor/Core/API/Handles.h) - Handle types
- [`Core/Macros.h`](Source/Falcor/Core/Macros.h) - Macros
- [`Core/Object.h`](Source/Falcor/Core/Object.h) - Base class
- [`Core/Error.h`](Source/Falcor/Core/Error.h) - Error handling (implementation)
- [`Core/ObjectPython.h`](Source/Falcor/Core/ObjectPython.h) - Python bindings (implementation)
- [`Utils/Scripting/ScriptBindings.h`](Source/Falcor/Utils/Scripting/ScriptBindings.h) - Script bindings (implementation)

### Indirect Dependencies
- ComparisonFunc enum (from Types.h)
- Object base class (from Core/Object.h)

## Known Issues and Limitations

1. **Thread Safety**: Not thread-safe for concurrent modification of Desc
2. **No Caching**: DepthStencilState instances are not cached; each creation constructs a new object
3. **No Validation**: No validation of stencil mask values (must be 0-255)
4. **No Default Objects**: No static default state objects (unlike BlendState, RasterizerState, and DepthStencilState in GraphicsStateObject)

## Usage Example

```cpp
// Create a depth-stencil state object
DepthStencilState::Desc desc = {};
desc.setDepthEnabled(true)
    .setDepthFunc(ComparisonFunc::Less)
    .setDepthWriteMask(true)
    .setStencilEnabled(true)
    .setStencilWriteMask(0xFF)
    .setStencilReadMask(0xFF)
    .setStencilRef(0x01)
    .setStencilFunc(Face::FrontAndBack, ComparisonFunc::Always)
    .setStencilOp(Face::FrontAndBack,
                  DepthStencilState::StencilOp::Keep,
                  DepthStencilState::StencilOp::Keep,
                  DepthStencilState::StencilOp::Replace);

ref<DepthStencilState> depthStencilState = DepthStencilState::create(desc);

// Use the depth-stencil state object
bool depthTestEnabled = depthStencilState->isDepthTestEnabled();
bool depthWriteEnabled = depthStencilState->isDepthWriteEnabled();
ComparisonFunc depthFunc = depthStencilState->getDepthFunc();
bool stencilTestEnabled = depthStencilState->isStencilTestEnabled();
uint8_t stencilReadMask = depthStencilState->getStencilReadMask();
uint8_t stencilWriteMask = depthStencilState->getStencilWriteMask();
uint8_t stencilRef = depthStencilState->getStencilRef();
const DepthStencilState::StencilDesc& stencilDesc = depthStencilState->getStencilDesc(DepthStencilState::Face::Front);
```

## Comparison with Other State Objects

### Similarities with BlendState
- Both use builder pattern for configuration
- Both are immutable after construction
- Both have Python bindings
- Both are used in GraphicsStateObject

### Differences from BlendState
- DepthStencilState has simpler structure (no per-render-target configuration)
- DepthStencilState has face-specific configuration (front, back, or both)
- DepthStencilState has fewer configuration options

### Similarities with RasterizerState
- Both are immutable after construction
- Both have Python bindings
- Both are used in GraphicsStateObject

### Differences from RasterizerState
- DepthStencilState uses builder pattern, RasterizerState does not
- DepthStencilState has face-specific configuration, RasterizerState does not

## Conclusion

DepthStencilState provides a comprehensive and efficient abstraction for depth and stencil testing configuration. The implementation is straightforward with excellent cache locality and minimal overhead.

**Strengths**:
- Clean builder pattern for configuration
- Excellent cache locality (all members fit in a single cache line)
- Lightweight construction and access
- Immutable after construction (thread-safe for reads)
- Face-specific configuration support
- Python bindings

**Weaknesses**:
- Not thread-safe for concurrent modification of Desc
- No caching of DepthStencilState instances
- No validation of stencil mask values
- No static default state objects

**Recommendations**:
1. Add validation for stencil mask values (must be 0-255)
2. Consider caching DepthStencilState instances based on descriptor
3. Add thread safety for concurrent modification of Desc if needed
4. Consider adding static default state objects for common configurations
5. Consider using bit fields for bool members to reduce memory footprint
