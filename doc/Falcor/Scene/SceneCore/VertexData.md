# VertexData - Vertex Data Structures

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **VertexData.slang** - Vertex data structures shader (49 lines)

### External Dependencies

- **Utils/HostDeviceShared.slangh** - Host device shared shader definitions

## Module Overview

VertexData provides a structure representing interpolated vertex attributes in world space. It includes position, shading normal, shading tangent, texture coordinate, face normal, curve radius, and cone texture LOD value. The tangent is not guaranteed to be orthogonal to the normal, and the bitangent should be computed as cross(normal, tangent.xyz) * tangent.w. The tangent space is orthogonalized in prepareShadingData(). This structure is used for shading operations in ray tracing and rasterization.

## Component Specifications

### VertexData Structure

**Purpose**: Struct representing interpolated vertex attributes in world space.

**Fields**:

#### Geometry Data

- [`posW`](Source/Falcor/Scene/VertexData.slang:40) - `float3` - Position in world space
- [`normalW`](Source/Falcor/Scene/VertexData.slang:41) - `float3` - Shading normal in world space (normalized)
- [`tangentW`](Source/Falcor/Scene/VertexData.slang:42) - `float4` - Shading tangent in world space (normalized). The last component is guaranteed to be +-1.0 or zero if tangents are missing
- [`faceNormalW`](Source/Falcor/Scene/VertexData.slang:44) - `float3` - Face normal in world space (normalized)

#### Texture Data

- [`texC`](Source/Falcor/Scene/VertexData.slang:43) - `float2` - Texture coordinate

#### Curve Data

- [`curveRadius`](Source/Falcor/Scene/VertexData.slang:45) - `float` - Curve cross-sectional radius. Valid only for geometry generated from curves

#### Ray Cone Data

- [`coneTexLODValue`](Source/Falcor/Scene/VertexData.slang:46) - `float` - Texture LOD data for cone tracing. This is zero, unless getVertexDataRayCones() is used

## Technical Details

### Geometry Data

**Position**:
- `posW` - Position in world space
- Used for shading calculations
- Used for lighting calculations

**Shading Normal**:
- `normalW` - Shading normal in world space (normalized)
- Used for shading calculations
- Used for lighting calculations
- Used for BRDF/BSDF evaluation

**Shading Tangent**:
- `tangentW` - Shading tangent in world space (normalized)
- Last component is guaranteed to be +-1.0 or zero if tangents are missing
- Not guaranteed to be orthogonal to normal
- Bitangent should be computed as cross(normal, tangent.xyz) * tangent.w
- Tangent space is orthogonalized in prepareShadingData()
- Used for anisotropic material evaluation
- Used for BRDF/BSDF evaluation

**Face Normal**:
- `faceNormalW` - Face normal in world space (normalized)
- Used for front/back face detection
- Used for material property evaluation
- Used for ray origin computation

### Texture Data

**Texture Coordinate**:
- `texC` - Texture coordinate
- Used for texture sampling
- Used for material property evaluation

### Curve Data

**Curve Radius**:
- `curveRadius` - Curve cross-sectional radius
- Valid only for geometry generated from curves
- Used for curve shading and intersection

### Ray Cone Data

**Cone Texture LOD Value**:
- `coneTexLODValue` - Texture LOD data for cone tracing
- Zero unless getVertexDataRayCones() is used
- Used for ray cone texture LOD
- Used for texture filtering

## Integration Points

### Shading Integration

**Shading Operations**:
- Used for shading calculations
- Used for lighting calculations
- Used for BRDF/BSDF evaluation

**Material Evaluation**:
- Used for material property evaluation
- Used for texture sampling
- Used for anisotropic material evaluation

### Rendering Integration

**Ray Tracing**:
- Used for ray tracing shading
- Used for path tracing shading
- Used for Monte Carlo integration

**Rasterization**:
- Used for rasterization shading
- Used for deferred shading
- Used for forward shading

### Curve Rendering

**Curve Shading**:
- Used for curve shading
- Used for curve intersection
- Used for curve material evaluation

### Ray Cone Integration

**Texture LOD**:
- Used for ray cone texture LOD
- Used for texture filtering
- Used for texture quality

## Architecture Patterns

### Data Aggregation Pattern

- Aggregates all vertex-related data in single structure
- Provides unified interface for vertex data
- Simplifies data passing between rendering stages
- Reduces parameter passing complexity

### Interpolation Pattern

- Represents interpolated vertex attributes
- Supports barycentric interpolation
- Supports smooth shading
- Supports flat shading

## Code Patterns

### VertexData Structure Pattern

```slang
struct VertexData
{
    float3 posW;            ///< Position in world space.
    float3 normalW;         ///< Shading normal in world space (normalized).
    float4 tangentW;        ///< Shading tangent in world space (normalized). The last component is guaranteed to be +-1.0 or zero if tangents are missing.
    float2 texC;            ///< Texture coordinate.
    float3 faceNormalW;     ///< Face normal in world space (normalized).
    float  curveRadius;     ///< Curve cross-sectional radius. Valid only for geometry generated from curves.
    float  coneTexLODValue; ///< Texture LOD data for cone tracing. This is zero, unless getVertexDataRayCones() is used.
}
```

## Use Cases

### Ray Tracing Shading

- **Hit Point Shading**:
  - Populate VertexData from ray tracing hit information
  - Evaluate material properties at hit point
  - Compute lighting contributions
  - Sample BRDF/BSDF for Monte Carlo integration

- **Path Tracing**:
  - Use VertexData for path tracing shading
  - Compute ray origins for secondary rays
  - Evaluate material properties for scattering
  - Compute oriented normals for lighting

### Rasterization Shading

- **Fragment Shading**:
  - Populate VertexData from rasterization fragment data
  - Evaluate material properties at fragment
  - Compute lighting contributions
  - Apply textures

- **Deferred Shading**:
  - Store VertexData in G-buffer
  - Evaluate materials in lighting pass
  - Compute lighting contributions
  - Apply post-processing

### Curve Shading

- **Curve Intersection**:
  - Use VertexData for curve intersection
  - Evaluate material properties at curve hit
  - Compute lighting contributions
  - Sample BRDF/BSDF for Monte Carlo integration

### Ray Cone Texture LOD

- **Texture Filtering**:
  - Use coneTexLODValue for texture LOD
  - Filter textures based on ray cone
  - Improve texture quality
  - Reduce texture aliasing

## Performance Considerations

### Memory Layout

**Data Packing**:
- Structured data layout for efficient GPU memory access
- No_diff attributes for non-differentiable fields
- Efficient memory alignment

**Memory Access**:
- Coalesced memory access for geometry data
- Efficient material data access
- Optimized for GPU performance

### Computation Performance

**Tangent Orthogonalization**:
- Tangent space orthogonalized in prepareShadingData()
- Efficient orthogonalization algorithm
- Minimal overhead for shading operations

**Bitangent Computation**:
- Bitangent computed as cross(normal, tangent.xyz) * tangent.w
- Efficient bitangent computation
- Minimal overhead for anisotropic materials

## Limitations

### Feature Limitations

- **Tangent Limitations**:
- Tangent not guaranteed to be orthogonal to normal
- Requires bitangent computation
- Requires tangent space orthogonalization

- **Curve Radius Limitations**:
- Curve radius only valid for geometry generated from curves
- Not valid for triangle meshes
- Not valid for other geometry types

- **Ray Cone Limitations**:
- coneTexLODValue only non-zero when getVertexDataRayCones() is used
- Requires ray cone computation
- Not suitable for all use cases

### Performance Limitations

- **Memory Overhead**:
- Structured data layout may have padding overhead
- May not be optimal for all use cases
- May have memory overhead for unused fields

- **Computation Overhead**:
- Tangent orthogonalization may have overhead
- Bitangent computation may have overhead
- May not be optimal for all use cases

### Integration Limitations

- **Rendering Pipeline Coupling**:
- Tightly coupled to rendering pipeline
- Requires specific data from rendering stages
- Not suitable for standalone use

## Best Practices

### Data Initialization

- **Geometry Data**:
- Initialize posW from hit position
- Initialize normalW from interpolated normal
- Initialize tangentW from interpolated tangent
- Initialize faceNormalW from face normal

- **Texture Data**:
- Initialize texC from texture coordinates
- Use for texture sampling
- Use for material property evaluation

- **Curve Data**:
- Initialize curveRadius for curve geometry
- Only valid for geometry generated from curves
- Use for curve shading and intersection

- **Ray Cone Data**:
- Initialize coneTexLODValue for ray cone texture LOD
- Only non-zero when getVertexDataRayCones() is used
- Use for texture filtering

### Shading Operations

- **Tangent Orthogonalization**:
- Use prepareShadingData() for tangent space orthogonalization
- Compute bitangent as cross(normal, tangent.xyz) * tangent.w
- Use orthogonalized tangent space for shading

- **Material Evaluation**:
- Use VertexData for material evaluation
- Sample textures using texC
- Evaluate BRDF/BSDF using normalW and tangentW

### Performance Optimization

- **Memory Optimization**:
- Use structured data layout for efficient memory access
- Minimize padding overhead
- Optimize for GPU performance

- **Computation Optimization**:
- Use efficient tangent orthogonalization
- Use efficient bitangent computation
- Minimize overhead for shading operations

## Progress Log

- **2026-01-08T01:01:00Z**: VertexData analysis completed. Analyzed VertexData.slang (49 lines) containing vertex data structure for interpolated vertex attributes in world space. Documented VertexData structure with geometry data (posW, normalW, tangentW, faceNormalW), texture data (texC), curve data (curveRadius), ray cone data (coneTexLODValue), position in world space, shading normal in world space (normalized), shading tangent in world space (normalized) with last component guaranteed to be +-1.0 or zero if tangents are missing, face normal in world space (normalized), texture coordinate, curve cross-sectional radius (valid only for geometry generated from curves), texture LOD data for cone tracing (zero unless getVertexDataRayCones() is used), tangent not guaranteed to be orthogonal to normal, bitangent computation as cross(normal, tangent.xyz) * tangent.w, tangent space orthogonalization in prepareShadingData(), data aggregation pattern, interpolation pattern, integration points (shading, rendering, curve rendering, ray cone), use cases (ray tracing shading, rasterization shading, curve shading, ray cone texture LOD), performance considerations (memory layout, computation performance), limitations (tangent limitations, curve radius limitations, ray cone limitations), and best practices (data initialization, shading operations, performance optimization). Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.

## Completion Status

The VertexData module is complete. All source files have been analyzed and documented with comprehensive technical specifications covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
