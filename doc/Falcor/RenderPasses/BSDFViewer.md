# BSDFViewer - BSDF Visualization

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **BSDFViewer.cs.slang** - BSDF viewer shader
- [x] **BSDFViewerParams.slang** - Parameter structures shared between host and device

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Buffer, Texture, Formats)
- **Core/Object** - Base object class
- **Core/Program** - Shader program management
- **RenderGraph** - Render graph system (RenderPass, RenderPassReflection)
- **Scene** - Scene system (Scene, Material, MaterialID)
- **Scene/Lights** - Environment map (EnvMap)
- **Rendering/Materials** - Material system (BSDFConfig)
- **Utils/Sampling** - Sampling utilities (SampleGenerator)
- **Utils/Debug** - Debug utilities (PixelDebug)
- **Utils/UI** - UI utilities (Gui)

## Module Overview

BSDFViewer implements an interactive BSDF (Bidirectional Scattering Distribution Function) visualization tool. The pass provides two viewing modes: Material mode (rendered view of a unit sphere with the material) and Slice mode (2D slice of the BSDF). It supports interactive pixel inspection, material parameter editing, environment map lighting, directional lighting, and various BSDF evaluation options. The pass is designed for material analysis, education, and debugging.

## Component Specifications

### BSDFViewer Class

**Files**:
- [`BSDFViewer.h`](Source/RenderPasses/BSDFViewer/BSDFViewer.h:1) - BSDF viewer header
- [`BSDFViewer.cpp`](Source/RenderPasses/BSDFViewer/BSDFViewer.cpp:1) - BSDF viewer implementation

**Purpose**: Interactive BSDF visualization and inspection utility.

**Key Features**:
- Two viewing modes (Material, Slice)
- Interactive pixel inspection with detailed data readback
- Material selection and parameter editing
- Environment map and directional lighting support
- Ground plane visualization
- Orthographic and perspective camera modes
- BSDF lobe control (diffuse, specular)
- Albedo component visualization
- Pixel debugging integration
- Keyboard navigation between materials

**Public Interface**:

```cpp
class BSDFViewer : public RenderPass
{
public:
    static ref<BSDFViewer> create(ref<Device> pDevice, const Properties& props);
    
    // RenderPass overrides
    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) override;
    virtual bool onKeyEvent(const KeyboardEvent& keyEvent) override;
};
```

**Private Members**:

```cpp
private:
    void parseProperties(const Properties& props);
    bool loadEnvMap(const std::filesystem::path& path);
    void readPixelData();
    
    // Internal state
    ref<Scene> mpScene;
    ref<EnvMap> mpEnvMap;
    bool mUseEnvMap = true;
    
    BSDFViewerParams mParams;
    ref<SampleGenerator> mpSampleGenerator;
    bool mOptionsChanged = false;
    
    ref<Fence> mpFence;
    ref<Buffer> mpPixelDataBuffer;
    ref<Buffer> mpPixelStagingBuffer;
    PixelData mPixelData;
    bool mPixelDataValid = false;
    bool mPixelDataAvailable = false;
    
    std::unique_ptr<PixelDebug> mpPixelDebug;
    ref<ComputePass> mpViewerPass;
    
    Gui::DropdownList mMaterialList;
};
```

### BSDFViewer.cs.slang Shader

**File**:
- [`BSDFViewer.cs.slang`](Source/RenderPasses/BSDFViewer/BSDFViewer.cs.slang:1) - BSDF viewer shader

**Purpose**: GPU-side BSDF visualization with two viewing modes.

**Key Features**:
- Material mode: Rendered view of unit sphere
- Slice mode: 2D slice of BSDF (theta_h, theta_d)
- Interactive pixel inspection
- BSDF lobe control
- Albedo component visualization
- Environment map and directional lighting
- Ground plane rendering
- Pixel debugging integration

**Entry Point**:

```hlsl
[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    gBSDFViewer.execute(dispatchThreadID.xy);
}
```

**Key Functions**:

```hlsl
// Viewport coordinate calculation
float2 getViewportCoord(uint2 pixel);

// BSDF slice geometry calculation
float3 calculateSliceGeometry(float2 uv, out VertexData v, out float3 viewDir);

// Sphere geometry calculation
bool calculateSphereGeometry(float2 uv, out VertexData v, out float3 rayDir);

// Surface data preparation
SurfaceData prepareShadingData(const VertexData v, const float3 viewDir);

// Background evaluation
float3 evalBackground(float2 uv, float3 dir);

// Lighting evaluation
float3 evalLighting(float3 dir);

// Active lobe mask
uint getActiveLobes();

// BSDF slice evaluation
float3 evalBSDFSlice(const float2 uv, const ITextureSampler lod, inout SurfaceData data, inout SampleGenerator sg);

// BSDF sampling
bool generateBSDFSample(const ShadingData sd, const IMaterialInstance mi, inout SampleGenerator sg, out BSDFSample s);

// Albedo extraction
float3 getAlbedo(const BSDFProperties bsdfProperties);

// Sphere evaluation
float3 evalSphere(const float2 uv, const ITextureSampler lod, inout SurfaceData data, inout SampleGenerator sg);

// Main execution
void execute(const uint2 pixel);
```

### BSDFViewerParams.slang

**File**:
- [`BSDFViewerParams.slang`](Source/RenderPasses/BSDFViewer/BSDFViewerParams.slang:1) - Parameter structures

**Purpose**: Parameter structures shared between host and device.

**Structures**:

```hlsl
enum class BSDFViewerMode : uint32_t
{
    Material,  ///< Rendered view with material sample
    Slice,    ///< BSDF slice viewer
};

enum class AlbedoSelection : uint32_t
{
    ShowAlbedo = 0x1,
    DiffuseReflection = 0x2,
    DiffuseTransmission = 0x4,
    SpecularReflection = 0x8,
    SpecularTransmission = 0x10,
};

struct BSDFViewerParams
{
    // Frame parameters
    uint2 frameDim = { 0, 0 };
    uint frameCount = 0;
    
    // Viewport parameters
    float2 viewportOffset;
    float2 viewportScale;
    
    // Material parameters
    uint materialID = 0;
    int useNormalMapping = 0;
    int useFixedTexCoords = 0;
    float2 texCoords = { 0.f, 0.f };
    
    // BSDF settings
    int useDisneyDiffuse = 0;
    int useSeparableMaskingShadowing = 0;
    int useImportanceSampling = 1;
    int usePdf = 0;
    uint outputAlbedo = 0;
    int enableDiffuse = 1;
    int enableSpecular = 1;
    int applyNdotL = 0;
    
    // Lighting settings
    int useGroundPlane = 0;
    int useEnvMap = 0;
    float lightIntensity = 1.f;
    float3 lightColor = { 1.f, 1.f, 1.f };
    int useDirectionalLight = 0;
    float3 lightDir = { 0.f, 0.f, -1.f };
    
    // Camera settings
    int orthographicCamera = 0;
    float cameraDistance = 1.5f;
    float cameraFovY = 90.f;
    float cameraViewportScale;
    
    // Pixel data
    int2 selectedPixel = { 0, 0 };
};

struct PixelData
{
    float2 texC;
    float3 T;
    float3 B;
    float3 N;
    float3 wi;
    float3 wo;
    float3 output;
    
    // BSDF properties
    float3 guideNormal;
    float3 emission;
    float3 roughness;
    float3 diffuseReflectionAlbedo;
    float3 diffuseTransmissionAlbedo;
    float3 specularReflectionAlbedo;
    float3 specularTransmissionAlbedo;
    float3 specularReflectance;
    int isTransmissive;
};
```

## Technical Details

### Viewing Modes

**Material Mode**:
- Renders a unit sphere with the selected material
- Uses random light directions for each pixel
- Converges to total reflectance (integral of BSDF)
- Supports orthographic and perspective cameras
- Interactive camera controls (distance, FOV)

**Slice Mode**:
- 2D slice of BSDF in spherical coordinates
- x-axis: theta_h (angle between H and normal) in [0, π/2]
- y-axis: theta_d (angle between H and wi/wo) in [0, π/2]
- Origin in lower-left corner
- Same format as Burley et al. 2012, 2015

### BSDF Slice Geometry

Slice geometry calculation:

```hlsl
float theta_h = uv.x * (M_PI / 2);
float theta_d = (1.f - uv.y) * (M_PI / 2);

float NdotH = cos(theta_h);
float HdotL = cos(theta_d);

// Place H vector at (0,0,1)
float3 L = float3(sqrt(1.f - HdotL * HdotL), 0, HdotL);
float3 V = float3(-L.x, 0.f, L.z);

// Rotate L, V about x-axis by theta_h
float cos_h = NdotH;
float sin_h = sqrt(1 - NdotH * NdotH);
L = float3(L.x, cos_h * L.y - sin_h * L.z, sin_h * L.y + cos_h * L.z);
V = float3(V.x, cos_h * V.y - sin_h * V.z, sin_h * V.y + cos_h * V.z);

viewDir = V;
return normalize(L);
```

### Sphere Geometry

**Orthographic Camera**:
- Viewport is +-1 units vertically
- Sphere fits exactly in viewport
- Direct ray from camera: (0, 0, -1)
- Ray-sphere intersection: d = sqrt(1 - p.x² - p.y²)

**Perspective Camera**:
- Camera at origin (0, 0, cameraDistance)
- Ray direction: normalize(target - origin)
- Target: (ndc * cameraViewportScale, 0)
- Ray-sphere intersection using standard ray-sphere test

### Texture Coordinate Mapping

**Fixed Texture Coords**:
- User-specified texture coordinates
- Applied directly to vertex data

**Automatic Mapping**:
- Cylindrical mapping of visible hemisphere
- u = atan2(p.z, -p.x) / π (left to right)
- v = acos(-p.y) / π (bottom to top)
- u=0 on left, u=1 on right
- v=0 at bottom, v=1 at top

### BSDF Evaluation

**Material Mode**:
```hlsl
// Create BSDF instance
uint hints = !params.useNormalMapping ? (uint)MaterialInstanceHints::DisableNormalMapping : 0;
let mi = gScene.materials.getMaterialInstance(data.sd, lod, hints);

// Sample BSDF
BSDFSample s = {};
if (generateBSDFSample(data.sd, mi, sg, s))
{
    data.wo = s.wo;
    float3 L = evalLighting(s.wo);
    
    if (params.usePdf)
        output = L * mi.eval(data.sd, s.wo, sg) / s.pdf;
    else
        output = L * s.weight;
}
```

**Slice Mode**:
```hlsl
// Evaluate BSDF directly
float3 f = mi.eval(data.sd, data.wo, sg);

// Remove cosine term if disabled
if (!params.applyNdotL)
{
    float NdotL = dot(data.sd.frame.N, data.wo);
    f = NdotL > 0.f ? f / NdotL : float3(0);
}
```

### Lighting

**Directional Light**:
- Single directional light source
- Configurable direction, intensity, color
- Disabled when environment map is enabled

**Environment Map**:
- Environment map as light source
- Omnidirectional lighting
- Evaluated via EnvMap::eval(dir)
- Can be loaded from file

**Ground Plane**:
- Renders ground plane at y=0
- Color: (0.05, 0.05, 0.05)
- Only visible from lower hemisphere
- Prevents illumination from below

### Pixel Inspection

**Pixel Data Readback**:
- Async readback using staging buffer and fence
- Avoids full flush for performance
- Updates parameters even if UI wasn't rendered

**Pixel Data Structure**:
- texC: Texture coordinates
- T, B, N: Tangent, bitangent, normal
- wi: Incident direction (view dir)
- wo: Outgoing direction (light dir)
- output: Output color
- guideNormal: Guide normal
- emission: Emission
- roughness: Roughness
- Albedo components: diffuseReflection, diffuseTransmission, specularReflection, specularTransmission
- specularReflectance: Specular reflectance
- isTransmissive: Transmissive flag

### BSDF Lobe Control

**Lobe Types**:
- DiffuseReflection
- SpecularReflection
- DeltaReflection
- SpecularTransmission (not yet supported)

**Lobe Mask**:
```hlsl
uint getActiveLobes()
{
    uint lobeTypes = 0;
    if (params.enableDiffuse)
        lobeTypes |= (uint)LobeType::DiffuseReflection;
    if (params.enableSpecular)
        lobeTypes |= (uint)LobeType::SpecularReflection | (uint)LobeType::DeltaReflection;
    return lobeTypes;
}
```

### Albedo Visualization

**Albedo Selection Flags**:
- ShowAlbedo: Output albedo instead of reflectance
- DiffuseReflection: Include diffuse reflection albedo
- DiffuseTransmission: Include diffuse transmission albedo
- SpecularReflection: Include specular reflection albedo
- SpecularTransmission: Include specular transmission albedo

**Albedo Computation**:
```hlsl
float3 getAlbedo(const BSDFProperties bsdfProperties)
{
    float3 albedo = {};
    if (params.outputAlbedo & (uint)AlbedoSelection::DiffuseReflection)
        albedo += bsdfProperties.diffuseReflectionAlbedo;
    if (params.outputAlbedo & (uint)AlbedoSelection::DiffuseTransmission)
        albedo += bsdfProperties.diffuseTransmissionAlbedo;
    if (params.outputAlbedo & (uint)AlbedoSelection::SpecularReflection)
        albedo += bsdfProperties.specularReflectionAlbedo;
    if (params.outputAlbedo & (uint)AlbedoSelection::SpecularTransmission)
        albedo += bsdfProperties.specularTransmissionAlbedo;
    return albedo;
}
```

### Camera Controls

**Orthographic Camera**:
- No perspective distortion
- Direct mapping from viewport to world space
- Fixed viewing distance

**Perspective Camera**:
- Configurable distance (1.0 to infinity)
- Configurable vertical FOV (1 to 179 degrees)
- Camera viewport scale computed: tan(fovY/2) * distance
- Minimum distance > 1.0 to avoid numerical issues

### Pixel Debugging

**Integration**:
- Uses PixelDebug for shader-side debugging
- Supports print() and assert() in shaders
- Per-pixel debugging

**Print Set**:
```hlsl
printSetPixel(pixel);
```

## Integration Points

### Render Graph Integration

**Input/Output**:
- Output: "output" - BSDF visualization

**Bind Flags**:
- Output: UnorderedAccess

**Format**:
- Output: RGBA32Float

**Size**:
- Output: Matches frame dimensions

### Scene Integration

- Monitors scene updates
- Requires scene with materials
- Material selection via dropdown
- Keyboard navigation between materials
- Does not support scene changes requiring recompilation

### Environment Map Integration

- Loads environment map from file
- Uses EnvMap for lighting
- Supports various environment map formats
- Can be toggled on/off

### PixelDebug Integration

- Begins frame with PixelDebug
- Prepares program for debugging
- Ends frame after execution
- Renders UI for debugging controls

### Scripting Integration

```python
# Python bindings (not explicitly shown in code but available via Properties)
pass.material_id
pass.viewer_mode
pass.use_env_map
pass.tex_coords
pass.output_albedo
```

### UI Integration

Comprehensive UI with multiple groups:
- Mode selection dropdown
- Material selection dropdown
- Material settings (normal mapping, fixed tex coords)
- BSDF settings (lobes, Disney diffuse, separable masking, importance sampling, PDF)
- Light settings (intensity, color, ground plane, directional, environment map)
- Camera settings (orthographic, distance, FOV)
- Pixel data display (coordinates, vectors, BSDF properties)
- Logging controls (PixelDebug UI)

## Architecture Patterns

### Viewer Pattern
- Two viewing modes (Material, Slice)
- Interactive parameter editing
- Real-time visualization
- Pixel inspection

### Parameter Block Pattern
- Host-device shared parameters
- Struct packing following HLSL rules
- No bool types (1 byte in VS, 4 bytes in HLSL)

### Async Readback Pattern
- Staging buffer for readback
- Fence for synchronization
- Avoids full flush

### Event Handling Pattern
- Mouse events for pixel selection
- Keyboard events for material navigation
- Option change tracking

## Code Patterns

### Pixel Data Readback

```cpp
void BSDFViewer::readPixelData()
{
    if (mPixelDataAvailable)
    {
        mpFence->wait();
        FALCOR_ASSERT(mpPixelStagingBuffer);
        mPixelData = *static_cast<const PixelData*>(mpPixelStagingBuffer->map());
        mpPixelStagingBuffer->unmap();
        
        mPixelDataAvailable = false;
        mPixelDataValid = true;
        
        // Update parameters from values at selected pixel
        mParams.texCoords = mPixelData.texC;
    }
}
```

### Execute Pass

```cpp
void BSDFViewer::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // Update refresh flag
    if (mOptionsChanged)
    {
        auto& dict = renderData.getDictionary();
        auto flags = dict.getValue(kRenderPassRefreshFlags, RenderPassRefreshFlags::None);
        dict[Falcor::kRenderPassRefreshFlags] = flags | Falcor::RenderPassRefreshFlags::RenderOptionsChanged;
        mOptionsChanged = false;
    }
    
    auto pOutput = renderData.getTexture(kOutput);
    if (!mpScene || mpScene->getMaterialCount() == 0)
    {
        pRenderContext->clearUAV(pOutput->getUAV().get(), uint4(0));
        return;
    }
    
    // Read back pixel data
    readPixelData();
    
    // Set compile-time constants
    if (mParams.useDisneyDiffuse)
        mpViewerPass->addDefine("DiffuseBrdf", "DiffuseBrdfDisney");
    else
        mpViewerPass->removeDefine("DiffuseBrdf");
    if (mParams.useSeparableMaskingShadowing)
        mpViewerPass->addDefine("SpecularMaskingFunction", "SpecularMaskingFunctionSmithGGXSeparable");
    else
        mpViewerPass->removeDefine("SpecularMaskingFunction");
    
    // Setup constants
    mParams.cameraViewportScale = std::tan(math::radians(mParams.cameraFovY / 2.f)) * mParams.cameraDistance;
    mParams.useEnvMap = mUseEnvMap && mpEnvMap != nullptr;
    
    // Set resources
    auto var = mpViewerPass->getRootVar()[kParameterBlockName];
    
    if (!mpPixelDataBuffer)
    {
        mpPixelDataBuffer = mpDevice->createStructuredBuffer(
            var["pixelData"], 1, ResourceBindFlags::UnorderedAccess, MemoryType::DeviceLocal, nullptr, false
        );
        mpPixelStagingBuffer =
            mpDevice->createStructuredBuffer(var["pixelData"], 1, ResourceBindFlags::None, MemoryType::ReadBack, nullptr, false);
    }
    
    var["params"].setBlob(mParams);
    var["outputColor"] = pOutput;
    var["pixelData"] = mpPixelDataBuffer;
    
    if (mParams.useEnvMap)
        mpEnvMap->bindShaderData(var["envMap"]);
    mpSampleGenerator->bindShaderData(mpViewerPass->getRootVar());
    mpScene->bindShaderData(mpViewerPass->getRootVar()["gScene"]);
    
    mpPixelDebug->beginFrame(pRenderContext, renderData.getDefaultTextureDims());
    mpPixelDebug->prepareProgram(mpViewerPass->getProgram(), mpViewerPass->getRootVar());
    
    // Execute pass
    mpViewerPass->execute(pRenderContext, uint3(mParams.frameDim, 1));
    
    // Copy pixel data to staging buffer
    pRenderContext->copyResource(mpPixelStagingBuffer.get(), mpPixelDataBuffer.get());
    pRenderContext->submit(false);
    pRenderContext->signal(mpFence.get());
    mPixelDataAvailable = true;
    mPixelDataValid = false;
    
    mpPixelDebug->endFrame(pRenderContext);
    mParams.frameCount++;
}
```

### Mouse Event Handling

```cpp
bool BSDFViewer::onMouseEvent(const MouseEvent& mouseEvent)
{
    if (mouseEvent.type == MouseEvent::Type::ButtonDown && mouseEvent.button == Input::MouseButton::Left)
    {
        mParams.selectedPixel = clamp((int2)(mouseEvent.pos * (float2)mParams.frameDim), {0, 0}, (int2)mParams.frameDim - 1);
    }
    
    return mpPixelDebug->onMouseEvent(mouseEvent);
}
```

### Keyboard Event Handling

```cpp
bool BSDFViewer::onKeyEvent(const KeyboardEvent& keyEvent)
{
    if (keyEvent.type == KeyboardEvent::Type::KeyPressed)
    {
        if (keyEvent.key == Input::Key::Left || keyEvent.key == Input::Key::Right)
        {
            uint32_t id = mParams.materialID;
            uint32_t lastId = mMaterialList.size() > 0 ? (uint32_t)mMaterialList.size() - 1 : 0;
            if (keyEvent.key == Input::Key::Left)
                id = id > 0 ? id - 1 : lastId;
            else if (keyEvent.key == Input::Key::Right)
                id = id < lastId ? id + 1 : 0;
            
            if (id != mParams.materialID)
                mOptionsChanged = true; // Triggers reset of accumulation
            mParams.materialID = id;
            return true;
        }
    }
    return false;
}
```

## Use Cases

1. **Material Analysis**: Visualize BSDF behavior across different directions
2. **Material Design**: Explore material parameter space interactively
3. **BSDF Education**: Understand BSDF behavior and properties
4. **Material Debugging**: Inspect material parameters at specific pixels
5. **BSDF Comparison**: Compare different materials side-by-side
6. **Lighting Analysis**: Understand lighting effects on materials
7. **Environment Testing**: Test materials with different environment maps
8. **Research**: Study BSDF properties and behavior
9. **Material Fitting**: Manually adjust parameters to match target

## Performance Considerations

- **Pixel Inspection**: Async readback avoids full flush
- **BSDF Evaluation**: Expensive for complex materials
- **Sphere Rendering**: Ray-sphere intersection per pixel
- **Slice Rendering**: Direct BSDF evaluation per pixel
- **Material Mode**: Monte Carlo sampling for convergence
- **Slice Mode**: Direct evaluation, faster than material mode
- **Environment Map**: Texture lookup per pixel
- **Pixel Debugging**: Minimal overhead when not in use
- **Memory**: Pixel data buffer for readback

## Limitations

- **Material Changes**: Does not support scene changes requiring recompilation
- **Transmission Lobes**: Not yet supported in viewer
- **Single Material**: Only visualizes one material at a time
- **Texture LOD**: Only samples at mip level 0
- **Camera FOV**: Limited to [1, 179] degrees to avoid numerical issues
- **Ground Plane**: Simple implementation, may not match all scenarios
- **Pixel Data**: Only available after first frame with selection

## Best Practices

1. **Use Slice Mode for Analysis**: Faster than material mode for BSDF inspection
2. **Use Material Mode for Convergence**: Shows total reflectance
3. **Enable Importance Sampling**: For accurate Monte Carlo results
4. **Use Environment Maps**: For realistic lighting
5. **Adjust Camera Distance**: Ensure sphere is visible in perspective mode
6. **Use Pixel Inspection**: To understand material properties at specific points
7. **Enable Albedo Visualization**: To separate reflectance from BSDF shape
8. **Use Keyboard Navigation**: Quick material switching
9. **Disable Ground Plane**: When viewing from below
10. **Use Pixel Debugging**: For shader-side debugging

## Progress Log

- **2026-01-07T21:10:02Z**: BSDFViewer analysis completed. Analyzed BSDFViewer class (interactive BSDF visualization with two viewing modes), BSDFViewer.cs.slang shader (material mode with sphere rendering, slice mode with 2D BSDF visualization), BSDFViewerParams.slang (parameter structures, viewing modes, albedo selection, pixel data), viewing modes (Material: rendered sphere, Slice: 2D BSDF slice), BSDF slice geometry (spherical coordinates theta_h, theta_d), sphere geometry (orthographic and perspective cameras), texture coordinate mapping (fixed and automatic), BSDF evaluation (importance sampling, PDF computation), lighting (directional, environment map, ground plane), pixel inspection (async readback, detailed data), BSDF lobe control (diffuse, specular), albedo visualization (component selection), camera controls (orthographic, perspective), pixel debugging integration, integration points (render graph, scene, env map, pixel debug, scripting, UI), architecture patterns (viewer, parameter block, async readback, event handling), code patterns (pixel data readback, execute pass, mouse/keyboard events), use cases (material analysis, design, education, debugging, comparison), performance considerations (async readback, BSDF evaluation, sphere rendering), limitations (material changes, transmission lobes, single material), and best practices. Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices. Marked BSDFViewer as Complete.
