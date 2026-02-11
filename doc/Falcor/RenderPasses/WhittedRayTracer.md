# WhittedRayTracer - Whitted Ray Tracer

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **WhittedRayTracer** - Whitted ray tracer
  - [x] **WhittedRayTracer.h** - WhittedRayTracer header (119 lines)
  - [x] **WhittedRayTracer.cpp** - WhittedRayTracer implementation (330 lines)
  - [x] **WhittedRayTracer.rt.slang** - Ray tracing shader (903 lines)
  - [x] **WhittedRayTracerTypes.slang** - Type definitions (59 lines)

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Buffer, Texture, Formats)
- **Core/Object** - Base object class
- **Core/Program** - Shader program management (Program, RtProgramVars, RtBindingTable)
- **RenderGraph** - Render graph system (RenderPass, RenderPassReflection, RenderPassHelpers)
- **Scene** - Scene system (Scene, Camera, Lights, Materials, Geometry)
- **Utils/Sampling** - Sampling utilities (SampleGenerator)
- **Rendering/Materials** - Material system (TexLODTypes, TexLODHelpers, Fresnel)
- **Rendering/Lights** - Light management (LightHelpers)
- **Utils/Math** - Mathematical utilities (MathHelpers, MathConstants)

## Module Overview

WhittedRayTracer implements a classic Whitted-style ray tracer for recursive ray tracing with shadow rays, reflection, refraction, and direct illumination. This pass serves as an example and testbed for texture level-of-detail (LOD) filtering methods, demonstrating different approaches to handling texture filtering in ray tracing including Mip0, Ray Cones, and Ray Diffs. The pass supports multiple bounce levels, various light types (analytic, emissive, environment), and configurable filtering modes for ray cones and ray differentials. Materials are overridden to add ideal specular reflection and refraction components for unbiased rendering.

## Component Specifications

### WhittedRayTracer Class

**File**: [`WhittedRayTracer.h`](Source/RenderPasses/WhittedRayTracer/WhittedRayTracer.h:47)

**Purpose**: Simple Whitted ray tracer for texture LOD testing.

**Key Features**:
- Recursive ray tracing with configurable bounce depth
- Shadow ray testing for direct illumination
- Specular reflection and refraction
- Multiple texture LOD modes (Mip0, Ray Cones, Ray Diffs)
- Configurable ray cone and ray differential filtering
- Analytic light integration
- Emissive light support
- Environment light and background support
- Roughness-based ray cone variance
- Fresnel term as BRDF option

**Public Interface**:

```cpp
class WhittedRayTracer : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(WhittedRayTracer, "WhittedRayTracer", "Simple Whitted ray tracer.");

    static ref<WhittedRayTracer> create(ref<Device> pDevice, const Properties& props)
    {
        return make_ref<WhittedRayTracer>(pDevice, props);
    }

    WhittedRayTracer(ref<Device> pDevice, const Properties& props);

    Properties getProperties() const override;
    RenderPassReflection reflect(const CompileData& compileData) override;
    void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    void renderUI(Gui::Widgets& widget) override;
    void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override { return false; }
    bool onKeyEvent(const KeyboardEvent& keyEvent) override { return false; }

    void setTexLODMode(const TexLODMode mode) { mTexLODMode = mode; }
    TexLODMode getTexLODMode() const { return mTexLODMode; }

    void setRayConeMode(const RayConeMode mode) { mRayConeMode = mode; }
    RayConeMode getRayConeMode() const { return mRayConeMode; }

    void setRayConeFilterMode(const RayFootprintFilterMode mode) { mRayConeFilterMode = mode; }
    RayFootprintFilterMode getRayConeFilterMode() const { return mRayConeFilterMode; }

    void setRayDiffFilterMode(const RayFootprintFilterMode mode) { mRayDiffFilterMode = mode; }
    RayFootprintFilterMode getRayDiffFilterMode() const { return mRayDiffFilterMode; }

    static void registerBindings(pybind11::module& m);

private:
    void prepareVars();
    void setStaticParams(Program* pProgram) const;

    // Internal state

    /// Current scene.
    ref<Scene> mpScene;
    /// GPU sample generator.
    ref<SampleGenerator> mpSampleGenerator;

    /// Max number of indirect bounces (0 = none).
    uint mMaxBounces = 3;
    /// Which texture LOD mode to use.
    TexLODMode mTexLODMode = TexLODMode::Mip0;
    /// Which variant of ray cones to use.
    RayConeMode mRayConeMode = RayConeMode::Combo;
    /// Which filter mode to use for ray cones.
    RayFootprintFilterMode mRayConeFilterMode = RayFootprintFilterMode::Isotropic;
    /// Which filter mode to use for ray differentials.
    RayFootprintFilterMode mRayDiffFilterMode = RayFootprintFilterMode::Isotropic;
    /// Visualize surface spread angle at the first hit for ray cones methods.
    bool mVisualizeSurfaceSpread = false;
    /// Use roughness to variance to grow ray cones based on BSDF roughness.
    bool mUseRoughnessToVariance = false;
    /// Use Fresnel term as BRDF (instead of hacky throughput adjustment)
    bool mUseFresnelAsBRDF = false;

    // Runtime data

    /// Frame count since scene was loaded.
    uint mFrameCount = 0;
    bool mOptionsChanged = false;

    // Ray tracing program.
    struct
    {
        ref<Program> pProgram;
        ref<RtBindingTable> pBindingTable;
        ref<RtProgramVars> pVars;
    } mTracer;
};
```

**Public Members**: None (all members are private)

**Private Members**:
- `ref<Scene> mpScene` - Scene reference
- `ref<SampleGenerator> mpSampleGenerator` - Sample generator
- `uint mMaxBounces` - Max bounces (default 3)
- `TexLODMode mTexLODMode` - Texture LOD mode (default Mip0)
- `RayConeMode mRayConeMode` - Ray cone mode (default Combo)
- `RayFootprintFilterMode mRayConeFilterMode` - Ray cone filter mode (default Isotropic)
- `RayFootprintFilterMode mRayDiffFilterMode` - Ray diff filter mode (default Isotropic)
- `bool mVisualizeSurfaceSpread` - Visualize surface spread (default false)
- `bool mUseRoughnessToVariance` - Use roughness to variance (default false)
- `bool mUseFresnelAsBRDF` - Use Fresnel as BRDF (default false)
- `uint mFrameCount` - Frame count
- `bool mOptionsChanged` - Options changed flag
- `struct { ref<Program> pProgram; ref<RtBindingTable> pBindingTable; ref<RtProgramVars> pVars; } mTracer` - Ray tracing program

**Private Methods**:
- `void prepareVars()` - Prepare program variables
- `void setStaticParams(Program* pProgram) const` - Set static parameters

**Public Methods**:
- `Properties getProperties() const override` - Get properties
- `RenderPassReflection reflect(const CompileData& compileData) override` - Reflect input/output
- `void execute(RenderContext* pRenderContext, const RenderData& renderData) override` - Execute pass
- `void renderUI(Gui::Widgets& widget) override` - Render UI
- `void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override` - Set scene
- `bool onMouseEvent(const MouseEvent& mouseEvent) override` - Mouse event handler
- `bool onKeyEvent(const KeyboardEvent& keyEvent) override` - Keyboard event handler
- `void setTexLODMode(const TexLODMode mode)` - Set texture LOD mode
- `TexLODMode getTexLODMode() const` - Get texture LOD mode
- `void setRayConeMode(const RayConeMode mode)` - Set ray cone mode
- `RayConeMode getRayConeMode() const` - Get ray cone mode
- `void setRayConeFilterMode(const RayFootprintFilterMode mode)` - Set ray cone filter mode
- `RayFootprintFilterMode getRayConeFilterMode() const` - Get ray cone filter mode
- `void setRayDiffFilterMode(const RayFootprintFilterMode mode)` - Set ray diff filter mode
- `RayFootprintFilterMode getRayDiffFilterMode() const` - Get ray diff filter mode
- `static void registerBindings(pybind11::module& m)` - Register Python bindings

### WhittedRayTracer.rt.slang

**File**: [`WhittedRayTracer.rt.slang`](Source/RenderPasses/WhittedRayTracer/WhittedRayTracer.rt.slang:1)

**Purpose**: Ray tracing shader for Whitted-style ray tracing with texture LOD.

**Features**:
- Ray generation shader
- Scatter ray handling (128B payload for ray cones, 164B for ray diffs)
- Shadow ray handling (simple visibility test)
- Multiple bounce support (configurable max bounces)
- Specular reflection and refraction
- Direct illumination from analytic lights
- Emissive light integration
- Environment light and background
- Texture LOD with multiple modes (Mip0, Ray Cones, Ray Diffs)
- Ray cone propagation (Combo, Unified modes)
- Ray differential propagation (Isotropic, Anisotropic, AnisotropicWhenRefraction)
- Roughness-based ray cone variance
- Fresnel term as BRDF option
- Surface spread angle visualization
- Total internal reflection (TIR) handling
- Alpha testing for non-opaque geometry

**Ray Generation Shader**:
- Entry point: `rayGen`
- Pixel dispatch: 1x1x1 (one ray per pixel)
- Primary ray direction from camera
- G-buffer data loading (posW, normalW, tangentW, faceNormalW, texC, mtlData, vbuffer)
- Texture sampler creation (ExplicitLodTextureSampler, ExplicitGradientTextureSampler)
- Material instance creation
- BSDF properties query
- Direct illumination from analytic lights (uniform sampling, one shadow ray per light)
- Scatter ray initialization
- Ray cone initialization for Ray Cones mode
- Ray differential initialization for Ray Diffs mode
- Sample generator state (up to 16B per ray)
- PRNG dimension support
- Screen space pixel spread angle computation

**Scatter Ray Shader**:
- Entry points: `scatterAnyHit` (anyhit), `scatterClosestHit` (closesthit), `scatterMiss` (miss)
- Payload: `ScatterRayData` (128B for ray cones, 164B for ray diffs)
- Hit handling with alpha test for non-opaque geometry
- Vertex data loading from G-buffer or V-buffer
- Shading data preparation with flipped normal for back-facing hits on double-sided materials
- Eta (relative index of refraction) computation
- Texture LOD computation based on mode:
  - **Mip0**: ExplicitLodTextureSampler(0.f)
  - **Ray Cones**: Ray cone propagation with curvature-based spread angle, ExplicitRayConesLodTextureSampler
  - **Ray Diffs**: Ray differential propagation, ExplicitRayDiffsIsotropicTextureSampler or ExplicitGradientTextureSampler
- Ray cone modes: Combo, Unified
- Ray cone filter modes: Isotropic, AnisotropicWhenRefraction
- Ray diff filter modes: Isotropic, Anisotropic, AnisotropicWhenRefraction
- Surface spread angle computation from curvature and ray cone width
- Roughness to variance: `roughnessToSpread(roughness)` with GGX alpha computation
- Fresnel term as BRDF: `evalFresnelSchlick` or throughput adjustment
- Total internal reflection (TIR) detection and handling
- Emissive light contribution
- Direct illumination from analytic lights
- Path termination based on max bounces
- Low throughput termination (hacky)
- Ray cone propagation to next hit
- Ray differential propagation to next hit
- Refraction ray generation with TIR handling
- Reflection ray generation

**Shadow Ray Shader**:
- Entry points: `shadowAnyHit` (anyhit), `shadowMiss` (miss)
- Payload: `ShadowRayData` (simple visibility flag)
- Alpha test for non-opaque geometry
- Simple visibility test (TraceRay with RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH)

**Constants**:
- `kMaxPayloadSizeBytes` = 164 - Max payload size for ray diffs
- `kMaxAttributeSizeBytes` = 8 - Max attribute size
- `kMaxRecursionDepth` = 2 - Max recursion depth
- `kRayTMax` = FLT_MAX - Max ray distance
- `kMinCosTheta` - Minimum cosine theta for light sampling

### WhittedRayTracerTypes.slang

**File**: [`WhittedRayTracerTypes.slang`](Source/RenderPasses/WhittedRayTracer/WhittedRayTracerTypes.slang:1)

**Purpose**: Type definitions for Whitted ray tracer.

**Features**:
- Ray footprint filter mode enum (Isotropic, Anisotropic, AnisotropicWhenRefraction)
- CPU/GPU shared enum
- Enum registration with FALCOR_ENUM_INFO and FALCOR_ENUM_REGISTER

**RayFootprintFilterMode Enum**:
```cpp
enum class RayFootprintFilterMode : uint32_t
{
    Isotropic = 0,
    Anisotropic = 1,
    AnisotropicWhenRefraction = 2
};
```

## Technical Details

### Render Graph Integration

**Input Channels**:
- `posW` - World-space position (xyz) and foreground flag (w)
- `normalW` - World-space shading normal (xyz)
- `tangentW` - World-space shading tangent (xyz) and sign (w)
- `faceNormalW` - Face normal in world space (xyz)
- `texC` - Texture coordinate
- `texGrads` - Texture gradients (optional)
- `mtlData` - Material data
- `vbuffer` - V-buffer buffer in packed format (optional)

**Output Channels**:
- `color` - Output color (sum of direct and indirect)

### Ray Tracing Program Setup

**Program Creation**:
- Shader library: WhittedRayTracer.rt.slang
- Scene shader modules integration
- Type conformances from scene
- Max payload size: 164 bytes (for ray diffs)
- Max attribute size: 8 bytes
- Max recursion depth: 2
- Ray type count: 2 (scatter, shadow)
- Geometry type: TriangleMesh only (procedural geometry warning)
- Hit groups: scatterClosestHit, shadowAnyHit
- Miss shaders: scatterMiss, shadowMiss
- Ray generation shader: rayGen

**Binding Table Configuration**:
- RayGen: rayGen
- Miss 0: scatterMiss
- Miss 1: shadowMiss
- Hit Group 0: scatterClosestHit (TriangleMesh geometry)
- Hit Group 1: shadowAnyHit (TriangleMesh geometry)

### Texture LOD Modes

**Mip0 Mode**:
- Uses ExplicitLodTextureSampler(0.f)
- No ray cone or ray differential tracking
- Simplest mode, no texture filtering
- Suitable for reference rendering

**Ray Cones Mode**:
- Ray cone propagation through path segments
- Surface spread angle computation from curvature
- Ray cone width: `rayConeWidth = hitT + rayCone.getWidth()`
- Ray cone spread angle: `surfaceSpreadAngle = computeSpreadAngleFromCurvatureIso(curvature, rayConeWidth, rayDir, v.normalW)`
- Ray cone LOD: `ExplicitRayConesLodTextureSampler(lambda)` where lambda is computed from spread angle
- Ray cone propagation: `rayData.rayCone.propagateDistance(hitT)`
- Sub-modes: Combo, Unified

**Ray Diffs Mode**:
- Ray differential propagation through path segments
- Ray differential propagation from camera to hit point
- Ray differential computation using vertex positions, texture coordinates, and normals
- Ray differential LOD: ExplicitRayDiffsIsotropicTextureSampler(dUVdx, dUVdy) or ExplicitGradientTextureSampler(ddx, ddy)
- Ray differential propagation: `rayData.rayDiff.propagate(rayOrg, rayDir, hitT, geometricNormal)`
- Sub-modes: Isotropic, Anisotropic, AnisotropicWhenRefraction

### Ray Cone Propagation

**Combo Mode**:
- Uses both ray cone and ray differential
- Ray cone for texture LOD at hit point
- Ray differential for texture LOD at hit point
- Reflection: ray cone propagation with 2.0x spread angle factor
- Refraction: ray cone propagation with surface spread angle from curvature

**Unified Mode**:
- Uses only ray cone (no ray differential)
- Ray cone for texture LOD at hit point
- Reflection: ray cone propagation with 2.0x spread angle factor
- Refraction: ray cone propagation with surface spread angle from curvature

### Ray Differential Propagation

**Isotropic Mode**:
- Uses ExplicitRayDiffsIsotropicTextureSampler(dUVdx, dUVdy)
- Computes single lambda for texture LOD
- Simple isotropic filtering

**Anisotropic Mode**:
- Uses ExplicitGradientTextureSampler(ddx, ddy)
- Computes anisotropic filtering using ray differentials
- More accurate filtering for anisotropic surfaces

**AnisotropicWhenRefraction Mode**:
- Isotropic filtering for reflection
- Anisotropic filtering for refraction
- Handles different filtering requirements for reflection vs. refraction

### Roughness to Variance

**Computation**:
```cpp
float roughnessToSpread(float roughness)
{
    const float ggxAlpha = roughness * roughness;
    const float s = ggxAlpha * ggxAlpha;
    const float sigma2 = (s / max(0.0001f, (1.0f - s))) * 0.5f;
    return 2.0f * sqrt(sigma2);
}
```

**Purpose**:
- Grows ray cones based on BSDF roughness
- Uses GGX alpha computation: `alpha = roughness^2`
- Computes variance: `sigma2 = (s / (1.0 - s)) * 0.5`
- Returns spread angle: `2.0 * sqrt(sigma2)`

**Effect**:
- Rougher materials have wider ray cones
- Smoother materials have narrower ray cones
- Improves texture filtering accuracy for rough surfaces

### Fresnel Term as BRDF

**Option Enabled**:
- Uses `evalFresnelSchlick(specular, 1.0f, max(0.00001f, dot(normal, wo)))`
- Multiplies throughput by Fresnel term
- More physically accurate
- Better for development and comparison

**Option Disabled**:
- Uses throughput adjustment: `(specular.x + specular.y + specular.z) / 3.0f`
- Hacky but faster
- Darker than Fresnel term

### Surface Spread Angle Visualization

**Purpose**:
- Visualize surface spread angle for ray cones methods
- Multiplies gOutputColor by 10x with surface spread angle
- Red channel: surface spread angle
- Green and Blue channels: 0
- Helps debug ray cone computation

**Implementation**:
```cpp
if (kVisualizeSurfaceSpread)
{
    const float eps = 1.0e-4f;
    float r = surfaceSpreadAngle > eps ? surfaceSpreadAngle : 0.0f;
    float g = surfaceSpreadAngle <= -eps ? -surfaceSpreadAngle : 0.0f;
    float b = abs(surfaceSpreadAngle) < eps ? 1.0f : 0.0f;
    gOutputColor[launchIndex] = float4(r, g, b, 1.0f) * 10.0f;
    return;
}
```

### Total Internal Reflection (TIR)

**Detection**:
- Occurs when refraction angle exceeds critical angle
- Computed using Snell's law and relative index of refraction
- Eta computation: `eta = frontFacing ? (IoR / insideIoR) : (insideIoR / IoR)`

**Handling**:
- Reflection ray generation instead of refraction
- Ray cone propagation with surface spread angle
- Ray differential propagation (if not TIR)
- Fresnel term applied to throughput
- Path continues with reflection

**Eta Computation**:
```cpp
float computeEta(const ShadingData sd)
{
    float insideIoR = gScene.materials.evalIoR(sd.materialID);
    float eta = sd.frontFacing ? (sd.IoR / insideIoR) : (insideIoR / sd.IoR);
    return eta;
}
```

**Purpose**:
- Computes relative index of refraction
- Handles both front-facing and back-facing hits
- Uses material IoR (index of refraction)
- Computes inside IoR from material system

### Direct Illumination

**Analytic Lights**:
- Uniform light selection: `invPdf = 1.0f / lightCount`
- One shadow ray per light
- Lower hemisphere test: `dot(ls.dir, getFlippedShadingNormal(sd)) > kMinCosTheta`
- Visibility test via shadow ray
- Contribution: `mi.eval(sd, ls.dir, sg) * ls.Li * invPdf`

**Light Sources**:
- Point lights
- Directional lights
- Rect lights
- Disk lights
- Analytic lights from Falcor's light system

**Shadow Ray Testing**:
```cpp
bool traceShadowRay(float3 origin, float3 dir, float distance)
{
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = dir;
    ray.TMin = 0.f;
    ray.TMax = distance;

    ShadowRayData rayData;
    rayData.visible = false; // Set to true by miss shader if ray is not terminated before.
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

**Features**:
- Simple visibility test
- Accepts first hit and ends search
- Alpha test in hit shader
- Instance inclusion mask: 0xff (all instances)

### Emissive Lights

**Integration**:
- Emissive geometry treated as light sources
- Direct illumination from emissive materials
- Emissive light sampling via light system
- Emissive light contribution at hit point

**Implementation**:
- Emissive light contribution: `rayData.radiance += rayData.thp * bsdfProperties.emission`
- Independent of emissive light flag
- Always added to output

### Environment Light and Background

**Environment Light**:
- Environment map sampling for miss shader
- LOD computation based on ray cone or ray differential
- Environment map evaluation: `gScene.envMap.eval(WorldRayDirection(), lod)`

**LOD Computation**:
```cpp
if (kUseEnvLight)
{
    float lod = 0.0f; // TexLODMode::Mip0
    if (kTexLODMode == TexLODMode::RayCones)
    {
        lod = computeEnvironmentMapLOD(rayData.rayCone.getSpreadAngle(), gScene.envMap.envMap);
    }
    else if (kTexLODMode == TexLODMode::RayDiffs)
    {
        lod = computeEnvironmentMapLOD(rayData.rayDiff.getdDdx(), rayData.rayDiff.getdDdy(), gScene.envMap.envMap);
    }
    float3 Le = gScene.envMap.eval(WorldRayDirection(), lod);
    rayData.radiance += rayData.thp * Le;
}
```

**Background**:
- Environment background for miss shader
- Default background color: `float3(0.25f, 0.25f, 0.25f)`
- Background color when environment map not available
- LOD computation similar to environment light

### Path Termination

**Max Bounces**:
- Configurable max bounces (default 3, range 0-10)
- Direct only: maxBounces = 0
- One indirect bounce: maxBounces = 1
- Two indirect bounces: maxBounces = 2

**Termination Conditions**:
- Max bounces reached: `rayData.pathLength >= kMaxBounces`
- Low throughput: `max(rayData.thp.x, rayData.thp.y), rayData.thp.z) < 0.01f`
- Miss shader: ray missed scene
- Alpha test failed (non-opaque geometry)

**Path Length Tracking**:
- Path length in number of path segments (0 at origin, 1 at first secondary hit, etc.)
- Max path length: 2^31
- Incremented at each hit: `rayData.pathLength++`

### Material Integration

**Material Override**:
- Scene materials overridden to add ideal specular reflection and refraction
- Unbiased rendering should not be expected
- Ideal components for testing texture LOD

**Material Instance Creation**:
```cpp
let mi = gScene.materials.getMaterialInstance(sd, lod);
let bsdfProperties = mi.getProperties(sd);
```

**BSDF Properties**:
- Specular reflectance
- Diffuse reflectance
- Roughness
- Emission
- IoR (index of refraction)
- Double-sided flag

### Sample Generator Integration

**Sample Generator**:
- Uniform sampling: `SAMPLE_GENERATOR_UNIFORM`
- Per-ray state: up to 16B
- Frame-based seeding: `SampleGenerator(launchIndex, gFrameCount)`
- PRNG dimension support for advanced sampling

**Dimension Advancement**:
```cpp
// Advance generator to the first available dimension.
// TODO: This is potentially expensive. We may want to store/restore state from memory if it becomes a problem.
for (uint i = 0; i < gPRNGDimension; i++)
    sampleNext1D(sg);
```

**Purpose**:
- Advances sample generator to first available PRNG dimension
- Allows integration with advanced sampling techniques
- Potential performance concern noted in code

### Ray Cone Computation

**Ray Cone Structure**:
- Width: `rayConeWidth = hitT + rayCone.getWidth()`
- Spread angle: `rayCone.getSpreadAngle()`
- Propagation: `rayCone.propagateDistance(hitT)`

**Spread Angle Computation**:
```cpp
surfaceSpreadAngle += roughnessToSpread(bsdfProperties.roughness);
if (eta == 1.0f) // Reflection
{
    // Reflection needs a factor of 2.0 (see RTG1 article on texture LOD).
    rayData.rayCone = rayData.rayCone.addToSpreadAngle(2.0f * surfaceSpreadAngle);
}
else // Refraction
{
    if (!refractRayCone(
                rayData.rayCone, rayOrg, rayDir, hitPoint, getFlippedShadingNormal(sd), surfaceSpreadAngle, eta, refractedRayDir
            ))
    {
        rayData.rayCone = rayData.rayCone.addToSpreadAngle(surfaceSpreadAngle);
        tir = true;
    }
}
```

**Curvature Computation**:
```cpp
float curvature = gScene.computeCurvatureIsotropic(instanceID, PrimitiveIndex());
```

**Purpose**:
- Computes isotropic curvature at hit point
- Used for ray cone spread angle computation
- Improves texture filtering accuracy

### Ray Differential Computation

**Ray Differential Structure**:
- dOdx, dOdy: Ray origin differentials
- dNdx, dNdy: Ray direction differentials
- dBarydx, dBarydy: Barycentric differentials
- dUVdx, dUVdy: UV differentials
- Unnormalized normals for ray differential computation

**Ray Differential Propagation**:
```cpp
RayDiff newRayDiff = rayData.rayDiff.propagate(rayOrg, rayDir, hitT, geometricNormal);
```

**Reflection**:
```cpp
reflectRayDifferential(newRayDiff, rayDir, unnormalizedN, v.normalW, dBarydx, dBarydy, normals);
```

**Refraction**:
```cpp
if (!refractRayDifferential(
            newRayDiff, rayDir, unnormalizedN, normalize(unnormalizedN), dBarydx, dBarydy, normals, eta, refractedRayDir
        ))
{
    reflectRayDifferential(newRayDiff, rayDir, unnormalizedN, normalize(unnormalizedN), dBarydx, dBarydy, normals);
    tir = true;
}
```

**TIR Detection**:
- Occurs when refraction fails
- Sets `tir = true` flag
- Reflection ray generated instead of refraction

### Ray Direction Differentials

**Computation**:
```cpp
computeRayDirectionDifferentials(
    nonNormalizedRayDir, gScene.camera.data.cameraU, gScene.camera.data.cameraV, launchDim, dDdx, dDdy
);
```

**Purpose**:
- Computes ray direction differentials from camera
- Used for ray differential initialization
- Computes dOdx, dOdy, dNdx, dNdy

### Barycentric Differentials

**Computation**:
```cpp
computeBarycentricDifferentials(rayData.rayDiff, rayDir, edge1, edge2, sd.faceN, dBarydx, dBarydy);
```

**Purpose**:
- Computes barycentric differentials from ray differential
- Used for anisotropic filtering
- Improves texture filtering accuracy

### Normal Differentials

**Computation**:
```cpp
computeNormalDifferentials(rayData.rayDiff, unnormalizedN, dBarydx, dBarydy, normals, dNdx, dNdy);
```

**Purpose**:
- Computes normal differentials from ray differential
- Used for anisotropic filtering
- Improves texture filtering accuracy

### Texture LOD Computation

**Ray Cone LOD**:
```cpp
float lambda = rayData.rayCone.computeLOD(v.coneTexLODValue, rayDir, v.normalW);
lod = ExplicitRayConesLodTextureSampler(lambda);
```

**Ray Diff LOD**:
```cpp
// When using ExplicitRayDiffsIsotropicTextureSampler, the texture sampler will compute a single lambda for texture LOD
// using dUVdx and dUVdy.
lod = ExplicitRayDiffsIsotropicTextureSampler(dUVdx, dUVdy);
```

**Ray Diff LOD (Anisotropic)**:
```cpp
// Use dUVdx and dUVdy directly --> gives you anisotropic filtering.
lod = ExplicitGradientTextureSampler(dUVdx, dUVdy);
```

**Environment Map LOD**:
```cpp
// Ray Cones
lod = computeEnvironmentMapLOD(rayData.rayCone.getSpreadAngle(), gScene.envMap.envMap);

// Ray Diffs
computeRayDirectionDifferentials(
    nonNormalizedRayDir, gScene.camera.data.cameraU, gScene.camera.data.cameraV, launchDim, dDdx, dDdy
);
lod = computeEnvironmentMapLOD(dDdx, dDdy, gScene.envMap.envMap);
```

### Flipped Shading Normal

**Purpose**:
- Reproduces legacy behavior before conventions were updated
- Handles double-sided materials correctly

**Computation**:
```cpp
float3 getFlippedShadingNormal(const ShadingData sd)
{
    return !sd.frontFacing && sd.mtl.isDoubleSided() ? -sd.frame.N : sd.frame.N;
}
```

**Purpose**:
- Flips shading normal for back-facing hits on double-sided materials
- Ensures correct lighting computation
- Maintains backward compatibility

## Architecture Patterns

### Render Pass Pattern
- Inherits from RenderPass base class
- Implements render() method
- Specifies input/output resources
- Manages shader compilation
- Parameter reflection
- Scene integration

### Ray Tracing Pattern
- DXR 1.1 ray tracing
- Hit groups for scatter and shadow rays
- Miss shaders for scatter and shadow rays
- Payload management
- Binding table configuration

### Texture LOD Pattern
- Multiple LOD modes (Mip0, Ray Cones, Ray Diffs)
- Ray cone propagation
- Ray differential propagation
- Curvature-based spread angle
- Roughness-based variance

### Material Override Pattern
- Scene materials overridden for ideal components
- Specular reflection and refraction
- Unbiased rendering
- Texture LOD testing

## Code Patterns

### Property Parsing
```cpp
void WhittedRayTracer::parseProperties(const Properties& props)
{
    for (const auto& [key, value] : props)
    {
        if (key == kMaxBounces)
            mMaxBounces = (uint32_t)value;
        else if (key == kTexLODMode)
            mTexLODMode = value;
        else if (key == kRayConeMode)
            mRayConeMode = value;
        else if (key == kRayConeFilterMode)
            mRayConeFilterMode = value;
        else if (key == kRayDiffFilterMode)
            mRayDiffFilterMode = value;
        else if (key == kUseRoughnessToVariance)
            mUseRoughnessToVariance = value;
        else
            logWarning("Unknown property '{}' in a WhittedRayTracer properties.", key);
    }
}
```

**Purpose**:
- Parse configuration properties
- Set configuration parameters
- Warn for unknown properties

### Property Serialization
```cpp
Properties WhittedRayTracer::getProperties() const
{
    Properties props;
    props[kMaxBounces] = mMaxBounces;
    props[kTexLODMode] = mTexLODMode;
    props[kRayConeMode] = mRayConeMode;
    props[kRayConeFilterMode] = mRayConeFilterMode;
    props[kRayDiffFilterMode] = mRayDiffFilterMode;
    props[kUseRoughnessToVariance] = mUseRoughnessToVariance;
    return props;
}
```

**Purpose**:
- Serialize configuration properties
- Enable save/load of configuration
- Maintain state across sessions

### Scene Setup
```cpp
void WhittedRayTracer::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    // Clear data for previous scene.
    // After changing scene, raytracing program should be recreated.
    mTracer.pProgram = nullptr;
    mTracer.pBindingTable = nullptr;
    mTracer.pVars = nullptr;
    mFrameCount = 0;

    // Set new scene.
    mpScene = pScene;

    if (mpScene)
    {
        if (mpScene->hasProceduralGeometry())
        {
            logWarning("WhittedRayTracer: This render pass only supports triangles. Other types of geometry will be ignored.");
        }

        // Create ray tracing program.
        ProgramDesc desc;
        desc.addShaderModules(mpScene->getShaderModules());
        desc.addShaderLibrary(kShaderFile);
        desc.addTypeConformances(mpScene->getTypeConformances());
        desc.setMaxPayloadSize(kMaxPayloadSizeBytes);
        desc.setMaxAttributeSize(kMaxAttributeSizeBytes);
        desc.setMaxTraceRecursionDepth(kMaxRecursionDepth);

        mTracer.pBindingTable = RtBindingTable::create(2, 2, mpScene->getGeometryCount());
        auto& sbt = mTracer.pBindingTable;
        sbt->setRayGen(desc.addRayGen("rayGen"));
        sbt->setMiss(0, desc.addMiss("scatterMiss"));
        sbt->setMiss(1, desc.addMiss("shadowMiss"));
        sbt->setHitGroup(
            0, mpScene->getGeometryIDs(Scene::GeometryType::TriangleMesh), desc.addHitGroup("scatterClosestHit", "scatterAnyHit")
        );
        sbt->setHitGroup(1, mpScene->getGeometryIDs(Scene::GeometryType::TriangleMesh), desc.addHitGroup("", "shadowAnyHit"));

        mTracer.pProgram = Program::create(mpDevice, desc, mpScene->getSceneDefines());
    }
}
```

**Purpose**:
- Set scene for ray tracing
- Create ray tracing program
- Configure binding table
- Handle scene changes

**Program Recreation**:
```cpp
// Clear data for previous scene.
// After changing scene, raytracing program should be recreated.
mTracer.pProgram = nullptr;
mTracer.pBindingTable = nullptr;
mTracer.pVars = nullptr;
```

**Purpose**:
- Clear previous scene data
- Force program recreation on scene change
- Handle scene changes correctly

### Program Preparation
```cpp
void WhittedRayTracer::prepareVars()
{
    FALCOR_ASSERT(mTracer.pProgram);

    // Configure program.
    mTracer.pProgram->addDefines(mpSampleGenerator->getDefines());

    // Create program variables for the current program.
    // This may trigger shader compilation. If it fails, throw an exception to abort rendering.
    mTracer.pVars = RtProgramVars::create(mpDevice, mTracer.pProgram, mTracer.pBindingTable);

    // Bind utility classes into shared data.
    auto var = mTracer.pVars->getRootVar();
    mpSampleGenerator->bindShaderData(var);
}
```

**Purpose**:
- Prepare program variables
- Bind sample generator
- Trigger shader compilation if needed

### Static Parameter Setting
```cpp
void WhittedRayTracer::setStaticParams(Program* pProgram) const
{
    DefineList defines;
    defines.add("MAX_BOUNCES", std::to_string(mMaxBounces));
    defines.add("TEX_LOD_MODE", std::to_string(static_cast<uint32_t>(mTexLODMode)));
    defines.add("RAY_CONE_MODE", std::to_string(static_cast<uint32_t>(mRayConeMode)));
    defines.add("VISUALIZE_SURFACE_SPREAD", mVisualizeSurfaceSpread ? "1" : "0");
    defines.add("RAY_CONE_FILTER_MODE", std::to_string(static_cast<uint32_t>(mRayConeFilterMode)));
    defines.add("RAY_DIFF_FILTER_MODE", std::to_string(static_cast<uint32_t>(mRayDiffFilterMode)));
    defines.add("USE_ANALYTIC_LIGHTS", mpScene->useAnalyticLights() ? "1" : "0");
    defines.add("USE_EMISSIVE_LIGHTS", mpScene->useEmissiveLights() ? "1" : "0");
    defines.add("USE_ENV_LIGHT", mpScene->useEnvLight() ? "1" : "0");
    defines.add("USE_ENV_BACKGROUND", mpScene->useEnvBackground() ? "1" : "0");
    defines.add("USE_ROUGHNESS_TO_VARIANCE", mUseRoughnessToVariance ? "1" : "0");
    defines.add("USE_FRESNEL_AS_BRDF", mUseFresnelAsBRDF ? "1" : "0");
    pProgram->addDefines(defines);
}
```

**Purpose**:
- Set static parameters as compile-time defines
- Configure shader behavior
- Enable/disable features

### Execute Implementation
```cpp
void WhittedRayTracer::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // Update refresh flag if options that affect output have changed.
    auto& dict = renderData.getDictionary();
    if (mOptionsChanged)
    {
        auto flags = dict.getValue(kRenderPassRefreshFlags, RenderPassRefreshFlags::None);
        dict[Falcor::kRenderPassRefreshFlags] = flags | Falcor::RenderPassRefreshFlags::RenderOptionsChanged;
        mOptionsChanged = false;
    }

    // If we have no scene, just clear the outputs and return.
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

    // Check for scene changes that require shader recompilation.
    if (is_set(mpScene->getUpdates(), IScene::UpdateFlags::RecompileNeeded) ||
        is_set(mpScene->getUpdates(), IScene::UpdateFlags::GeometryChanged))
    {
        FALCOR_THROW("This render pass does not support scene changes that require shader recompilation.");
    }

    setStaticParams(mTracer.pProgram.get());

    // For optional I/O resources, set 'is_valid_<name>' defines to inform the program of which ones it can access.
    // TODO: This should be moved to a more general mechanism using Slang.
    mTracer.pProgram->addDefines(getValidResourceDefines(kInputChannels, renderData));
    mTracer.pProgram->addDefines(getValidResourceDefines(kOutputChannels, renderData));

    // Prepare program vars. This may trigger shader compilation.
    // The program should have all necessary defines set at this point.
    if (!mTracer.pVars)
        prepareVars();
    FALCOR_ASSERT(mTracer.pVars);

    // Get dimensions of ray dispatch.
    const uint2 targetDim = renderData.getDefaultTextureDims();
    FALCOR_ASSERT(targetDim.x > 0 && targetDim.y > 0);

    // Set constants.
    auto var = mTracer.pVars->getRootVar();
    var["CB"]["gFrameCount"] = mFrameCount;
    var["CB"]["gPRNGDimension"] = dict.keyExists(kRenderPassPRNGDimension) ? dict[kRenderPassPRNGDimension] : 0u;
    // Set up screen space pixel angle for texture LOD using ray cones
    var["CB"]["gScreenSpacePixelSpreadAngle"] = mpScene->getCamera()->computeScreenSpacePixelSpreadAngle(targetDim.y);

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

    // Spawn rays.
    mpScene->raytrace(pRenderContext, mTracer.pProgram.get(), mTracer.pVars, uint3(targetDim, 1));

    mFrameCount++;
}
```

**Purpose**:
- Execute ray tracing pass
- Update refresh flags
- Handle scene changes
- Bind resources
- Spawn rays

### Resource Binding
```cpp
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
```

**Purpose**:
- Bind input/output resources
- Handle optional resources
- Per-frame binding

### Refresh Flag Management
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

**Purpose**:
- Notify other passes of render option changes
- Reset temporal data in other passes
- Handle configuration changes

### UI Rendering
```cpp
void WhittedRayTracer::renderUI(Gui::Widgets& widget)
{
    bool dirty = false;

    dirty |= widget.var("Max bounces", mMaxBounces, 0u, 10u);
    widget.tooltip("Maximum path length for indirect illumination.\n0 = direct only\n1 = one indirect bounce etc.", true);

    if (auto mode = mTexLODMode; widget.dropdown("Texture LOD mode", mode))
    {
        setTexLODMode(mode);
        dirty = true;
    }
    widget.tooltip("The texture level-of-detail mode to use.");
    if (mTexLODMode == TexLODMode::RayCones)
    {
        if (auto mode = mRayConeMode; widget.dropdown("Ray cone mode", mode))
        {
            setRayConeMode(mode);
            dirty = true;
        }
        widget.tooltip("The variant of ray cones to use.");

        if (auto mode = mRayConeFilterMode; widget.dropdown("Ray cone filter mode", mode))
        {
            setRayConeFilterMode(mode);
            dirty = true;
        }
        widget.tooltip("What type of ray cone filter method to use beyond the first hit");

        dirty |= widget.checkbox("Use BSDF roughness", mUseRoughnessToVariance);
        widget.tooltip("Grow ray cones based on BSDF roughness.");

        dirty |= widget.checkbox("Visualize surface spread", mVisualizeSurfaceSpread);
        widget.tooltip("Visualize the surface spread angle for ray cones methods times 10.");
    }

    if (mTexLODMode == TexLODMode::RayDiffs)
    {
        if (auto mode = mRayDiffFilterMode; widget.dropdown("Ray diff filter mode", mode))
        {
            setRayDiffFilterMode(mode);
            dirty = true;
        }
        widget.tooltip("What type of ray diff filter method to use beyond the first hit");
    }
    dirty |= widget.checkbox("Use Fresnel As BRDF", mUseFresnelAsBRDF);

    // If rendering options that modify the output have changed, set flag to indicate that.
    // In execute() we will pass the flag to other passes for reset of temporal data etc.
    if (dirty)
    {
        mOptionsChanged = true;
    }
}
```

**Purpose**:
- Render UI controls
- Update configuration parameters
- Track option changes

## Integration Points

### Render Graph Integration
- Input/output specification via reflect()
- Resource binding in execute()
- Refresh flag management for temporal passes
- Scene change detection
- Optional resource handling

### Scene Integration
- Scene reference management
- Ray tracing program creation
- Geometry type validation (triangles only)
- Light system integration (analytic, emissive, environment)
- Material system integration (material override)
- Camera integration (ray direction computation)

### Material System Integration
- Material instance creation
- BSDF properties query
- Shading data preparation
- Eta computation for refraction
- Specular reflection and refraction

### Light System Integration
- Analytic light sampling
- Emissive light integration
- Environment light integration
- Light visibility testing
- Direct illumination computation

### Sampling Integration
- Sample generator integration
- PRNG dimension support
- Frame-based seeding
- Dimension advancement

### Python Bindings
```cpp
void WhittedRayTracer::registerBindings(pybind11::module& m)
{
    pybind11::class_<WhittedRayTracer, RenderPass, ref<WhittedRayTracer>> pass(m, "WhittedRayTracer");
}
```

**Purpose**:
- Register WhittedRayTracer class with Python
- Enable scripting support
- Allow configuration from Python

### Plugin Registration
```cpp
extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, WhittedRayTracer>();
    ScriptBindings::registerBinding(WhittedRayTracer::registerBindings);
}
```

**Purpose**:
- Register WhittedRayTracer plugin
- Enable render pass discovery
- Register script bindings

## Use Cases

### Texture LOD Testing
- Baseline for comparing texture LOD methods
- Testbed for texture LOD research
- Evaluation of different filtering approaches
- Performance comparison

### Rendering Applications
- Reference rendering for validation
- Educational use for ray tracing concepts
- Research and development of texture LOD techniques
- Benchmarking different LOD methods

### Material System Testing
- Test ideal specular reflection and refraction
- Validate material integration
- Test material parameter effects on ray tracing

### Light System Testing
- Test analytic light integration
- Test emissive light integration
- Test environment light integration
- Test shadow ray visibility

## Performance Considerations

### GPU Computation
- Ray tracing: O(width × height) rays per frame
- Shadow rays: O(width × height × lightCount) rays per frame
- Bounce depth: O(width × height × maxBounces) operations per frame
- Total: O(width × height × (1 + lightCount + maxBounces)) per frame

### Memory Usage
- Payload size: 164B for ray diffs, 128B for ray cones
- Sample generator state: 16B per ray
- G-buffer inputs: O(width × height × sizeof(PackedHitInfo)) bytes
- Output texture: O(width × height × 16) bytes

### Computational Complexity
- Primary ray generation: O(width × height) per frame
- Scatter ray tracing: O(width × height × maxBounces) per frame
- Shadow ray testing: O(width × height × lightCount) per frame
- Ray cone propagation: O(width × height × maxBounces) per frame
- Ray differential propagation: O(width × height × maxBounces) per frame
- Overall: O(width × height × (1 + lightCount + maxBounces)) per frame

### Performance Optimization
- Ray cone propagation reduces texture lookups
- Ray differential propagation improves filtering accuracy
- Roughness-based variance adapts to material properties
- Alpha test optimization for non-opaque geometry
- Instance inclusion mask: 0xff (all instances)

### Performance Limitations
- No temporal accumulation
- No denoising
- No adaptive sampling
- No importance sampling
- Uniform light sampling (inefficient)
- Ray cone and ray differential computation overhead

## Limitations

### Feature Limitations
- Triangle-only geometry support (procedural geometry ignored)
- No custom material models (uses ideal components)
- No global illumination (only direct + indirect bounces)
- No advanced lighting techniques
- No volumetric effects
- No subsurface scattering
- No anisotropic materials (isotropic filtering only)

### Texture LOD Limitations
- Mip0 mode: no texture filtering
- Ray Cones mode: approximate filtering
- Ray Diffs mode: anisotropic filtering only
- No adaptive LOD selection
- No texture LOD bias compensation

### Material Limitations
- Ideal specular reflection (not physically based)
- Ideal refraction (not physically based)
- No subsurface scattering
- No anisotropic materials
- No complex material models
- No texture LOD in materials

### Light Limitations
- Uniform light sampling (inefficient)
- No importance sampling
- No light tree sampling
- No light culling
- No light clustering

### Performance Limitations
- No temporal accumulation
- No denoising
- No adaptive sampling
- No progressive rendering
- No performance optimization for large scenes
- No parallel reduction optimization

### Integration Limitations
- Limited to Falcor's scene system
- Limited to Falcor's material system
- Limited to Falcor's light system
- Limited to Falcor's ray tracing API
- No external data sources

### UI Limitations
- No preset management
- No undo/redo functionality
- No real-time preview
- Limited configuration options
- No visualization of ray paths
- No debugging tools

## Best Practices

### Texture LOD Configuration
- Use Mip0 mode for reference rendering
- Use Ray Cones mode for better filtering
- Use Ray Diffs mode for anisotropic filtering
- Configure ray cone mode based on scene
- Configure ray cone filter mode based on scene
- Use roughness to variance for rough materials
- Visualize surface spread for debugging

### Bounce Count Configuration
- Use 0 bounces for direct illumination only
- Use 1 bounce for simple indirect illumination
- Use 2-3 bounces for complex scenes
- Avoid high bounce counts for performance
- Balance quality vs. performance

### Material Configuration
- Use appropriate material models
- Configure material properties correctly
- Test with different materials
- Verify BSDF evaluation
- Check refraction behavior

### Light Configuration
- Use appropriate light types for scene
- Configure light properties correctly
- Test with different light configurations
- Verify shadow ray visibility
- Check light contribution

### Performance Optimization
- Minimize ray cone computation overhead
- Optimize ray differential computation
- Use appropriate filter modes
- Profile GPU performance
- Test with different resolutions
- Balance quality vs. performance

### Debugging
- Use surface spread visualization for debugging
- Use Fresnel as BRDF for comparison
- Test with different texture LOD modes
- Profile ray cone computation
- Profile ray differential computation
- Analyze performance bottlenecks

### Research and Development
- Study texture LOD techniques
- Analyze ray cone propagation
- Research ray differential propagation
- Investigate filtering methods
- Develop new techniques
- Compare different approaches

### Educational Use
- Demonstrate ray tracing concepts
- Show texture LOD techniques
- Explain ray cone propagation
- Explain ray differential propagation
- Teach material evaluation
- Visualize surface spread

### Integration Best Practices
- Connect to appropriate render graph inputs
- Use correct texture formats
- Handle optional inputs/outputs
- Test with different render graphs
- Validate compatibility
- Ensure proper synchronization
