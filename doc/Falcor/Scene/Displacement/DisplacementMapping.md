# DisplacementMapping - Displacement Mapping Shaders

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **DisplacementMapping.slang** - Displacement mapping shaders (703 lines)

### External Dependencies

- **Scene/SceneTypes.slang** - Scene type definitions
- **Scene/Material/BasicMaterialData.slang** - Basic material data structures
- **Utils/Math/Ray.slang** - Ray mathematics
- **Utils/Geometry/IntersectionHelpers.slang** - Intersection helper functions
- **Scene/Displacement/DisplacementData.slang** - Displacement data structures

## Module Overview

DisplacementMapping provides comprehensive displacement mapping functionality for displaced triangle meshes. It includes displacement value mapping, shell min/max calculation, ray marching for height map estimation, bilinear patch intersection testing, displaced triangle intersection calculation, and displaced triangle normal computation. The system supports both shell and surface displacement, with multiple sampling modes and debug visualization options.

## Component Specifications

### DisplacementData Extension

**Purpose**: Extension to add displacement mapping functionality to DisplacementData struct.

**Methods**:

#### Value Mapping

- [`mapValue(const float raw)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:53) - Maps a raw value to a displacement value
  - Parameters: raw - Raw displacement texture value
  - Returns: float - Object-space distance
  - Implementation: `scale * (raw + bias)`

- [`mapValue(const float2 raw)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:58) - Maps a raw value to a displacement value
  - Parameters: raw - Raw displacement texture value (float2)
  - Returns: float2 - Object-space distance
  - Implementation: `scale * (raw + bias)`

- [`mapValue(const float4 raw)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:63) - Maps a raw value to a displacement value
  - Parameters: raw - Raw displacement texture value (float4)
  - Returns: float4 - Object-space distance
  - Implementation: `scale * (raw + bias)`

#### Value Reading

- [`readValue(const float2 texelPos, const float lod = 0.f)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:69) - Reads displacement value at a given texel
  - Parameters:
    - texelPos - Texture coordinate
    - lod - Level of detail (default: 0.f)
  - Returns: float - Displacement value
  - Implementation: `mapValue(texture.SampleLevel(samplerState, texelPos / size, lod).x)`
  - Note: Returns 0.f if DISPLACEMENT_DISABLED

- [`readValueMinMax(const float2 texelPos, const float lod = 0.f)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:78) - Reads displacement value at a given texel with min/max samplers
  - Parameters:
    - texelPos - Texture coordinate
    - lod - Level of detail (default: 0.f)
  - Returns: float4 - Displacement value (min/max)
  - Implementation: `mapValue(texture.SampleLevel(samplerState, texelPos / size, lod))`
  - Note: Returns float4(0.f) if DISPLACEMENT_DISABLED

#### Global Min/Max

- [`getConservativeGlobalExpansion()`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:87) - Gets conservative global expansion
  - Returns: float - Maximum expansion distance
  - Implementation: `max(abs(getGlobalMinMax().x), abs(getGlobalMinMax().y))`

- [`getGlobalMinMax()`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:92) - Gets global min/max displacement values
  - Returns: float2 - Min/max displacement values
  - Implementation: `float2(mapValue(0.f), mapValue(1.f))`
  - Note: Only valid if displacement map values are in [0, 1]
  - TODO: Sample largest MIP for real min/max values

#### Shell Min/Max

- [`getShellMinMax(const float2 texCrd0, const float2 texCrd1, const float2 texCrd2)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:99) - Gets shell min/max displacement for a triangle
  - Parameters:
    - texCrd0 - First texture coordinate
    - texCrd1 - Second texture coordinate
    - texCrd2 - Third texture coordinate
  - Returns: float2 - Min/max displacement values
  - Implementation:
    - Computes triangle center UV
    - Samples local displacement using min/max samplers
    - Uses conservative MIP level for triangle
    - Adds shell min-max margin
    - Handles two-sided displacement with surface thickness

- [`getTriangleConservativeMipLevel(const float2 texCrd0, const float2 texCrd1, const float2 texCrd2)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:138) - Gets conservative MIP level for a triangle
  - Parameters:
    - texCrd0 - First texture coordinate
    - texCrd1 - Second texture coordinate
    - texCrd2 - Third texture coordinate
  - Returns: float - MIP level
  - Implementation:
    - Computes average UV
    - Computes min/max UV
    - Computes bounding box UV
    - Computes max component UV
    - Computes triangle LOD: `ceil(log2(maxCompUV) + 4.f)`

### DisplacementIntersection Structure

**Purpose**: Displacement intersection data structure.

**Fields**:
- [`barycentrics`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:153) - `float2` - Barycentric coordinates
- [`displacement`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:154) - `float` - Displacement value
- [`t`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:155) - `float` - Intersection t value

**Methods**:

- [`getBarycentricWeights()`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:157) - Gets barycentric weights
  - Returns: float3 - Barycentric weights
  - Implementation: `float3(1.f - barycentrics.x - barycentrics.y, barycentrics.x, barycentrics.y)`

### IntersectAttribute Structure

**Purpose**: Intersection attribute structure for ray-bilinear patch intersection.

**Fields**:
- [`intersected`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:182) - `bool` - Intersection happened
- [`t`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:184) - `float` - Intersected t in, could be negative
- [`u`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:188) - `float` - Triangle intersection: barycentric coordinate {u, v, 1 - u - v}
- [`v`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:189) - `float` - Bilinear patch intersection: patch coordinate {u, v}, could use linear interpolation(u) to convert to projected
- [`barycentric`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:191) - `float3` - Triangle barycentric, v could be used to interpolated projected height in texture space
- [`textureSpaceHeight`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:192) - `float3` - Texture space height
- [`hitNormal`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:194) - `float3` - Hit normal (only valid for triangle intersection)
- [`backFace`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:195) - `bool` - Back face flag

**Methods**:

- [`max(const IntersectAttribute other)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:197) - Updates to max intersection
  - Parameters: other - Other intersection attribute
  - Returns: bool - True if updated
  - Implementation: Updates if other is intersected and has larger t value

- [`min(const IntersectAttribute other)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:216) - Updates to min intersection
  - Parameters: other - Other intersection attribute
  - Returns: bool - True if updated
  - Implementation: Updates if other is intersected and has smaller t value

- [`updateTriangleBarycentricAndHeight(const float texSpaceHeight)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:235) - Updates triangle barycentric and height
  - Parameters: texSpaceHeight - Texture space height
  - Implementation: `barycentric = float3(u, v, 1.f - u - v); textureSpaceHeight = texSpaceHeight;`

- [`updateBilinearPatchBarycentricAndHeight(const float3 p00Barycentric, const float3 p10Barycentric, const float minHeight, const float maxHeight)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:244) - Updates bilinear patch barycentric and height
  - Parameters:
    - p00Barycentric - P00 barycentric
    - p10Barycentric - P10 barycentric
    - minHeight - Minimum height
    - maxHeight - Maximum height
  - Implementation: `barycentric = lerp(p00Barycentric, p10Barycentric, u); textureSpaceHeight = lerp(minHeight, maxHeight, v);`

### Helper Functions

#### Triangle Intersection

- [`calcDebugTriangleIntersection(const Ray ray, const float3 positions[3], out DisplacementIntersection result)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:163) - Calculates debug triangle intersection
  - Parameters:
    - ray - Ray in world space
    - positions - Triangle positions
    - result - Intersection result (output)
  - Returns: bool - True if intersection occurred
  - Implementation: Uses intersectRayTriangle to test intersection

#### Solve T

- [`solveT(const float3 r, const float3 q, const float3 p)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:281) - Solves for t in ray equation
  - Parameters:
    - r - Ray origin
    - q - Ray direction
    - p - Intersected point
  - Returns: float - t value
  - Implementation: Solves t = (p - r) / q for dominant component

#### Copy Sign

- [`copySign(const float x, const float y)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:299) - Copies sign from x to y
  - Parameters:
    - x - X value
    - y - Y value
  - Returns: float - `abs(x) * sign(y)`

#### Ray Triangle Intersection Test

- [`rayTriangleIntersectionTest(float3 p0, float3 p1, float3 p2, float3 rayOrigin, float3 rayDirection, float triangleHeight, out IntersectAttribute intersectAttri)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:251) - Tests ray-triangle intersection
  - Parameters:
    - p0, p1, p2 - Triangle positions
    - rayOrigin - Ray origin
    - rayDirection - Ray direction
    - triangleHeight - Triangle height
    - intersectAttri - Intersection attribute (output)
  - Implementation: Uses intersectRayTriangle to test intersection, computes hit normal and back face

#### Ray Bilinear Patch Intersection Test

- [`rayBilinearPatchIntersectionTest(float3 q00, float3 q01, float3 q10, float3 q11, float3 p0Barycentric, float3 p1Barycentric, float3 rayOrigin, float3 rayDir, float rayTmax, float minHeight, float maxHeight, out IntersectAttribute intersectAttri0, out IntersectAttribute intersectAttri1)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:306) - Tests ray-bilinear patch intersection
  - Parameters:
    - q00, q01, q10, q11 - Bilinear patch corners
    - p0Barycentric, p1Barycentric - Barycentric coordinates
    - rayOrigin - Ray origin
    - rayDir - Ray direction
    - rayTmax - Maximum t value
    - minHeight - Minimum height
    - maxHeight - Maximum height
    - intersectAttri0 - First intersection attribute (output)
    - intersectAttri1 - Second intersection attribute (output)
  - Implementation: Implements geometric approach to ray-bilinear patch intersections from RT Gems Chapter 8
  - Reference: "Cool Patches: A Geometric Approach to Ray/Bilinear Patch Intersections" RT Gems

### Height Map Ray Marching

- [`traceHeightMapEstimated(const DisplacementData displacementData, float3 startPoint, float3 endPoint, out float intersectedT, out float intersectedHeight)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:416) - Traces height map with estimated intersection
  - Parameters:
    - displacementData - Displacement data
    - startPoint - Start point
    - endPoint - End point
    - intersectedT - Intersected t (output)
    - intersectedHeight - Intersected height (output)
  - Returns: bool - True if intersection found
  - Implementation:
    - Reads entry height data
    - Handles two-sided displacement with surface thickness
    - Performs ray marching with adaptive step count
    - Uses height delta ratio for interpolation
    - Estimates intersection t and height

### Displacement Intersection

- [`calcDisplacementIntersection(const Ray ray, const StaticVertexData vertices[3], const DisplacementData displacementData, out DisplacementIntersection result)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:471) - Calculates displacement intersection
  - Parameters:
    - ray - Ray in world space
    - vertices - Triangle vertices
    - displacementData - Displacement data
    - result - Intersection result (output)
  - Returns: bool - True if intersection found
  - Implementation:
    - Computes shell min/max displacement
    - Extrudes triangle to min/max heights
    - Performs ray "prism" intersection to grab valid start/end t
    - Performs slab/fin intersections for min/max heights
    - Tests base triangle intersection
    - Converts intersection data from object space to trace space
    - Traces height map in texture space
    - Interpolates barycentric coordinates
    - Returns result if valid

### Triangle Vertex Interpolation

- [`TriangleVertex::interpolate(const TriangleVertex vertices[3], const float3 barycentrics, const bool normalizeNormal = true)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:635) - Interpolates triangle vertex
  - Parameters:
    - vertices - Triangle vertices
    - barycentrics - Barycentric weights
    - normalizeNormal - Normalize normal flag (default: true)
  - Returns: TriangleVertex - Interpolated vertex
  - Implementation: Interpolates position, normal, and texture coordinates

### Displaced Triangle Intersection

- [`intersectDisplacedTriangle(const Ray ray, StaticVertexData vertices[3], const float4x4 worldMat, const DisplacementData displacementData, out DisplacementIntersection result)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:589) - Intersects displaced triangle
  - Parameters:
    - ray - Ray in world space
    - vertices - Triangle vertices
    - worldMat - Triangle object to world transform
    - displacementData - Displacement data
    - result - Intersection result (output)
  - Returns: bool - True if intersection found
  - Implementation:
    - Transforms vertices to world space
    - Computes real-length normals
    - Calls calcDisplacementIntersection for intersection
    - Transforms displacement to world space
    - Handles precise length normalization if kDisplacementScalingUsePreciseLength
    - Handles debug intersection if DISPLACEMENT_DISABLED

### Displaced Triangle Normal Computation

- [`computeDisplacedTriangleNormal(const float3 barycentrics, const StaticVertexData vertices[3], const float4x4 worldMat, const float3x3 worldInvTransposeMat, const DisplacementData displacementData)`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:659) - Computes displaced triangle normal
  - Parameters:
    - barycentrics - Barycentric weights
    - vertices - Triangle vertices
    - worldMat - Triangle object to world transform
    - worldInvTransposeMat - Triangle object to world inverse transpose transform
    - displacementData - Displacement data
  - Returns: float3 - Displaced normal in world space
  - Implementation:
    - Transforms vertices to world space
    - Computes barycentric delta for gradient sampling
    - Samples displacement at perturbed UV coordinates
    - Computes displaced positions
    - Computes normal using central gradient if kHitFaceNormalUseCentralGrad
    - Computes normal using simple cross product if not

## Technical Details

### Constants

- [`kShellForceMaxThickness`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:38) - `static const bool` - Force max thickness for shell displacement (false)
- [`kShellBoundsCalcUseSampleGrad`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:39) - `static const bool` - Use sample gradient for shell bounds calculation (true)
- [`kHitFaceNormalUseCentralGrad`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:40) - `static const bool` - Use central gradient for hit face normal (true)
- [`kDisplacementScalingUsePreciseLength`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:41) - `static const bool` - Use precise length for displacement scaling (false)
- [`kSurfaceThickness`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:43) - `static const float` - Surface thickness (0.008f)
- [`kRaymarchingMaxSampleCount`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:44) - `static const int` - Maximum ray marching sample count (32)
- [`kRaymarchingSampleCountFactor`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:45) - `static const int` - Ray marching sample count factor (2)

### Defines

- [`DISPLACEMENT_DISABLED`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:34) - `0` - Debug: Used to disable displacement mapping and intersect base triangles instead
- [`DISPLACEMENT_TWO_SIDED`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:35) - `0` - Displaced surface is two sided. This can create some cracks
- [`DISPLACEMENT_DEBUG_DISPLAY_SHELL`](Source/Falcor/Scene/Displacement/DisplacementMapping.slang:36) - `0` - Display shell instead of displaced surface

### Displacement Value Mapping

**Raw Value Mapping**:
- Maps raw displacement texture values to object-space distances
- Applies scale and bias
- Supports float, float2, and float4 values

**Texture Sampling**:
- Samples displacement texture at given texel coordinates
- Supports level-of-detail sampling
- Uses main sampler for normal sampling
- Uses min/max samplers for min/max queries

**Global Min/Max**:
- Computes conservative global expansion
- Returns global min/max displacement values
- Only valid if displacement map values are in [0, 1]
- TODO: Sample largest MIP for real min/max values

### Shell Displacement

**Shell Min/Max Calculation**:
- Computes triangle center UV
- Samples local displacement using min/max samplers
- Uses conservative MIP level for triangle
- Adds shell min-max margin
- Handles two-sided displacement with surface thickness

**Conservative MIP Level**:
- Computes average UV
- Computes min/max UV
- Computes bounding box UV
- Computes max component UV
- Computes triangle LOD using `ceil(log2(maxCompUV) + 4.f)`

### Ray Marching

**Height Map Estimation**:
- Reads entry height data
- Handles two-sided displacement with surface thickness
- Performs ray marching with adaptive step count
- Uses height delta ratio for interpolation
- Estimates intersection t and height

**Ray Marching Algorithm**:
- Computes view direction
- Computes max sample count: `min(maxCount * kRaymarchingSampleCountFactor, kRaymarchingMaxSampleCount)`
- Prevents freeze due to corner cases
- Uses incremental sampling along ray direction
- Detects height sign changes for intersection

### Ray-Bilinear Patch Intersection

**Geometric Approach**:
- Implements geometric approach to ray-bilinear patch intersections
- Based on RT Gems Chapter 8
- Reference: "Cool Patches: A Geometric Approach to Ray/Bilinear Patch Intersections"
- Solves quadratic equations for intersection
- Handles multiple intersection cases

**Intersection Testing**:
- Tests ray against bilinear patch
- Updates min/max intersection attributes
- Computes barycentric coordinates
- Computes texture space height

### Displacement Intersection

**Ray "Prism" Intersection**:
- Performs ray "prism" intersection to grab valid start/end t for valid segment
- Extrudes triangle to min/max heights
- Tests extruded triangle intersection

**Slab/Fin Intersection**:
- Performs slab/fin intersections for min/max heights
- Tests ray against three slab/fin combinations
- Updates min/max intersection attributes

**Base Triangle Intersection**:
- Tests base triangle intersection
- Updates min/max intersection attributes
- Converts intersection data from object space to trace space

**Height Map Tracing**:
- Traces height map in texture space between min and max intersections
- Interpolates barycentric coordinates
- Computes texture space coordinates
- Computes intersection t and height
- Supports debug display of shell

### Displaced Triangle Intersection

**World Space Transformation**:
- Transforms vertices to world space
- Computes real-length normals
- Calls calcDisplacementIntersection for intersection

**Displacement Scaling**:
- Transforms displacement to world space
- Uses precise length normalization if kDisplacementScalingUsePreciseLength
- Uses simple length normalization otherwise

**Debug Intersection**:
- Handles debug intersection if DISPLACEMENT_DISABLED
- Tests base triangle intersection in object space

### Displaced Triangle Normal Computation

**Barycentric Delta**:
- Computes barycentric delta for gradient sampling
- Uses texture size for delta calculation

**Displacement Sampling**:
- Samples displacement at perturbed UV coordinates
- Uses sample LOD for gradient computation

**Normal Computation**:
- Uses central gradient if kHitFaceNormalUseCentralGrad
- Computes normal using simple cross product otherwise
- Normalizes result

## Integration Points

### Scene Integration

**Scene Type Integration**:
- Uses SceneTypes for geometry type definitions
- Uses VertexData for vertex data structures

**Material Integration**:
- Uses BasicMaterialData for material data structures

**Geometry Integration**:
- Uses IntersectionHelpers for intersection helper functions
- Uses Ray for ray mathematics

### Displacement Data Integration

**DisplacementData Extension**:
- Extends DisplacementData struct with displacement mapping functionality
- Provides unified interface for displacement operations

### Rendering Integration

**Displaced Triangle Mesh Rendering**:
- Supports displaced triangle mesh rendering
- Supports shell and surface displacement
- Supports two-sided displacement
- Supports debug display of shell

**Ray Tracing Integration**:
- Supports ray tracing against displaced triangles
- Supports height map estimation
- Supports bilinear patch intersection
- Supports displacement intersection

## Architecture Patterns

### Extension Pattern

- Extends DisplacementData struct with displacement mapping functionality
- Provides unified interface for displacement operations
- Simplifies resource passing

### Intersection Pattern

- Multiple intersection testing strategies
- Ray-triangle intersection
- Ray-bilinear patch intersection
- Ray "prism" intersection
- Slab/fin intersection

### Ray Marching Pattern

- Adaptive ray marching for height map estimation
- Height delta ratio for interpolation
- Efficient sampling along ray direction

### Normal Computation Pattern

- Multiple normal computation strategies
- Central gradient method
- Simple cross product method
- Precise length normalization

## Code Patterns

### DisplacementData Extension Pattern

```slang
extension DisplacementData
{
    float mapValue(const float raw)
    {
        return scale * (raw + bias);
    }

    float2 mapValue(const float2 raw)
    {
        return scale * (raw + bias);
    }

    float4 mapValue(const float4 raw)
    {
        return scale * (raw + bias);
    }

    float readValue(const float2 texelPos, const float lod = 0.f)
    {
        #if DISPLACEMENT_DISABLED == 0
            return mapValue(texture.SampleLevel(samplerState, texelPos / size, lod).x);
        #else
            return 0.f;
        #endif
    }
}
```

### Ray Marching Pattern

```slang
bool traceHeightMapEstimated(const DisplacementData displacementData, float3 startPoint, float3 endPoint, out float intersectedT, out float intersectedHeight)
{
    const float entryHeightData = displacementData.readValue(startPoint.xy);
    float3 viewDirection = endPoint - startPoint;
    int maxCount = ceil(abs(viewDirection.x)) + ceil(abs(viewDirection.y)) + 2;
    maxCount = min(maxCount * kRaymarchingSampleCountFactor, kRaymarchingMaxSampleCount);
    float3 incrementUnit = viewDirection / maxCount;
    viewDirection = normalize(viewDirection);
    const float tMax = solveT(startPoint, viewDirection, endPoint);

    float3 lastTestPoint;
    float3 currentTestPoint = startPoint;
    float lastHeightDelta = currentTestPoint.z - entryHeightData;

    for (int i = 0; i < maxCount; i++)
    {
        lastTestPoint = currentTestPoint;
        currentTestPoint += incrementUnit;

        const float heightData = displacementData.readValue(currentTestPoint.xy);
        const float currentHeightDelta = currentTestPoint.z - heightData;

        #if DISPLACEMENT_TWO_SIDED == 0
            if (currentHeightDelta < 0.f)
            #else
            if ( ((currentHeightDelta <= 0.f) && (currentHeightDelta > -kSurfaceThickness)) || (sign(lastHeightDelta) != sign(currentHeightDelta)) )
            #endif
            {
                float heightDeltaRatio = abs(currentHeightDelta) / (abs(currentHeightDelta) + abs(lastHeightDelta));
                float estimatedT = lerp(float(i) + 1.f, float(i), heightDeltaRatio);

                intersectedT = tMax * (estimatedT / maxCount);
                intersectedHeight = lerp(currentTestPoint.z, lastTestPoint.z, heightDeltaRatio);

                return true;
            }
            lastHeightDelta = currentHeightDelta;
        }
    }

    return false;
}
```

### Displaced Triangle Intersection Pattern

```slang
bool calcDisplacementIntersection(const Ray ray, const StaticVertexData vertices[3], const DisplacementData displacementData, out DisplacementIntersection result)
{
    result = {};

    const float2 shellMinMax = displacementData.getShellMinMax(vertices[0].texCrd, vertices[1].texCrd, vertices[2].texCrd);
    const float minHeight = shellMinMax.x;
    const float maxHeight = shellMinMax.y;

    float3 extrudedP0 = vertices[0].position + vertices[0].normal * maxHeight;
    float3 extrudedP1 = vertices[1].position + vertices[1].normal * maxHeight;
    float3 extrudedP2 = vertices[2].position + vertices[2].normal * maxHeight;
    float3 intrudedP0 = vertices[0].position + vertices[0].normal * minHeight;
    float3 intrudedP1 = vertices[1].position + vertices[1].normal * minHeight;
    float3 intrudedP2 = vertices[2].position + vertices[2].normal * minHeight;

    // ray "prism" intersection to grab valid start/end t for valid segment along ray direction
    float tStart = ray.tMin;

    // extruded triangle
    IntersectAttribute extrudedTriangleIntersectAttri;
    rayTriangleIntersectionTest(extrudedP0, extrudedP1, extrudedP2, ray.origin, ray.dir, maxHeight,
        extrudedTriangleIntersectAttri);

    // min: entry point, max: exit point
    IntersectAttribute minIntersection = extrudedTriangleIntersectAttri;
    IntersectAttribute maxIntersection = extrudedTriangleIntersectAttri;

    // first slab/fin
    IntersectAttribute slabIntersectAttri0;
    IntersectAttribute slabIntersectAttri1;
    rayBilinearPatchIntersectionTest(intrudedP0, extrudedP0, intrudedP1, extrudedP1, float3(1, 0, 0), float3(0, 1, 0),
        ray.origin, ray.dir, ray.tMax, minHeight, maxHeight, slabIntersectAttri0, slabIntersectAttri1);
    minIntersection.min(slabIntersectAttri0);
    maxIntersection.max(slabIntersectAttri0);
    maxIntersection.max(slabIntersectAttri1);

    // second slab/fin
    rayBilinearPatchIntersectionTest(intrudedP1, extrudedP1, intrudedP2, extrudedP2, float3(0, 1, 0), float3(0, 0, 1),
        ray.origin, ray.dir, ray.tMax, minHeight, maxHeight, slabIntersectAttri0, slabIntersectAttri1);
    minIntersection.min(slabIntersectAttri0);
    maxIntersection.max(slabIntersectAttri0);
    maxIntersection.max(slabIntersectAttri1);

    // third slab/fin
    rayBilinearPatchIntersectionTest(intrudedP2, extrudedP2, intrudedP0, extrudedP1, float3(0, 0, 1), float3(0, 0, 1),
        ray.origin, ray.dir, ray.tMax, minHeight, maxHeight, slabIntersectAttri0, slabIntersectAttri1);
    minIntersection.min(slabIntersectAttri0);
    maxIntersection.max(slabIntersectAttri0);
    maxIntersection.max(slabIntersectAttri1);

    // test base triangle lastly
    IntersectAttribute baseTriangleIntersectAttri;
    rayTriangleIntersectionTest(intrudedP0, intrudedP1, intrudedP2, ray.origin, ray.dir, minHeight,
        baseTriangleIntersectAttri);
    minIntersection.min(baseTriangleIntersectAttri);
    maxIntersection.max(baseTriangleIntersectAttri);

    // Early out when no hit.
    if (!(minIntersection.intersected && maxIntersection.intersected) || (minIntersection.t == maxIntersection.t) || (minIntersection.t > ray.tMax) || (maxIntersection.t < ray.tMin) )
    {
        return false;
    }

    // 2. convert intersection data from object space to trace space
    float2 minIntersectionUV = minIntersection.barycentric[0] * vertices[0].texCrd.xy + minIntersection.barycentric[1] * vertices[1].texCrd.xy + minIntersection.barycentric[2] * vertices[2].texCrd.xy;
    float2 maxIntersectionUV = maxIntersection.barycentric[0] * vertices[0].texCrd.xy + maxIntersection.barycentric[1] * vertices[1].texCrd.xy + maxIntersection.barycentric[2] * vertices[2].texCrd.xy;

    // start/end point texture space point
    float3 minIntersectionTexSpaceCoord = float3(minIntersectionUV * displacementData.size, minIntersection.textureSpaceHeight);
    float3 maxIntersectionTexSpaceCoord = float3(maxIntersectionUV * displacementData.size, maxIntersection.textureSpaceHeight);
    float validStartT = max(tStart, minIntersection.t);
    float validEndT = maxIntersection.t);
    minIntersectionTexSpaceCoord = lerp(minIntersectionTexSpaceCoord, maxIntersectionTexSpaceCoord,     (validStartT - minIntersection.t) / (maxIntersection.t - minIntersection.t));
    float3 startBarycentric = lerp(minIntersection.barycentric, maxIntersection.barycentric,            (validStartT - minIntersection.t) / (maxIntersection.t - minIntersection.t));

    // 3. trace height map in texture space
    float intersectedT = 0.f;
    float intersectedHeight = 0.f;
    bool ret = true;
    #if DISPLACEMENT_DEBUG_DISPLAY_SHELL == 0
        ret = traceHeightMapEstimated(displacementData, minIntersectionTexSpaceCoord, maxIntersectionTexSpaceCoord, intersectedT, intersectedHeight);

        if (ret)
        {
            const float3 traceSpaceViewDirection = normalize(maxIntersectionTexSpaceCoord - minIntersectionTexSpaceCoord);
            // get t max in trace space.
            const float tMax = solveT(minIntersectionTexSpaceCoord, traceSpaceViewDirection, maxIntersectionTexSpaceCoord);
            // intersectedT is in trace space, interpolate barycentric in there.
            float3 intersectionProjectedBarycentric = lerp(startBarycentric, endBarycentric, intersectedT / tMax);
            result.barycentrics = float2(intersectionProjectedBarycentric.y, 1.f - intersectionProjectedBarycentric.x - intersectionProjectedBarycentric.y);
            result.displacement = intersectedHeight;
            result.t = lerp(validStartT, validEndT, intersectedT / tMax);

            return result.t > ray.tMin && result.t < ray.tMax;
        }
    #else
        IntersectAttribute debugIntersection = maxIntersection;
        if (debugIntersection.intersected)
        {
            result.t = debugIntersection.t;
            result.barycentrics = float2(debugIntersection.barycentric.y, 1.f - debugIntersection.barycentric.x - debugIntersection.barycentric.y);
            result.displacement = 0.f;
            return result.t > ray.tMin && result.t < ray.tMax;
        }
    #endif

    return false;
}
```

## Use Cases

### Displacement Mapping

- **Texture-Based Displacement**:
  - Use displacement texture for displacement mapping
  - Map raw displacement values to object-space distances
  - Apply scale and bias to displacement values

- **Multi-Sampling**:
  - Use main sampler for normal displacement sampling
  - Use min/max samplers for min/max displacement queries
  - Support for shell displacement

### Shell Displacement

- **Shell Min/Max Calculation**:
  - Compute shell min/max displacement for triangle
  - Use conservative MIP level for robust bounds
  - Handle two-sided displacement with surface thickness

### Height Map Ray Marching

- **Height Map Estimation**:
  - Trace height map between start and end points
  - Use adaptive ray marching for efficiency
  - Estimate intersection t and height

### Ray Tracing

- **Displaced Triangle Intersection**:
  - Intersect ray against displaced triangle
  - Use ray "prism" intersection for robust bounds
  - Use slab/fin intersection for precise intersection
  - Trace height map for accurate intersection

- **Bilinear Patch Intersection**:
  - Intersect ray against bilinear patch
  - Use geometric approach from RT Gems
  - Handle multiple intersection cases

### Displaced Triangle Normal Computation

- **Normal Calculation**:
  - Compute displaced triangle normal
  - Use central gradient for accuracy
  - Use simple cross product for efficiency
  - Normalize result

## Performance Considerations

### Memory Performance

**Compact Data Structures**:
- Compact intersection data structures
- Efficient memory layout for barycentric coordinates
- Minimal memory overhead

### Computation Performance

**Ray Marching**:
- Adaptive ray marching for efficiency
- Height delta ratio for accurate interpolation
- Efficient sampling along ray direction

**Intersection Testing**:
- Multiple intersection strategies for robustness
- Early exit for no-hit cases
- Efficient barycentric coordinate computation

**Normal Computation**:
- Multiple normal computation strategies
- Central gradient for accuracy
- Simple cross product for efficiency

## Limitations

### Feature Limitations

- **Displacement Limitations**:
- Limited to single displacement texture
- Limited to Texture2D
- Limited to 3 samplers (main, min, max)

- **Shell Displacement Limitations**:
- Two-sided displacement can create cracks
- Surface thickness is fixed (0.008f)
- Conservative MIP level may not be optimal

- **Ray Marching Limitations**:
- Adaptive step count may not be optimal for all cases
- Height delta ratio may not be accurate for all cases
- May have precision issues

### Performance Limitations

- **Ray Marching Overhead**:
- Multiple samples along ray direction
- May be expensive for long rays
- May not be optimal for real-time rendering

- **Intersection Testing Overhead**:
- Multiple intersection tests
- May be expensive for complex geometry
- May not be optimal for real-time rendering

### Integration Limitations

- **Displacement System Coupling**:
- Tightly coupled to displacement system
- Requires specific displacement texture format
- Not suitable for standalone use

## Best Practices

### Displacement Mapping

- **Texture Sampling**:
- Use appropriate sampler for displacement sampling
- Use main sampler for normal displacement
- Use min/max samplers for shell displacement

- **Parameter Tuning**:
- Use scale parameter to control displacement magnitude
- Use bias parameter to control displacement offset
- Tune parameters for visual quality

### Shell Displacement

- **Conservative MIP Level**:
- Use conservative MIP level for robust bounds
- Handle two-sided displacement with surface thickness
- Use shell min-max margin for safety

### Ray Marching

- **Adaptive Sampling**:
- Use adaptive step count for efficiency
- Use height delta ratio for accurate interpolation
- Prevent freeze due to corner cases

### Intersection Testing

- **Early Exit**:
- Use early exit for no-hit cases
- Minimize unnecessary computations
- Optimize for performance

### Normal Computation

- **Strategy Selection**:
- Use central gradient for accuracy
- Use simple cross product for efficiency
- Normalize result for correctness

## Progress Log

- **2026-01-08T01:15:00Z**: DisplacementMapping analysis completed. Analyzed DisplacementMapping.slang (703 lines) containing comprehensive displacement mapping shaders. Documented DisplacementData extension with methods (mapValue for float/float2/float4, readValue, readValueMinMax, getConservativeGlobalExpansion, getGlobalMinMax, getShellMinMax, getTriangleConservativeMipLevel), DisplacementIntersection structure with fields (barycentrics, displacement, t) and method (getBarycentricWeights), IntersectAttribute structure with fields (intersected, t, u, v, barycentric, textureSpaceHeight, hitNormal, backFace) and methods (max, min, updateTriangleBarycentricAndHeight, updateBilinearPatchBarycentricAndHeight), helper functions (calcDebugTriangleIntersection, solveT, copySign), ray triangle intersection test, ray bilinear patch intersection test (based on RT Gems Chapter 8), height map ray marching with estimated intersection, displacement intersection calculation, displaced triangle intersection, triangle vertex interpolation, displaced triangle normal computation, constants (kShellForceMaxThickness, kShellBoundsCalcUseSampleGrad, kHitFaceNormalUseCentralGrad, kDisplacementScalingUsePreciseLength, kSurfaceThickness, kRaymarchingMaxSampleCount, kRaymarchingSampleCountFactor), defines (DISPLACEMENT_DISABLED, DISPLACEMENT_TWO_SIDED, DISPLACEMENT_DEBUG_DISPLAY_SHELL), imports (Scene.SceneTypes, Scene.Material.BasicMaterialData, Utils.Math.Ray, Utils.Geometry.IntersectionHelpers, Scene.Displacement.DisplacementData), extension pattern, intersection pattern, ray marching pattern, normal computation pattern, integration points (scene, material, geometry, displacement data), use cases (displacement mapping, shell displacement, height map ray marching, ray tracing, displaced triangle intersection, normal computation), performance considerations (memory performance, computation performance), limitations (feature limitations, performance limitations, integration limitations), and best practices (displacement mapping, shell displacement, ray marching, intersection testing, normal computation). Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.

## Completion Status

The DisplacementMapping module is complete. All source files have been analyzed and documented with comprehensive technical specifications covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
