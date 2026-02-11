# MinimalPathTracer - Minimal Path Tracer

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **MinimalPathTracer** - Minimal path tracer
  - [x] **MinimalPathTracer.h** - Minimal path tracer header
  - [x] **MinimalPathTracer.cpp** - Minimal path tracer implementation
  - [x] **MinimalPathTracer.rt.slang** - Ray tracing shader

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Texture, Formats, Ray Tracing)
- **Core/Object** - Base object class
- **Core/Program** - Shader program management (Program, RtProgramVars, RtBindingTable)
- **RenderGraph** - Render graph system (RenderPass, RenderPassReflection, RenderPassHelpers, RenderPassStandardFlags)
- **Scene** - Scene system (Scene, Camera, Lights, Materials, Geometry)
- **Utils/Sampling** - SampleGenerator for random number generation
- **Utils/Math** - Mathematical utilities (MathConstants, MathHelpers)
- **Rendering/Lights** - Light utilities (LightHelpers)

## Module Overview

The MinimalPathTracer implements a simple, brute-force path tracer designed for validation of more complex renderers. It uses naive Monte Carlo path tracing without importance sampling or variance reduction techniques. The output is unbiased/consistent ground truth images that can be used to validate other rendering techniques. The tracer supports multiple geometry types (triangle meshes, displaced triangle meshes, curves, SDF grids), various light sources (analytic, emissive, environment), and configurable path length. It serves as a reference implementation for comparing against more advanced path tracers.

## Component Specifications

### MinimalPathTracer Class

**File**: [`MinimalPathTracer.h`](Source/RenderPasses/MinimalPathTracer/MinimalPathTracer.h:45)

**Purpose**: Simple brute-force path tracer for validation and ground truth rendering.

**Public Interface**:

```cpp
class MinimalPathTracer : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(MinimalPathTracer, "MinimalPathTracer", "Minimal path tracer.");

    static ref<MinimalPathTracer> create(ref<Device> pDevice, const Properties& props);
    MinimalPathTracer(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) override { return false; }
    virtual bool onKeyEvent(const KeyboardEvent& keyEvent) override { return false; }
};
```

**Public Members**: None (all members are private)

**Private Members**:
- `ref<Scene> mpScene` - Current scene
- `ref<SampleGenerator> mpSampleGenerator` - GPU sample generator
- `uint mMaxBounces` - Maximum number of indirect bounces (default: 3)
- `bool mComputeDirect` - Compute direct illumination (default: true)
- `bool mUseImportanceSampling` - Use importance sampling for materials (default: true)
- `uint mFrameCount` - Frame count since scene was loaded
- `bool mOptionsChanged` - Options changed flag for render graph refresh

**Ray Tracing Program**:
```cpp
struct
{
    ref<Program> pProgram;
    ref<RtBindingTable> pBindingTable;
    ref<RtProgramVars> pVars;
} mTracer;
```

**Private Methods**:
- `void parseProperties(const Properties& props)` - Parse configuration properties
- `void prepareVars()` - Prepare program variables

### Ray Tracing Shader

**File**: [`MinimalPathTracer.rt.slang`](Source/RenderPasses/MinimalPathTracer/MinimalPathTracer.rt.slang:28)

**Purpose**: Implement minimal path tracing algorithm with naive Monte Carlo sampling.

**Shader Resources**:
- `Texture2D<PackedHitInfo> gVBuffer` - Visibility buffer with packed hit information
- `Texture2D<float4> gViewW` - World-space view direction (optional)

**Output Resources**:
- `RWTexture2D<float4> gOutputColor` - Output color texture (sum of direct and indirect)

**Constant Buffer** (`CB`):
- `uint gFrameCount` - Frame count since scene was loaded
- `uint gPRNGDimension` - First available PRNG dimension

**Static Configuration** (based on defines):
- `MAX_BOUNCES` - Maximum number of indirect bounces (0 = direct only)
- `COMPUTE_DIRECT` - Compute direct illumination
- `USE_IMPORTANCE_SAMPLING` - Use importance sampling for materials
- `USE_ANALYTIC_LIGHTS` - Use Falcor's analytic lights
- `USE_EMISSIVE_LIGHTS` - Use emissive geometry as lights
- `USE_ENV_LIGHT` - Use environment map as light
- `USE_ENV_BACKGROUND` - Use environment map as background
- `is_valid_<name>` - 1 if optional I/O buffer with this name should be used

**Payload Structures**:

**ShadowRayData** (for shadow rays):
```cpp
struct ShadowRayData
{
    bool visible;  // Set to true by miss shader if ray is not terminated before
};
```

**ScatterRayData** (for scatter rays, up to 72B):
```cpp
struct ScatterRayData
{
    float3 radiance;    ///< Accumulated outgoing radiance from path.
    bool terminated;     ///< Set to true when path is terminated.
    float3 thp;         ///< Current path throughput. This is updated at each path vertex.
    uint pathLength;     ///< Path length in number of path segments (0 at origin, 1 at first secondary hit, etc.). Max 2^31.
    float3 origin;        ///< Next path segment origin.
    float3 direction;     ///< Next path segment direction.
    SampleGenerator sg; ///< Per-ray state for sample generator (up to 16B).

    __init(SampleGenerator sg)
    {
        this.terminated = false;
        this.pathLength = 0;
        this.radiance = float3(0, 0, 0);
        this.thp = float3(1, 1, 1);
        this.origin = float3(0, 0, 0);
        this.direction = float3(0, 0, 0);
        this.sg = sg;
    }
};
```

**Key Constants**:
- `kMaxPayloadSizeBytes = 72u` - Maximum payload size for traversal stack
- `kMaxRecursionDepth = 2u` - Maximum recursion depth
- `kDefaultBackgroundColor = float3(0, 0, 0)` - Default background color
- `kRayTMax = FLT_MAX` - Maximum ray distance

**Thread Group Size**: Not specified (uses ray generation shader)

## Technical Details

### Path Tracing Algorithm

The minimal path tracer uses a simple brute-force approach:

**Algorithm**:
1. **Primary Ray**: Cast primary ray from camera through pixel
2. **Primary Hit**: Load hit information from V-buffer
3. **Path Tracing Loop**: For each bounce up to max bounces:
   - Load shading data at hit point
   - Sample material BSDF
   - Generate scatter ray
   - Trace scatter ray
   - Accumulate radiance contribution
4. **Termination**: Stop at max bounces or when ray misses scene
5. **Output**: Write accumulated radiance to output texture

**Key Features**:
- Naive Monte Carlo sampling (no importance sampling)
- Uniform hemisphere sampling at each bounce
- One shadow ray and one scatter ray per path vertex
- Fixed path length (no Russian roulette)
- No next event estimation (NEE)
- No MIS (multiple importance sampling)

### Payload Management

The tracer uses two payload types:

**ShadowRayData** (for shadow rays):
- 1 byte: `bool visible`
- Purpose: Test visibility to light sources
- Set to true by miss shader if ray reaches light without hitting geometry

**ScatterRayData** (for scatter rays):
- Up to 72 bytes total
- Purpose: Carry path state through ray tracing
- Fields:
  - `float3 radiance` (12 bytes) - Accumulated radiance
  - `bool terminated` (1 byte) - Path termination flag
  - `float3 thp` (12 bytes) - Path throughput
  - `uint pathLength` (4 bytes) - Path segment count
  - `float3 origin` (12 bytes) - Next ray origin
  - `float3 direction` (12 bytes) - Next ray direction
  - `SampleGenerator sg` (16 bytes) - PRNG state
- Total: 69 bytes (within 72 byte limit)

### Primary Ray Direction

The tracer supports two methods for primary ray direction:

**Method 1: View Direction Buffer**:
```cpp
float3 getPrimaryRayDir(uint2 launchIndex, uint2 launchDim, const Camera camera)
{
    if (is_valid(gViewW))
    {
        // If we have a view vector bound as a buffer, just fetch it.
        return -gViewW[launchIndex].xyz;
    }
    else
    {
        // Compute view vector. This must exactly match what the G-buffer pass is doing.
        return camera.computeRayPinhole(launchIndex, launchDim).dir;
    }
}
```

**Method 2: Camera Computation**:
- Used when view direction buffer is not provided
- Matches G-buffer pass ray computation
- Does not account for depth-of-field

**Depth-of-Field Warning**:
- If DOF is enabled and view direction buffer is not provided, warning is logged
- DOF requires view direction buffer for correct shading

### Shadow Ray Tracing

The tracer tests visibility to light sources using shadow rays:

**Function**:
```cpp
bool traceShadowRay(float3 origin, float3 dir, float distance)
{
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = dir;
    ray.TMin = 0.f;
    ray.TMax = distance;

    ShadowRayData rayData;
    rayData.visible = false; // Set to true by miss shader if ray is not terminated before

    TraceRay(
        gScene.rtAccel,
        RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
        0xff /* instanceInclusionMask */,
        1 /* hitIdx */,
        rayTypeCount,
        1 /* missIdx */,
        ray,
        rayData
    );

    return rayData.visible;
}
```

**Purpose**: Test if light is visible from shading point
**Behavior**:
- Miss shader sets `visible = true` if ray reaches light
- Any hit shader sets `visible = true` if ray is not terminated before
- Terminates on first hit (no shadow ray continuation)

### Scatter Ray Tracing

The tracer generates scatter rays for path continuation:

**Function**:
```cpp
void traceScatterRay(inout ScatterRayData rayData)
{
    RayDesc ray;
    ray.Origin = rayData.origin;
    ray.Direction = rayData.direction;
    ray.TMin = 0.f;
    ray.TMax = kRayTMax;

    uint rayFlags = 0; // TODO: Set cull mode from the app
    TraceRay(gScene.rtAccel, rayFlags, 0xff /* instanceInclusionMask */, 0 /* hitIdx */, rayTypeCount, 0 /* missIdx */, ray, rayData);
}
```

**Purpose**: Trace scatter ray for next path segment
**Behavior**:
- Continues path based on payload state
- Hit shaders sample direct illumination and generate next scatter ray
- Miss shaders terminate path and add environment light contribution

### Direct Illumination

The tracer supports computing direct illumination:

**Configuration**:
- `COMPUTE_DIRECT` define: Nonzero if direct illumination should be included
- `mComputeDirect` member: Compute direct illumination (default: true)
- When disabled, only indirect illumination is computed (when max bounces > 0)

**Implementation**:
```cpp
if (kComputeDirect)
{
    // Always output directly emitted light, independent of whether emissive materials are treated as light sources or not.
    outColor += mi.getProperties(sd).emission;

    // Add contribution of direct light from analytic lights.
    // Light probe and mesh lights are handled by the scatter ray hit/miss shaders.
    outColor += kUseAnalyticLights ? evalDirectAnalytic(sd, mi, sg) : float3(0.f);
}
```

**Analytic Light Sampling**:
```cpp
float3 evalDirectAnalytic(const ShadingData sd, const IMaterialInstance mi, inout SampleGenerator sg)
{
    const uint lightCount = gScene.getLightCount();
    if (lightCount == 0)
        return float3(0.f);

    // Pick one of the analytic light sources randomly with equal probability.
    const uint lightIndex = min(uint(sampleNext1D(sg) * lightCount), lightCount - 1);
    float invPdf = lightCount; // Light selection pdf = 1.0 / lightCount.

    // Sample local light source.
    AnalyticLightSample ls;
    if (!sampleLight(sd.posW, gScene.getLight(lightIndex), sg, ls))
        return float3(0.f);

    // Reject sample if not in the hemisphere of a BSDF lobe.
    const uint lobeTypes = mi.getLobeTypes(sd);
    const bool hasReflection = lobeTypes & uint(LobeType::Reflection);
    const bool hasTransmission = lobeTypes & uint(LobeType::Transmission);
    float NdotL = dot(sd.getOrientedFaceNormal(), ls.dir);
    if ((NdotL <= kMinCosTheta && !hasTransmission) || (NdotL >= -kMinCosTheta && !hasReflection))
        return float3(0.f);

    // Get origin with offset applied in direction of geometry normal to avoid self-intersection.
    const float3 origin = computeRayOrigin(sd.posW, dot(sd.faceN, ls.dir) >= 0.f ? sd.faceN : -sd.faceN);

    // Test visibility by tracing a shadow ray.
    bool V = traceShadowRay(origin, ls.dir, ls.distance);

    if (!V)
        return float3(0.f);

    // Evaluate contribution.
    return mi.eval(sd, ls.dir, sg) * ls.Li * invPdf;
}
```

**Features**:
- Uniform light source selection
- Hemisphere test for BSDF lobe compatibility
- Shadow ray for visibility testing
- Self-intersection avoidance with normal offset

### Indirect Illumination

The tracer computes indirect illumination through path tracing:

**Hit Handling**:
```cpp
void handleHit(const HitInfo hit, inout ScatterRayData rayData)
{
    const bool isCurveHit = hit.getType() == HitType::Curve;

    // Load shading data.
    ShadingData sd = loadShadingData(hit, rayData.origin, rayData.direction);

    // Create material instance.
    let lod = ExplicitLodTextureSampler(0.f);
    let mi = gScene.materials.getMaterialInstance(sd, lod);

    // Add emitted light.
    if (kUseEmissiveLights && (kComputeDirect || rayData.pathLength > 0))
    {
        rayData.radiance += rayData.thp * mi.getProperties(sd).emission;
    }

    // Check whether to terminate based on max depth.
    if (rayData.pathLength >= kMaxBounces)
    {
        rayData.terminated = true;
        return;
    }

    // Compute ray origin for new rays spawned from the hit.
    float3 rayOrigin;
    if (isCurveHit)
    {
        // For curves, we set the new origin at the sphere center.
        rayOrigin = sd.posW - sd.curveRadius * sd.frame.N;
    }
    else
    {
        rayOrigin = sd.computeRayOrigin();
    }

    // Add contribution of direct light from analytic lights.
    if (kUseAnalyticLights)
    {
        float3 Lr = evalDirectAnalytic(sd, mi, rayData.sg);
        rayData.radiance += rayData.thp * Lr;
    }

    // Generate scatter ray for the next path segment.
    // The raygen shader will continue to path based on the returned payload.
    if (!generateScatterRay(sd, mi, isCurveHit, rayOrigin, rayData))
    {
        rayData.terminated = true;
        return;
    }

    rayData.pathLength++;
}
```

**Features**:
- Emissive light contribution
- Analytic light contribution
- Material BSDF sampling
- Curve hit handling (sphere center origin)
- Max bounce termination
- Path throughput accumulation

### Material Sampling

The tracer supports two material sampling modes:

**Mode 1: Uniform Sampling** (default):
```cpp
BSDFSample bsdfSample;
if (mi.sample(sd, rayData.sg, bsdfSample, false)) // importanceSampling = false
{
    rayData.origin = rayOrigin;
    if (!isCurveHit && bsdfSample.isLobe(LobeType::Transmission))
    {
        rayData.origin = sd.computeRayOrigin(false);
    }
    rayData.direction = bsdfSample.wo;
    rayData.thp *= bsdfSample.weight;
    return any(rayData.thp > 0.f);
}
```

**Mode 2: Importance Sampling**:
```cpp
BSDFSample bsdfSample;
if (mi.sample(sd, rayData.sg, bsdfSample, kUseImportanceSampling)) // importanceSampling = true
{
    rayData.origin = rayOrigin;
    if (!isCurveHit && bsdfSample.isLobe(LobeType::Transmission))
    {
        rayData.origin = sd.computeRayOrigin(false);
    }
    rayData.direction = bsdfSample.wo;
    rayData.thp *= bsdfSample.weight;
    return any(rayData.thp > 0.f);
}
```

**Difference**:
- Uniform: Naive hemisphere sampling
- Importance: Material importance sampling (if supported)
- Transmission handling: Ray origin offset for transmission

### Geometry Type Support

The tracer supports multiple geometry types:

**TriangleMesh**:
- Hit group 0: Scatter (closest hit)
- Hit group 1: Shadow (any hit)
- Vertex data: `getVertexData()`
- Material ID: `getMaterialID(instanceID)`
- Alpha test: `alphaTest(v, materialID, 0.f)`

**DisplacedTriangleMesh**:
- Hit group 0: Scatter (closest hit)
- Hit group 1: Shadow (any hit)
- Vertex data: `getVertexData(instanceID, PrimitiveIndex(), -rayDir)`
- Material ID: `getMaterialID(instanceID)`
- Displacement: `displacement` attribute
- Alpha test: `alphaTest(v, materialID, 0.f)`

**Curve**:
- Hit group 0: Scatter (closest hit)
- Hit group 1: Shadow (any hit)
- Vertex data: `getVertexDataFromCurve(curveHit)`
- Material ID: `getMaterialID(curveHit.instanceID)`
- No alpha test

**SDFGrid**:
- Hit group 0: Scatter (closest hit)
- Hit group 1: Shadow (any hit)
- Vertex data: `getVertexDataFromSDFGrid(sdfGridHit, rayOrigin, rayDir)`
- Material ID: `getMaterialID(sdfGridHit.instanceID)`
- No alpha test

### Environment Light Integration

The tracer supports environment map as light source:

**Enable**: `USE_ENV_LIGHT` define
**Implementation** (in miss shader):
```cpp
void scatterMiss(inout ScatterRayData rayData)
{
    // Ray missed scene. Mark ray as terminated.
    rayData.terminated = true;

    // Add contribution from distant light (env map) in this direction.
    if (kUseEnvLight && (kComputeDirect || rayData.pathLength > 0))
    {
        float3 Le = gScene.envMap.eval(WorldRayDirection());
        rayData.radiance += rayData.thp * Le;
    }
}
```

**Purpose**: Provide infinite environment light for rays that miss all geometry

### Environment Background

The tracer supports environment map as background:

**Enable**: `USE_ENV_BACKGROUND` define
**Implementation** (in main shader):
```cpp
else
{
    // Background pixel.
    outColor = kUseEnvBackground ? gScene.envMap.eval(primaryRayDir) : kDefaultBackgroundColor;
}
```

**Purpose**: Provide background color for rays that miss all geometry

### Sample Generator Integration

The tracer uses [`SampleGenerator`](Source/Falcor/Utils/Sampling/SampleGenerator.h:1) for random number generation:

**Integration**:
```cpp
// Create a sample generator.
mpSampleGenerator = SampleGenerator::create(mpDevice, SAMPLE_GENERATOR_UNIFORM);
```

**Usage**:
- Per-ray state for random number generation
- Up to 16 bytes per ray
- Stored in `ScatterRayData.sg`
- Advanced to first available dimension at each path vertex

**Dimension Handling**:
```cpp
// Advance generator to first available dimension.
// TODO: This is potentially expensive. We may want to store/restore state from memory if it becomes a problem.
for (uint i = 0; i < gPRNGDimension; i++)
    sampleNext1D(sg);
```

### Render Graph Integration

**Input Channels**:
- `vbuffer` (required): Visibility buffer in packed format
- `viewW` (optional): World-space view direction (xyz float format)

**Output Channels**:
- `color` (required): Output color (sum of direct and indirect, format: RGBA32Float)

**Reflection**:
```cpp
RenderPassReflection reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;

    // Define our input/output channels.
    addRenderPassInputs(reflector, kInputChannels);
    addRenderPassOutputs(reflector, kOutputChannels);

    return reflector;
}
```

**Refresh Flags**:
```cpp
// Update refresh flag if options that affect output have changed.
auto& dict = renderData.getDictionary();
if (mOptionsChanged)
{
    auto flags = dict.getValue(kRenderPassRefreshFlags, RenderPassRefreshFlags::None);
    dict[Falcor::kRenderPassRefreshFlags] = flags | Falcor::RenderPassRefreshFlags::RenderOptionsChanged;
    mOptionsChanged = false;
}
```

### Property Serialization

The pass supports property serialization for configuration:

```cpp
Properties getProperties() const
{
    Properties props;
    props[kMaxBounces] = mMaxBounces;
    props[kComputeDirect] = mComputeDirect;
    props[kUseImportanceSampling] = mUseImportanceSampling;
    return props;
}
```

**Property Keys**:
- `maxBounces` - Maximum number of indirect bounces
- `computeDirect` - Compute direct illumination
- `useImportanceSampling` - Use importance sampling for materials

### Ray Tracing Program Setup

The tracer creates a ray tracing program with multiple hit groups:

**Program Creation**:
```cpp
// Create ray tracing program.
ProgramDesc desc;
desc.addShaderModules(mpScene->getShaderModules());
desc.addShaderLibrary(kShaderFile);
desc.setMaxPayloadSize(kMaxPayloadSizeBytes);
desc.setMaxAttributeSize(mpScene->getRaytracingMaxAttributeSize());
desc.setMaxTraceRecursionDepth(kMaxRecursionDepth);

mTracer.pBindingTable = RtBindingTable::create(2, 2, mpScene->getGeometryCount());
auto& sbt = mTracer.pBindingTable;
sbt->setRayGen(desc.addRayGen("rayGen"));
sbt->setMiss(0, desc.addMiss("scatterMiss"));
sbt->setMiss(1, desc.addMiss("shadowMiss"));
```

**Hit Groups**:
- Hit group 0: Scatter rays (closest hit)
- Hit group 1: Shadow rays (any hit)

**Miss Groups**:
- Miss 0: Scatter rays
- Miss 1: Shadow rays

**Geometry Types**:
- TriangleMesh
- DisplacedTriangleMesh
- Curve
- SDFGrid

### Shading Data Loading

The tracer loads shading data for different geometry types:

**Function**:
```cpp
ShadingData loadShadingData(const HitInfo hit, const float3 rayOrigin, const float3 rayDir)
{
    VertexData v = {};
    uint materialID = {};

    if (hit.getType() == HitType::Triangle)
    {
        const TriangleHit triangleHit = hit.getTriangleHit();
        v = gScene.getVertexData(triangleHit);
        materialID = gScene.getMaterialID(triangleHit.instanceID);
    }
    else if (hit.getType() == HitType::DisplacedTriangle)
    {
        const DisplacedTriangleHit displacedTriangleHit = hit.getDisplacedTriangleHit();
        v = gScene.getVertexData(displacedTriangleHit, -rayDir);
        materialID = gScene.getMaterialID(displacedTriangleHit.instanceID);
    }
    else if (hit.getType() == HitType::Curve)
    {
        const CurveHit curveHit = hit.getCurveHit();
        v = gScene.getVertexDataFromCurve(curveHit);
        materialID = gScene.getMaterialID(curveHit.instanceID);
    }
    else if (hit.getType() == HitType::SDFGrid)
    {
        const SDFGridHit sdfGridHit = hit.getSDFGridHit();
        v = gScene.getVertexDataFromSDFGrid(sdfGridHit, rayOrigin, rayDir);
        materialID = gScene.getMaterialID(sdfGridHit.instanceID);
    }

    ShadingData sd = gScene.materials.prepareShadingData(v, materialID, -rayDir);

    return sd;
}
```

**Purpose**: Load vertex data and material information for hit point

### Ray Origin Computation

The tracer computes ray origins with self-intersection avoidance:

**Function**:
```cpp
float3 computeRayOrigin(const ShadingData sd, const float3 rayDir, const bool useOffset)
{
    if (useOffset)
    {
        // Get origin with offset applied in direction of geometry normal to avoid self-intersection.
        return sd.posW + dot(sd.faceN, rayDir) >= 0.f ? sd.faceN : -sd.faceN;
    }
    else
    {
        return sd.posW;
    }
}
```

**Purpose**: Avoid self-intersection when spawning new rays from hit point

**Usage**:
- Scatter rays: Use normal offset
- Transmission: Use normal offset
- Primary rays: No offset

### Path Tracing Loop

The main shader implements a for-loop over path segments:

**Structure**:
```cpp
float3 tracePath(const uint2 pixel, const uint2 frameDim)
{
    float3 outColor = float3(0.f);

    const float3 primaryRayOrigin = gScene.camera.getPosition();
    const float3 primaryRayDir = getPrimaryRayDir(pixel, frameDim, gScene.camera);

    const HitInfo hit = HitInfo(gVBuffer[pixel]);
    if (hit.isValid())
    {
        // Pixel represents a valid primary hit. Compute its contribution.

        const bool isCurveHit = hit.getType() == HitType::Curve;

        // Load shading data.
        ShadingData sd = loadShadingData(hit, primaryRayOrigin, primaryRayDir);

        // Create material instance at shading point.
        let lod = ExplicitLodTextureSampler(0.f);
        let mi = gScene.materials.getMaterialInstance(sd, lod);

        // Create sample generator.
        SampleGenerator sg = SampleGenerator(pixel, gFrameCount);

        // Advance generator to first available dimension.
        for (uint i = 0; i < gPRNGDimension; i++)
            sampleNext1D(sg);

        // Compute ray origin for new rays spawned from G-buffer.
        float3 rayOrigin;
        if (isCurveHit)
        {
            // For curves, we set the new origin at the sphere center.
            rayOrigin = sd.posW - sd.curveRadius * sd.frame.N;
        }
        else
        {
            rayOrigin = sd.computeRayOrigin();
        }

        if (kComputeDirect)
        {
            // Always output directly emitted light, independent of whether emissive materials are treated as light sources or not.
            outColor += mi.getProperties(sd).emission;

            // Add contribution of direct light from analytic lights.
            // Light probe and mesh lights are handled by the scatter ray hit/miss shaders.
            outColor += kUseAnalyticLights ? evalDirectAnalytic(sd, mi, sg) : float3(0.f);
        }

        // Prepare ray payload.
        ScatterRayData rayData = ScatterRayData(sg);

        // Generate scatter ray.
        if (!generateScatterRay(sd, mi, isCurveHit, rayOrigin, rayData))
            rayData.terminated = true;
            return;

        // Follow path into scene and compute its total contribution.
        for (uint depth = 0; depth <= kMaxBounces && !rayData.terminated; depth++)
        {
            // Trace scatter ray. If it hits geometry, the closest hit shader samples
            // direct illumination and generates a next scatter ray.
            traceScatterRay(rayData);
        }

        // Store contribution from scatter ray.
        outColor += rayData.radiance;
    }
    else
    {
        // Background pixel.
        outColor = kUseEnvBackground ? gScene.envMap.eval(primaryRayDir) : kDefaultBackgroundColor;
    }

    return outColor;
}
```

**Features**:
- For-loop over path segments
- Maximum bounce limit
- Radiance accumulation
- Background handling
- Direct and indirect illumination

## Architecture Patterns

### Render Pass Pattern

- Inherits from [`RenderPass`](Source/Falcor/RenderGraph/RenderPass.h:1) base class
- Implements standard render pass interface:
  - `reflect()` - Defines input/output resources
  - `execute()` - Performs path tracing
  - `renderUI()` - Provides UI controls
  - `getProperties()` - Serializes configuration
  - `setScene()` - Sets scene and creates ray tracing program

### Ray Tracing Pattern

- Uses [`Program`](Source/Falcor/Core/Program/Program.h:1) for ray tracing
- Uses [`RtBindingTable`](Source/Falcor/Core/API/RtBindingTable.h:1) for binding table
- Uses [`RtProgramVars`](Source/Falcor/Core/Program/RtProgramVars.h:1) for program variables
- Multiple hit groups for different ray types
- Multiple miss groups for scatter and shadow rays

### Payload Pattern

- Uses payload-based ray tracing
- Two payload types: ShadowRayData, ScatterRayData
- Payload size limited to 72 bytes for traversal stack
- State carried through ray tracing pipeline

### Sample Generator Pattern

- Uses [`SampleGenerator`](Source/Falcor/Utils/Sampling/SampleGenerator.h:1) for random numbers
- Per-ray state for independent sampling
- Up to 16 bytes per ray
- Multiple dimensions for different PRNG streams

### Property Pattern

- Implements property-based configuration
- Supports serialization/deserialization via `Properties` class
- Enables configuration through render graph and scripting

### Scene Integration Pattern

- Integrates with [`Scene`](Source/Falcor/Scene/Scene.h:1) system
- Uses scene geometry, materials, lights
- Supports multiple geometry types
- Supports analytic lights, emissive lights, environment lights

## Code Patterns

### Property Parsing

```cpp
void parseProperties(const Properties& props)
{
    for (const auto& [key, value] : props)
    {
        if (key == kMaxBounces)
            mMaxBounces = value;
        else if (key == kComputeDirect)
            mComputeDirect = value;
        else if (key == kUseImportanceSampling)
            mUseImportanceSampling = value;
        else
            logWarning("Unknown property '{}' in MinimalPathTracer properties.", key);
    }
}
```

### Scene Setup

```cpp
void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    // Clear data for previous scene.
    // After changing scene, the raytracing program should to be recreated.
    mTracer.pProgram = nullptr;
    mTracer.pBindingTable = nullptr;
    mTracer.pVars = nullptr;
    mFrameCount = 0;

    // Set new scene.
    mpScene = pScene;

    if (mpScene)
    {
        if (pScene->hasGeometryType(Scene::GeometryType::Custom))
        {
            logWarning("MinimalPathTracer: This render pass does not support custom primitives.");
        }

        // Create ray tracing program.
        ProgramDesc desc;
        desc.addShaderModules(mpScene->getShaderModules());
        desc.addShaderLibrary(kShaderFile);
        desc.setMaxPayloadSize(kMaxPayloadSizeBytes);
        desc.setMaxAttributeSize(mpScene->getRaytracingMaxAttributeSize());
        desc.setMaxTraceRecursionDepth(kMaxRecursionDepth);

        mTracer.pBindingTable = RtBindingTable::create(2, 2, mpScene->getGeometryCount());
        auto& sbt = mTracer.pBindingTable;
        sbt->setRayGen(desc.addRayGen("rayGen"));
        sbt->setMiss(0, desc.addMiss("scatterMiss"));
        sbt->setMiss(1, desc.addMiss("shadowMiss"));

        // Set hit groups for each geometry type
        if (mpScene->hasGeometryType(Scene::GeometryType::TriangleMesh))
        {
            sbt->setHitGroup(
                0,
                mpScene->getGeometryIDs(Scene::GeometryType::TriangleMesh),
                desc.addHitGroup("scatterTriangleMeshClosestHit", "scatterTriangleMeshAnyHit")
            );
            sbt->setHitGroup(
                1, mpScene->getGeometryIDs(Scene::GeometryType::TriangleMesh), desc.addHitGroup("", "shadowTriangleMeshAnyHit")
            );
        }

        if (mpScene->hasGeometryType(Scene::GeometryType::DisplacedTriangleMesh))
        {
            sbt->setHitGroup(
                0,
                mpScene->getGeometryIDs(Scene::GeometryType::DisplacedTriangleMesh),
                desc.addHitGroup("scatterDisplacedTriangleMeshClosestHit", "", "displacedTriangleMeshIntersection")
            );
            sbt->setHitGroup(
                1,
                mpScene->getGeometryIDs(Scene::GeometryType::DisplacedTriangleMesh),
                desc.addHitGroup("", "", "displacedTriangleMeshIntersection")
            );
        }

        if (mpScene->hasGeometryType(Scene::GeometryType::Curve))
        {
            sbt->setHitGroup(
                0, mpScene->getGeometryIDs(Scene::GeometryType::Curve), desc.addHitGroup("scatterCurveClosestHit", "", "curveIntersection")
            );
            sbt->setHitGroup(1, mpScene->getGeometryIDs(Scene::GeometryType::Curve), desc.addHitGroup("", "", "curveIntersection"));
        }

        if (mpScene->hasGeometryType(Scene::GeometryType::SDFGrid))
        {
            sbt->setHitGroup(
                0,
                mpScene->getGeometryIDs(Scene::GeometryType::SDFGrid),
                desc.addHitGroup("scatterSdfGridClosestHit", "", "sdfGridIntersection")
            );
            sbt->setHitGroup(1, mpScene->getGeometryIDs(Scene::GeometryType::SDFGrid), desc.addHitGroup("", "", "sdfGridIntersection"));
        }
    }

    mTracer.pProgram = Program::create(mpDevice, desc, mpScene->getSceneDefines());
}
```

### Program Preparation

```cpp
void prepareVars()
{
    FALCOR_ASSERT(mpScene);
    FALCOR_ASSERT(mTracer.pProgram);

    // Configure program.
    mTracer.pProgram->addDefines(mpSampleGenerator->getDefines());
    mTracer.pProgram->setTypeConformances(mpScene->getTypeConformances());

    // Create program variables for current program.
    // This may trigger shader compilation. If it fails, throw an exception to abort rendering.
    mTracer.pVars = RtProgramVars::create(mpDevice, mTracer.pProgram, mTracer.pBindingTable);

    // Bind utility classes into shared data.
    auto var = mTracer.pVars->getRootVar();
    mpSampleGenerator->bindShaderData(var);
}
```

### Execute

```cpp
void execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // Update refresh flag if options that affect output have changed.
    auto& dict = renderData.getDictionary();
    if (mOptionsChanged)
    {
        auto flags = dict.getValue(kRenderPassRefreshFlags, RenderPassRefreshFlags::None);
        dict[Falcor::kRenderPassRefreshFlags] = flags | Falcor::RenderPassRefreshFlags::RenderOptionsChanged;
        mOptionsChanged = false;
    }

    // If we have no scene, just clear outputs and return.
    if (!mpScene)
    {
        for (auto it : kOutputChannels)
        {
            Texture* pDst = renderData.getTexture(it.name).get();
            if (pDst)
                pRenderContext->clearTexture(pDst);
        }
        return;
    }

    // Request light collection if emissive lights are enabled.
    if (mpScene->getRenderSettings().useEmissiveLights)
    {
        mpScene->getLightCollection(pRenderContext);
    }

    // Configure depth-of-field.
    const bool useDOF = mpScene->getCamera()->getApertureRadius() > 0.f;
    if (useDOF && renderData[kInputViewDir] == nullptr)
    {
        logWarning("Depth-of-field requires '{}' input. Expect incorrect shading.", kInputViewDir);
    }

    // Specialize program.
    // These defines should not modify program vars. Do not trigger program vars re-creation.
    mTracer.pProgram->addDefine("MAX_BOUNCES", std::to_string(mMaxBounces));
    mTracer.pProgram->addDefine("COMPUTE_DIRECT", mComputeDirect ? "1" : "0");
    mTracer.pProgram->addDefine("USE_IMPORTANCE_SAMPLING", mUseImportanceSampling ? "1" : "0");
    mTracer.pProgram->addDefine("USE_ANALYTIC_LIGHTS", mpScene->useAnalyticLights() ? "1" : "0");
    mTracer.pProgram->addDefine("USE_EMISSIVE_LIGHTS", mpScene->useEmissiveLights() ? "1" : "0");
    mTracer.pProgram->addDefine("USE_ENV_LIGHT", mpScene->useEnvLight() ? "1" : "0");
    mTracer.pProgram->addDefine("USE_ENV_BACKGROUND", mpScene->useEnvBackground() ? "1" : "0");

    // For optional I/O resources, set 'is_valid_<name>' defines to inform the program of which ones it can access.
    // TODO: This should be moved to a more general mechanism using Slang.
    mTracer.pProgram->addDefines(getValidResourceDefines(kInputChannels, renderData));
    mTracer.pProgram->addDefines(getValidResourceDefines(kOutputChannels, renderData));

    // Prepare program vars. This may trigger shader compilation.
    // The program should have all necessary defines set at this point.
    if (!mTracer.pVars)
        prepareVars();

    FALCOR_ASSERT(mTracer.pVars);

    // Set constants.
    auto var = mTracer.pVars->getRootVar();
    var["CB"]["gFrameCount"] = mFrameCount;
    var["CB"]["gPRNGDimension"] = dict.keyExists(kRenderPassPRNGDimension) ? dict[kRenderPassPRNGDimension] : 0u;

    // Bind I/O buffers. These needs to be done per-frame as buffers may change anytime.
    auto bind = [&](const ChannelDesc& desc)
    {
        if (!desc.texname.empty())
        {
            var[desc.texname] = renderData.getTexture(desc.name);
        }
    };
    for (auto channel : kInputChannels)
        bind(channel);
    for (auto channel : kOutputChannels)
        bind(channel);

    // Get dimensions of ray dispatch.
    const uint2 targetDim = renderData.getDefaultTextureDims();
    FALCOR_ASSERT(targetDim.x > 0 && targetDim.y > 0);

    // Spawn rays.
    mpScene->raytrace(pRenderContext, mTracer.pProgram.get(), mTracer.pVars, uint3(targetDim, 1));

    mFrameCount++;
}
```

### UI Rendering

```cpp
void renderUI(Gui::Widgets& widget)
{
    bool dirty = false;

    dirty |= widget.var("Max bounces", mMaxBounces, 0u, 1u << 16);
    widget.tooltip("Maximum path length for indirect illumination.\n0 = direct only\n1 = one indirect bounce etc.", true);

    dirty |= widget.checkbox("Evaluate direct illumination", mComputeDirect);
    widget.tooltip("Compute direct illumination.\nIf disabled only indirect is computed (when max bounces > 0).", true);

    dirty |= widget.checkbox("Use importance sampling", mUseImportanceSampling);
    widget.tooltip("Use importance sampling for materials", true);

    // If rendering options that modify the output have changed, set flag to indicate that.
    // In execute() we will pass the flag to other passes for reset of temporal data etc.
    if (dirty)
    {
        mOptionsChanged = true;
    }
}
```

## Integration Points

### Render Graph Integration

The pass integrates with render graph system:
- Input channels: vbuffer, viewW (optional)
- Output channels: color
- Reflects input/output resources via `reflect()` method
- Executes path tracing in `execute()` method
- Supports property-based configuration
- Triggers graph refresh when options change

### Scene Integration

The pass integrates with scene system:
- Uses scene geometry (triangle meshes, displaced meshes, curves, SDF grids)
- Uses scene materials
- Uses scene lights (analytic, emissive, environment)
- Uses scene camera
- Recreates ray tracing program when scene changes

### Sample Generator Integration

The pass uses sample generator for random numbers:
- `SampleGenerator::create()` - Create sample generator
- `mpSampleGenerator->bindShaderData()` - Bind to shader
- `sampleNext1D()` - Advance PRNG state
- Per-ray state for independent sampling

### Ray Tracing Integration

The pass uses ray tracing API:
- `Program::create()` - Create ray tracing program
- `RtBindingTable::create()` - Create binding table
- `RtProgramVars::create()` - Create program variables
- `TraceRay()` - Trace rays with payload
- Multiple hit groups for different ray types
- Multiple miss groups for scatter and shadow rays

### Scripting Integration

The pass supports scripting through:
- Property serialization via `getProperties()` method
- Configurable parameters: maxBounces, computeDirect, useImportanceSampling
- Can be instantiated and configured from Python scripts

### UI Integration

The pass provides UI controls:
- Slider for max bounces
- Checkbox for direct illumination
- Checkbox for importance sampling
- Options change tracking for graph refresh

## Use Cases

### Ground Truth Rendering

Generate unbiased ground truth images:
- Validate complex path tracers
- Compare against advanced techniques
- Test new rendering algorithms
- Benchmark performance

### Educational Use

Educational applications:
- Demonstrate path tracing fundamentals
- Show naive Monte Carlo sampling
- Explain direct vs. indirect illumination
- Visualize path tracing concepts
- Teaching rendering algorithms

### Algorithm Validation

Validate rendering algorithms:
- Test new sampling techniques
- Compare different path lengths
- Validate BSDF implementations
- Test light sampling strategies
- Verify correctness of complex tracers

### Performance Benchmarking

Benchmark rendering performance:
- Compare different ray tracing implementations
- Measure impact of bounce count
- Test geometry type performance
- Evaluate material sampling overhead
- Profile ray tracing performance

### Research and Development

Research applications:
- Study path tracing behavior
- Analyze variance characteristics
- Test new light transport techniques
- Validate theoretical predictions
- Develop new rendering algorithms

### Debugging and Testing

Debugging and testing applications:
- Verify scene rendering correctness
- Test material system integration
- Validate light sampling
- Debug ray tracing pipeline
- Test geometry type support

## Performance Considerations

### GPU Computation

- Path tracing is GPU-accelerated
- One ray per pixel per bounce
- Payload-based ray tracing with limited size
- Memory bandwidth limited by texture reads and ray tracing

### Memory Usage

- V-buffer: Depends on scene resolution
- Output color: RGBA32Float (16 bytes per pixel)
- Ray tracing state: Depends on scene complexity
- Total memory: O(width × height) bytes

### Computational Complexity

- Path tracing: O(N × B × R) where N = pixels, B = bounces, R = rays per bounce
- Shadow rays: O(N × B × L) where L = number of lights
- Overall: O(N × B × (R + L)) per frame

### Sample Generator Overhead

- PRNG state: Up to 16 bytes per ray
- Dimension advancement: Up to gPRNGDimension dimensions
- State storage: In payload
- Minimal overhead

### Ray Tracing Overhead

- Payload size: Up to 72 bytes
- Traversal stack: Limited by payload size
- Hit group selection: Multiple groups for different ray types
- Miss shader invocation: Per ray

### Scene Change Overhead

- Program recreation: Required when scene changes
- Binding table recreation: Required when scene changes
- Program vars recreation: Required when scene changes
- Shader recompilation: May be required

## Limitations

### Path Tracing Limitations

- No Russian roulette termination
- No next event estimation (NEE)
- No multiple importance sampling (MIS)
- Fixed path length (no adaptive termination)
- No path regularization techniques
- No variance reduction

### Material Sampling Limitations

- Only uniform or importance sampling
- No advanced sampling techniques
- No adaptive sampling
- No stratified sampling
- No blue noise sampling

### Light Sampling Limitations

- Uniform analytic light selection
- No importance sampling for lights
- No light PDF computation
- No MIS for light sampling

### Geometry Limitations

- No custom primitive support
- Limited to built-in geometry types
- No procedural geometry
- No instancing support

### Performance Limitations

- No temporal accumulation
- No denoising
- No adaptive sampling
- No progressive rendering
- No early ray termination

### Feature Limitations

- No transmission support (transmission handled with ray origin offset)
- No nested dielectrics
- No volume rendering
- No subsurface scattering
- No participating media

## Best Practices

### Bounce Count Configuration

- Use 0 for direct illumination only
- Use 1 for one indirect bounce
- Use 2-3 for balanced quality/performance
- Higher bounces increase computation exponentially
- Test bounce count impact on quality

### Direct Illumination

- Enable for accurate ground truth
- Disable when comparing indirect-only techniques
- Understand impact on total radiance
- Test with and without direct illumination

### Importance Sampling

- Enable for better material sampling
- Disable for uniform sampling comparison
- Understand impact on variance
- Test with and without importance sampling

### Scene Configuration

- Use appropriate geometry types for use case
- Enable required light types
- Configure environment map correctly
- Test scene compatibility

### Performance Optimization

- Minimize bounce count for preview
- Use uniform sampling for simplicity
- Disable unnecessary features
- Optimize payload size
- Test different geometry types

### Validation Use

- Compare against analytical solutions
- Test with known scenes
- Verify energy conservation
- Check for bias/variance
- Validate against other tracers

### Debugging

- Use low bounce counts for debugging
- Enable direct illumination only
- Test individual features
- Verify hit correctness
- Check material sampling

## Related Components

### Utils/Sampling/SampleGenerator

Sample generator for random numbers:
- `SampleGenerator::create()` - Create sample generator
- `getDefines()` - Get shader defines
- `bindShaderData()` - Bind to shader variables

### Scene/Scene

Scene system for rendering:
- `getShaderModules()` - Get shader modules
- `getSceneDefines()` - Get scene defines
- `getRaytracingMaxAttributeSize()` - Get max attribute size
- `getGeometryCount()` - Get geometry count
- `getGeometryIDs()` - Get geometry IDs
- `useAnalyticLights()` - Check analytic lights
- `useEmissiveLights()` - Check emissive lights
- `useEnvLight()` - Check environment light
- `useEnvBackground()` - Check environment background
- `raytrace()` - Execute ray tracing

### RenderGraph/RenderPassHelpers

Render pass helper utilities:
- `addRenderPassInputs()` - Add input channels
- `addRenderPassOutputs()` - Add output channels
- `calculateIOSize()` - Calculate I/O size

## References

**Path Tracing Fundamentals**:
- Monte Carlo integration
- Path tracing algorithm
- Light transport equation
- BSDF sampling
- Russian roulette termination

**Ray Tracing in Falcor**:
- Ray tracing API
- Payload-based ray tracing
- Hit/miss shaders
- Ray generation shaders

**Minimal Path Tracing**:
- Purpose: Validation and ground truth
- Design: Simple and naive implementation
- Use case: Compare against advanced techniques

## Future Enhancements

### Advanced Sampling Techniques

- Russian roulette termination
- Next event estimation (NEE)
- Multiple importance sampling (MIS)
- Adaptive sampling
- Stratified sampling
- Blue noise sampling

### Path Regularization

- Path regularization techniques
- Gradient-domain path tracing
- Manifold exploration
- Photon mapping integration
- Bidirectional path tracing

### Performance Optimizations

- Early ray termination
- Adaptive path length
- Progressive rendering
- Temporal accumulation
- Denoising integration
- Async ray tracing

### Feature Support

- Transmission support
- Nested dielectrics
- Volume rendering
- Subsurface scattering
- Participating media

### Geometry Support

- Custom primitives
- Procedural geometry
- Instancing support
- Displacement mapping
- Level of detail

### Light Support

- Importance sampling for lights
- Light PDF computation
- MIS for light sampling
- Area light sources
- Volume light sources

### Debugging Features

- Path visualization
- Per-bounce contribution display
- Radiance heatmaps
- Sampling pattern visualization
- Ray count statistics
