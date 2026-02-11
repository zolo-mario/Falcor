# RasterizerState

## Overview
[`RasterizerState`](Source/Falcor/Core/API/RasterizerState.h:40) is a graphics pipeline state object that encapsulates rasterizer configuration for rendering operations. It provides a builder pattern for configuring rasterization settings including culling mode, fill mode, depth bias, and various rasterization features.

## Source Files
- Header: [`Source/Falcor/Core/API/RasterizerState.h`](Source/Falcor/Core/API/RasterizerState.h) (244 lines)
- Implementation: [`Source/Falcor/Core/API/RasterizerState.cpp`](Source/Falcor/Core/API/RasterizerState.cpp) (46 lines)

## Class Hierarchy
```
Object (base class)
  └── RasterizerState
```

## Dependencies

### Direct Dependencies
- [`Handles.h`](Source/Falcor/Core/API/Handles.h) - Handle types
- [`Core/Macros.h`](Source/Falcor/Core/Macros.h) - Macros
- [`Core/Object.h`](Source/Falcor/Core/Object.h) - Base class
- [`Core/Enum.h`](Source/Falcor/Core/Enum.h) - Enum registration macros
- [`GFXAPI.h`](Source/Falcor/Core/API/GFXAPI.h) - GFX API (implementation)
- [`Core/ObjectPython.h`](Source/Falcor/Core/ObjectPython.h) - Python bindings (implementation)
- [`Utils/Scripting/ScriptBindings.h`](Source/Falcor/Utils/Scripting/ScriptBindings.h) - Script bindings (implementation)

### Indirect Dependencies
- Object base class (from Core/Object.h)
- Enum registration macros (from Core/Enum.h)

## Known Issues and Limitations

1. **Thread Safety**: Not thread-safe for concurrent modification of Desc
2. **No Caching**: RasterizerState instances are not cached; each creation constructs a new object
3. **No Validation**: No validation of forced sample count value
4. **No Default Objects**: No static default state objects (unlike BlendState, RasterizerState, and DepthStencilState in GraphicsStateObject)

## Usage Example

```cpp
// Create a rasterizer state object
RasterizerState::Desc desc = {};
desc.setCullMode(RasterizerState::CullMode::Back)
    .setFillMode(RasterizerState::FillMode::Solid)
    .setFrontCounterCW(true)
    .setDepthBias(0, 0.0f)
    .setDepthClamp(false)
    .setLineAntiAliasing(true)
    .setScissorTest(false)
    .setConservativeRasterization(false)
    .setForcedSampleCount(0);

ref<RasterizerState> rasterizerState = RasterizerState::create(desc);

// Use the rasterizer state object
RasterizerState::CullMode cullMode = rasterizerState->getCullMode();
RasterizerState::FillMode fillMode = rasterizerState->getFillMode();
bool isFrontCCW = rasterizerState->isFrontCounterCW();
float slopeScaledDepthBias = rasterizerState->getSlopeScaledDepthBias();
int32_t depthBias = rasterizerState->getDepthBias();
bool isDepthClampEnabled = rasterizerState->isDepthClampEnabled();
bool isScissorTestEnabled = rasterizerState->isScissorTestEnabled();
bool isLineAntiAliasingEnabled = rasterizerState->isLineAntiAliasingEnabled();
bool isConservativeRasterizationEnabled = rasterizerState->isConservativeRasterizationEnabled();
uint32_t forcedSampleCount = rasterizerState->getForcedSampleCount();
```

## Comparison with Other State Objects

### Similarities with BlendState
- Both use builder pattern for configuration
- Both are immutable after construction
- Both have Python bindings
- Both are used in GraphicsStateObject

### Differences from BlendState
- RasterizerState has simpler structure (no per-render-target configuration)
- RasterizerState has fewer configuration options
- RasterizerState has enum registration (CullMode)

### Similarities with DepthStencilState
- Both use builder pattern for configuration
- Both are immutable after construction
- Both have Python bindings
- Both are used in GraphicsStateObject

### Differences from DepthStencilState
- RasterizerState has simpler structure (no face-specific configuration)
- RasterizerState has fewer configuration options
- RasterizerState has enum registration (CullMode)

## Conclusion

RasterizerState provides a comprehensive and efficient abstraction for rasterizer configuration. The implementation is straightforward with excellent cache locality and minimal overhead.

**Strengths**:
- Clean builder pattern for configuration
- Excellent cache locality (all members fit in a single cache line)
- Lightweight construction and access
- Immutable after construction (thread-safe for reads)
- Comprehensive rasterization configuration options
- Python bindings
- Enum registration for reflection and scripting support

**Weaknesses**:
- Not thread-safe for concurrent modification of Desc
- No caching of RasterizerState instances
- No validation of forced sample count value
- No static default state objects

**Recommendations**:
1. Add validation for forced sample count value
2. Consider caching RasterizerState instances based on descriptor
3. Add thread safety for concurrent modification of Desc if needed
4. Consider adding static default state objects for common configurations
5. Consider using bit fields for bool members to reduce memory footprint
