# Raytracing - Ray Tracing Shaders

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **Raytracing.slang** - Ray tracing shader
- [x] **RaytracingInline.slang** - Ray tracing inline shader

### External Dependencies

- **Utils/Attributes** - Shader attributes
- **Rendering/Materials/TexLODHelpers** - Texture LOD helpers
- **Scene/Shading** - Shading system

## Module Overview

Raytracing provides ray tracing shaders for hardware-accelerated ray tracing. It includes convenience wrapper functions for TraceRay, helper functions for getting geometry instance ID, vertex data interpolation, and previous frame position computation. The module supports ray cones for texture LOD, multiple ray types, and integration with the scene system.

## Component Specifications

### DxrPerFrame Constant Buffer

**Purpose**: Per-frame constant buffer for DXR (DirectX Raytracing).

**Key Features**:
- Ray type count for ray tracing pipeline
- Per-frame update
- Used by ray tracing shaders

**Fields**:
- [`rayTypeCount`](Source/Falcor/Scene/Raytracing.slang:34) - `uint` - Number of ray types in the ray tracing pipeline

**Usage**:
- Passed to TraceRay for ray type specification
- Used for ray type indexing
- Configured on CPU side
- Updated per frame

### TraceRayScene Function

**Purpose**: Convenience wrapper for TraceRay() that sets Scene parameters internally.

**Signature**:
```slang
void TraceRayScene<payload_t>(
    uint rayFlags,
    uint instanceInclusionMask,
    uint rayType,
    uint missIndex,
    RayDesc ray,
    inout payload_t payload)
```

**Parameters**:
- [`rayFlags`](Source/Falcor/Scene/Raytracing.slang:46) - `uint` - Ray flags. Set to RAY_FLAG_NONE if not used.
- [`instanceInclusionMask`](Source/Falcor/Scene/Raytracing.slang:47) - `uint` - Instance inclusion mask. Set to 0xff if not needed.
- [`rayType`](Source/Falcor/Scene/Raytracing.slang:48) - `uint` - The ray type index.
- [`missIndex`](Source/Falcor/Scene/Raytracing.slang:49) - `uint` - The miss shader index.
- [`ray`](Source/Falcor/Scene/Raytracing.slang:50) - `RayDesc` - The ray description.
- [`payload`](Source/Falcor/Scene/Raytracing.slang:51) - `inout payload_t` - The payload struct.

**Template Parameter**:
- `payload_t` - Payload type for ray tracing

**Implementation**:
- Calls [`TraceRay(gScene.rtAccel, rayFlags, instanceInclusionMask, rayType, rayTypeCount, missIndex, ray, payload)`](Source/Falcor/Scene/Raytracing.slang:53)
- Uses scene acceleration structure: `gScene.rtAccel`
- Uses ray type count from constant buffer: `rayTypeCount`
- Passes all parameters to TraceRay

**Convenience**:
- Wraps TraceRay with scene-specific parameters
- Automatically uses scene acceleration structure
- Automatically uses ray type count
- Simplifies ray tracing calls

### getGeometryInstanceID Function

**Purpose**: Return global geometry instance ID in scene from a DXR 1.0 hit group shader.

**Signature**:
```slang
GeometryInstanceID getGeometryInstanceID()
```

**Implementation**:
- Returns [`GeometryInstanceID(InstanceID(), GeometryIndex())`](Source/Falcor/Scene/Raytracing.slang:63)
- Uses DXR system values: `InstanceID()` and `GeometryIndex()`
- Creates GeometryInstanceID from instance ID and geometry index

**DXR System Values**:
- `InstanceID()` - Instance ID from DXR
- `GeometryIndex()` - Geometry index from DXR
- System-provided values
- Only available in hit group shaders

**Usage**:
- Can only be called from closestHit, anyHit, or intersection shader
- Returns global geometry instance ID
- Used for scene data access
- Simplifies instance ID retrieval

**Limitations**:
- Can only be called from hit group shaders
- Cannot be called from other shader stages
- Requires DXR 1.0 or higher

### getVertexData Function (Overload 1)

**Purpose**: Returns interpolated vertex attributes in a ray tracing hit program.

**Signature**:
```slang
VertexData getVertexData(GeometryInstanceID instanceID, uint triangleIndex, BuiltInTriangleIntersectionAttributes attribs)
```

**Parameters**:
- [`instanceID`](Source/Falcor/Scene/Raytracing.slang:72) - `GeometryInstanceID` - Global geometry instance ID
- [`triangleIndex`](Source/Falcor/Scene/Raytracing.slang:72) - `uint` - Index of the triangle in the current mesh (= PrimitiveIndex())
- [`attribs`](Source/Falcor/Scene/Raytracing.slang:72) - `BuiltInTriangleIntersectionAttributes` - Intersection attributes provided by DXR

**Implementation**:
- Computes barycentrics: [`float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y)`](Source/Falcor/Scene/Raytracing.slang:74)
- Calls [`gScene.getVertexData(instanceID, triangleIndex, barycentrics)`](Source/Falcor/Scene/Raytracing.slang:75)
- Returns [`VertexData`](Source/Falcor/Scene/Raytracing.slang:75) structure

**Barycentric Coordinates**:
- Computed from DXR intersection attributes
- First weight: `1.0 - attribs.barycentrics.x - attribs.barycentrics.y`
- Second weight: `attribs.barycentrics.x`
- Third weight: `attribs.barycentrics.y`
- Used for vertex attribute interpolation

**Vertex Data**:
- Interpolated vertex attributes
- Includes position, normal, tangent, texture coordinate
- Used for shading
- Used for material evaluation

### getVertexData Function (Overload 2)

**Purpose**: Returns interpolated vertex attributes in a ray tracing hit program with unpacked vertices.

**Signature**:
```slang
VertexData getVertexData(GeometryInstanceID instanceID, uint triangleIndex, BuiltInTriangleIntersectionAttributes attribs, out float3 barycentrics, out StaticVertexData vertices[3])
```

**Parameters**:
- [`instanceID`](Source/Falcor/Scene/Raytracing.slang:85) - `GeometryInstanceID` - Global geometry instance ID
- [`triangleIndex`](Source/Falcor/Scene/Raytracing.slang:85) - `uint` - Index of the triangle in the current mesh (= PrimitiveIndex())
- [`attribs`](Source/Falcor/Scene/Raytracing.slang:85) - `BuiltInTriangleIntersectionAttributes` - Intersection attributes provided by DXR
- [`barycentrics`](Source/Falcor/Scene/Raytracing.slang:85) - `out float3` - Output barycentric coordinates
- [`vertices`](Source/Falcor/Scene/Raytracing.slang:85) - `out StaticVertexData[3]` - Output unpacked fetched vertices

**Implementation**:
- Computes barycentrics: [`float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y)`](Source/Falcor/Scene/Raytracing.slang:87)
- Calls [`gScene.getVertexData(instanceID, triangleIndex, barycentrics, vertices)`](Source/Falcor/Scene/Raytracing.slang:88)
- Returns [`VertexData`](Source/Falcor/Scene/Raytracing.slang:88) structure

**Barycentric Coordinates**:
- Computed from DXR intersection attributes
- First weight: `1.0 - attribs.barycentrics.x - attribs.barycentrics.y`
- Second weight: `attribs.barycentrics.x`
- Third weight: `attribs.barycentrics.y`
- Used for vertex attribute interpolation

**Unpacked Vertices**:
- Output parameter: `out StaticVertexData vertices[3]`
- Unpacked fetched vertices
- Can be used for further computations involving individual vertices
- Useful for custom interpolation or analysis

**Vertex Data**:
- Interpolated vertex attributes
- Includes position, normal, tangent, texture coordinate
- Used for shading
- Used for material evaluation

### getVertexDataRayCones Function

**Purpose**: Returns interpolated vertex attributes in a ray tracing hit program when ray cones are used for texture LOD.

**Signature**:
```slang
VertexData getVertexDataRayCones(GeometryInstanceID instanceID, uint triangleIndex, BuiltInTriangleIntersectionAttributes attribs)
```

**Parameters**:
- [`instanceID`](Source/Falcor/Scene/Raytracing.slang:97) - `GeometryInstanceID` - Global geometry instance ID
- [`triangleIndex`](Source/Falcor/Scene/Raytracing.slang:97) - `uint` - Index of the triangle in the current mesh (= PrimitiveIndex())
- [`attribs`](Source/Falcor/Scene/Raytracing.slang:97) - `BuiltInTriangleIntersectionAttributes` - Intersection attributes provided by DXR

**Implementation**:
- Computes barycentrics: [`float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y)`](Source/Falcor/Scene/Raytracing.slang:99)
- Calls [`gScene.getVertexDataRayCones(instanceID, triangleIndex, barycentrics)`](Source/Falcor/Scene/Raytracing.slang:100)
- Returns [`VertexData`](Source/Falcor/Scene/Raytracing.slang:100) structure

**Ray Cones**:
- Used for texture LOD computation
- Improves texture quality for ray tracing
- Reduces texture aliasing
- Uses cone angle for LOD selection

**Barycentric Coordinates**:
- Computed from DXR intersection attributes
- First weight: `1.0 - attribs.barycentrics.x - attribs.barycentrics.y`
- Second weight: `attribs.barycentrics.x`
- Third weight: `attribs.barycentrics.y`
- Used for vertex attribute interpolation

**Vertex Data**:
- Interpolated vertex attributes
- Includes position, normal, tangent, texture coordinate
- Used for shading
- Used for material evaluation

### getPrevPosW Function

**Purpose**: Returns interpolated position on a triangle in world space for previous frame.

**Signature**:
```slang
float3 getPrevPosW(GeometryInstanceID instanceID, uint triangleIndex, BuiltInTriangleIntersectionAttributes attribs)
```

**Parameters**:
- [`instanceID`](Source/Falcor/Scene/Raytracing.slang:109) - `GeometryInstanceID` - Global geometry instance ID
- [`triangleIndex`](Source/Falcor/Scene/Raytracing.slang:109) - `uint` - Index of the triangle in the current mesh (= PrimitiveIndex())
- [`attribs`](Source/Falcor/Scene/Raytracing.slang:109) - `BuiltInTriangleIntersectionAttributes` - Intersection attributes provided by DXR

**Implementation**:
- Computes barycentrics: [`float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y)`](Source/Falcor/Scene/Raytracing.slang:111)
- Calls [`gScene.getPrevPosW(instanceID, triangleIndex, barycentrics)`](Source/Falcor/Scene/Raytracing.slang:112)
- Returns [`float3`](Source/Falcor/Scene/Raytracing.slang:112) - Previous frame position in world space

**Barycentric Coordinates**:
- Computed from DXR intersection attributes
- First weight: `1.0 - attribs.barycentrics.x - attribs.barycentrics.y`
- Second weight: `attribs.barycentrics.x`
- Third weight: `attribs.barycentrics.y`
- Used for position interpolation

**Previous Frame Position**:
- Interpolated position on triangle
- In world space
- For previous frame
- Used for motion vectors

**Motion Vectors**:
- Previous frame position used for motion vector computation
- Current frame position: from getVertexData
- Previous frame position: from getPrevPosW
- Motion vector: prevPosW - posW

### getRayTypeCount Function

**Purpose**: Get the ray type count.

**Signature**:
```slang
uint getRayTypeCount()
```

**Implementation**:
- Returns [`rayTypeCount`](Source/Falcor/Scene/Raytracing.slang:115) from constant buffer

**Usage**:
- Returns number of ray types in ray tracing pipeline
- Used for ray type indexing
- Configured on CPU side
- Updated per frame

## Technical Details

### Ray Tracing Pipeline

**Ray Type Count**:
- Number of ray types in ray tracing pipeline
- Stored in constant buffer: `DxrPerFrame.rayTypeCount`
- Used for ray type indexing
- Configured on CPU side

**Ray Types**:
- Different ray types for different purposes
- Primary rays, shadow rays, etc.
- Each ray type has its own hit group and miss shader
- Indexed by ray type number

**Ray Tracing Acceleration Structure**:
- Scene acceleration structure: `gScene.rtAccel`
- Top-Level Acceleration Structure (TLAS)
- Contains scene geometry
- Used for ray intersection queries

### TraceRay Wrapper

**Purpose**: Simplify ray tracing calls.

**Convenience**:
- Wraps TraceRay with scene-specific parameters
- Automatically uses scene acceleration structure
- Automatically uses ray type count
- Simplifies ray tracing calls

**Parameters**:
- `rayFlags` - Ray flags for ray tracing behavior
- `instanceInclusionMask` - Instance inclusion mask for filtering
- `rayType` - Ray type index for hit group selection
- `missIndex` - Miss shader index for miss shader selection
- `ray` - Ray description (origin, direction, tMin, tMax)
- `payload` - Payload for ray tracing data

**Ray Flags**:
- `RAY_FLAG_NONE` - No special flags
- `RAY_FLAG_FORCE_OPAQUE` - Force opaque intersection
- `RAY_FLAG_FORCE_NON_OPAQUE` - Force non-opaque intersection
- `RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH` - Accept first hit and end search
- `RAY_FLAG_SKIP_CLOSEST_HIT_SHADER` - Skip closest hit shader
- `RAY_FLAG_CULL_BACK_FACING_TRIANGLES` - Cull back-facing triangles
- `RAY_FLAG_CULL_FRONT_FACING_TRIANGLES` - Cull front-facing triangles
- `RAY_FLAG_SKIP_TRIANGLES` - Skip triangle geometry
- `RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES` - Skip procedural primitives

**Instance Inclusion Mask**:
- `0xff` - Include all instances
- Custom mask for instance filtering
- Bitmask for instance selection
- Used for ray filtering

### Barycentric Coordinates

**Purpose**: Interpolate vertex attributes on triangle.

**Computation**:
- From DXR intersection attributes: `attribs.barycentrics`
- First weight: `1.0 - attribs.barycentrics.x - attribs.barycentrics.y`
- Second weight: `attribs.barycentrics.x`
- Third weight: `attribs.barycentrics.y`
- Weights sum to 1.0

**Interpolation**:
- Used for vertex attribute interpolation
- Position: `v0 * w0 + v1 * w1 + v2 * w2`
- Normal: `n0 * w0 + n1 * w1 + n2 * w2`
- Tangent: `t0 * w0 + t1 * w1 + t2 * w2`
- Texture coordinate: `uv0 * w0 + uv1 * w1 + uv2 * w2`

**DXR Intersection Attributes**:
- Provided by DXR ray tracing
- Contains barycentric coordinates
- Used for vertex attribute interpolation
- Used for hit point computation

### Vertex Data Interpolation

**Purpose**: Get interpolated vertex data for shading.

**Scene Integration**:
- Calls [`gScene.getVertexData(instanceID, triangleIndex, barycentrics)`](Source/Falcor/Scene/Raytracing.slang:75)
- Scene provides vertex data
- Scene handles vertex fetching
- Scene handles interpolation

**Vertex Data Structure**:
- Position in world space
- Normal in world space
- Tangent in world space
- Texture coordinate
- Used for shading

**Overloads**:
- `getVertexData(instanceID, triangleIndex, attribs)` - Basic interpolation
- `getVertexData(instanceID, triangleIndex, attribs, out barycentrics, out vertices[3])` - With unpacked vertices
- `getVertexDataRayCones(instanceID, triangleIndex, attribs)` - With ray cones for texture LOD

**Ray Cones**:
- Used for texture LOD computation
- Improves texture quality for ray tracing
- Reduces texture aliasing
- Uses cone angle for LOD selection

### Previous Frame Position

**Purpose**: Get previous frame position for motion vectors.

**Scene Integration**:
- Calls [`gScene.getPrevPosW(instanceID, triangleIndex, barycentrics)`](Source/Falcor/Scene/Raytracing.slang:112)
- Scene provides previous frame position
- Scene handles vertex fetching
- Scene handles interpolation

**Barycentric Interpolation**:
- Same barycentric computation as current frame
- Used for position interpolation
- Interpolates previous frame vertices
- Returns world space position

**Motion Vectors**:
- Previous frame position: from getPrevPosW
- Current frame position: from getVertexData
- Motion vector: prevPosW - posW
- Used for temporal effects

### DXR System Values

**Purpose**: Use DXR-provided system values.

**InstanceID()**:
- System-provided instance ID
- Available in hit group shaders
- Identifies which instance was hit
- Used for instance-specific data

**GeometryIndex()**:
- System-provided geometry index
- Available in hit group shaders
- Identifies which geometry was hit
- Used for geometry-specific data

**PrimitiveIndex()**:
- System-provided primitive index
- Available in hit group shaders
- Identifies which primitive was hit
- Used for primitive-specific data

**BuiltInTriangleIntersectionAttributes**:
- System-provided intersection attributes
- Contains barycentric coordinates
- Used for vertex attribute interpolation
- Used for hit point computation

**Limitations**:
- Only available in hit group shaders (closestHit, anyHit, intersection)
- Not available in other shader stages
- Requires DXR 1.0 or higher

## Integration Points

### Scene Integration

**Acceleration Structure**:
- `gScene.rtAccel` - Scene ray tracing acceleration structure
- Used for ray intersection queries
- Top-Level Acceleration Structure (TLAS)
- Contains scene geometry

**Vertex Data Access**:
- [`gScene.getVertexData(instanceID, triangleIndex, barycentrics)`](Source/Falcor/Scene/Raytracing.slang:75) - Get vertex data
- [`gScene.getVertexData(instanceID, triangleIndex, barycentrics, vertices)`](Source/Falcor/Scene/Raytracing.slang:88) - Get vertex data with unpacked vertices
- [`gScene.getVertexDataRayCones(instanceID, triangleIndex, barycentrics)`](Source/Falcor/Scene/Raytracing.slang:100) - Get vertex data with ray cones
- [`gScene.getPrevPosW(instanceID, triangleIndex, barycentrics)`](Source/Falcor/Scene/Raytracing.slang:112) - Get previous frame position

### DXR Integration

**TraceRay**:
- Wraps DXR TraceRay function
- Uses scene acceleration structure
- Uses ray type count
- Simplifies ray tracing calls

**System Values**:
- `InstanceID()` - Instance ID from DXR
- `GeometryIndex()` - Geometry index from DXR
- `PrimitiveIndex()` - Primitive index from DXR
- `BuiltInTriangleIntersectionAttributes` - Intersection attributes from DXR

**Hit Group Shaders**:
- closestHit - Closest hit shader
- anyHit - Any hit shader
- intersection - Intersection shader
- Can access DXR system values

### Utils Integration

**Attributes Import**:
- `import Utils.Attributes` - Import shader attributes
- Provides shader attribute support
- Used for system value access

### Rendering Integration

**TexLODHelpers Import**:
- `import Rendering.Materials.TexLODHelpers` - Import texture LOD helpers
- Provides texture LOD computation
- Used for ray cone texture LOD

### Shading Integration

**Shading System**:
- `__exported import Scene.Shading` - Import shading system
- Provides shading data structures
- Provides shading functions
- Provides material evaluation

## Architecture Patterns

### Wrapper Pattern

- Wraps DXR TraceRay function
- Simplifies ray tracing calls
- Automatically uses scene parameters
- Reduces boilerplate code

### Helper Function Pattern

- `getGeometryInstanceID()` - Get geometry instance ID
- `getVertexData()` - Get interpolated vertex data (multiple overloads)
- `getPrevPosW()` - Get previous frame position
- `getRayTypeCount()` - Get ray type count
- Reusable functions for common operations

### Overload Pattern

- Multiple overloads of `getVertexData()`
- Basic interpolation
- With unpacked vertices
- With ray cones for texture LOD
- Flexible API for different use cases

### Barycentric Interpolation Pattern

- Compute barycentric coordinates from DXR attributes
- Use barycentric coordinates for interpolation
- Interpolate vertex attributes on triangle
- Standard technique for triangle interpolation

## Code Patterns

### TraceRay Wrapper Pattern

```slang
void TraceRayScene<payload_t>(
    uint rayFlags,
    uint instanceInclusionMask,
    uint rayType,
    uint missIndex,
    RayDesc ray,
    inout payload_t payload)
{
    TraceRay(gScene.rtAccel, rayFlags, instanceInclusionMask, rayType, rayTypeCount, missIndex, ray, payload);
}
```

### Geometry Instance ID Pattern

```slang
GeometryInstanceID getGeometryInstanceID()
{
    return GeometryInstanceID(InstanceID(), GeometryIndex());
}
```

### Barycentric Computation Pattern

```slang
float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
```

### Vertex Data Pattern

```slang
VertexData getVertexData(GeometryInstanceID instanceID, uint triangleIndex, BuiltInTriangleIntersectionAttributes attribs)
{
    float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
    return gScene.getVertexData(instanceID, triangleIndex, barycentrics);
}
```

### Previous Frame Position Pattern

```slang
float3 getPrevPosW(GeometryInstanceID instanceID, uint triangleIndex, BuiltInTriangleIntersectionAttributes attribs)
{
    float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
    return gScene.getPrevPosW(instanceID, triangleIndex, barycentrics);
}
```

## Use Cases

### Ray Tracing

- **Primary Rays**:
  - Trace primary rays from camera
  - Use TraceRayScene for ray tracing
  - Get vertex data for shading
  - Evaluate materials

- **Shadow Rays**:
  - Trace shadow rays to lights
  - Use TraceRayScene for ray tracing
  - Check for occlusion
  - Compute shadows

- **Reflection/Refraction Rays**:
  - Trace reflection/refraction rays
  - Use TraceRayScene for ray tracing
  - Get vertex data for shading
  - Evaluate materials

### Motion Vectors

- **Dynamic Geometry**:
  - Get previous frame position
  - Compute motion vectors
  - Use for temporal effects
  - Use for motion blur

- **Temporal Effects**:
  - Use motion vectors for temporal anti-aliasing
  - Use motion vectors for motion blur
  - Use motion vectors for temporal reprojection
  - Use motion vectors for temporal accumulation

### Texture LOD

- **Ray Cones**:
  - Use ray cones for texture LOD
  - Improve texture quality for ray tracing
  - Reduce texture aliasing
  - Use getVertexDataRayCones

- **Texture Sampling**:
  - Use interpolated texture coordinates
  - Use correct LOD for ray cones
  - Improve texture quality
  - Reduce texture aliasing

### Shading

- **Material Shading**:
  - Get vertex data for shading
  - Use vertex data for material evaluation
  - Evaluate materials per hit
  - Compute lighting

- **Lighting**:
  - Use vertex data for lighting calculations
  - Use normal for BRDF evaluation
  - Use tangent for normal mapping
  - Use texture coordinate for texture sampling

## Performance Considerations

### GPU Computation

**Ray Tracing**:
- O(1) per ray (single intersection test)
- TLAS traversal overhead
- Ray intersection testing overhead
- Hit group shader execution overhead

**Vertex Data Interpolation**:
- O(1) per hit (single interpolation)
- Barycentric coordinate computation
- Vertex data fetching
- Vertex data interpolation

**Previous Frame Position**:
- O(1) per hit (single interpolation)
- Barycentric coordinate computation
- Previous vertex data fetching
- Previous position interpolation

### Memory Access

**Acceleration Structure**:
- TLAS for ray intersection queries
- Depends on scene complexity
- Shared across all rays
- Efficient memory access

**Vertex Data**:
- Vertex data fetching from scene
- Barycentric interpolation
- Vertex data structure
- Efficient memory access

**Previous Vertex Data**:
- Previous vertex data fetching from scene
- Barycentric interpolation
- Previous position computation
- Efficient memory access

### Optimization Opportunities

**Ray Cones**:
- Use ray cones for texture LOD
- Improve texture quality for ray tracing
- Reduce texture aliasing
- Optimize texture sampling

**Barycentric Interpolation**:
- Compute barycentric coordinates once
- Reuse for multiple interpolations
- Reduce computation overhead
- Optimize vertex data fetching

**Wrapper Functions**:
- Use wrapper functions for common operations
- Reduce boilerplate code
- Simplify ray tracing calls
- Improve code readability

## Limitations

### Feature Limitations

- **DXR 1.0**:
  - Requires DXR 1.0 or higher
  - Not compatible with older hardware
  - Limited to DXR 1.0 features
  - No support for newer DXR features

- **Hit Group Shaders Only**:
  - `getGeometryInstanceID()` can only be called from hit group shaders
  - Cannot be called from other shader stages
  - Limited to closestHit, anyHit, intersection shaders
  - Not available in ray generation or miss shaders

- **Triangle-Only**:
  - Only supports triangle geometry
  - No support for other geometry types
  - Limited to triangle lists

### Performance Limitations

- **TLAS Traversal**:
  - TLAS traversal overhead
  - Depends on scene complexity
  - May be expensive for complex scenes
  - Optimized by acceleration structure

- **Vertex Data Fetching**:
  - Vertex data fetching from scene
  - May be expensive for large scenes
  - Depends on vertex buffer layout
  - Optimized by caching

### Integration Limitations

- **Scene Coupling**:
  - Tightly coupled to Falcor scene system
  - Requires scene to be initialized
  - Not suitable for standalone use
  - Requires specific scene implementation

- **DXR Coupling**:
  - Tightly coupled to DXR ray tracing
  - Requires DXR support
  - Not suitable for other ray tracing APIs
  - Requires DXR-compatible hardware

### Debugging Limitations

- **Limited Error Reporting**:
  - No detailed error messages
  - No logging of failures
  - Difficult to debug ray tracing issues
  - No validation of input parameters

## Best Practices

### Ray Tracing

- **Use Wrapper Functions**:
  - Use TraceRayScene for ray tracing
  - Simplify ray tracing calls
  - Automatically use scene parameters
  - Reduce boilerplate code

- **Ray Flags**:
  - Use appropriate ray flags for use case
  - Use RAY_FLAG_NONE for default behavior
  - Use RAY_FLAG_FORCE_OPAQUE for opaque geometry
  - Use RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES for triangle-only

- **Instance Inclusion Mask**:
  - Use 0xff for all instances
  - Use custom mask for instance filtering
  - Optimize ray tracing with filtering
  - Reduce unnecessary intersection tests

### Vertex Data

- **Barycentric Interpolation**:
  - Compute barycentric coordinates once
  - Reuse for multiple interpolations
  - Reduce computation overhead
  - Optimize vertex data fetching

- **Vertex Data Overloads**:
  - Use appropriate overload for use case
  - Use basic interpolation for simple cases
  - Use unpacked vertices for complex cases
  - Use ray cones for texture LOD

### Motion Vectors

- **Previous Frame Position**:
  - Get previous frame position for dynamic geometry
  - Compute motion vectors correctly
  - Use for temporal effects
  - Optimize for mixed static/dynamic scenes

- **Temporal Effects**:
  - Use motion vectors for temporal anti-aliasing
  - Use motion vectors for motion blur
  - Use motion vectors for temporal reprojection
  - Use motion vectors for temporal accumulation

### Texture LOD

- **Ray Cones**:
  - Use ray cones for texture LOD
  - Improve texture quality for ray tracing
  - Reduce texture aliasing
  - Use getVertexDataRayCones

- **Texture Sampling**:
  - Use interpolated texture coordinates
  - Use correct LOD for ray cones
  - Improve texture quality
  - Reduce texture aliasing

### Performance Optimization

- **Wrapper Functions**:
  - Use wrapper functions for common operations
  - Reduce boilerplate code
  - Simplify ray tracing calls
  - Improve code readability

- **Barycentric Interpolation**:
  - Compute barycentric coordinates once
  - Reuse for multiple interpolations
  - Reduce computation overhead
  - Optimize vertex data fetching

- **Ray Cones**:
  - Use ray cones for texture LOD
  - Improve texture quality for ray tracing
  - Reduce texture aliasing
  - Optimize texture sampling

### Debugging

- **Validate Input Parameters**:
  - Check ray flags are valid
  - Check instance inclusion mask is valid
  - Check ray type is valid
  - Check miss index is valid

- **Visual Debugging**:
  - Visualize ray directions
  - Visualize hit points
  - Visualize barycentric coordinates
  - Verify ray tracing

- **Performance Profiling**:
  - Measure ray tracing performance
  - Profile TLAS traversal
  - Profile vertex data fetching
  - Identify bottlenecks

## Progress Log

- **2026-01-08T00:06:00Z**: Raytracing analysis completed. Analyzed Raytracing.slang (115 lines) containing ray tracing shaders. Documented DxrPerFrame constant buffer with rayTypeCount, TraceRayScene convenience wrapper function for TraceRay with scene parameters, getGeometryInstanceID function for getting global geometry instance ID from DXR hit group shader, getVertexData function (3 overloads) for getting interpolated vertex attributes with barycentric interpolation, getVertexDataRayCones function for ray cone texture LOD, getPrevPosW function for getting previous frame position for motion vectors, and getRayTypeCount function for getting ray type count. Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.

## Completion Status

The Raytracing module is complete. All source files have been analyzed and documented with comprehensive technical specifications covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
