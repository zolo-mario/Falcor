# RTXDIPass - RTXDI Pass

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **RTXDIPass** - RTXDI pass
  - [x] **RTXDIPass.h** - RTXDI pass header
  - [x] **RTXDIPass.cpp** - RTXDI pass implementation
  - [x] **PrepareSurfaceData.cs.slang** - Surface data preparation shader
  - [x] **FinalShading.cs.slang** - Final shading shader

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Texture, Buffer)
- **Core/Object** - Base object class
- **Core/Program** - Shader program management (Program, ProgramVars, ComputePass)
- **RenderGraph** - Render graph system (RenderPass, RenderPassReflection, RenderPassHelpers)
- **Scene** - Scene system (Scene, Camera, Lights, Materials)
- **Rendering/RTXDI** - RTXDI integration (RTXDI, RTXDI::Options)
- **Rendering/Materials** - Material system (MaterialInstance, BSDF)
- **Utils/UI** - UI utilities (Gui)

## Module Overview

The RTXDIPass is a render pass that demonstrates how to use the RTXDI (RTX Direct Illumination) module available in the Falcor rendering framework. The pass provides efficient direct illumination sampling using RTXDI's resampling techniques. It consists of two compute passes: PrepareSurfaceData.slang sets up surface data required by RTXDI to perform light sampling, and FinalShading.slang takes final RTXDI light samples, checks visibility, and shades pixels by evaluating the actual material's BSDF. The pass supports multiple output channels including color, emission, diffuse illumination, diffuse reflectance, specular illumination, and specular reflectance.

## Component Specifications

### RTXDIPass Class

**File**: [`RTXDIPass.h`](Source/RenderPasses/RTXDIPass/RTXDIPass.h:52)

**Purpose**: Standalone pass for direct lighting using RTXDI.

**Public Interface**:

```cpp
class RTXDIPass : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(RTXDIPass, "RTXDIPass", {"Standalone pass for direct lighting using RTXDI."})

    static ref<RTXDIPass> create(ref<Device> pDevice, const Properties& props) { return make_ref<RTXDIPass>(pDevice, props); }

    RTXDIPass(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;

    virtual bool onMouseEvent(const MouseEvent& mouseEvent) override;

private:
    void parseProperties(const Properties& props);
    void recreatePrograms();
    void prepareSurfaceData(RenderContext* pRenderContext, const ref<Texture>& pVBuffer);
    void finalShading(RenderContext* pRenderContext, const ref<Texture>& pVBuffer, const RenderData& renderData);

    ref<Scene> mpScene;

    std::unique_ptr<RTXDI> mpRTXDI;
    RTXDI::Options mOptions;

    ref<ComputePass> mpPrepareSurfaceDataPass;
    ref<ComputePass> mpFinalShadingPass;

    uint2 mFrameDim = {0, 0};
    bool mOptionsChanged = false;
    bool mGBufferAdjustShadingNormals = false;
};
```

**Public Members**: None (all members are private)

**Private Members**:
- `ref<Scene> mpScene` - Scene reference
- `std::unique_ptr<RTXDI> mpRTXDI` - RTXDI instance
- `RTXDI::Options mOptions` - RTXDI options
- `ref<ComputePass> mpPrepareSurfaceDataPass` - Prepare surface data pass
- `ref<ComputePass> mpFinalShadingPass` - Final shading pass
- `uint2 mFrameDim` - Frame dimensions (default: {0, 0})
- `bool mOptionsChanged` - Options changed flag (default: false)
- `bool mGBufferAdjustShadingNormals` - GBuffer adjust shading normals flag (default: false)

**Private Methods**:
- `void parseProperties(const Properties& props)` - Parse properties from configuration
- `void recreatePrograms()` - Recreate programs on scene changes
- `void prepareSurfaceData(RenderContext* pRenderContext, const ref<Texture>& pVBuffer)` - Prepare surface data for RTXDI
- `void finalShading(RenderContext* pRenderContext, const ref<Texture>& pVBuffer, const RenderData& renderData)` - Final shading with RTXDI samples

### Input Channels

**File**: [`RTXDIPass.cpp`](Source/RenderPasses/RTXDIPass/RTXDIPass.cpp:43)

**Purpose**: Define input channels for RTXDI pass.

**Input Channels**:
```cpp
const Falcor::ChannelList kInputChannels = {
    // clang-format off
    { kInputVBuffer,            "gVBuffer",                 "Visibility buffer in packed format"                       },
    { kInputTexGrads,           "gTextureGrads",            "Texture gradients", true /* optional */                   },
    { kInputMotionVectors,      "gMotionVector",            "Motion vector buffer (float format)", true /* optional */ },
    // clang-format on
};
```

**Input Names**:
- `vbuffer` (required): Visibility buffer in packed format
- `texGrads` (optional): Texture gradients
- `mvec` (optional): Motion vector buffer (float format)

### Output Channels

**File**: [`RTXDIPass.cpp`](Source/RenderPasses/RTXDIPass/RTXDIPass.cpp:51)

**Purpose**: Define output channels for RTXDI pass.

**Output Channels**:
```cpp
const Falcor::ChannelList kOutputChannels = {
    // clang-format off
    { "color",                  "gColor",                   "Final color",              true /* optional */, ResourceFormat::RGBA32Float },
    { "emission",               "gEmission",                "Emissive color",           true /* optional */, ResourceFormat::RGBA32Float },
    { "diffuseIllumination",    "gDiffuseIllumination",     "Diffuse illumination",     true /* optional */, ResourceFormat::RGBA32Float },
    { "diffuseReflectance",     "gDiffuseReflectance",      "Diffuse reflectance",      true /* optional */, ResourceFormat::RGBA32Float },
    { "specularIllumination",   "gSpecularIllumination",    "Specular illumination",    true /* optional */, ResourceFormat::RGBA32Float },
    { "specularReflectance",    "gSpecularReflectance",     "Specular reflectance",     true /* optional */, ResourceFormat::RGBA32Float },
    // clang-format on
};
```

**Output Names**:
- `color` (optional): Final color (RGBA32Float)
- `emission` (optional): Emissive color (RGBA32Float)
- `diffuseIllumination` (optional): Diffuse illumination (RGBA32Float)
- `diffuseReflectance` (optional): Diffuse reflectance (RGBA32Float)
- `specularIllumination` (optional): Specular illumination (RGBA32Float)
- `specularReflectance` (optional): Specular reflectance (RGBA32Float)

## Technical Details

### Render Graph Integration

**Input Channels**:
- `vbuffer` (required): Visibility buffer in packed format
- `texGrads` (optional): Texture gradients
- `mvec` (optional): Motion vector buffer (float format)

**Output Channels**:
- `color` (optional): Final color (RGBA32Float)
- `emission` (optional): Emissive color (RGBA32Float)
- `diffuseIllumination` (optional): Diffuse illumination (RGBA32Float)
- `diffuseReflectance` (optional): Diffuse reflectance (RGBA32Float)
- `specularIllumination` (optional): Specular illumination (RGBA32Float)
- `specularReflectance` (optional): Specular reflectance (RGBA32Float)

**Reflection**:
```cpp
RenderPassReflection reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;

    addRenderPassOutputs(reflector, kOutputChannels);
    addRenderPassInputs(reflector, kInputChannels);

    return reflector;
}
```

### Execute

**Implementation**:
```cpp
void RTXDIPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // Clear outputs if no scene is loaded.
    if (!mpScene)
    {
        clearRenderPassChannels(pRenderContext, kOutputChannels, renderData);
        return;
    }

    // Check for scene changes that require shader recompilation.
    if (is_set(mpScene->getUpdates(), IScene::UpdateFlags::RecompileNeeded) ||
        is_set(mpScene->getUpdates(), IScene::UpdateFlags::GeometryChanged))
    {
        recreatePrograms();
    }

    FALCOR_ASSERT(mpRTXDI);

    const auto& pVBuffer = renderData.getTexture(kInputVBuffer);
    const auto& pMotionVectors = renderData.getTexture(kInputMotionVectors);

    auto& dict = renderData.getDictionary();

    // Update refresh flag if changes that affect output have occured.
    if (mOptionsChanged)
    {
        auto flags = dict.getValue(kRenderPassRefreshFlags, Falcor::RenderPassRefreshFlags::None);
        flags |= Falcor::RenderPassRefreshFlags::RenderOptionsChanged;
        dict[Falcor::kRenderPassRefreshFlags] = flags;
        mOptionsChanged = false;
    }

    // Check if GBuffer has adjusted shading normals enabled.
    mGBufferAdjustShadingNormals = dict.getValue(Falcor::kRenderPassGBufferAdjustShadingNormals, false);

    mpRTXDI->beginFrame(pRenderContext, mFrameDim);

    prepareSurfaceData(pRenderContext, pVBuffer);

    mpRTXDI->update(pRenderContext, pMotionVectors);

    finalShading(pRenderContext, pVBuffer, renderData);

    mpRTXDI->endFrame(pRenderContext);
}
```

**Features**:
- Scene change detection for shader recompilation
- Output clearing when no scene is loaded
- Refresh flag management for option changes
- GBuffer adjust shading normals detection
- RTXDI frame management (beginFrame, update, endFrame)
- Two-pass execution (prepareSurfaceData, finalShading)

### Prepare Surface Data

**Implementation**:
```cpp
void RTXDIPass::prepareSurfaceData(RenderContext* pRenderContext, const ref<Texture>& pVBuffer)
{
    FALCOR_ASSERT(mpRTXDI);
    FALCOR_ASSERT(pVBuffer);

    FALCOR_PROFILE(pRenderContext, "prepareSurfaceData");

    if (!mpPrepareSurfaceDataPass)
    {
        ProgramDesc desc;
        desc.addShaderModules(mpScene->getShaderModules());
        desc.addShaderLibrary(kPrepareSurfaceDataFile).csEntry("main");
        desc.addTypeConformances(mpScene->getTypeConformances());

        auto defines = mpScene->getSceneDefines();
        defines.add(mpRTXDI->getDefines());
        defines.add("GBUFFER_ADJUST_SHADING_NORMALS", mGBufferAdjustShadingNormals ? "1" : "0");

        mpPrepareSurfaceDataPass = ComputePass::create(mpDevice, desc, defines, true);
    }

    mpPrepareSurfaceDataPass->addDefine("GBUFFER_ADJUST_SHADING_NORMALS", mGBufferAdjustShadingNormals ? "1" : "0");

    auto rootVar = mpPrepareSurfaceDataPass->getRootVar();
    mpScene->bindShaderData(rootVar["gScene"]);
    mpRTXDI->bindShaderData(rootVar);

    auto var = rootVar["gPrepareSurfaceData"];
    var["vbuffer"] = pVBuffer;
    var["frameDim"] = mFrameDim;

    mpPrepareSurfaceDataPass->execute(pRenderContext, mFrameDim.x, mFrameDim.y);
}
```

**Features**:
- Compute pass for surface data preparation
- Program creation with scene shader modules
- Type conformances from scene
- Defines for GBuffer adjust shading normals
- RTXDI defines integration
- Scene and RTXDI shader data binding
- VBuffer and frame dimension binding
- 16x16 thread group execution

### Final Shading

**Implementation**:
```cpp
void RTXDIPass::finalShading(RenderContext* pRenderContext, const ref<Texture>& pVBuffer, const RenderData& renderData)
{
    FALCOR_ASSERT(mpRTXDI);
    FALCOR_ASSERT(pVBuffer);

    FALCOR_PROFILE(pRenderContext, "finalShading");

    if (!mpFinalShadingPass)
    {
        ProgramDesc desc;
        desc.addShaderModules(mpScene->getShaderModules());
        desc.addShaderLibrary(kFinalShadingFile).csEntry("main");
        desc.addTypeConformances(mpScene->getTypeConformances());

        auto defines = mpScene->getSceneDefines();
        defines.add(mpRTXDI->getDefines());
        defines.add("GBUFFER_ADJUST_SHADING_NORMALS", mGBufferAdjustShadingNormals ? "1" : "0");
        defines.add("USE_ENV_BACKGROUND", mpScene->useEnvBackground() ? "1" : "0");
        defines.add(getValidResourceDefines(kOutputChannels, renderData));

        mpFinalShadingPass = ComputePass::create(mpDevice, desc, defines, true);
    }

    mpFinalShadingPass->addDefine("GBUFFER_ADJUST_SHADING_NORMALS", mGBufferAdjustShadingNormals ? "1" : "0");
    mpFinalShadingPass->addDefine("USE_ENV_BACKGROUND", mpScene->useEnvBackground() ? "1" : "0");

    // For optional I/O resources, set 'is_valid_<name>' defines to inform program of which ones it can access.
    // TODO: This should be moved to a more general mechanism using Slang.
    mpFinalShadingPass->getProgram()->addDefines(getValidResourceDefines(kOutputChannels, renderData));

    auto rootVar = mpFinalShadingPass->getRootVar();
    mpScene->bindShaderData(rootVar["gScene"]);
    mpRTXDI->bindShaderData(rootVar);

    auto var = rootVar["gFinalShading"];
    var["vbuffer"] = pVBuffer;
    var["frameDim"] = mFrameDim;

    // Bind output channels as UAV buffers.
    auto bind = [&](const ChannelDesc& channel)
    {
        ref<Texture> pTex = renderData.getTexture(channel.name);
        rootVar[channel.texname] = pTex;
    };
    for (const auto& channel : kOutputChannels)
        bind(channel);

    mpFinalShadingPass->execute(pRenderContext, mFrameDim.x, mFrameDim.y);
}
```

**Features**:
- Compute pass for final shading
- Program creation with scene shader modules
- Type conformances from scene
- Defines for GBuffer adjust shading normals
- Defines for environment background
- Valid resource defines for optional outputs
- Scene and RTXDI shader data binding
- VBuffer and frame dimension binding
- Output channel binding with lambda function
- 16x16 thread group execution

### Set Scene

**Implementation**:
```cpp
void RTXDIPass::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    mpScene = pScene;
    mpRTXDI = nullptr;

    recreatePrograms();

    if (mpScene)
    {
        if (pScene->hasProceduralGeometry())
        {
            logWarning("RTXDIPass: This render pass only supports triangles. Other types of geometry will be ignored.");
        }

        mpRTXDI = std::make_unique<RTXDI>(mpScene, mOptions);
    }
}
```

**Features**:
- Scene reference management
- RTXDI instance creation with scene and options
- Procedural geometry warning
- Program recreation on scene change

### Mouse Event Handling

**Implementation**:
```cpp
bool RTXDIPass::onMouseEvent(const MouseEvent& mouseEvent)
{
    return mpRTXDI ? mpRTXDI->getPixelDebug().onMouseEvent(mouseEvent) : false;
}
```

**Features**:
- Delegates to RTXDI pixel debug
- Returns false if RTXDI not initialized

### Property Serialization

**Implementation**:
```cpp
Properties RTXDIPass::getProperties() const
{
    Properties props;
    props[kOptions] = mOptions;
    return props;
}
```

**Features**:
- Serializes RTXDI options
- Single property for RTXDI configuration

### Property Parsing

**Implementation**:
```cpp
void RTXDIPass::parseProperties(const Properties& props)
{
    for (const auto& [key, value] : props)
    {
        if (key == kOptions)
            mOptions = value;
        else
            logWarning("Unknown property '{}' in RTXDIPass properties.", key);
    }
}
```

**Features**:
- Parses options property
- Logs warning for unknown properties

### Compile

**Implementation**:
```cpp
void RTXDIPass::compile(RenderContext* pRenderContext, const CompileData& compileData)
{
    mFrameDim = compileData.defaultTexDims;
}
```

**Features**:
- Stores frame dimensions for dispatch
- Called during render graph compilation

### UI Rendering

**Implementation**:
```cpp
void RTXDIPass::renderUI(Gui::Widgets& widget)
{
    // Show user's RTXDI module GUI, and determine if the user changed anything.
    if (mpRTXDI)
    {
        mOptionsChanged = mpRTXDI->renderUI(widget);
        if (mOptionsChanged)
            mOptions = mpRTXDI->getOptions();
    }
}
```

**Features**:
- Delegates to RTXDI UI rendering
- Tracks options changed flag
- Updates options if changed

### Program Recreation

**Implementation**:
```cpp
void RTXDIPass::recreatePrograms()
{
    mpPrepareSurfaceDataPass = nullptr;
    mpFinalShadingPass = nullptr;
}
```

**Features**:
- Cleans up existing programs
- Triggers recreation on next execute

### Prepare Surface Data Shader

**File**: [`PrepareSurfaceData.cs.slang`](Source/RenderPasses/RTXDIPass/PrepareSurfaceData.cs.slang:1)

**Purpose**: Prepares RTXDI surface data from VBuffer.

**Shader Resources**:
- `Texture2D<PackedHitInfo> vbuffer` - Visibility buffer in packed format
- `Texture2D<float2> texGrads` - Texture gradients (optional)

**Output Resources**:
- RTXDI surface data (via `gRTXDI.setSurfaceData()`)

**Constant Buffer**:
- `uint2 frameDim` - Frame dimensions

**Static Configuration**:
- `GBUFFER_ADJUST_SHADING_NORMALS` - GBuffer adjust shading normals define

**Thread Group Size**: 16x16x1

**Main Function** (`main`):
```cpp
[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadId: SV_DispatchThreadID)
{
    gPrepareSurfaceData.execute(dispatchThreadId.xy);
}
```

**Execute Function**:
```cpp
void execute(const uint2 pixel)
{
    if (any(pixel >= frameDim))
        return;

    ShadingData sd;
    bool isValidSurface = loadShadingData(pixel, frameDim, gScene.camera, vbuffer, sd);

    if (isValidSurface)
    {
        // Create material instance and query its properties.
        let lod = ExplicitLodTextureSampler(0.f); // TODO: Implement texture level-of-detail.
        let hints = getMaterialInstanceHints();
        let mi = gScene.materials.getMaterialInstance(sd, lod, hints);
        let bsdfProperties = mi.getProperties(sd);

        // RTXDI uses a simple material model with only diffuse and specular reflection lobes.
        // We query the BSDF for the diffuse albedo and specular reflectance, and use their luminances as weights.
        // Note: Final shading uses the full material model, simplified model is only used for resampling purposes.
        gRTXDI.setSurfaceData(
            pixel,
                sd.computeRayOrigin(),
                bsdfProperties.guideNormal,
                bsdfProperties.diffuseReflectionAlbedo,
                bsdfProperties.specularReflectance,
                bsdfProperties.roughness
        );
    }
    else
    {
        gRTXDI.setInvalidSurfaceData(pixel);
    }
}
```

**Features**:
- Bounds checking for pixel coordinates
- Shading data loading from VBuffer
- Material instance creation with LOD
- BSDF properties query
- RTXDI surface data setting with ray origin, guide normal, diffuse albedo, specular reflectance, and roughness
- Invalid surface data handling
- Simple material model for resampling (diffuse and specular reflection lobes only)

### Final Shading Shader

**File**: [`FinalShading.cs.slang`](Source/RenderPasses/RTXDIPass/FinalShading.cs.slang:1)

**Purpose**: Perform shading with final samples from RTXDI.

**Shader Resources**:
- `Texture2D<PackedHitInfo> vbuffer` - Visibility buffer in packed format

**Output Resources**:
- `RWTexture2D<float4> gColor` - Final color
- `RWTexture2D<float4> gEmission` - Emissive color
- `RWTexture2D<float4> gDiffuseIllumination` - Diffuse illumination
- `RWTexture2D<float4> gDiffuseReflectance` - Diffuse reflectance
- `RWTexture2D<float4> gSpecularIllumination` - Specular illumination
- `RWTexture2D<float4> gSpecularReflectance` - Specular reflectance

**Static Configuration**:
- `USE_ENV_BACKGROUND` - Use environment background define
- `is_valid_<name>` - Defines for optional I/O resources

**Thread Group Size**: 16x16x1

**Main Function** (`main`):
```cpp
[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadId: SV_DispatchThreadID)
{
    gFinalShading.execute(dispatchThreadId.xy);
}
```

**Execute Function**:
```cpp
void execute(const uint2 pixel)
{
    if (any(pixel >= frameDim))
        return;

    float3 color = {};
    float3 emission = {};
    float3 diffuseReflectance = {};
    float3 diffuseIllumination = {};
    float3 specularReflectance = {};
    float3 specularIllumination = {};
    float hitT = 10000.f; // TODO: Remove magic number also used in SpatioTemporalResampling pass.

    ShadingData sd;
    if (loadShadingData(pixel, frameDim, gScene.camera, vbuffer, sd))
    {
        // Create material instance and query its properties.
        let lod = ExplicitLodTextureSampler(0.f); // TODO: Implement texture level-of-detail.
        let hints = getMaterialInstanceHints();
        let mi = gScene.materials.getMaterialInstance(sd, lod, hints);
        let bsdfProperties = mi.getProperties(sd);

        // Get final sample from RTXDI.
        float3 dir;
        float distance;
        float3 Li;
        bool valid = gRTXDI.getFinalSample(pixel, dir, distance, Li);

        // Check light visibility.
        if (valid)
        {
            // Create a DXR 1.1 query object to trace a ray (the <1> means use alpha testing)
            SceneRayQuery<1> rayQuery;
            const Ray ray = Ray(sd.computeRayOrigin(), dir, 0.f, distance);
            if (!rayQuery.traceVisibilityRay(ray))
            {
                valid = false;
            }
        }

        // Evaluate diffuse and specular lobes.
        float3 diffuse = {};
        float3 specular = {};
        if (valid)
        {
            TinyUniformSampleGenerator sg = TinyUniformSampleGenerator(pixel, gRTXDI.frameIndex);

            sd.mtl.setActiveLobes((uint)LobeType::DiffuseReflection);
            diffuse = mi.eval(sd, dir, sg) * Li;

            sd.mtl.setActiveLobes((uint)LobeType::SpecularReflection);
            specular = mi.eval(sd, dir, sg) * Li;

            hitT = distance;
        }

        emission = bsdfProperties.emission;
        color = diffuse + specular + emission;

        // RTXDI uses a simple material model with only diffuse and specular reflection lobes.
        // We query the BSDF for the respective albedos and use them for demodulation below.

        // Demodulate diffuse reflectance (albedo) from diffuse color.
        diffuseReflectance = bsdfProperties.diffuseReflectionAlbedo;
        float3 diffuseFactor = select(diffuseReflectance <= 0.f, 0.f, 1.f / diffuseReflectance);
        diffuseIllumination = diffuse * diffuseFactor;

        // Demodulate preintegrated specular reflectance from specular color.
        // We use an approximate shading model with a single specular lobe.
        float NdotV = saturate(dot(bsdfProperties.guideNormal, sd.V));
        float ggxAlpha = bsdfProperties.roughness * bsdfProperties.roughness;
        specularReflectance = approxSpecularIntegralGGX(bsdfProperties.specularReflectance, ggxAlpha, NdotV);
        float3 specularFactor = select(specularReflectance <= 0.f, 0.f, 1.f / specularReflectance);
        specularIllumination = specular * specularFactor;
    }
    else
    {
        if (kUseEnvBackground)
        {
            float3 dir = gScene.camera.computeRayPinhole(pixel, frameDim).dir;
            emission = gScene.envMap.eval(dir);
            color = emission;
        }
    }

    // Write active outputs.
    if (is_valid(gColor))
        gColor[pixel] = float4(color, 1.f);
    if (is_valid(gEmission))
        gEmission[pixel] = float4(emission, 1.f);
    if (is_valid(gDiffuseReflectance))
        gDiffuseReflectance[pixel] = float4(diffuseReflectance, 1.f);
    if (is_valid(gDiffuseIllumination))
        gDiffuseIllumination[pixel] = float4(diffuseIllumination, hitT);
    if (is_valid(gSpecularReflectance))
        gSpecularReflectance[pixel] = float4(specularReflectance, 1.f);
    if (is_valid(gSpecularIllumination))
        gSpecularIllumination[pixel] = float4(specularIllumination, hitT);
}
```

**Features**:
- Bounds checking for pixel coordinates
- Shading data loading from VBuffer
- Material instance creation with LOD
- BSDF properties query
- RTXDI final sample retrieval (direction, distance, radiance)
- Visibility ray tracing with DXR 1.1 query
- Diffuse and specular lobe evaluation with full material model
- Emission contribution
- Color composition (diffuse + specular + emission)
- Diffuse demodulation (diffuse reflectance / diffuse color)
- Specular demodulation (preintegrated specular reflectance / specular color)
- Approximate specular integral GGX calculation
- Environment background evaluation for invalid surfaces
- Conditional output writing based on availability
- Hit distance storage in alpha channel of illumination outputs

## Architecture Patterns

### Render Pass Pattern

- Inherits from [`RenderPass`](Source/Falcor/RenderGraph/RenderPass.h:1) base class
- Implements standard render pass interface:
  - `reflect()` - Defines input/output resources
  - `execute()` - Executes two-pass rendering (prepareSurfaceData, finalShading)
  - `renderUI()` - Delegates to RTXDI UI
  - `getProperties()` - Serializes RTXDI options
  - `setScene()` - Sets scene and creates RTXDI instance
  - `onMouseEvent()` - Delegates to RTXDI pixel debug

### Compute Pass Pattern

- Uses two compute shaders for rendering:
- PrepareSurfaceData.cs.slang: Surface data preparation
- FinalShading.cs.slang: Final shading with RTXDI samples
- Thread group size: 16x16x1 for both passes

### RTXDI Integration Pattern

- RTXDI instance management (creation, beginFrame, update, endFrame)
- Surface data preparation for RTXDI resampling
- Final sample retrieval from RTXDI
- Visibility ray tracing for RTXDI samples
- Simple material model for resampling (diffuse and specular reflection lobes only)
- Full material model for final shading

### Scene Integration Pattern

- Scene reference management
- Scene change detection for shader recompilation
- Scene shader modules integration
- Type conformances from scene
- Scene defines for shader compilation
- Procedural geometry warning

### Material System Integration Pattern

- Material instance creation with LOD
- BSDF properties query
- Diffuse and specular lobe evaluation
- Active lobe setting (DiffuseReflection, SpecularReflection)
- Material evaluation with sample generator

### Visibility Ray Tracing Pattern

- DXR 1.1 query object for visibility testing
- Alpha testing support (SceneRayQuery<1>)
- Ray creation with origin, direction, tmin, tmax
- Visibility check for RTXDI samples

### Demodulation Pattern

- Diffuse demodulation: diffuseIllumination = diffuse / diffuseReflectance
- Specular demodulation: specularIllumination = specular / preintegratedSpecularReflectance
- Preintegrated specular reflectance using approximate GGX integral
- Prevents division by zero with select function

### Property Pattern

- Property parsing from configuration
- Property serialization for save/load
- Options changed flag management
- Refresh flag propagation

## Code Patterns

### Property Parsing Pattern

```cpp
void RTXDIPass::parseProperties(const Properties& props)
{
    for (const auto& [key, value] : props)
    {
        if (key == kOptions)
            mOptions = value;
        else
            logWarning("Unknown property '{}' in RTXDIPass properties.", key);
    }
}
```

### Property Serialization Pattern

```cpp
Properties RTXDIPass::getProperties() const
{
    Properties props;
    props[kOptions] = mOptions;
    return props;
}
```

### Scene Change Detection Pattern

```cpp
if (is_set(mpScene->getUpdates(), IScene::UpdateFlags::RecompileNeeded) ||
    is_set(mpScene->getUpdates(), IScene::UpdateFlags::GeometryChanged))
{
    recreatePrograms();
}
```

### Refresh Flag Management Pattern

```cpp
if (mOptionsChanged)
{
    auto flags = dict.getValue(kRenderPassRefreshFlags, Falcor::RenderPassRefreshFlags::None);
    flags |= Falcor::RenderPassRefreshFlags::RenderOptionsChanged;
    dict[Falcor::kRenderPassRefreshFlags] = flags;
    mOptionsChanged = false;
}
```

### Program Creation Pattern

```cpp
if (!mpPrepareSurfaceDataPass)
{
    ProgramDesc desc;
    desc.addShaderModules(mpScene->getShaderModules());
    desc.addShaderLibrary(kPrepareSurfaceDataFile).csEntry("main");
    desc.addTypeConformances(mpScene->getTypeConformances());

    auto defines = mpScene->getSceneDefines();
    defines.add(mpRTXDI->getDefines());
    defines.add("GBUFFER_ADJUST_SHADING_NORMALS", mGBufferAdjustShadingNormals ? "1" : "0");

    mpPrepareSurfaceDataPass = ComputePass::create(mpDevice, desc, defines, true);
}
```

### Scene and RTXDI Binding Pattern

```cpp
auto rootVar = mpPrepareSurfaceDataPass->getRootVar();
mpScene->bindShaderData(rootVar["gScene"]);
mpRTXDI->bindShaderData(rootVar);
```

### Output Channel Binding Pattern

```cpp
auto bind = [&](const ChannelDesc& channel)
{
    ref<Texture> pTex = renderData.getTexture(channel.name);
    rootVar[channel.texname] = pTex;
};
for (const auto& channel : kOutputChannels)
    bind(channel);
```

### Material Instance Creation Pattern

```cpp
let lod = ExplicitLodTextureSampler(0.f);
let hints = getMaterialInstanceHints();
let mi = gScene.materials.getMaterialInstance(sd, lod, hints);
let bsdfProperties = mi.getProperties(sd);
```

### RTXDI Surface Data Setting Pattern

```cpp
gRTXDI.setSurfaceData(
    pixel,
    sd.computeRayOrigin(),
    bsdfProperties.guideNormal,
    bsdfProperties.diffuseReflectionAlbedo,
    bsdfProperties.specularReflectance,
    bsdfProperties.roughness
);
```

### RTXDI Final Sample Retrieval Pattern

```cpp
float3 dir;
float distance;
float3 Li;
bool valid = gRTXDI.getFinalSample(pixel, dir, distance, Li);
```

### Visibility Ray Tracing Pattern

```cpp
SceneRayQuery<1> rayQuery;
const Ray ray = Ray(sd.computeRayOrigin(), dir, 0.f, distance);
if (!rayQuery.traceVisibilityRay(ray))
{
    valid = false;
}
```

### Diffuse and Specular Lobe Evaluation Pattern

```cpp
TinyUniformSampleGenerator sg = TinyUniformSampleGenerator(pixel, gRTXDI.frameIndex);

sd.mtl.setActiveLobes((uint)LobeType::DiffuseReflection);
diffuse = mi.eval(sd, dir, sg) * Li;

sd.mtl.setActiveLobes((uint)LobeType::SpecularReflection);
specular = mi.eval(sd, dir, sg) * Li;
```

### Diffuse Demodulation Pattern

```cpp
diffuseReflectance = bsdfProperties.diffuseReflectionAlbedo;
float3 diffuseFactor = select(diffuseReflectance <= 0.f, 0.f, 1.f / diffuseReflectance);
diffuseIllumination = diffuse * diffuseFactor;
```

### Specular Demodulation Pattern

```cpp
float NdotV = saturate(dot(bsdfProperties.guideNormal, sd.V));
float ggxAlpha = bsdfProperties.roughness * bsdfProperties.roughness;
specularReflectance = approxSpecularIntegralGGX(bsdfProperties.specularReflectance, ggxAlpha, NdotV);
float3 specularFactor = select(specularReflectance <= 0.f, 0.f, 1.f / specularReflectance);
specularIllumination = specular * specularFactor;
```

### Conditional Output Writing Pattern

```cpp
if (is_valid(gColor))
    gColor[pixel] = float4(color, 1.f);
if (is_valid(gEmission))
    gEmission[pixel] = float4(emission, 1.f);
if (is_valid(gDiffuseReflectance))
    gDiffuseReflectance[pixel] = float4(diffuseReflectance, 1.f);
if (is_valid(gDiffuseIllumination))
    gDiffuseIllumination[pixel] = float4(diffuseIllumination, hitT);
if (is_valid(gSpecularReflectance))
    gSpecularReflectance[pixel] = float4(specularReflectance, 1.f);
if (is_valid(gSpecularIllumination))
    gSpecularIllumination[pixel] = float4(specularIllumination, hitT);
```

### Plugin Registration Pattern

```cpp
extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, RTXDIPass>();
}
```

## Integration Points

### Render Graph Integration

The pass integrates with render graph system:
- Input channels: 3 (1 required: vbuffer, 2 optional: texGrads, mvec)
- Output channels: 6 (all optional: color, emission, diffuseIllumination, diffuseReflectance, specularIllumination, specularReflectance)
- Reflects input/output resources via `reflect()` method
- Executes two-pass rendering in `execute()` method
- Supports optional inputs/outputs with defines
- No configurable parameters (delegates to RTXDI)

### Scene Integration

The pass integrates with scene system:
- Scene reference management
- RTXDI instance creation with scene and options
- Scene change detection for shader recompilation
- Scene shader modules integration
- Type conformances from scene
- Scene defines for shader compilation
- Procedural geometry warning

### RTXDI Integration

The pass integrates with RTXDI system:
- RTXDI instance management
- Surface data preparation for RTXDI resampling
- RTXDI frame management (beginFrame, update, endFrame)
- Final sample retrieval from RTXDI
- RTXDI defines integration
- RTXDI shader data binding
- RTXDI UI rendering delegation
- RTXDI pixel debug delegation

### Material System Integration

The pass integrates with material system:
- Material instance creation with LOD
- BSDF properties query
- Diffuse and specular lobe evaluation
- Active lobe setting (DiffuseReflection, SpecularReflection)
- Material evaluation with sample generator
- Simple material model for resampling
- Full material model for final shading

### Ray Tracing Integration

The pass integrates with ray tracing system:
- DXR 1.1 query object for visibility testing
- Alpha testing support (SceneRayQuery<1>)
- Ray creation with origin, direction, tmin, tmax
- Visibility check for RTXDI samples

### UI Integration

The pass provides UI:
- Delegates to RTXDI UI rendering
- Tracks options changed flag
- Updates options if changed

### Event Integration

The pass handles events:
- Delegates mouse events to RTXDI pixel debug
- Returns false if RTXDI not initialized

## Use Cases

### Direct Illumination Rendering

Rendering applications:
- Real-time direct illumination
- Efficient light sampling
- Resampling-based direct lighting
- Temporal accumulation support
- Motion vector integration

### RTXDI Integration

Integration applications:
- RTXDI SDK demonstration
- RTXDI resampling techniques
- RTXDI light sampling
- RTXDI pixel debugging
- RTXDI configuration

### Material System Testing

Testing applications:
- Material model validation
- BSDF evaluation testing
- Diffuse and specular lobe testing
- Demodulation techniques
- Material property inspection

### Performance Benchmarking

Benchmarking applications:
- RTXDI performance testing
- Direct illumination performance
- Resampling overhead measurement
- Visibility ray tracing cost
- Material evaluation cost

### Research and Development

Research applications:
- RTXDI algorithm research
- Resampling techniques
- Light sampling methods
- Material model research
- Visibility testing optimization

### Educational Use

Educational applications:
- Demonstrate RTXDI integration
- Show resampling techniques
- Explain light sampling
- Teach material evaluation
- Visualize direct illumination

## Performance Considerations

### GPU Computation

- Prepare surface data pass: 16x16 thread groups, O(width × height / 256) dispatches
- Final shading pass: 16x16 thread groups, O(width × height / 256) dispatches
- RTXDI update: O(width × height) operations
- Visibility ray tracing: O(width × height) rays per frame
- Material evaluation: O(width × height) evaluations per frame
- Total: O(width × height) per frame

### Memory Usage

- VBuffer input: O(width × height × sizeof(PackedHitInfo)) bytes
- Output textures: O(width × height × 16 × 6) bytes (6 RGBA32Float outputs)
- RTXDI buffers: O(width × height × RTXDI_buffer_size) bytes
- Total memory: O(width × height × (sizeof(PackedHitInfo) + 96 + RTXDI_buffer_size)) bytes

### Computational Complexity

- Prepare surface data: O(width × height) per frame
- RTXDI update: O(width × height) per frame
- Final shading: O(width × height) per frame
- Overall: O(width × height) per frame

### RTXDI Overhead

- RTXDI initialization: O(1) on scene change
- RTXDI beginFrame: O(1) per frame
- RTXDI update: O(width × height) per frame
- RTXDI endFrame: O(1) per frame
- RTXDI sample retrieval: O(1) per pixel

### Visibility Ray Tracing Overhead

- Visibility ray tracing: O(width × height) rays per frame
- DXR 1.1 query overhead: O(1) per ray
- Alpha testing overhead: O(1) per ray

### Material Evaluation Overhead

- Material instance creation: O(1) per pixel (cached)
- BSDF properties query: O(1) per pixel
- Diffuse lobe evaluation: O(1) per pixel
- Specular lobe evaluation: O(1) per pixel
- Total: O(1) per pixel

### Program Management Overhead

- Program creation: O(1) on scene change
- Shader compilation: O(1) on scene change
- State management: O(1) per frame
- Minimal overhead after initialization

## Limitations

### Feature Limitations

- Triangle-only geometry support (procedural geometry ignored)
- No texture level-of-detail (TODO)
- Magic number for hitT (10000.f, TODO)
- No custom material models
- No advanced lighting techniques
- No indirect illumination
- No global illumination
- No volumetric effects

### RTXDI Limitations

- Requires RTXDI SDK installation
- Simple material model for resampling (diffuse and specular reflection lobes only)
- Limited to RTXDI-supported features
- RTXDI configuration complexity
- RTXDI performance dependency

### Geometry Limitations

- Only triangle geometry supported
- Procedural geometry ignored
- No curve geometry support
- No SDF geometry support

### Material Limitations

- Simple material model for resampling
- Limited to diffuse and specular reflection lobes
- No transmission lobes
- No subsurface scattering
- No anisotropic materials

### Performance Limitations

- High memory usage for RTXDI buffers
- Visibility ray tracing overhead
- Material evaluation overhead
- RTXDI update overhead
- Limited performance optimization

### Integration Limitations

- Limited to Falcor's scene system
- Requires VBuffer input
- Limited to Falcor's material system
- Limited to RTXDI SDK
- No external data sources

### UI Limitations

- Delegates to RTXDI UI
- No custom UI controls
- Limited configuration options
- No preset management

## Best Practices

### RTXDI Configuration

- Install RTXDI SDK properly
- Configure RTXDI options for scene
- Test with different RTXDI settings
- Balance quality vs. performance
- Use motion vectors for temporal accumulation
- Enable pixel debugging for analysis

### Geometry Configuration

- Use triangle geometry only
- Avoid procedural geometry
- Ensure proper VBuffer generation
- Use appropriate shading normals
- Test with different geometry types

### Material Configuration

- Use appropriate material models
- Configure material properties correctly
- Test with different materials
- Verify BSDF evaluation
- Check demodulation results

### Performance Optimization

- Minimize RTXDI buffer size
- Optimize visibility ray tracing
- Reduce material evaluation cost
- Use appropriate thread group size
- Profile GPU performance
- Test with different resolutions

### Debugging

- Use RTXDI pixel debugging
- Verify surface data preparation
- Check visibility ray tracing
- Validate material evaluation
- Test demodulation
- Profile performance

### Research and Development

- Study RTXDI algorithms
- Analyze resampling techniques
- Research light sampling methods
- Investigate material models
- Develop new techniques

### Educational Use

- Demonstrate RTXDI integration
- Show resampling techniques
- Explain light sampling
- Teach material evaluation
- Visualize direct illumination

### Integration Best Practices

- Connect to appropriate render graph inputs
- Use correct texture formats
- Handle optional inputs/outputs
- Test with different render graphs
- Validate compatibility
- Ensure proper synchronization

## Related Components

### RenderGraph/RenderPass

Render pass base class:
- `RenderPass::reflect()` - Define input/output resources
- `RenderPass::execute()` - Execute render pass
- `RenderPass::renderUI()` - Render UI controls
- `RenderPass::getProperties()` - Serialize configuration
- `RenderPass::setScene()` - Set scene reference
- `RenderPass::onMouseEvent()` - Handle mouse events
- `RenderPass::compile()` - Compile pass

### RenderGraph/RenderPassHelpers

Render pass helper utilities:
- `addRenderPassInputs()` - Add input channels
- `addRenderPassOutputs()` - Add output channels
- `getValidResourceDefines()` - Get valid resource defines
- `clearRenderPassChannels()` - Clear render pass channels

### Core/Program/ComputePass

Compute pass management:
- `ComputePass::create()` - Create compute pass
- `ComputePass::getRootVar()` - Get root variable
- `ComputePass::execute()` - Execute compute pass
- `ComputePass::addDefine()` - Add shader define

### Rendering/RTXDI/RTXDI

RTXDI integration:
- `RTXDI::RTXDI()` - RTXDI constructor
- `RTXDI::beginFrame()` - Begin RTXDI frame
- `RTXDI::update()` - Update RTXDI
- `RTXDI::endFrame()` - End RTXDI frame
- `RTXDI::getFinalSample()` - Get final sample
- `RTXDI::setSurfaceData()` - Set surface data
- `RTXDI::setInvalidSurfaceData()` - Set invalid surface data
- `RTXDI::getDefines()` - Get RTXDI defines
- `RTXDI::bindShaderData()` - Bind shader data
- `RTXDI::renderUI()` - Render UI
- `RTXDI::getOptions()` - Get options
- `RTXDI::getPixelDebug()` - Get pixel debug

### Scene/Scene

Scene system:
- `Scene::bindShaderData()` - Bind shader data
- `Scene::getShaderModules()` - Get shader modules
- `Scene::getTypeConformances()` - Get type conformances
- `Scene::getSceneDefines()` - Get scene defines
- `Scene::getUpdates()` - Get scene updates
- `Scene::useEnvBackground()` - Use environment background
- `Scene::hasProceduralGeometry()` - Has procedural geometry

### Utils/UI/Gui

UI framework for rendering controls:
- Provides checkbox, var, group, text controls
- Simple UI widget interface
- Real-time control updates

### Utils/Sampling/TinyUniformSampleGenerator

Sample generator for material evaluation:
- Provides uniform sampling
- Per-pixel state management
- Frame index integration
