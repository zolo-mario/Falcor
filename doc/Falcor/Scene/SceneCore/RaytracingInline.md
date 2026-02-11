# RaytracingInline - Inline Ray Tracing Utilities

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **RaytracingInline.slang** - Inline ray tracing shader

### External Dependencies

- **Utils/Attributes** - Shader attributes
- **Scene/Intersection** - Intersection helpers
- **Scene/SDFs/SDFGridHitData** - SDF grid hit data
- **Scene/Shading** - Shading system
- **Scene/SceneRayQueryInterface** - Scene ray query interface
- **Scene/SceneDefines** - Scene defines

## Module Overview

RaytracingInline provides utilities for inline ray tracing using DXR 1.1. It includes functions for getting geometry types, creating triangle hits, tracing scene rays with alpha testing and procedural geometry support, tracing visibility rays, and a scene ray query implementation. The module supports triangle geometry, displaced triangle meshes, curves, and SDF grids with configurable alpha testing.

## Component Specifications

### getCommittedGeometryType Function

**Purpose**: Return geometry type for a committed hit.

**Signature**:
```slang
GeometryType getCommittedGeometryType<let Flags : int>(RayQuery<Flags> rayQuery)
```

**Parameters**:
- [`rayQuery`](Source/Falcor/Scene/RaytracingInline.slang:44) - `RayQuery<Flags>` - RayQuery object

**Template Parameter**:
- `Flags : int` - Ray query flags

**Implementation**:
- Gets instance ID: [`GeometryInstanceID(rayQuery.CommittedInstanceID(), rayQuery.CommittedGeometryIndex())`](Source/Falcor/Scene/RaytracingInline.slang:46)
- Returns geometry type: [`gScene.getGeometryInstanceType(instanceID)`](Source/Falcor/Scene/RaytracingInline.slang:47)

**DXR System Values**:
- `rayQuery.CommittedInstanceID()` - Committed instance ID from DXR
- `rayQuery.CommittedGeometryIndex()` - Committed geometry index from DXR

**Usage**:
- Can only be called from hit group shaders
- Returns geometry type for committed hit
- Used for hit-specific processing
- Used for procedural geometry handling

### getCandidateGeometryType Function

**Purpose**: Return geometry type for a candidate hit.

**Signature**:
```slang
GeometryType getCandidateGeometryType<let Flags : int>(RayQuery<Flags> rayQuery)
```

**Parameters**:
- [`rayQuery`](Source/Falcor/Scene/RaytracingInline.slang:52) - `RayQuery<Flags>` - RayQuery object

**Template Parameter**:
- `Flags : int` - Ray query flags

**Implementation**:
- Gets instance ID: [`GeometryInstanceID(rayQuery.CandidateInstanceID(), rayQuery.CandidateGeometryIndex())`](Source/Falcor/Scene/RaytracingInline.slang:54)
- Returns geometry type: [`gScene.getGeometryInstanceType(instanceID)`](Source/Falcor/Scene/RaytracingInline.slang:55)

**DXR System Values**:
- `rayQuery.CandidateInstanceID()` - Candidate instance ID from DXR
- `rayQuery.CandidateGeometryIndex()` - Candidate geometry index from DXR

**Usage**:
- Can only be called from hit group shaders
- Returns geometry type for candidate hit
- Used for alpha testing
- Used for procedural geometry handling

### getCommittedTriangleHit Function

**Purpose**: Create a TriangleHit for a RayQuery committed hit on triangles.

**Signature**:
```slang
TriangleHit getCommittedTriangleHit<let Flags : int>(RayQuery<Flags> rayQuery)
```

**Parameters**:
- [`rayQuery`](Source/Falcor/Scene/RaytracingInline.slang:62) - `RayQuery<Flags>` - RayQuery object

**Template Parameter**:
- `Flags : int` - Ray query flags

**Implementation**:
- Creates [`TriangleHit`](Source/Falcor/Scene/RaytracingInline.slang:64) structure
- Sets instance ID: [`hit.instanceID = GeometryInstanceID(rayQuery.CommittedInstanceID(), rayQuery.CommittedGeometryIndex())`](Source/Falcor/Scene/RaytracingInline.slang:65)
- Sets primitive index: [`hit.primitiveIndex = rayQuery.CommittedPrimitiveIndex()`](Source/Falcor/Scene/RaytracingInline.slang:66)
- Sets barycentrics: [`hit.barycentrics = rayQuery.CommittedTriangleBarycentrics()`](Source/Falcor/Scene/RaytracingInline.slang:67)
- Returns [`TriangleHit`](Source/Falcor/Scene/RaytracingInline.slang:68) structure

**DXR System Values**:
- `rayQuery.CommittedInstanceID()` - Committed instance ID from DXR
- `rayQuery.CommittedGeometryIndex()` - Committed geometry index from DXR
- `rayQuery.CommittedPrimitiveIndex()` - Committed primitive index from DXR
- `rayQuery.CommittedTriangleBarycentrics()` - Committed triangle barycentrics from DXR

**Usage**:
- Can only be called from hit group shaders
- Creates TriangleHit for committed hit
- Used for hit information extraction
- Used for shading

### getCandidateTriangleHit Function

**Purpose**: Create a TriangleHit for a RayQuery candidate hit on triangles.

**Signature**:
```slang
TriangleHit getCandidateTriangleHit<let Flags : int>(RayQuery<Flags> rayQuery)
```

**Parameters**:
- [`rayQuery`](Source/Falcor/Scene/RaytracingInline.slang:75) - `RayQuery<Flags>` - RayQuery object

**Template Parameter**:
- `Flags : int` - Ray query flags

**Implementation**:
- Creates [`TriangleHit`](Source/Falcor/Scene/RaytracingInline.slang:77) structure
- Sets instance ID: [`hit.instanceID = GeometryInstanceID(rayQuery.CandidateInstanceID(), rayQuery.CandidateGeometryIndex())`](Source/Falcor/Scene/RaytracingInline.slang:78)
- Sets primitive index: [`hit.primitiveIndex = rayQuery.CandidatePrimitiveIndex()`](Source/Falcor/Scene/RaytracingInline.slang:79)
- Sets barycentrics: [`hit.barycentrics = rayQuery.CandidateTriangleBarycentrics()`](Source/Falcor/Scene/RaytracingInline.slang:80)
- Returns [`TriangleHit`](Source/Falcor/Scene/RaytracingInline.slang:81) structure

**DXR System Values**:
- `rayQuery.CandidateInstanceID()` - Candidate instance ID from DXR
- `rayQuery.CandidateGeometryIndex()` - Candidate geometry index from DXR
- `rayQuery.CandidatePrimitiveIndex()` - Candidate primitive index from DXR
- `rayQuery.CandidateTriangleBarycentrics()` - Candidate triangle barycentrics from DXR

**Usage**:
- Can only be called from hit group shaders
- Creates TriangleHit for candidate hit
- Used for alpha testing
- Used for procedural geometry handling

### traceSceneRayImpl Function

**Purpose**: Main ray tracing implementation with alpha testing and procedural geometry support.

**Signature**:
```slang
HitInfo traceSceneRayImpl<let Flags : int>(const bool useAlphaTest, inout RayQuery<Flags> rayQuery, const Ray ray, out float hitT, uint rayFlags, uint instanceInclusionMask)
```

**Parameters**:
- [`useAlphaTest`](Source/Falcor/Scene/RaytracingInline.slang:86) - `bool` - Whether to use alpha testing
- [`rayQuery`](Source/Falcor/Scene/RaytracingInline.slang:86) - `inout RayQuery<Flags>` - RayQuery object
- [`ray`](Source/Falcor/Scene/RaytracingInline.slang:86) - `Ray` - Ray to trace
- [`hitT`](Source/Falcor/Scene/RaytracingInline.slang:86) - `out float` - Output hit t
- [`rayFlags`](Source/Falcor/Scene/RaytracingInline.slang:86) - `uint` - Ray flags
- [`instanceInclusionMask`](Source/Falcor/Scene/RaytracingInline.slang:86) - `uint` - Instance inclusion mask

**Template Parameter**:
- `Flags : int` - Ray query flags

**Implementation**:
1. **Trace Ray**:
   - Calls [`rayQuery.TraceRayInline(gScene.rtAccel, rayFlags, instanceInclusionMask, ray.toRayDesc())`](Source/Falcor/Scene/RaytracingInline.slang:88)
   - Uses scene acceleration structure
   - Uses ray flags and instance inclusion mask

2. **Initialize Attribs**:
   - Creates [`CurveIntersector::Attribs curveCommittedAttribs`](Source/Falcor/Scene/RaytracingInline.slang:90)
   - Creates [`DisplacedTriangleMeshIntersector::Attribs displacedTriangleMeshCommittedAttribs`](Source/Falcor/Scene/RaytracingInline.slang:91)
   - Creates [`SDFGridHitData sdfGridCommittedHitData`](Source/Falcor/Scene/RaytracingInline.slang:92)

3. **Process Intersections**:
   - While loop: [`while (rayQuery.Proceed())`](Source/Falcor/Scene/RaytracingInline.slang:94)
   - Processes all intersections

4. **Alpha Testing for Triangles**:
   - Check if alpha testing enabled and candidate is non-opaque triangle: [`if (useAlphaTest && rayQuery.CandidateType() == CANDIDATE_NON_OPAQUE_TRIANGLE)`](Source/Falcor/Scene/RaytracingInline.slang:97)
   - Gets candidate triangle hit: [`const TriangleHit hit = getCandidateTriangleHit(rayQuery)`](Source/Falcor/Scene/RaytracingInline.slang:100)
   - Gets vertex data: [`const VertexData v = gScene.getVertexData(hit)`](Source/Falcor/Scene/RaytracingInline.slang:101)
   - Gets material ID: [`const uint materialID = gScene.getMaterialID(hit.instanceID)`](Source/Falcor/Scene/RaytracingInline.slang:102)
   - Performs alpha test: [`if (gScene.materials.alphaTest(v, materialID, 0.f)) continue;`](Source/Falcor/Scene/RaytracingInline.slang:104)
   - Commits non-opaque triangle hit: [`rayQuery.CommitNonOpaqueTriangleHit();`](Source/Falcor/Scene/RaytracingInline.slang:106)

5. **Procedural Geometry**:
   - Check if candidate is procedural primitive: [`if (rayQuery.CandidateType() == CANDIDATE_PROCEDURAL_PRIMITIVE)`](Source/Falcor/Scene/RaytracingInline.slang:110)
   - Gets instance ID: [`const GeometryInstanceID instanceID = GeometryInstanceID(rayQuery.CandidateInstanceID(), rayQuery.CandidateGeometryIndex())`](Source/Falcor/Scene/RaytracingInline.slang:112)
   - Gets primitive index: [`const uint primitiveIndex = rayQuery.CandidatePrimitiveIndex()`](Source/Falcor/Scene/RaytracingInline.slang:113)
   - Gets geometry type: [`const GeometryType type = gScene.getGeometryInstanceType(instanceID)`](Source/Falcor/Scene/RaytracingInline.slang:114)
   - Creates ray segment: [`const Ray raySegment = Ray(ray.origin, ray.dir, rayQuery.RayTMin(), rayQuery.CommittedRayT())`](Source/Falcor/Scene/RaytracingInline.slang:115)
   - Switch on geometry type

6. **Displaced Triangle Mesh**:
   - Case: [`GeometryType::DisplacedTriangleMesh`](Source/Falcor/Scene/RaytracingInline.slang:119)
   - Creates attribs: [`DisplacedTriangleMeshIntersector::Attribs attribs`](Source/Falcor/Scene/RaytracingInline.slang:121)
   - Intersects ray: [`if (DisplacedTriangleMeshIntersector::intersect(raySegment, instanceID, primitiveIndex, attribs, t))`](Source/Falcor/Scene/RaytracingInline.slang:123)
   - Commits procedural primitive hit: [`rayQuery.CommitProceduralPrimitiveHit(t);`](Source/Falcor/Scene/RaytracingInline.slang:125)
   - Stores attribs: [`displacedTriangleMeshCommittedAttribs = attribs`](Source/Falcor/Scene/RaytracingInline.slang:126)

7. **Curve**:
   - Case: [`GeometryType::Curve`](Source/Falcor/Scene/RaytracingInline.slang:132)
   - Creates attribs: [`CurveIntersector::Attribs attribs`](Source/Falcor/Scene/RaytracingInline.slang:134)
   - Intersects ray: [`if (CurveIntersector::intersect(raySegment, instanceID, primitiveIndex, attribs, t))`](Source/Falcor/Scene/RaytracingInline.slang:136)
   - Commits procedural primitive hit: [`rayQuery.CommitProceduralPrimitiveHit(t);`](Source/Falcor/Scene/RaytracingInline.slang:138)
   - Stores attribs: [`curveCommittedAttribs = attribs`](Source/Falcor/Scene/RaytracingInline.slang:139)

8. **SDF Grid**:
   - Case: [`GeometryType::SDFGrid`](Source/Falcor/Scene/RaytracingInline.slang:145)
   - Creates hit data: [`SDFGridHitData sdfGridHitData`](Source/Falcor/Scene/RaytracingInline.slang:147)
   - Intersects ray: [`if (SDFGridIntersector::intersect(raySegment, instanceID, primitiveIndex, sdfGridHitData, t))`](Source/Falcor/Scene/RaytracingInline.slang:149)
   - Commits procedural primitive hit: [`rayQuery.CommitProceduralPrimitiveHit(t);`](Source/Falcor/Scene/RaytracingInline.slang:151)
   - Stores hit data: [`sdfGridCommittedHitData = sdfGridHitData`](Source/Falcor/Scene/RaytracingInline.slang:152)

9. **Create HitInfo**:
   - Creates invalid hit: [`HitInfo hit = makeInvalidHit()`](Source/Falcor/Scene/RaytracingInline.slang:164)
   - Sets hit t: [`hitT = 0.f`](Source/Falcor/Scene/RaytracingInline.slang:165)

10. **Triangle Hit**:
    - Check if committed status is triangle hit: [`if (rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT)`](Source/Falcor/Scene/RaytracingInline.slang:167)
    - Creates hit: [`hit = HitInfo(getCommittedTriangleHit(rayQuery))`](Source/Falcor/Scene/RaytracingInline.slang:169)
    - Sets hit t: [`hitT = rayQuery.CommittedRayT()`](Source/Falcor/Scene/RaytracingInline.slang:170)
    - Returns hit

11. **Procedural Primitive Hit**:
    - Check if committed status is procedural primitive hit: [`if (rayQuery.CommittedStatus() == COMMITTED_PROCEDURAL_PRIMITIVE_HIT)`](Source/Falcor/Scene/RaytracingInline.slang:175)
    - Gets instance ID: [`const GeometryInstanceID instanceID = GeometryInstanceID(rayQuery.CommittedInstanceID(), rayQuery.CommittedGeometryIndex())`](Source/Falcor/Scene/RaytracingInline.slang:177)
    - Gets primitive index: [`const uint primitiveIndex = rayQuery.CommittedPrimitiveIndex()`](Source/Falcor/Scene/RaytracingInline.slang:178)
    - Gets geometry type: [`const GeometryType type = gScene.getGeometryInstanceType(instanceID)`](Source/Falcor/Scene/RaytracingInline.slang:179)
    - Switch on geometry type

12. **Displaced Triangle Hit**:
    - Case: [`GeometryType::DisplacedTriangleMesh`](Source/Falcor/Scene/RaytracingInline.slang:183)
    - Creates hit: [`DisplacedTriangleHit displacedTriangleHit`](Source/Falcor/Scene/RaytracingInline.slang:185)
    - Sets instance ID: [`displacedTriangleHit.instanceID = instanceID`](Source/Falcor/Scene/RaytracingInline.slang:186)
    - Sets primitive index: [`displacedTriangleHit.primitiveIndex = primitiveIndex`](Source/Falcor/Scene/RaytracingInline.slang:187)
    - Sets barycentrics: [`displacedTriangleHit.barycentrics = displacedTriangleMeshCommittedAttribs.barycentrics`](Source/Falcor/Scene/RaytracingInline.slang:188)
    - Sets displacement: [`displacedTriangleHit.displacement = displacedTriangleMeshCommittedAttribs.displacement`](Source/Falcor/Scene/RaytracingInline.slang:189)
    - Creates hit: [`hit = HitInfo(displacedTriangleHit)`](Source/Falcor/Scene/RaytracingInline.slang:190)
    - Sets hit t: [`hitT = rayQuery.CommittedRayT()`](Source/Falcor/Scene/RaytracingInline.slang:191)
    - Returns hit

13. **Curve Hit**:
    - Case: [`GeometryType::Curve`](Source/Falcor/Scene/RaytracingInline.slang:196)
    - Creates hit: [`CurveHit curveHit`](Source/Falcor/Scene/RaytracingInline.slang:198)
    - Sets instance ID: [`curveHit.instanceID = instanceID`](Source/Falcor/Scene/RaytracingInline.slang:199)
    - Sets primitive index: [`curveHit.primitiveIndex = primitiveIndex`](Source/Falcor/Scene/RaytracingInline.slang:200)
    - Sets barycentrics: [`curveHit.barycentrics = curveCommittedAttribs.barycentrics`](Source/Falcor/Scene/RaytracingInline.slang:201)
    - Creates hit: [`hit = HitInfo(curveHit)`](Source/Falcor/Scene/RaytracingInline.slang:202)
    - Sets hit t: [`hitT = rayQuery.CommittedRayT()`](Source/Falcor/Scene/RaytracingInline.slang:203)
    - Returns hit

14. **SDF Grid Hit**:
    - Case: [`GeometryType::SDFGrid`](Source/Falcor/Scene/RaytracingInline.slang:209)
    - Creates hit: [`SDFGridHit sdfGridHit`](Source/Falcor/Scene/RaytracingInline.slang:210)
    - Sets instance ID: [`sdfGridHit.instanceID = instanceID`](Source/Falcor/Scene/RaytracingInline.slang:211)
    - Sets hit data: [`sdfGridHit.hitData = sdfGridCommittedHitData`](Source/Falcor/Scene/RaytracingInline.slang:212)
    - Creates hit: [`hit = HitInfo(sdfGridHit)`](Source/Falcor/Scene/RaytracingInline.slang:213)
    - Sets hit t: [`hitT = rayQuery.CommittedRayT()`](Source/Falcor/Scene/RaytracingInline.slang:214)
    - Returns hit

15. **Return Hit**:
    - Returns [`HitInfo`](Source/Falcor/Scene/RaytracingInline.slang:224) structure

**Alpha Testing**:
- Enabled via `useAlphaTest` parameter
- Only for non-opaque triangles
- Uses material system for alpha test
- Discards fragments with low alpha

**Procedural Geometry Support**:
- Displaced triangle meshes
- Curves
- SDF grids
- Intersection testing for each type
- Hit information extraction for each type

### traceSceneRay Function

**Purpose**: Wrapper for traceSceneRayImpl.

**Signature**:
```slang
HitInfo traceSceneRay<let UseAlphaTest : int>(const Ray ray, out float hitT, uint rayFlags, uint instanceInclusionMask)
```

**Parameters**:
- [`ray`](Source/Falcor/Scene/RaytracingInline.slang:228) - `Ray` - Ray to trace
- [`hitT`](Source/Falcor/Scene/RaytracingInline.slang:228) - `out float` - Output hit t
- [`rayFlags`](Source/Falcor/Scene/RaytracingInline.slang:228) - `uint` - Ray flags
- [`instanceInclusionMask`](Source/Falcor/Scene/RaytracingInline.slang:228) - `uint` - Instance inclusion mask

**Template Parameter**:
- `UseAlphaTest : int` - Whether to use alpha testing

**Implementation**:
1. **Check Alpha Test**:
   - If alpha testing enabled: [`if (UseAlphaTest)`](Source/Falcor/Scene/RaytracingInline.slang:232)
   - If procedural geometry exists: `#if SCENE_HAS_PROCEDURAL_GEOMETRY()`
   - Create RayQuery: [`RayQuery<RAY_FLAG_NONE> rayQuery;`](Source/Falcor/Scene/RaytracingInline.slang:235)
   - Else: [`RayQuery<RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES> rayQuery;`](Source/Falcor/Scene/RaytracingInline.slang:237)
   - Call [`traceSceneRayImpl(true, rayQuery, ray, hitT, rayFlags, instanceInclusionMask)`](Source/Falcor/Scene/RaytracingInline.slang:239)

2. **Else**:
   - If procedural geometry exists: `#if SCENE_HAS_PROCEDURAL_GEOMETRY()`
   - Create RayQuery: [`RayQuery<RAY_FLAG_FORCE_OPAQUE> rayQuery;`](Source/Falcor/Scene/RaytracingInline.slang:244)
   - Else: [`RayQuery<RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES | RAY_FLAG_FORCE_OPAQUE> rayQuery;`](Source/Falcor/Scene/RaytracingInline.slang:246)
   - Call [`traceSceneRayImpl(false, rayQuery, ray, hitT, rayFlags, instanceInclusionMask)`](Source/Falcor/Scene/RaytracingInline.slang:248)

3. **Return Hit**:
   - Returns [`HitInfo`](Source/Falcor/Scene/RaytracingInline.slang:239) structure

**Ray Query Flags**:
- Alpha testing enabled: `RAY_FLAG_NONE` (with procedural geometry) or `RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES` (without procedural geometry)
- Alpha testing disabled: `RAY_FLAG_FORCE_OPAQUE` (with procedural geometry) or `RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES | RAY_FLAG_FORCE_OPAQUE` (without procedural geometry)

**Convenience**:
- Wraps traceSceneRayImpl with appropriate ray query flags
- Automatically selects ray query flags based on alpha testing
- Simplifies ray tracing calls

### traceSceneVisibilityRayImpl Function

**Purpose**: Visibility ray implementation.

**Signature**:
```slang
bool traceSceneVisibilityRayImpl<let Flags : int>(const bool useAlphaTest, inout RayQuery<Flags> rayQuery, const Ray ray, uint rayFlags, uint instanceInclusionMask)
```

**Parameters**:
- [`useAlphaTest`](Source/Falcor/Scene/RaytracingInline.slang:253) - `bool` - Whether to use alpha testing
- [`rayQuery`](Source/Falcor/Scene/RaytracingInline.slang:253) - `inout RayQuery<Flags>` - RayQuery object
- [`ray`](Source/Falcor/Scene/RaytracingInline.slang:253) - `Ray` - Ray to trace
- [`rayFlags`](Source/Falcor/Scene/RaytracingInline.slang:253) - `uint` - Ray flags
- [`instanceInclusionMask`](Source/Falcor/Scene/RaytracingInline.slang:253) - `uint` - Instance inclusion mask

**Template Parameter**:
- `Flags : int` - Ray query flags

**Implementation**:
1. **Trace Ray**:
   - Calls [`rayQuery.TraceRayInline(gScene.rtAccel, rayFlags, instanceInclusionMask, ray.toRayDesc())`](Source/Falcor/Scene/RaytracingInline.slang:255)
   - Uses scene acceleration structure
   - Uses ray flags and instance inclusion mask

2. **Process Intersections**:
   - While loop: [`while (rayQuery.Proceed())`](Source/Falcor/Scene/RaytracingInline.slang:257)
   - Processes all intersections

3. **Alpha Testing for Triangles**:
   - Check if alpha testing enabled and candidate is non-opaque triangle: [`if (useAlphaTest && rayQuery.CandidateType() == CANDIDATE_NON_OPAQUE_TRIANGLE)`](Source/Falcor/Scene/RaytracingInline.slang:260)
   - Gets candidate triangle hit: [`const TriangleHit hit = getCandidateTriangleHit(rayQuery)`](Source/Falcor/Scene/RaytracingInline.slang:263)
   - Gets vertex data: [`const VertexData v = gScene.getVertexData(hit)`](Source/Falcor/Scene/RaytracingInline.slang:264)
   - Gets material ID: [`const uint materialID = gScene.getMaterialID(hit.instanceID)`](Source/Falcor/Scene/RaytracingInline.slang:265)
   - Performs alpha test: [`if (gScene.materials.alphaTest(v, materialID, 0.f)) continue;`](Source/Falcor/Scene/RaytracingInline.slang:267)
   - Commits non-opaque triangle hit: [`rayQuery.CommitNonOpaqueTriangleHit();`](Source/Falcor/Scene/RaytracingInline.slang:269)

4. **Procedural Geometry**:
   - Check if candidate is procedural primitive: [`if (rayQuery.CandidateType() == CANDIDATE_PROCEDURAL_PRIMITIVE)`](Source/Falcor/Scene/RaytracingInline.slang:273)
   - Gets instance ID: [`const GeometryInstanceID instanceID = GeometryInstanceID(rayQuery.CandidateInstanceID(), rayQuery.CandidateGeometryIndex())`](Source/Falcor/Scene/RaytracingInline.slang:275)
   - Gets primitive index: [`const uint primitiveIndex = rayQuery.CandidatePrimitiveIndex()`](Source/Falcor/Scene/RaytracingInline.slang:276)
   - Gets geometry type: [`const GeometryType type = gScene.getGeometryInstanceType(instanceID)`](Source/Falcor/Scene/RaytracingInline.slang:277)
   - Creates ray segment: [`const Ray raySegment = Ray(ray.origin, ray.dir, rayQuery.RayTMin(), rayQuery.CommittedRayT())`](Source/Falcor/Scene/RaytracingInline.slang:278)
   - Switch on geometry type

5. **Displaced Triangle Mesh**:
   - Case: [`GeometryType::DisplacedTriangleMesh`](Source/Falcor/Scene/RaytracingInline.slang:282)
   - Creates attribs: [`DisplacedTriangleMeshIntersector::Attribs attribs`](Source/Falcor/Scene/RaytracingInline.slang:284)
   - Intersects ray: [`if (DisplacedTriangleMeshIntersector::intersect(raySegment, instanceID, primitiveIndex, attribs, t))`](Source/Falcor/Scene/RaytracingInline.slang:286)
   - Commits procedural primitive hit: [`rayQuery.CommitProceduralPrimitiveHit(t);`](Source/Falcor/Scene/RaytracingInline.slang:288)

6. **Curve**:
   - Case: [`GeometryType::Curve`](Source/Falcor/Scene/RaytracingInline.slang:294)
   - Creates attribs: [`CurveIntersector::Attribs attribs`](Source/Falcor/Scene/RaytracingInline.slang:296)
   - Intersects ray: [`if (CurveIntersector::intersect(raySegment, instanceID, primitiveIndex, attribs, t))`](Source/Falcor/Scene/RaytracingInline.slang:298)
   - Commits procedural primitive hit: [`rayQuery.CommitProceduralPrimitiveHit(t);`](Source/Falcor/Scene/RaytracingInline.slang:300)

7. **SDF Grid**:
   - Case: [`GeometryType::SDFGrid`](Source/Falcor/Scene/RaytracingInline.slang:306)
   - Intersects ray: [`if (SDFGridIntersector::intersectAny(raySegment, instanceID, primitiveIndex))`](Source/Falcor/Scene/RaytracingInline.slang:308)
   - Commits procedural primitive hit: [`rayQuery.CommitProceduralPrimitiveHit(ray.tMin);`](Source/Falcor/Scene/RaytracingInline.slang:310)

8. **Return Visibility**:
   - Returns [`rayQuery.CommittedStatus() == COMMITTED_NOTHING`](Source/Falcor/Scene/RaytracingInline.slang:322)
   - True if no hit (visible)
   - False if hit (occluded)

**Visibility Testing**:
- Returns true if ray is not occluded
- Returns false if ray is occluded
- Used for shadow ray testing
- Used for occlusion culling

**Procedural Geometry Support**:
- Displaced triangle meshes
- Curves
- SDF grids
- Intersection testing for each type
- Uses `intersectAny` for SDF grids (optimized)

### traceSceneVisibilityRay Function

**Purpose**: Wrapper for traceSceneVisibilityRayImpl.

**Signature**:
```slang
bool traceSceneVisibilityRay<let UseAlphaTest : int>(const Ray ray, uint rayFlags, uint instanceInclusionMask)
```

**Parameters**:
- [`ray`](Source/Falcor/Scene/RaytracingInline.slang:325) - `Ray` - Ray to trace
- [`rayFlags`](Source/Falcor/Scene/RaytracingInline.slang:325) - `uint` - Ray flags
- [`instanceInclusionMask`](Source/Falcor/Scene/RaytracingInline.slang:325) - `uint` - Instance inclusion mask

**Template Parameter**:
- `UseAlphaTest : int` - Whether to use alpha testing

**Implementation**:
1. **Check Alpha Test**:
   - If alpha testing enabled: [`if (UseAlphaTest)`](Source/Falcor/Scene/RaytracingInline.slang:329)
   - If procedural geometry exists: `#if SCENE_HAS_PROCEDURAL_GEOMETRY()`
   - Create RayQuery: [`RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> rayQuery;`](Source/Falcor/Scene/RaytracingInline.slang:332)
   - Else: [`RayQuery<RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> rayQuery;`](Source/Falcor/Scene/RaytracingInline.slang:334)
   - Call [`traceSceneVisibilityRayImpl(true, rayQuery, ray, rayFlags, instanceInclusionMask)`](Source/Falcor/Scene/RaytracingInline.slang:336)

2. **Else**:
   - If procedural geometry exists: `#if SCENE_HAS_PROCEDURAL_GEOMETRY()`
   - Create RayQuery: [`RayQuery<RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> rayQuery;`](Source/Falcor/Scene/RaytracingInline.slang:341)
   - Else: [`RayQuery<RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES | RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> rayQuery;`](Source/Falcor/Scene/RaytracingInline.slang:343)
   - Call [`traceSceneVisibilityRayImpl(false, rayQuery, ray, rayFlags, instanceInclusionMask)`](Source/Falcor/Scene/RaytracingInline.slang:345)

3. **Return Visibility**:
   - Returns [`bool`](Source/Falcor/Scene/RaytracingInline.slang:336) - True if visible, false if occluded

**Ray Query Flags**:
- Alpha testing enabled: `RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH` (with procedural geometry) or `RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH` (without procedural geometry)
- Alpha testing disabled: `RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH` (with procedural geometry) or `RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES | RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH` (without procedural geometry)

**Convenience**:
- Wraps traceSceneVisibilityRayImpl with appropriate ray query flags
- Automatically selects ray query flags based on alpha testing
- Simplifies visibility ray calls

### SceneRayQuery Structure

**Purpose**: Scene ray query implementation.

**Signature**:
```slang
struct SceneRayQuery<let UseAlphaTest : int> : ISceneRayQuery
```

**Template Parameter**:
- `UseAlphaTest : int` - Whether to use alpha testing

**Methods**:
- [`traceRay(const Ray ray, out float hitT, uint rayFlags = RAY_FLAG_NONE, uint instanceInclusionMask = 0xff)`](Source/Falcor/Scene/RaytracingInline.slang:353) - Trace a ray
  - Calls [`traceSceneRay<UseAlphaTest>(ray, hitT, rayFlags, instanceInclusionMask)`](Source/Falcor/Scene/RaytracingInline.slang:355)
  - Returns [`HitInfo`](Source/Falcor/Scene/RaytracingInline.slang:355) structure

- [`traceVisibilityRay(const Ray ray, uint rayFlags = RAY_FLAG_NONE, uint instanceInclusionMask = 0xff)`](Source/Falcor/Scene/RaytracingInline.slang:358) - Trace a visibility ray
  - Calls [`traceSceneVisibilityRay<UseAlphaTest>(ray, rayFlags, instanceInclusionMask)`](Source/Falcor/Scene/RaytracingInline.slang:360)
  - Returns [`bool`](Source/Falcor/Scene/RaytracingInline.slang:360) - True if visible, false if occluded

**Interface Implementation**:
- Implements [`ISceneRayQuery`](Source/Falcor/Scene/RaytracingInline.slang:351) interface
- Provides ray tracing functionality
- Provides visibility ray functionality
- Used by ray tracing passes

## Technical Details

### DXR 1.1 Inline Ray Tracing

**Purpose**: Use DXR 1.1 inline ray tracing API.

**RayQuery**:
- Template type with ray flags
- Supports inline ray tracing
- Provides intersection query functionality
- Used for ray intersection testing

**Ray Query Methods**:
- [`TraceRayInline()`](Source/Falcor/Scene/RaytracingInline.slang:88) - Trace ray against TLAS
- [`Proceed()`](Source/Falcor/Scene/RaytracingInline.slang:94) - Process ray query
- [`CommittedStatus()`](Source/Falcor/Scene/RaytracingInline.slang:167) - Get committed status
- [`CommittedInstanceID()`](Source/Falcor/Scene/RaytracingInline.slang:46) - Get committed instance ID
- [`CommittedGeometryIndex()`](Source/Falcor/Scene/RaytracingInline.slang:46) - Get committed geometry index
- [`CommittedPrimitiveIndex()`](Source/Falcor/Scene/RaytracingInline.slang:66) - Get committed primitive index
- [`CommittedTriangleBarycentrics()`](Source/Falcor/Scene/RaytracingInline.slang:67) - Get committed triangle barycentrics
- [`CommittedRayT()`](Source/Falcor/Scene/RaytracingInline.slang:170) - Get committed ray t
- [`CandidateType()`](Source/Falcor/Scene/RaytracingInline.slang:97) - Get candidate type
- [`CandidateInstanceID()`](Source/Falcor/Scene/RaytracingInline.slang:54) - Get candidate instance ID
- [`CandidateGeometryIndex()`](Source/Falcor/Scene/RaytracingInline.slang:54) - Get candidate geometry index
- [`CandidatePrimitiveIndex()`](Source/Falcor/Scene/RaytracingInline.slang:79) - Get candidate primitive index
- [`CandidateTriangleBarycentrics()`](Source/Falcor/Scene/RaytracingInline.slang:80) - Get candidate triangle barycentrics
- [`RayTMin()`](Source/Falcor/Scene/RaytracingInline.slang:115) - Get ray t min
- [`CommittedRayT()`](Source/Falcor/Scene/RaytracingInline.slang:115) - Get committed ray t
- [`CommitNonOpaqueTriangleHit()`](Source/Falcor/Scene/RaytracingInline.slang:106) - Commit non-opaque triangle hit
- [`CommitProceduralPrimitiveHit()`](Source/Falcor/Scene/RaytracingInline.slang:125) - Commit procedural primitive hit

### Alpha Testing

**Purpose**: Discard fragments based on alpha value.

**Alpha Test**:
- Enabled via `useAlphaTest` parameter
- Only for non-opaque triangles
- Uses material system for alpha test
- Discards fragments with low alpha

**Alpha Test Process**:
1. Check if candidate is non-opaque triangle
2. Get candidate triangle hit
3. Get vertex data
4. Get material ID
5. Perform alpha test
6. If alpha test passes, commit non-opaque triangle hit
7. If alpha test fails, continue to next candidate

**Alpha Test Function**:
- [`gScene.materials.alphaTest(v, materialID, 0.f)`](Source/Falcor/Scene/RaytracingInline.slang:104)
- Returns true if fragment should be discarded
- Returns false if fragment should be kept
- Used for cutout materials

### Procedural Geometry Support

**Purpose**: Support procedural geometry types.

**Geometry Types**:
- Displaced triangle meshes
- Curves
- SDF grids

**Procedural Geometry Process**:
1. Check if candidate is procedural primitive
2. Get instance ID and primitive index
3. Get geometry type
4. Create ray segment
5. Switch on geometry type
6. Intersect ray with procedural geometry
7. Commit procedural primitive hit if intersection found

**Displaced Triangle Mesh**:
- Uses [`DisplacedTriangleMeshIntersector::intersect()`](Source/Falcor/Scene/RaytracingInline.slang:123)
- Stores barycentrics and displacement
- Commits procedural primitive hit
- Creates DisplacedTriangleHit

**Curve**:
- Uses [`CurveIntersector::intersect()`](Source/Falcor/Scene/RaytracingInline.slang:136)
- Stores barycentrics
- Commits procedural primitive hit
- Creates CurveHit

**SDF Grid**:
- Uses [`SDFGridIntersector::intersect()`](Source/Falcor/Scene/RaytracingInline.slang:149) for regular rays
- Uses [`SDFGridIntersector::intersectAny()`](Source/Falcor/Scene/RaytracingInline.slang:308) for visibility rays
- Stores SDF grid hit data
- Commits procedural primitive hit
- Creates SDFGridHit

### Hit Information

**Purpose**: Extract hit information from ray query.

**Triangle Hit**:
- Instance ID
- Primitive index
- Barycentric coordinates
- Used for shading

**Displaced Triangle Hit**:
- Instance ID
- Primitive index
- Barycentric coordinates
- Displacement value
- Used for shading

**Curve Hit**:
- Instance ID
- Primitive index
- Barycentric coordinates
- Used for shading

**SDF Grid Hit**:
- Instance ID
- SDF grid hit data
- Used for shading

**HitInfo**:
- Polymorphic container for hit information
- Supports multiple hit types
- Used for shading
- Used for material evaluation

### Visibility Rays

**Purpose**: Test if ray is occluded.

**Visibility Test**:
- Returns true if ray is not occluded
- Returns false if ray is occluded
- Used for shadow ray testing
- Used for occlusion culling

**Visibility Ray Flags**:
- `RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH` - Accept first hit and end search
- Optimized for visibility testing
- Stops after first hit
- Reduces ray tracing cost

**Visibility Ray Process**:
1. Trace ray with visibility flags
2. Process all intersections
3. Alpha test for non-opaque triangles
4. Intersect procedural geometry
5. Return true if no hit, false if hit

### Ray Query Flags

**Purpose**: Control ray tracing behavior.

**Ray Flags**:
- `RAY_FLAG_NONE` - No special flags
- `RAY_FLAG_FORCE_OPAQUE` - Force opaque intersection
- `RAY_FLAG_FORCE_NON_OPAQUE` - Force non-opaque intersection
- `RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH` - Accept first hit and end search
- `RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES` - Skip procedural primitives

**Flag Selection**:
- Alpha testing enabled: `RAY_FLAG_NONE` (with procedural geometry) or `RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES` (without procedural geometry)
- Alpha testing disabled: `RAY_FLAG_FORCE_OPAQUE` (with procedural geometry) or `RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES | RAY_FLAG_FORCE_OPAQUE` (without procedural geometry)
- Visibility rays: `RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH` added to above flags

### Compile-Time Flags

**Purpose**: Enable/disable geometry types at compile time.

**Flags**:
- `SCENE_HAS_GEOMETRY_TYPE(GEOMETRY_TYPE_TRIANGLE_MESH)` - Has triangle mesh geometry
- `SCENE_HAS_GEOMETRY_TYPE(GEOMETRY_TYPE_DISPLACED_TRIANGLE_MESH)` - Has displaced triangle mesh geometry
- `SCENE_HAS_GEOMETRY_TYPE(GEOMETRY_TYPE_CURVE)` - Has curve geometry
- `SCENE_HAS_GEOMETRY_TYPE(GEOMETRY_TYPE_SDF_GRID)` - Has SDF grid geometry
- `SCENE_HAS_PROCEDURAL_GEOMETRY()` - Has any procedural geometry

**Usage**:
- Conditional compilation of geometry type support
- Reduces code size for scenes without certain geometry types
- Optimizes ray tracing for specific scene configurations

### Ray Segment

**Purpose**: Create ray segment for procedural geometry intersection.

**Ray Segment**:
- Origin: `ray.origin`
- Direction: `ray.dir`
- TMin: `rayQuery.RayTMin()`
- TMax: `rayQuery.CommittedRayT()`
- Used for procedural geometry intersection

**Usage**:
- Creates ray segment from original ray
- Uses ray query t min and committed t
- Used for procedural geometry intersection
- Ensures correct intersection testing

## Integration Points

### Scene Integration

**Acceleration Structure**:
- `gScene.rtAccel` - Scene ray tracing acceleration structure
- Used for ray intersection queries
- Top-Level Acceleration Structure (TLAS)
- Contains scene geometry

**Geometry Instance Access**:
- `gScene.getGeometryInstanceType(instanceID)` - Get geometry instance type
- `gScene.getMaterialID(instanceID)` - Get material ID
- `gScene.getVertexData(hit)` - Get vertex data

**Material Integration**:
- `gScene.materials.alphaTest(v, materialID, 0.f)` - Perform alpha test
- Used for cutout materials
- Discards fragments with low alpha

### Intersection Integration

**Intersectors**:
- `DisplacedTriangleMeshIntersector::intersect()` - Displaced triangle mesh intersection
- `CurveIntersector::intersect()` - Curve intersection
- `SDFGridIntersector::intersect()` - SDF grid intersection
- `SDFGridIntersector::intersectAny()` - SDF grid visibility intersection

**Intersection Attributes**:
- `DisplacedTriangleMeshIntersector::Attribs` - Displaced triangle mesh intersection attributes
- `CurveIntersector::Attribs` - Curve intersection attributes
- `SDFGridHitData` - SDF grid hit data

### SceneRayQueryInterface Integration

**Interface Implementation**:
- Implements `ISceneRayQuery` interface
- Provides `traceRay()` method
- Provides `traceVisibilityRay()` method
- Used by ray tracing passes

### Utils Integration

**Attributes Import**:
- `import Utils.Attributes` - Import shader attributes
- Provides shader attribute support
- Used for system value access

## Architecture Patterns

### Template Pattern

- Template parameter `Flags : int` for ray query flags
- Template parameter `UseAlphaTest : int` for alpha testing
- Compile-time configuration
- Optimizes ray tracing for specific use cases

### Wrapper Pattern

- Wraps DXR RayQuery API
- Simplifies ray tracing calls
- Automatically selects ray query flags
- Reduces boilerplate code

### Procedural Geometry Pattern

- Switch on geometry type
- Different intersection logic for each type
- Hit information extraction for each type
- Extensible for new geometry types

### Alpha Testing Pattern

- Conditional alpha testing
- Only for non-opaque triangles
- Uses material system for alpha test
- Discards fragments with low alpha

### Visibility Ray Pattern

- Optimized for visibility testing
- Accepts first hit and ends search
- Returns true if visible, false if occluded
- Used for shadow ray testing

## Code Patterns

### Geometry Type Pattern

```slang
GeometryType getCommittedGeometryType<let Flags : int>(RayQuery<Flags> rayQuery)
{
    GeometryInstanceID instanceID = GeometryInstanceID(rayQuery.CommittedInstanceID(), rayQuery.CommittedGeometryIndex());
    return gScene.getGeometryInstanceType(instanceID);
}
```

### Triangle Hit Pattern

```slang
TriangleHit getCommittedTriangleHit<let Flags : int>(RayQuery<Flags> rayQuery)
{
    TriangleHit hit;
    hit.instanceID = GeometryInstanceID(rayQuery.CommittedInstanceID(), rayQuery.CommittedGeometryIndex());
    hit.primitiveIndex = rayQuery.CommittedPrimitiveIndex();
    hit.barycentrics = rayQuery.CommittedTriangleBarycentrics();
    return hit;
}
```

### Alpha Testing Pattern

```slang
if (useAlphaTest && rayQuery.CandidateType() == CANDIDATE_NON_OPAQUE_TRIANGLE)
{
    const TriangleHit hit = getCandidateTriangleHit(rayQuery);
    const VertexData v = gScene.getVertexData(hit);
    const uint materialID = gScene.getMaterialID(hit.instanceID);
    
    if (gScene.materials.alphaTest(v, materialID, 0.f)) continue;
    
    rayQuery.CommitNonOpaqueTriangleHit();
}
```

### Procedural Geometry Pattern

```slang
if (rayQuery.CandidateType() == CANDIDATE_PROCEDURAL_PRIMITIVE)
{
    const GeometryInstanceID instanceID = GeometryInstanceID(rayQuery.CandidateInstanceID(), rayQuery.CandidateGeometryIndex());
    const uint primitiveIndex = rayQuery.CandidatePrimitiveIndex();
    const GeometryType type = gScene.getGeometryInstanceType(instanceID);
    const Ray raySegment = Ray(ray.origin, ray.dir, rayQuery.RayTMin(), rayQuery.CommittedRayT());
    switch (type)
    {
    case GeometryType::DisplacedTriangleMesh:
        {
            DisplacedTriangleMeshIntersector::Attribs attribs;
            float t;
            if (DisplacedTriangleMeshIntersector::intersect(raySegment, instanceID, primitiveIndex, attribs, t))
            {
                rayQuery.CommitProceduralPrimitiveHit(t);
                displacedTriangleMeshCommittedAttribs = attribs;
            }
        }
        break;
    case GeometryType::Curve:
        {
            CurveIntersector::Attribs attribs;
            float t;
            if (CurveIntersector::intersect(raySegment, instanceID, primitiveIndex, attribs, t))
            {
                rayQuery.CommitProceduralPrimitiveHit(t);
                curveCommittedAttribs = attribs;
            }
        }
        break;
    case GeometryType::SDFGrid:
        {
            SDFGridHitData sdfGridHitData;
            float t;
            if (SDFGridIntersector::intersect(raySegment, instanceID, primitiveIndex, sdfGridHitData, t))
            {
                rayQuery.CommitProceduralPrimitiveHit(t);
                sdfGridCommittedHitData = sdfGridHitData;
            }
        }
        break;
    default:
        // Ignore other types of geometry.
    }
}
```

### SceneRayQuery Pattern

```slang
struct SceneRayQuery<let UseAlphaTest : int> : ISceneRayQuery
{
    HitInfo traceRay(const Ray ray, out float hitT, uint rayFlags = RAY_FLAG_NONE, uint instanceInclusionMask = 0xff)
    {
        return traceSceneRay<UseAlphaTest>(ray, hitT, rayFlags, instanceInclusionMask);
    }
    
    bool traceVisibilityRay(const Ray ray, uint rayFlags = RAY_FLAG_NONE, uint instanceInclusionMask = 0xff)
    {
        return traceSceneVisibilityRay<UseAlphaTest>(ray, rayFlags, instanceInclusionMask);
    }
};
```

## Use Cases

### Ray Tracing

- **Primary Rays**:
  - Trace primary rays from camera
  - Use traceSceneRay for ray tracing
  - Get hit information for shading
  - Evaluate materials

- **Shadow Rays**:
  - Trace shadow rays to lights
  - Use traceSceneVisibilityRay for visibility testing
  - Check for occlusion
  - Compute shadows

- **Reflection/Refraction Rays**:
  - Trace reflection/refraction rays
  - Use traceSceneRay for ray tracing
  - Get hit information for shading
  - Evaluate materials

### Procedural Geometry

- **Displaced Triangle Meshes**:
  - Intersect displaced triangle meshes
  - Get displacement value
  - Use for displaced surface rendering
  - Use for normal mapping

- **Curves**:
  - Intersect curves
  - Get barycentric coordinates
  - Use for curve rendering
  - Use for hair rendering

- **SDF Grids**:
  - Intersect SDF grids
  - Get SDF grid hit data
  - Use for volumetric rendering
  - Use for signed distance fields

### Alpha Testing

- **Cutout Materials**:
  - Use alpha testing for cutout materials
  - Discard fragments with low alpha
  - Support cutout materials (foliage, fences, etc.)
  - Efficient rendering of cutout materials

### Visibility Rays

- **Shadow Rays**:
  - Use visibility rays for shadow testing
  - Check for occlusion
  - Compute shadows
  - Optimize with first hit acceptance

- **Occlusion Culling**:
  - Use visibility rays for occlusion culling
  - Check for occlusion
  - Optimize rendering
  - Reduce unnecessary ray tracing

## Performance Considerations

### GPU Computation

**Ray Tracing**:
- O(1) per ray (single intersection test)
- TLAS traversal overhead
- Ray intersection testing overhead
- Hit group shader execution overhead

**Alpha Testing**:
- O(1) per candidate (single alpha test)
- Vertex data fetching
- Material data fetching
- Alpha test evaluation

**Procedural Geometry**:
- O(1) per procedural primitive (single intersection test)
- Displaced triangle mesh intersection
- Curve intersection
- SDF grid intersection

**Visibility Rays**:
- O(1) per ray (first hit acceptance)
- TLAS traversal overhead
- Ray intersection testing overhead
- Optimized with first hit acceptance

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

**Material Data**:
- Material data fetching from scene
- Alpha test evaluation
- Efficient memory access

### Optimization Opportunities

**Alpha Testing**:
- Use alpha testing only for cutout materials
- Skip alpha testing for opaque materials
- Optimize for mixed opaque/cutout scenes
- Reduce unnecessary alpha tests

**Procedural Geometry**:
- Use compile-time flags to disable unused geometry types
- Reduce code size
- Optimize ray tracing for specific scene configurations
- Reduce unnecessary intersection tests

**Visibility Rays**:
- Use first hit acceptance for visibility rays
- Stop after first hit
- Reduce ray tracing cost
- Optimize for shadow rays

**Compile-Time Flags**:
- Use `SCENE_HAS_GEOMETRY_TYPE` flags to enable/disable geometry types
- Reduce code size for scenes without certain geometry types
- Optimize ray tracing for specific scene configurations
- Reduce unnecessary intersection tests

## Limitations

### Feature Limitations

- **DXR 1.1**:
  - Requires DXR 1.1 or higher
  - Not compatible with older hardware
  - Limited to DXR 1.1 features
  - No support for newer DXR features

- **Hit Group Shaders Only**:
  - Can only be called from hit group shaders
  - Cannot be called from other shader stages
  - Limited to closestHit, anyHit, intersection shaders
  - Not available in ray generation or miss shaders

- **Geometry Types**:
  - Only supports specific geometry types
  - Limited to triangle meshes, displaced triangle meshes, curves, SDF grids
  - No support for other geometry types
  - Limited to specific intersection logic

### Performance Limitations

- **TLAS Traversal**:
  - TLAS traversal overhead
  - Depends on scene complexity
  - May be expensive for complex scenes
  - Optimized by acceleration structure

- **Procedural Geometry**:
  - Procedural geometry intersection overhead
  - May be expensive for complex procedural geometry
  - Depends on intersection algorithm
  - May impact performance for procedural geometry

### Integration Limitations

- **Scene Coupling**:
  - Tightly coupled to Falcor scene system
  - Requires scene to be initialized
  - Not suitable for standalone use
  - Requires specific scene implementation

- **DXR Coupling**:
  - Tightly coupled to DXR 1.1 ray tracing
  - Requires DXR 1.1 support
  - Not suitable for other ray tracing APIs
  - Requires DXR 1.1-compatible hardware

### Debugging Limitations

- **Limited Error Reporting**:
  - No detailed error messages
  - No logging of failures
  - Difficult to debug ray tracing issues
  - No validation of input parameters

## Best Practices

### Ray Tracing

- **Use Wrapper Functions**:
  - Use traceSceneRay for ray tracing
  - Use traceSceneVisibilityRay for visibility testing
  - Simplify ray tracing calls
  - Reduce boilerplate code

- **Ray Query Flags**:
  - Use appropriate ray flags for use case
  - Use RAY_FLAG_NONE for default behavior
  - Use RAY_FLAG_FORCE_OPAQUE for opaque geometry
  - Use RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH for visibility rays

- **Instance Inclusion Mask**:
  - Use 0xff for all instances
  - Use custom mask for instance filtering
  - Optimize ray tracing with filtering
  - Reduce unnecessary intersection tests

### Alpha Testing

- **Cutout Materials**:
  - Use alpha testing only for cutout materials
  - Skip alpha testing for opaque materials
  - Optimize for mixed opaque/cutout scenes
  - Reduce unnecessary alpha tests

- **Material System**:
  - Use material system for alpha test
  - Discard fragments with low alpha
  - Support cutout materials efficiently
  - Use texture LOD for correct alpha sampling

### Procedural Geometry

- **Compile-Time Flags**:
  - Use `SCENE_HAS_GEOMETRY_TYPE` flags to enable/disable geometry types
  - Reduce code size for scenes without certain geometry types
  - Optimize ray tracing for specific scene configurations
  - Reduce unnecessary intersection tests

- **Intersection Logic**:
  - Use appropriate intersection logic for each geometry type
  - Extract hit information correctly
  - Create correct hit structures
  - Use for shading

### Visibility Rays

- **First Hit Acceptance**:
  - Use first hit acceptance for visibility rays
  - Stop after first hit
  - Reduce ray tracing cost
  - Optimize for shadow rays

- **Visibility Testing**:
  - Use traceSceneVisibilityRay for visibility testing
  - Check for occlusion
  - Compute shadows
  - Optimize for shadow rays

### Performance Optimization

- **Compile-Time Flags**:
  - Use `SCENE_HAS_GEOMETRY_TYPE` flags to enable/disable geometry types
  - Reduce code size for scenes without certain geometry types
  - Optimize ray tracing for specific scene configurations
  - Reduce unnecessary intersection tests

- **Alpha Testing**:
  - Use alpha testing only for cutout materials
  - Skip alpha testing for opaque materials
  - Optimize for mixed opaque/cutout scenes
  - Reduce unnecessary alpha tests

- **Visibility Rays**:
  - Use first hit acceptance for visibility rays
  - Stop after first hit
  - Reduce ray tracing cost
  - Optimize for shadow rays

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
  - Profile procedural geometry intersection
  - Identify bottlenecks

## Progress Log

- **2026-01-08T00:09:00Z**: RaytracingInline analysis completed. Analyzed RaytracingInline.slang (362 lines) containing inline ray tracing utilities for DXR 1.1. Documented getCommittedGeometryType and getCandidateGeometryType functions for getting geometry types, getCommittedTriangleHit and getCandidateTriangleHit functions for creating triangle hits, traceSceneRayImpl function for main ray tracing implementation with alpha testing and procedural geometry support (displaced triangle meshes, curves, SDF grids), traceSceneRay wrapper function for traceSceneRayImpl, traceSceneVisibilityRayImpl function for visibility ray implementation with optimized first hit acceptance, traceSceneVisibilityRay wrapper function for traceSceneVisibilityRayImpl, and SceneRayQuery structure implementing ISceneRayQuery interface with traceRay and traceVisibilityRay methods. Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.

## Completion Status

The RaytracingInline module is complete. All source files have been analyzed and documented with comprehensive technical specifications covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
