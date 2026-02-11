# DisplacementData - Displacement Data Structures

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **DisplacementData.slang** - Displacement data structures shader (43 lines)

### External Dependencies

- **Scene/SceneTypes.slang** - Scene type definitions
- **Scene/VertexData.slang** - Vertex data structures
- **Utils/HostDeviceShared.slangh** - Host device shared shader definitions

## Module Overview

DisplacementData provides data and resources needed for displacement mapping. It includes displacement texture, displacement texture samplers (samplerState, samplerStateMin, samplerStateMax), texture size, displacement scale, and displacement bias. The structure is used for displaced triangle mesh rendering and supports texture-based displacement mapping with multiple sampling modes.

## Component Specifications

### DisplacementData Structure

**Purpose**: Data and resources needed for displacement mapping.

**Fields**:

#### Constants

- [`kShellMinMaxMargin`](Source/Falcor/Scene/Displacement/DisplacementData.slang:33) - `static const float2` - Shell min-max margin: (-0.0001, 0.0001)
- [`kSurfaceSafetyScaleBias`](Source/Falcor/Scene/Displacement/DisplacementData.slang:34) - `static const float2` - Surface safety scale bias: (1.0001, 0.0001)

#### Resources

- [`texture`](Source/Falcor/Scene/Displacement/DisplacementData.slang:36) - `Texture2D` - Displacement texture
- [`samplerState`](Source/Falcor/Scene/Displacement/DisplacementData.slang:37) - `SamplerState` - Displacement texture sampler
- [`samplerStateMin`](Source/Falcor/Scene/Displacement/DisplacementData.slang:38) - `SamplerState` - Displacement texture sampler Min
- [`samplerStateMax`](Source/Falcor/Scene/Displacement/DisplacementData.slang:39) - `SamplerState` - Displacement texture sampler Max

#### Parameters

- [`size`](Source/Falcor/Scene/Displacement/DisplacementData.slang:40) - `float2` - Texture size in texels
- [`scale`](Source/Falcor/Scene/Displacement/DisplacementData.slang:41) - `float` - Displacement scale
- [`bias`](Source/Falcor/Scene/Displacement/DisplacementData.slang:42) - `float` - Displacement bias

## Technical Details

### Constants

**Shell Min-Max Margin**:
- `kShellMinMaxMargin` - (-0.0001, 0.0001)
- Used for shell displacement
- Provides small margin for min/max calculations
- Prevents precision issues

**Surface Safety Scale Bias**:
- `kSurfaceSafetyScaleBias` - (1.0001, 0.0001)
- Used for surface displacement
- Provides safety scale and bias
- Prevents precision issues

### Resources

**Displacement Texture**:
- `texture` - Texture2D for displacement mapping
- Contains displacement values
- Used for texture-based displacement

**Displacement Texture Samplers**:
- `samplerState` - Main displacement texture sampler
- `samplerStateMin` - Min displacement texture sampler
- `samplerStateMax` - Max displacement texture sampler
- Support for multiple sampling modes
- Support for min/max displacement queries

### Parameters

**Texture Size**:
- `size` - Texture size in texels
- Used for texture coordinate normalization
- Used for displacement calculation
- Used for min/max displacement queries

**Displacement Scale**:
- `scale` - Displacement scale factor
- Multiplies displacement values
- Controls displacement magnitude
- Used for displacement tuning

**Displacement Bias**:
- `bias` - Displacement bias
- Adds offset to displacement values
- Controls displacement offset
- Used for displacement tuning

## Integration Points

### Displacement Mapping Integration

**Texture-Based Displacement**:
- Uses displacement texture for displacement mapping
- Supports texture-based displacement
- Supports high-resolution displacement

**Multi-Sampling Support**:
- Supports main, min, and max samplers
- Supports multiple sampling modes
- Supports min/max displacement queries

### Rendering Integration

**Displaced Triangle Mesh Rendering**:
- Used for displaced triangle mesh rendering
- Supports shell displacement
- Supports surface displacement
- Supports dynamic displacement updates

### Scene Integration

**Scene Type Integration**:
- Uses SceneTypes for geometry type definitions
- Uses VertexData for vertex data structures
- Supports displaced triangle mesh geometry type

## Architecture Patterns

### Resource Aggregation Pattern

- Aggregates all displacement-related resources in single structure
- Provides unified interface for displacement mapping
- Simplifies resource passing between rendering stages
- Reduces parameter passing complexity

### Multi-Sampling Pattern

- Supports multiple samplers for different sampling modes
- Supports main, min, and max samplers
- Provides flexibility for displacement queries
- Supports min/max displacement calculations

### Parameter Pattern

- Provides parameters for displacement tuning
- Supports scale and bias parameters
- Supports texture size parameter
- Provides flexibility for displacement control

## Code Patterns

### DisplacementData Structure Pattern

```slang
struct DisplacementData
{
    static const float2 kShellMinMaxMargin = float2(-0.0001f, 0.0001f);
    static const float2 kSurfaceSafetyScaleBias = float2(1.0001, 0.0001);

    Texture2D texture;              ///< Displacement texture.
    SamplerState samplerState;      ///< Displacement texture sampler.
    SamplerState samplerStateMin;   ///< Displacement texture sampler Min.
    SamplerState samplerStateMax;   ///< Displacement texture sampler Max.
    float2 size;                    ///< Texture size in texels.
    float scale;                    ///< Displacement scale.
    float bias;                     ///< Displacement bias.
}
```

## Use Cases

### Displacement Mapping

- **Texture-Based Displacement**:
  - Use displacement texture for displacement mapping
  - Sample displacement values from texture
  - Apply displacement to vertices

- **Multi-Sampling**:
  - Use main sampler for normal displacement sampling
  - Use min sampler for min displacement queries
  - Use max sampler for max displacement queries

### Displaced Triangle Mesh Rendering

- **Shell Displacement**:
  - Use kShellMinMaxMargin for shell displacement
  - Apply displacement to shell geometry
  - Use min/max samplers for shell displacement

- **Surface Displacement**:
  - Use kSurfaceSafetyScaleBias for surface displacement
  - Apply displacement to surface geometry
  - Use main sampler for surface displacement

### Displacement Tuning

- **Displacement Scale**:
  - Use scale parameter to control displacement magnitude
  - Tune displacement for visual quality
  - Adjust displacement for performance

- **Displacement Bias**:
  - Use bias parameter to control displacement offset
  - Tune displacement for visual quality
  - Adjust displacement for precision

## Performance Considerations

### Memory Performance

**Resource Storage**:
- Compact data structure for displacement resources
- Efficient memory layout
- Minimal memory overhead

**Texture Access**:
- Efficient texture sampling
- Coalesced memory access
- Optimized for GPU performance

### Computation Performance

**Multi-Sampling**:
- Efficient multi-sampler support
- Optimized for min/max queries
- Minimal overhead for displacement calculations

**Displacement Calculation**:
- Efficient displacement calculation
- Optimized for high-resolution displacement
- Minimal overhead for displacement application

## Limitations

### Feature Limitations

- **Texture Limitations**:
- Limited to single displacement texture
- Cannot support multiple displacement textures
- Limited to Texture2D

- **Sampler Limitations**:
- Limited to 3 samplers (main, min, max)
- Cannot support arbitrary sampling modes
- Limited to SamplerState

- **Parameter Limitations**:
- Limited to scale and bias parameters
- Cannot support arbitrary displacement parameters
- Limited to single scale and bias

### Performance Limitations

- **Texture Resolution Limitations**:
- High-resolution displacement textures may be expensive
- May not be optimal for real-time rendering
- May have performance overhead

- **Multi-Sampling Overhead**:
- Multiple samplers may have overhead
- May not be optimal for all use cases
- May have performance overhead

### Integration Limitations

- **Displacement System Coupling**:
- Tightly coupled to displacement mapping system
- Requires specific displacement texture format
- Not suitable for standalone use

## Best Practices

### Resource Usage

- **Texture Sampling**:
- Use appropriate sampler for displacement sampling
- Use main sampler for normal displacement
- Use min/max samplers for min/max queries

- **Parameter Tuning**:
- Use scale parameter to control displacement magnitude
- Use bias parameter to control displacement offset
- Tune parameters for visual quality

- **Precision Handling**:
- Use kShellMinMaxMargin for shell displacement
- Use kSurfaceSafetyScaleBias for surface displacement
- Handle precision issues with margins and biases

### Performance Optimization

- **Memory Optimization**:
- Use compact data structure
- Minimize memory overhead
- Optimize for GPU performance

- **Computation Optimization**:
- Use efficient texture sampling
- Use efficient displacement calculation
- Minimize overhead for displacement application

## Progress Log

- **2026-01-08T01:14:00Z**: DisplacementData analysis completed. Analyzed DisplacementData.slang (43 lines) containing displacement data structures. Documented DisplacementData structure with constants (kShellMinMaxMargin, kSurfaceSafetyScaleBias), resources (texture, samplerState, samplerStateMin, samplerStateMax), parameters (size, scale, bias), shell min-max margin (-0.0001, 0.0001), surface safety scale bias (1.0001, 0.0001), displacement texture (Texture2D), displacement texture samplers (SamplerState for main, min, max), texture size in texels, displacement scale, displacement bias, resource aggregation pattern, multi-sampling pattern, parameter pattern, integration points (displacement mapping, displaced triangle mesh rendering, scene), use cases (displacement mapping, displaced triangle mesh rendering, displacement tuning), performance considerations (memory performance, computation performance), limitations (texture limitations, sampler limitations, parameter limitations), and best practices (resource usage, parameter tuning, precision handling, performance optimization). Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.

## Completion Status

The DisplacementData module is complete. All source files have been analyzed and documented with comprehensive technical specifications covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
