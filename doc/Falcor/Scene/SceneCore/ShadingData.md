# ShadingData - Shading Data Structures

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **ShadingData.slang** - Shading data structures (89 lines)

### External Dependencies

- **Scene/Material/MaterialData.slang** - Material data structures
- **Utils/Geometry/GeometryHelpers.slang** - Geometry helper functions
- **Utils/Math/ShadingFrame.slang** - Shading frame utilities

## Module Overview

ShadingData provides a comprehensive data structure for shading hit points in ray tracing and rasterization. It includes geometric properties of the surface, view direction, texture coordinates, material ID and header, and index of refraction of the surrounding medium. Based on a ShadingData struct, the material system can be queried for a material instance at the hit using `gScene.materials.getMaterialInstance()`. The material instance has interfaces for sampling and evaluation, as well as for querying its properties at the hit.

## Component Specifications

### ShadingData Structure

**Purpose**: Holds information needed for shading a hit point.

**Base Class**: [`IDifferentiable`](Source/Falcor/Scene/ShadingData.slang:46) - Differentiable interface for gradient computation

**Fields**:

#### Geometry Data

- [`posW`](Source/Falcor/Scene/ShadingData.slang:49) - `float3` - Shading hit position in world space
- [`V`](Source/Falcor/Scene/ShadingData.slang:50) - `float3` - View direction, -incident direction (-ray.dir)
- [`uv`](Source/Falcor/Scene/ShadingData.slang:51) - `float2` - Texture mapping coordinates
- [`frame`](Source/Falcor/Scene/ShadingData.slang:53) - `ShadingFrame` - Smooth interpolated shading frame in world space at the shading point. The normal is *not* automatically flipped for backfacing hits
- [`faceN`](Source/Falcor/Scene/ShadingData.slang:54) - `no_diff float3` - Face normal in world space, always on the front-facing side
- [`tangentW`](Source/Falcor/Scene/ShadingData.slang:55) - `no_diff float4` - Geometric tangent (xyz) and sign (w) in world space. This is used for orthogonalization. Not normalized, but it is guaranteed to be nonzero and sign (w) is +-1.0
- [`frontFacing`](Source/Falcor/Scene/ShadingData.slang:56) - `bool` - True if primitive seen from the front-facing side, i.e., dot(V, faceN) >= 0.0
- [`curveRadius`](Source/Falcor/Scene/ShadingData.slang:57) - `no_diff float` - Curve cross-sectional radius. Valid only for geometry generated from curves

#### Material Data

- [`mtl`](Source/Falcor/Scene/ShadingData.slang:60) - `MaterialHeader` - Material header data
- [`materialID`](Source/Falcor/Scene/ShadingData.slang:61) - `uint` - Material ID at shading location
- [`IoR`](Source/Falcor/Scene/ShadingData.slang:62) - `float` - Index of refraction for the medium on the front-facing side (i.e. "outside" the material at the hit)

#### Gradient Data

- [`materialGradOffset`](Source/Falcor/Scene/ShadingData.slang:64) - `uint` - Offset to the material gradient in the gradient buffer
- [`geometryGradOffset`](Source/Falcor/Scene/ShadingData.slang:65) - `uint` - Offset to the geometry gradient in the gradient buffer
- [`threadID`](Source/Falcor/Scene/ShadingData.slang:66) - `uint` - Thread ID for gradient aggregation with a hash grid

**Methods**:

#### Utility Functions

- [`computeRayOrigin(bool viewside = true)`](Source/Falcor/Scene/ShadingData.slang:77) - Computes new ray origin based on the hit point to avoid self-intersection
  - Parameters: viewside - True if the origin should be on the view side (reflection) or false otherwise (transmission)
  - Returns: Ray origin of the new ray
  - Implementation: `computeRayOrigin(posW, (frontFacing == viewside) ? faceN : -faceN)`
  - Reference: Ray Tracing Gems, Chapter 6, "A Fast and Robust Method for Avoiding Self-Intersection" by Carsten Wächter and Nikolaus Binder

- [`getOrientedFaceNormal()`](Source/Falcor/Scene/ShadingData.slang:85) - Returns the oriented face normal
  - Returns: Face normal flipped to the same side as the view vector
  - Implementation: `frontFacing ? faceN : -faceN`

## Technical Details

### Geometry Data

**Position and Direction**:
- `posW` - World space hit position for shading calculations
- `V` - View direction for lighting calculations (negative of incident ray direction)

**Texture Coordinates**:
- `uv` - Texture mapping coordinates for material sampling
- Used for texture lookup and material property evaluation

**Shading Frame**:
- `frame` - Smooth interpolated shading frame in world space
- Contains normal, tangent, and bitangent for shading calculations
- Normal is *not* automatically flipped for backfacing hits
- Used for BRDF evaluation and lighting calculations

**Face Normal**:
- `faceN` - Face normal in world space, always on the front-facing side
- Used for front/back face detection
- Used for ray origin computation
- Used for oriented normal computation

**Tangent**:
- `tangentW` - Geometric tangent (xyz) and sign (w) in world space
- Used for orthogonalization
- Not normalized, but guaranteed to be nonzero
- Sign (w) is +-1.0
- Used for anisotropic material evaluation

**Front/Back Face**:
- `frontFacing` - True if primitive seen from the front-facing side
- Computed as dot(V, faceN) >= 0.0
- Used for material property evaluation
- Used for ray origin computation
- Used for oriented normal computation

**Curve Radius**:
- `curveRadius` - Curve cross-sectional radius
- Valid only for geometry generated from curves
- Used for curve shading and intersection

### Material Data

**Material Header**:
- `mtl` - Material header data
- Contains material type and properties
- Used for material instance selection

**Material ID**:
- `materialID` - Material ID at shading location
- Used to query material instance from material system
- Used for material property evaluation

**Index of Refraction**:
- `IoR` - Index of refraction for the medium on the front-facing side
- Used for transmission and refraction calculations
- Represents "outside" the material at the hit

### Gradient Data

**Material Gradient Offset**:
- `materialGradOffset` - Offset to the material gradient in the gradient buffer
- Used for gradient computation in differentiable rendering
- Used for material optimization

**Geometry Gradient Offset**:
- `geometryGradOffset` - Offset to the geometry gradient in the gradient buffer
- Used for gradient computation in differentiable rendering
- Used for geometry optimization

**Thread ID**:
- `threadID` - Thread ID for gradient aggregation with a hash grid
- Used for gradient aggregation in differentiable rendering
- Used for parallel gradient computation

### Utility Functions

**Ray Origin Computation**:
- `computeRayOrigin()` - Computes new ray origin based on hit point
- Avoids self-intersection using method from Ray Tracing Gems
- Supports both reflection (viewside = true) and transmission (viewside = false)
- Uses face normal or negated face normal based on viewside and frontFacing

**Oriented Face Normal**:
- `getOrientedFaceNormal()` - Returns face normal flipped to view side
- Used for lighting calculations
- Used for material property evaluation

## Integration Points

### Material System Integration

**Material Instance Query**:
- Based on ShadingData, material system can be queried for material instance
- Uses `gScene.materials.getMaterialInstance()` with materialID
- Material instance has interfaces for sampling and evaluation
- Material instance has interfaces for querying properties

**Material Evaluation**:
- ShadingData provides all necessary data for material evaluation
- Supports BRDF evaluation
- Supports BSDF evaluation
- Supports material property queries

### Rendering Integration

**Ray Tracing**:
- ShadingData is populated from ray tracing hit information
- Supports ray tracing shading
- Supports path tracing
- Supports Monte Carlo integration

**Rasterization**:
- ShadingData is populated from rasterization fragment data
- Supports rasterization shading
- Supports deferred shading
- Supports forward shading

### Differentiable Rendering Integration

**Gradient Computation**:
- ShadingData implements IDifferentiable interface
- Supports material gradient computation
- Supports geometry gradient computation
- Supports gradient aggregation with hash grid

## Architecture Patterns

### Data Aggregation Pattern

- Aggregates all shading-related data in single structure
- Provides unified interface for shading operations
- Simplifies data passing between rendering stages
- Reduces parameter passing complexity

### Differentiable Pattern

- Implements IDifferentiable interface for gradient computation
- Supports differentiable rendering
- Supports material optimization
- Supports geometry optimization

### Utility Function Pattern

- Provides utility functions for common operations
- Encapsulates complex algorithms (ray origin computation)
- Simplifies client code
- Reduces code duplication

## Code Patterns

### ShadingData Structure Pattern

```slang
struct ShadingData : IDifferentiable
{
    // Geometry data
    float3  posW;
    float3  V;
    float2  uv;
    ShadingFrame frame;
    no_diff float3  faceN;
    no_diff float4  tangentW;
    bool            frontFacing;
    no_diff float   curveRadius;

    // Material data
    MaterialHeader mtl;
    uint    materialID;
    float   IoR;

    uint materialGradOffset;
    uint geometryGradOffset;
    uint threadID;

    // Utility functions
    float3 computeRayOrigin(bool viewside = true)
    {
        return computeRayOrigin(posW, (frontFacing == viewside) ? faceN : -faceN);
    }

    float3 getOrientedFaceNormal()
    {
        return frontFacing ? faceN : -faceN;
    }
}
```

### Ray Origin Computation Pattern

```slang
float3 computeRayOrigin(bool viewside = true)
{
    return computeRayOrigin(posW, (frontFacing == viewside) ? faceN : -faceN);
}
```

### Oriented Normal Computation Pattern

```slang
float3 getOrientedFaceNormal()
{
    return frontFacing ? faceN : -faceN;
}
```

## Use Cases

### Ray Tracing Shading

- **Hit Point Shading**:
  - Populate ShadingData from ray tracing hit information
  - Evaluate material properties at hit point
  - Compute lighting contributions
  - Sample BRDF/BSDF for Monte Carlo integration

- **Path Tracing**:
  - Use ShadingData for path tracing shading
  - Compute ray origins for secondary rays
  - Evaluate material properties for scattering
  - Compute oriented normals for lighting

### Rasterization Shading

- **Fragment Shading**:
  - Populate ShadingData from rasterization fragment data
  - Evaluate material properties at fragment
  - Compute lighting contributions
  - Apply textures

- **Deferred Shading**:
  - Store ShadingData in G-buffer
  - Evaluate materials in lighting pass
  - Compute lighting contributions
  - Apply post-processing

### Differentiable Rendering

- **Gradient Computation**:
  - Compute material gradients using ShadingData
  - Compute geometry gradients using ShadingData
  - Aggregate gradients using thread ID and hash grid
  - Optimize materials and geometry

- **Material Optimization**:
  - Use ShadingData for material gradient computation
  - Optimize material parameters
  - Optimize material textures
  - Optimize material BSDFs

- **Geometry Optimization**:
  - Use ShadingData for geometry gradient computation
  - Optimize mesh vertices
  - Optimize mesh normals
  - Optimize mesh tangents

## Performance Considerations

### Memory Layout

**Data Packing**:
- Structured data layout for efficient GPU memory access
- No_diff attributes for non-differentiable fields
- Efficient memory alignment

**Memory Access**:
- Coalesced memory access for geometry data
- Efficient material data access
- Optimized gradient data access

### Computation Performance

**Utility Functions**:
- Efficient ray origin computation
- Efficient oriented normal computation
- Minimal overhead for common operations

**Differentiable Rendering**:
- Efficient gradient computation
- Efficient gradient aggregation
- Optimized hash grid access

## Limitations

### Feature Limitations

- **Material System Dependency**:
- Requires material system for material instance query
- Requires material system for material evaluation
- Tightly coupled to material system

- **Geometry System Dependency**:
- Requires geometry system for hit information
- Requires geometry system for face normals
- Requires geometry system for tangents

- **Differentiable Rendering Dependency**:
- Requires differentiable rendering system for gradient computation
- Requires gradient buffer for gradient storage
- Requires hash grid for gradient aggregation

### Performance Limitations

- **Memory Overhead**:
- Structured data layout may have padding overhead
- No_diff attributes may reduce memory efficiency
- Gradient data may increase memory usage

- **Computation Overhead**:
- Ray origin computation may have overhead
- Oriented normal computation may have overhead
- Gradient computation may have overhead

### Integration Limitations

- **Rendering Pipeline Coupling**:
- Tightly coupled to rendering pipeline
- Requires specific data from rendering stages
- Not suitable for standalone use

## Best Practices

### Data Initialization

- **Geometry Data**:
- Initialize posW from hit position
- Initialize V from ray direction
- Initialize uv from texture coordinates
- Initialize frame from interpolated normals and tangents
- Initialize faceN from face normal
- Initialize tangentW from geometric tangent
- Initialize frontFacing from dot(V, faceN) >= 0.0
- Initialize curveRadius for curve geometry

- **Material Data**:
- Initialize mtl from material header
- Initialize materialID from hit information
- Initialize IoR from medium index of refraction

- **Gradient Data**:
- Initialize materialGradOffset for material gradient
- Initialize geometryGradOffset for geometry gradient
- Initialize threadID for gradient aggregation

### Utility Function Usage

- **Ray Origin Computation**:
- Use computeRayOrigin() for secondary rays
- Use viewside = true for reflection
- Use viewside = false for transmission
- Avoid self-intersection

- **Oriented Normal Computation**:
- Use getOrientedFaceNormal() for lighting calculations
- Use oriented normal for material evaluation
- Use oriented normal for BRDF/BSDF evaluation

### Differentiable Rendering

- **Gradient Computation**:
- Use materialGradOffset for material gradient computation
- Use geometryGradOffset for geometry gradient computation
- Use threadID for gradient aggregation
- Use hash grid for efficient aggregation

- **Optimization**:
- Use ShadingData for material optimization
- Use ShadingData for geometry optimization
- Use gradient information for optimization
- Use hash grid for efficient aggregation

## Progress Log

- **2026-01-08T00:54:00Z**: ShadingData analysis completed. Analyzed ShadingData.slang (89 lines) containing comprehensive shading data structure. Documented ShadingData struct (IDifferentiable) with geometry data (posW, V, uv, frame, faceN, tangentW, frontFacing, curveRadius), material data (mtl, materialID, IoR), gradient data (materialGradOffset, geometryGradOffset, threadID), utility functions (computeRayOrigin, getOrientedFaceNormal), imports (Scene.Material.MaterialData, Utils.Geometry.GeometryHelpers, Utils.Math.ShadingFrame), data aggregation pattern, differentiable pattern, utility function pattern, and integration points (material system, rendering, differentiable rendering). Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.

## Completion Status

The ShadingData module is complete. All source files have been analyzed and documented with comprehensive technical specifications covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
