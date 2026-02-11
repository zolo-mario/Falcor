# SceneDebugger - Scene Debugger Pass

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **SceneDebugger** - Scene debugger pass
  - [x] **SceneDebugger.h** - Scene debugger header
  - [x] **SceneDebugger.cpp** - Scene debugger implementation
  - [x] **SceneDebugger.cs.slang** - Scene debugger shader
  - [x] **SharedTypes.slang** - Shared type definitions

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Texture, Buffer, Fence)
- **Core/Object** - Base object class
- **Core/Program** - Shader program management (Program, ProgramVars, ComputePass)
- **RenderGraph** - Render graph system (RenderPass, RenderPassReflection, RenderPassHelpers)
- **Scene** - Scene system (Scene, Camera, Lights, Materials, Mesh, Curve, SDFGrid)
- **Utils/Debug** - Debug utilities (PixelDebug)
- **Utils/Sampling** - Sampling utilities (SampleGenerator)
- **Utils/Math** - Mathematical utilities (HashUtils, ShadingFrame, IntersectionHelpers)
- **Utils/Color** - Color utilities (ColorMap, ColorHelpers)

## Module Overview

The SceneDebugger is a comprehensive debugging and analysis tool for identifying asset issues such as incorrect normals, geometry problems, and material issues. The pass provides multiple visualization modes for different aspects of the scene including geometry data, shading data, material properties, and hit types. It supports pixel selection with mouse interaction, detailed pixel data readback, and extensive UI for displaying mesh, instance, material, and shading information. The pass integrates with PixelDebug utility for shader-based data extraction and supports multiple geometry types including triangles, displaced triangles, curves, and SDF grids.

## Component Specifications

### SceneDebugger Class

**File**: [`SceneDebugger.h`](Source/RenderPasses/SceneDebugger/SceneDebugger.h:43)

**Purpose**: Scene debugger for identifying asset issues.

**Public Interface**:

```cpp
class SceneDebugger : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(SceneDebugger, "SceneDebugger", "Scene debugger for identifying asset issues.");

    static ref<SceneDebugger> create(ref<Device> pDevice, const Properties& props) { return make_ref<SceneDebugger>(pDevice, props); }

    SceneDebugger(ref<Device> pDevice, const Properties& props);

    Properties getProperties() const override;
    RenderPassReflection reflect(const CompileData& compileData) override;
    void compile(RenderContext* pRenderContext, const CompileData& compileData) override;
    void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    void renderUI(Gui::Widgets& widget) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override { return false; }

    // Scripting functions
    SceneDebuggerMode getMode() const { return (SceneDebuggerMode)mParams.mode; }
    void setMode(SceneDebuggerMode mode) { mParams.mode = (uint32_t)mode; }

private:
    void renderPixelDataUI(Gui::Widgets& widget);
    void initInstanceInfo();

    // Internal state

    std::unique_ptr<PixelDebug> mpPixelDebug; ///< Utility class for pixel debugging (print in shaders).
    ref<SampleGenerator> mpSampleGenerator;
    ref<Scene> mpScene;
    sigs::Connection mUpdateFlagsConnection; ///< Connection to UpdateFlags signal.
    /// IScene::UpdateFlags accumulated since last `beginFrame()`
    IScene::UpdateFlags mUpdateFlags = IScene::UpdateFlags::None;

    SceneDebuggerParams mParams;
    ref<ComputePass> mpDebugPass;
    ref<Fence> mpFence;
    /// Buffer for recording pixel data at the selected pixel.
    ref<Buffer> mpPixelData;
    /// Readback buffer.
    ref<Buffer> mpPixelDataStaging;
    ref<Buffer> mpMeshToBlasID;
    ref<Buffer> mpInstanceInfo;
    bool mPixelDataAvailable = false;
    bool mVBufferAvailable = false;
};
```

**Public Members**: None (all members are private)

**Private Members**:
- `std::unique_ptr<PixelDebug> mpPixelDebug` - Pixel debug utility for shader printing
- `ref<SampleGenerator> mpSampleGenerator` - Sample generator
- `ref<Scene> mpScene` - Scene reference
- `sigs::Connection mUpdateFlagsConnection` - Connection to UpdateFlags signal
- `IScene::UpdateFlags mUpdateFlags` - Accumulated update flags
- `SceneDebuggerParams mParams` - Scene debugger parameters
- `ref<ComputePass> mpDebugPass` - Debug compute pass
- `ref<Fence> mpFence` - Fence for synchronization
- `ref<Buffer> mpPixelData` - Pixel data buffer (structured buffer)
- `ref<Buffer> mpPixelDataStaging` - Pixel data staging buffer (readback)
- `ref<Buffer> mpMeshToBlasID` - Mesh to BLAS ID lookup table
- `ref<Buffer> mpInstanceInfo` - Instance info buffer
- `bool mPixelDataAvailable` - Pixel data available flag
- `bool mVBufferAvailable` - VBuffer available flag

**Private Methods**:
- `void renderPixelDataUI(Gui::Widgets& widget)` - Render pixel data UI
- `void initInstanceInfo()` - Initialize instance metadata

**Public Methods**:
- `SceneDebuggerMode getMode() const` - Get current visualization mode
- `void setMode(SceneDebuggerMode mode)` - Set visualization mode

### Input Channels

**File**: [`SceneDebugger.cpp`](Source/RenderPasses/SceneDebugger/SceneDebugger.cpp:148)

**Purpose**: Define input channels for scene debugger.

**Input Channels**:
```cpp
reflector.addInput("vbuffer", "Visibility buffer in packed format")
    .texture2D()
    .format(ResourceFormat::RGBA32Uint)
    .flags(RenderPassReflection::Field::Flags::Optional);
```

**Input Names**:
- `vbuffer` (optional): Visibility buffer in packed format (RGBA32Uint)

### Output Channels

**File**: [`SceneDebugger.cpp`](Source/RenderPasses/SceneDebugger/SceneDebugger.cpp:155)

**Purpose**: Define output channels for scene debugger.

**Output Channels**:
```cpp
reflector.addOutput(kOutput, "Scene debugger output").bindFlags(ResourceBindFlags::UnorderedAccess).format(ResourceFormat::RGBA32Float);
```

**Output Names**:
- `output` (required): Scene debugger output (RGBA32Float)

### SceneDebuggerMode Enumeration

**File**: [`SharedTypes.slang`](Source/RenderPasses/SceneDebugger/SharedTypes.slang:33)

**Purpose**: Visualization mode enumeration.

**Modes**:
```cpp
enum class SceneDebuggerMode : uint32_t
{
    FlatShaded,              // Flat shaded
    TriangleDensity,         // Triangle density
    // Geometry
    HitType,                // Hit type in pseudocolor
    InstanceID,             // Instance ID in pseudocolor
    MaterialID,              // Material ID in pseudocolor
    PrimitiveID,             // Primitive ID in pseudocolor
    GeometryID,             // Geometry ID in pseudocolor
    BlasID,                  // Raytracing bottom-level acceleration structure (BLAS) ID in pseudocolor
    InstancedGeometry,       // Green = instanced geometry, Red = non-instanced geometry
    MaterialType,            // Material type in pseudocolor
    // Shading data
    FaceNormal,              // Face normal in RGB color
    ShadingNormal,           // Shading normal in RGB color
    ShadingTangent,          // Shading tangent in RGB color
    ShadingBitangent,         // Shading bitangent in RGB color
    FrontFacingFlag,         // Green = front-facing, Red = back-facing
    BackfacingShadingNormal, // Pixels where shading normal is back-facing with respect to view vector are highlighted
    TexCoords,              // Texture coordinates in RG color wrapped to [0,1]
    // Material properties
    BSDFProperties,           // BSDF properties
};
```

### SceneDebuggerBSDFProperty Enumeration

**File**: [`SharedTypes.slang`](Source/RenderPasses/SceneDebugger/SharedTypes.slang:86)

**Purpose**: BSDF property enumeration.

**Properties**:
```cpp
enum class SceneDebuggerBSDFProperty : uint32_t
{
    Emission,                     // Emission
    Roughness,                    // Roughness
    GuideNormal,                  // Guide normal
    DiffuseReflectionAlbedo,        // Diffuse reflection albedo
    DiffuseTransmissionAlbedo,     // Diffuse transmission albedo
    SpecularReflectionAlbedo,       // Specular reflection albedo
    SpecularTransmissionAlbedo,      // Specular transmission albedo
    SpecularReflectance,           // Specular reflectance
    IsTransmissive,                // Is transmissive
};
```

### SceneDebuggerParams Structure

**File**: [`SharedTypes.slang`](Source/RenderPasses/SceneDebugger/SharedTypes.slang:115)

**Purpose**: Scene debugger parameters.

**Parameters**:
```cpp
struct SceneDebuggerParams
{
    uint mode = (uint)SceneDebuggerMode::FaceNormal; ///< Current visualization mode. See SceneDebuggerMode.
    uint2 frameDim = { 0, 0 };
    uint frameCount = 0;

    uint bsdfProperty = 0; ///< Current BSDF property. See SceneDebuggerBSDFProperty.
    uint bsdfIndex = 0;    ///< Current BSDF layer index.

    uint2 selectedPixel = { 0, 0 }; ///< The currently selected pixel for readback.

    int flipSign = false;           ///< Flip sign before visualization.
    int remapRange = true;          ///< Remap valid range to [0,1] before output.
    int clamp = true;               ///< Clamp pixel values to [0,1] before output.

    int showVolumes = true;         ///< Show volumes.
    float volumeDensityScale = 1.f; ///< Volume density scale factor.
    int useVBuffer = false;

    int profileSecondaryRays = false;       ///< Trace secondary rays for profiling purposes.
    int profileSecondaryLoadHit = false;    ///< Load HitInfo for secondary rays, otherwise just visibility rays.
    float profileSecondaryConeAngle = 90.f; ///< Cone angle (degrees) for randomly distributed ray directions.

    float2 triangleDensityLogRange = { -16.f, 16.f };
};
```

### PixelData Structure

**File**: [`SharedTypes.slang`](Source/RenderPasses/SceneDebugger/SharedTypes.slang:140)

**Purpose**: Pixel data structure for readback.

**Structure**:
```cpp
struct PixelData
{
    static constexpr uint kInvalidID = 0xffffffff;

    // Geometry
    uint hitType;
    uint instanceID;
    uint materialID;
    uint geometryID;
    uint blasID;

    // ShadingData
    float3 posW;
    float3 V;
    float3 N;
    float3 T;
    float3 B;
    float2 uv;
    float3 faceN;
    float4 tangentW;
    uint frontFacing;
    float curveRadius;
};
```

### InstanceInfo Structure

**File**: [`SharedTypes.slang`](Source/RenderPasses/SceneDebugger/SharedTypes.slang:169)

**Purpose**: Instance metadata structure.

**Structure**:
```cpp
enum class InstanceInfoFlags : uint32_t
{
    IsInstanced = 0x1,
};

struct InstanceInfo
{
    uint flags = 0; ///< Flags as a combination of 'InstanceInfoFlags' flags.
};
```

## Technical Details

### Render Graph Integration

**Input Channels**:
- `vbuffer` (optional): Visibility buffer in packed format (RGBA32Uint)

**Output Channels**:
- `output` (required): Scene debugger output (RGBA32Float)

**Reflection**:
```cpp
RenderPassReflection reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    reflector.addInput("vbuffer", "Visibility buffer in packed format")
        .texture2D()
        .format(ResourceFormat::RGBA32Uint)
        .flags(RenderPassReflection::Field::Flags::Optional);
    reflector.addOutput(kOutput, "Scene debugger output").bindFlags(ResourceBindFlags::UnorderedAccess).format(ResourceFormat::RGBA32Float);

    return reflector;
}
```

### Execute

**Implementation**:
```cpp
void SceneDebugger::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    mPixelDataAvailable = false;
    const auto& pOutput = renderData.getTexture(kOutput);
    const auto& pVBuffer = renderData.getTexture("vbuffer");

    if (mParams.useVBuffer && pVBuffer == nullptr)
    {
        logWarningOnce("SceneDebugger cannot use vbuffer as none is connected");
        mParams.useVBuffer = false;
    }

    if (mpScene == nullptr)
    {
        pRenderContext->clearUAV(pOutput->getUAV().get(), float4(0.f));
        return;
    }

    // Check for scene changes that require shader recompilation.
    if (is_set(mUpdateFlags, IScene::UpdateFlags::RecompileNeeded) || is_set(mUpdateFlags, IScene::UpdateFlags::GeometryChanged))
    {
        FALCOR_THROW("This render pass does not support scene changes that require shader recompilation.");
    }

    if (mpScene)
        mpScene->bindShaderDataForRaytracing(pRenderContext, mpDebugPass->getRootVar()["gScene"]);

    ShaderVar var = mpDebugPass->getRootVar()["CB"]["gSceneDebugger"];
    var["params"].setBlob(mParams);
    var["output"] = pOutput;
    var["vbuffer"] = pVBuffer;

    mpPixelDebug->beginFrame(pRenderContext, renderData.getDefaultTextureDims());
    mpPixelDebug->prepareProgram(mpDebugPass->getProgram(), mpDebugPass->getRootVar());
    mpSampleGenerator->bindShaderData(mpDebugPass->getRootVar());

    mpDebugPass->execute(pRenderContext, uint3(mParams.frameDim, 1));

    pRenderContext->copyResource(mpPixelDataStaging.get(), mpPixelData.get());
    pRenderContext->submit(false);
    pRenderContext->signal(mpFence.get());

    mPixelDataAvailable = true;
    mParams.frameCount++;

    mpPixelDebug->endFrame(pRenderContext);

    mUpdateFlags = IScene::UpdateFlags::None;
}
```

**Features**:
- Scene change detection (throws if recompilation needed)
- Output clearing when no scene is loaded
- VBuffer availability check
- Scene and sample generator shader data binding
- PixelDebug frame management (beginFrame, prepareProgram, endFrame)
- Compute pass execution
- Pixel data readback with fence synchronization
- Frame count increment

### Set Scene

**Implementation**:
```cpp
void SceneDebugger::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    mUpdateFlagsConnection = {};
    mUpdateFlags = IScene::UpdateFlags::None;

    mpScene = pScene;
    mpMeshToBlasID = nullptr;
    mpDebugPass = nullptr;
    mUpdateFlags = IScene::UpdateFlags::None;

    if (mpScene)
    {
        mUpdateFlagsConnection = mpScene->getUpdateFlagsSignal().connect([&](IScene::UpdateFlags flags) { mUpdateFlags |= flags; });

        // Prepare our programs for the scene.
        ProgramDesc desc;
        desc.addShaderModules(mpScene->getShaderModules());
        desc.addShaderLibrary(kShaderFile).csEntry("main");
        desc.addTypeConformances(mpScene->getTypeConformances());

        DefineList defines = mpScene->getSceneDefines();
        defines.add(mpSampleGenerator->getDefines());
        mpDebugPass = ComputePass::create(mpDevice, desc, defines);

        // Create lookup table for mesh to BLAS ID.
        auto blasIDs = mpScene->getMeshBlasIDs();
        if (!blasIDs.empty())
        {
            mpMeshToBlasID = mpDevice->createStructuredBuffer(
                    sizeof(uint32_t),
                    (uint32_t)blasIDs.size(),
                    ResourceBindFlags::ShaderResource,
                    MemoryType::DeviceLocal,
                    blasIDs.data(),
                    false
                );
        }

        // Create instance metadata.
        initInstanceInfo();

        // Bind variables.
        auto var = mpDebugPass->getRootVar()["CB"]["gSceneDebugger"];
        if (!mpPixelData)
        {
            mpPixelData = mpDevice->createStructuredBuffer(
                    var["pixelData"],
                    1,
                    ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
                    MemoryType::DeviceLocal,
                    nullptr,
                    false
                );
            mpPixelDataStaging =
                mpDevice->createStructuredBuffer(var["pixelData"], 1, ResourceBindFlags::None, MemoryType::ReadBack, nullptr, false);
        }
        var["pixelData"] = mpPixelData;
        var["meshToBlasID"] = mpMeshToBlasID;
        var["instanceInfo"] = mpInstanceInfo;
    }
}
```

**Features**:
- Scene reference management
- Update flags signal connection
- Scene change tracking
- Program creation with scene shader modules
- Type conformances from scene
- Mesh to BLAS ID lookup table creation
- Instance metadata initialization
- Pixel data buffer creation (structured buffer)
- Pixel data staging buffer creation (readback)
- Shader variable binding

### Mouse Event Handling

**Implementation**:
```cpp
bool SceneDebugger::onMouseEvent(const MouseEvent& mouseEvent)
{
    if (mouseEvent.type == MouseEvent::Type::ButtonDown && mouseEvent.button == Input::MouseButton::Left)
    {
        float2 cursorPos = mouseEvent.pos * (float2)mParams.frameDim;
        mParams.selectedPixel = (uint2)clamp(cursorPos, float2(0.f), float2(mParams.frameDim.x - 1, mParams.frameDim.y - 1));
    }

    return mpPixelDebug->onMouseEvent(mouseEvent);
}
```

**Features**:
- Left mouse button click detection
- Cursor position to pixel coordinate conversion
- Pixel coordinate clamping to valid range
- Delegates to PixelDebug mouse event handling

### Property Serialization

**Implementation**:
```cpp
Properties SceneDebugger::getProperties() const
{
    Properties props;
    props[kMode] = SceneDebuggerMode(mParams.mode);
    props[kShowVolumes] = mParams.showVolumes;
    props[kUseVBuffer] = mParams.useVBuffer;
    return props;
}
```

**Features**:
- Serializes mode, showVolumes, and useVBuffer parameters

### Property Parsing

**Implementation**:
```cpp
// Parse dictionary.
for (const auto& [key, value] : props)
{
    if (key == kMode)
        mParams.mode = (uint32_t)value.operator SceneDebuggerMode();
    else if (key == kShowVolumes)
        mParams.showVolumes = value;
    else if (key == kUseVBuffer)
        mParams.useVBuffer = static_cast<bool>(value);
    else
        logWarning("Unknown property '{}' in a SceneDebugger properties.", key);
}
```

**Features**:
- Parses mode, showVolumes, and useVBuffer properties
- Logs warning for unknown properties

### Compile

**Implementation**:
```cpp
void SceneDebugger::compile(RenderContext* pRenderContext, const CompileData& compileData)
{
    mParams.frameDim = compileData.defaultTexDims;
    mVBufferAvailable = compileData.connectedResources.getField("vbuffer");
}
```

**Features**:
- Stores frame dimensions for dispatch
- Tracks VBuffer availability

### UI Rendering

**Implementation**:
```cpp
void SceneDebugger::renderUI(Gui::Widgets& widget)
{
    if (mVBufferAvailable)
        widget.checkbox("Use VBuffer", mParams.useVBuffer);

    widget.dropdown("Mode", reinterpret_cast<SceneDebuggerMode&>(mParams.mode));
    widget.tooltip("Selects visualization mode");

    if (mParams.mode == (uint32_t)SceneDebuggerMode::TriangleDensity)
    {
        widget.var("Triangle density range (log2)", mParams.triangleDensityLogRange);
    }

    if (mParams.mode == (uint32_t)SceneDebuggerMode::BSDFProperties)
    {
        widget.dropdown("BSDF property", reinterpret_cast<SceneDebuggerBSDFProperty&>(mParams.bsdfProperty));
        widget.var("BSDF index", mParams.bsdfIndex, 0u, 15u, 1u);
    }

    widget.checkbox("Clamp to [0,1]", mParams.clamp);
    widget.tooltip("Clamp pixel values to [0,1] before output.");

    if ((SceneDebuggerMode)mParams.mode == SceneDebuggerMode::FaceNormal ||
        (SceneDebuggerMode)mParams.mode == SceneDebuggerMode::ShadingNormal ||
        (SceneDebuggerMode)mParams.mode == SceneDebuggerMode::ShadingTangent ||
        (SceneDebuggerMode)mParams.mode == SceneDebuggerMode::ShadingBitangent ||
        (SceneDebuggerMode)mParams.mode == SceneDebuggerMode::TexCoords)
    {
        widget.checkbox("Flip sign", mParams.flipSign);
        widget.checkbox("Remap to [0,1]", mParams.remapRange);
        widget.tooltip("Remap range from [-1,1] to [0,1] before output.");
    }

    widget.checkbox("Show volumes", mParams.showVolumes);
    if (mParams.showVolumes)
    {
        widget.var("Volume density scale", mParams.volumeDensityScale, 0.f, 1000.f, 0.1f);
    }

    widget.textWrapped("Description:\n" + getModeDesc((SceneDebuggerMode)mParams.mode));

    // Show data for the currently selected pixel.
    widget.dummy("#spacer0", {1, 20});
    widget.var("Selected pixel", mParams.selectedPixel);

    if (mpScene)
        renderPixelDataUI(widget);

    widget.dummy("#spacer1", {1, 20});
    widget.text("Scene: " + (mpScene ? mpScene->getPath().string() : "No scene loaded"));

    if (auto loggingGroup = widget.group("Logging", false))
    {
        mpPixelDebug->renderUI(widget);
    }

    if (auto g = widget.group("Profiling", false))
    {
        widget.checkbox("Trace secondary rays", mParams.profileSecondaryRays);
        if (mParams.profileSecondaryRays)
        {
            widget.checkbox("Load hit info", mParams.profileSecondaryLoadHit);

            widget.var("Cone angle (deg)", mParams.profileSecondaryConeAngle, 0.f, 90.f, 1.f);
            widget.tooltip(
                "Traces secondary rays from the primary hits. The secondary rays have directions that are randomly distributed in a cone "
                "around the face normal."
            );
        }
    }
}
```

**Features**:
- VBuffer usage checkbox
- Mode dropdown with 18 visualization modes
- Triangle density range slider
- BSDF property dropdown and index slider
- Clamp to [0,1] checkbox
- Flip sign and remap to [0,1] checkboxes
- Show volumes checkbox and density scale slider
- Mode description display
- Selected pixel display
- Pixel data UI rendering (mesh info, instance info, shading data, material info)
- Scene path display
- PixelDebug logging UI
- Profiling group with secondary ray tracing options

### Render Pixel Data UI

**Implementation**:
```cpp
void SceneDebugger::renderPixelDataUI(Gui::Widgets& widget)
{
    if (!mPixelDataAvailable)
        return;

    FALCOR_ASSERT(mpPixelDataStaging);
    mpFence->wait();
    const PixelData data = mpPixelDataStaging->getElement<PixelData>(0);

    switch ((HitType)data.hitType)
    {
    case HitType::Triangle:
    {
        std::string text;
        text += fmt::format("Mesh ID: {}\n", data.geometryID);
        text += fmt::format("Mesh name: {}\n", mpScene->hasMesh(data.geometryID) ? mpScene->getMeshName(data.geometryID) : "unknown");
        text += fmt::format("Instance ID: {}\n", data.instanceID);
        text += fmt::format("Material ID: {}\n", data.materialID);
        text += fmt::format("BLAS ID: {}\n", data.blasID);
        widget.text(text);
        widget.dummy("#spacer2", {1, 10});

        // Show mesh details.
        if (auto g = widget.group("Mesh info"); g.open())
        {
            FALCOR_ASSERT(data.geometryID < mpScene->getMeshCount());
            const auto& mesh = mpScene->getMesh(MeshID{data.geometryID});
            std::string text;
            text += fmt::format("flags: 0x{:08x}\n", mesh.flags);
            text += fmt::format("materialID: {}\n", mesh.materialID);
            text += fmt::format("vertexCount: {}\n", mesh.vertexCount);
            text += fmt::format("indexCount: {}\n", mesh.indexCount);
            text += fmt::format("triangleCount: {}\n", mesh.getTriangleCount());
            text += fmt::format("vbOffset: {}\n", mesh.vbOffset);
            text += fmt::format("ibOffset: {}\n", mesh.ibOffset);
            text += fmt::format("skinningVbOffset: {}\n", mesh.skinningVbOffset);
            text += fmt::format("prevVbOffset: {}\n", mesh.prevVbOffset);
            text += fmt::format("use16BitIndices: {}\n", mesh.use16BitIndices());
            text += fmt::format("isFrontFaceCW: {}\n", mesh.isFrontFaceCW());
            g.text(text);
        }

        // Show mesh instance info.
        if (auto g = widget.group("Mesh instance info"); g.open())
        {
            FALCOR_ASSERT(data.instanceID < mpScene->getGeometryInstanceCount());
            const auto& instance = mpScene->getGeometryInstance(data.instanceID);
            std::string text;
            text += fmt::format("flags: 0x{:08x}\n", instance.flags);
            text += fmt::format("nodeID: {}\n", instance.globalMatrixID);
            text += fmt::format("meshID: {}\n", instance.geometryID);
            text += fmt::format("materialID: {}\n", instance.materialID);
            text += fmt::format("vbOffset: {}\n", instance.vbOffset);
            text += fmt::format("ibOffset: {}\n", instance.ibOffset);
            text += fmt::format("isDynamic: {}\n", instance.isDynamic());
            g.text(text);

            // Print the list of scene graph nodes affecting this mesh instance.
            std::vector<NodeID> nodes;
            {
                NodeID nodeID{instance.globalMatrixID};
                while (nodeID != NodeID::Invalid())
                {
                    nodes.push_back(nodeID);
                    nodeID = mpScene->getParentNodeID(nodeID);
                }
            }
            FALCOR_ASSERT(!nodes.empty());

            g.text("Scene graph (root first):");
            const auto& localMatrices = mpScene->getAnimationController()->getLocalMatrices();
            for (auto it = nodes.rbegin(); it != nodes.rend(); it++)
            {
                auto nodeID = *it;
                float4x4 mat = localMatrices[nodeID.get()];
                if (auto nodeGroup = widget.group("ID " + to_string(nodeID)); nodeGroup.open())
                {
                    g.matrix("", mat);
                }
            }
        }
        break;
    }
    case HitType::Curve:
    {
        std::string text;
        text += fmt::format("Curve ID: {}\n", data.geometryID);
        text += fmt::format("Instance ID: {}\n", data.instanceID);
        text += fmt::format("Material ID: {}\n", data.materialID);
        text += fmt::format("BLAS ID: {}\n", data.blasID);
        widget.text(text);
        widget.dummy("#spacer2", {1, 10});

        // Show curve details.
        if (auto g = widget.group("Curve info"); g.open())
        {
            const auto& curve = mpScene->getCurve(CurveID{data.geometryID});
            std::string text;
            text += fmt::format("degree: {}\n", curve.degree);
            text += fmt::format("vertexCount: {}\n", curve.vertexCount);
            text += fmt::format("indexCount: {}\n", curve.indexCount);
            text += fmt::format("vbOffset: {}\n", curve.vbOffset);
            text += fmt::format("ibOffset: {}\n", curve.ibOffset);
            g.text(text);
        }
        break;
    }
    case HitType::SDFGrid:
    {
        std::string text;
        text += fmt::format("SDF Grid ID: {}\n", data.geometryID);
        text += fmt::format("Instance ID: {}\n", data.instanceID);
        text += fmt::format("Material ID: {}\n", data.materialID);
        text += fmt::format("BLAS ID: {}\n", data.blasID);
        widget.text(text);
        widget.dummy("#spacer2", {1, 10});

        // Show SDF grid details.
        if (auto g = widget.group("SDF grid info"); g.open())
        {
            const ref<SDFGrid>& pSDFGrid = mpScene->getSDFGrid(SdfGridID{data.geometryID});
            std::string text;
            text += fmt::format("gridWidth: {}\n", pSDFGrid->getGridWidth());
            g.text(text);
        }
        break;
    }
    case HitType::None:
        widget.text("Background pixel");
        break;
    default:
        widget.text("Unsupported hit type");
        break;
    }

    // Show shading data.
    if ((HitType)data.hitType != HitType::None)
    {
        if (auto g = widget.group("Shading data"); g.open())
        {
            std::string text;
            text += fmt::format("posW: {}\n", data.posW);
            text += fmt::format("V: {}\n", data.V);
            text += fmt::format("N: {}\n", data.N);
            text += fmt::format("T: {}\n", data.T);
            text += fmt::format("B: {}\n", data.B);
            text += fmt::format("uv: {}\n", data.uv);
            text += fmt::format("faceN: {}\n", data.faceN);
            text += fmt::format("tangentW: {}\n", data.tangentW);
            text += fmt::format("frontFacing: {}\n", data.frontFacing);
            text += fmt::format("curveRadius: {}\n", data.curveRadius);
            g.text(text);
        }
    }

    // Show material info.
    if (data.materialID != PixelData::kInvalidID)
    {
        if (auto g = widget.group("Material info"); g.open())
        {
            const auto& material = *mpScene->getMaterial(MaterialID{data.materialID});
            const auto& header = material.getHeader();
            std::string text;
            text += fmt::format("name: {}\n", material.getName());
            text += fmt::format("materialType: {}\n", to_string(header.getMaterialType()));
            text += fmt::format("alphaMode: {}\n", (uint32_t)header.getAlphaMode());
            text += fmt::format("alphaThreshold: {}\n", (float)header.getAlphaThreshold());
            text += fmt::format("nestedPriority: {}\n", header.getNestedPriority());
            text += fmt::format("activeLobes: 0x{:08x}\n", (uint32_t)header.getActiveLobes());
            text += fmt::format("defaultTextureSamplerID: {}\n", header.getDefaultTextureSamplerID());
            text += fmt::format("doubleSided: {}\n", header.isDoubleSided());
            text += fmt::format("thinSurface: {}\n", header.isThinSurface());
            text += fmt::format("emissive: {}\n", header.isEmissive());
            text += fmt::format("basicMaterial: {}\n", header.isBasicMaterial());
            text += fmt::format("lightProfileEnabled: {}\n", header.isLightProfileEnabled());
            text += fmt::format("deltaSpecular: {}\n", header.isDeltaSpecular());
            g.text(text);
        }
    }
}
```

**Features**:
- Fence wait for pixel data readback
- Pixel data extraction from staging buffer
- Hit type-based display (Triangle, Curve, SDFGrid, None)
- Mesh info display (flags, materialID, vertexCount, indexCount, triangleCount, offsets, use16BitIndices, isFrontFaceCW)
- Mesh instance info display (flags, nodeID, meshID, materialID, offsets, isDynamic)
- Scene graph display (node hierarchy with local matrices)
- Curve info display (degree, vertexCount, indexCount, offsets)
- SDF grid info display (gridWidth)
- Shading data display (posW, V, N, T, B, uv, faceN, tangentW, frontFacing, curveRadius)
- Material info display (name, materialType, alphaMode, alphaThreshold, nestedPriority, activeLobes, defaultTextureSamplerID, doubleSided, thinSurface, emissive, basicMaterial, lightProfileEnabled, deltaSpecular)

### Initialize Instance Info

**Implementation**:
```cpp
void SceneDebugger::initInstanceInfo()
{
    const uint32_t instanceCount = mpScene ? mpScene->getGeometryInstanceCount() : 0;

    // If there are no instances. Just clear buffer and return.
    if (instanceCount == 0)
    {
        mpInstanceInfo = nullptr;
        return;
    }

    // Count number of times each geometry is used.
    std::vector<std::vector<uint32_t>> instanceCounts((size_t)GeometryType::Count);
    for (auto& counts : instanceCounts)
        counts.resize(mpScene->getGeometryCount());

    for (uint32_t instanceID = 0; instanceID < instanceCount; instanceID++)
    {
        const auto& instance = mpScene->getGeometryInstance(instanceID);
        instanceCounts[(size_t)instance.getType()][instance.geometryID]++;
    }

    // Setup instance metadata.
    std::vector<InstanceInfo> instanceInfo(instanceCount);
    for (uint32_t instanceID = 0; instanceID < instanceCount; instanceID++)
    {
        const auto& instance = mpScene->getGeometryInstance(instanceID);
        auto& info = instanceInfo[instanceID];
        if (instanceCounts[(size_t)instance.getType()][instance.geometryID] > 1)
        {
            info.flags |= (uint32_t)InstanceInfoFlags::IsInstanced;
        }
    }

    // Create GPU buffer.
    mpInstanceInfo = mpDevice->createStructuredBuffer(
        sizeof(InstanceInfo),
        (uint32_t)instanceInfo.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        instanceInfo.data(),
        false
    );
}
```

**Features**:
- Instance count retrieval from scene
- Geometry usage counting per geometry type
- Instance metadata generation with instancing flags
- GPU buffer creation for instance info

### Scene Debugger Shader

**File**: [`SceneDebugger.cs.slang`](Source/RenderPasses/SceneDebugger/SceneDebugger.cs.slang:1)

**Purpose**: Compute shader for scene debugging with multiple visualization modes.

**Shader Resources**:
- `Texture2D<PackedHitInfo> vbuffer` - Visibility buffer in packed format (optional)
- `StructuredBuffer<uint> meshToBlasID` - Mesh to BLAS ID lookup table
- `StructuredBuffer<InstanceInfo> instanceInfo` - Instance metadata

**Output Resources**:
- `RWTexture2D<float4> output` - Scene debugger output
- `RWStructuredBuffer<PixelData> pixelData` - Pixel data buffer

**Constant Buffer** (`CB`):
- `SceneDebuggerParams gSceneDebugger` - Scene debugger parameters

**Thread Group Size**: 16x16x1

**Main Function** (`main`):
```cpp
[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadId: SV_DispatchThreadID)
{
    gSceneDebugger.execute(dispatchThreadId.xy);
}
```

**Execute Function**:
```cpp
void execute(const uint2 pixel)
{
    if (any(pixel >= params.frameDim))
        return;

    printSetPixel(pixel);

    // Initialize pixel data for the selected pixel.
    if (all(pixel == params.selectedPixel))
    {
        PixelData d = {};
        d.hitType = (uint)HitType::None;
        d.instanceID = PixelData::kInvalidID;
        d.materialID = PixelData::kInvalidID;
        d.geometryID = PixelData::kInvalidID;
        d.blasID = PixelData::kInvalidID;
        pixelData[0] = d;
    }

    // Trace primary ray.
    const Ray ray = gScene.camera.computeRayPinhole(pixel, params.frameDim);

    float3 color = float3(0);

    HitInfo hit;
    float hitT = 1e30f;

    if ((bool)params.useVBuffer)
    {
        hit = unpackHitInfo(vbuffer[pixel]);
        if (hit.isValid())
        {
            if ((bool)params.showVolumes)
            {
                float3 posW = getPosW(ray.origin, ray.dir, hit);
                hitT = length(posW - ray.origin);
            }
        }
    }
    else
    {
        SceneRayQuery<kUseAlphaTest> sceneRayQuery;
        hit = sceneRayQuery.traceRay(ray, hitT);
    }

    if (hit.isValid())
    {
        color = handleHit(pixel, ray.origin, ray.dir, hit);
    }
    else
    {
        hitT = 1e30f;
        color = handleMiss(pixel, ray.dir);
    }

    // Process volumes.
    if ((bool)params.showVolumes)
    {
        color = handleVolumes(color, ray.origin, ray.dir, hitT);
    }

    // Clamp pixel values if necessary.
    if ((bool)params.clamp)
        color = saturate(color);

    // Write output.
    output[pixel] = float4(color, 1.f);
}
```

**Features**:
- Bounds checking for pixel coordinates
- PixelDebug printSetPixel for selected pixel initialization
- Primary ray tracing from camera
- VBuffer hit unpacking (optional)
- Scene ray query for ray tracing (if VBuffer not used)
- Hit and miss handling
- Volume transmittance evaluation (optional)
- Pixel value clamping (optional)
- Output writing

### Handle Hit Function

**Implementation**:
```cpp
float3 handleHit(const uint2 pixel, const float3 orig, const float3 dir, const HitInfo hit)
{
    uint instanceID = PixelData::kInvalidID;
    uint materialID = PixelData::kInvalidID;
    uint primitiveID = PixelData::kInvalidID;
    uint geometryID = PixelData::kInvalidID;
    uint blasID = PixelData::kInvalidID;

    VertexData v = {};
    switch (hit.getType())
    {
    case HitType::Triangle:
    {
        const TriangleHit triangleHit = hit.getTriangleHit();
        instanceID = triangleHit.instanceID.index;
        geometryID = gScene.getGeometryInstance(triangleHit.instanceID).geometryID;
        primitiveID = triangleHit.primitiveIndex;
        blasID = meshToBlasID[geometryID];
        materialID = gScene.getMaterialID(triangleHit.instanceID);

        // Load vertex attributes.
        v = gScene.getVertexData(triangleHit);
        break;
    }
    case HitType::DisplacedTriangle:
    {
        const DisplacedTriangleHit displacedTriangleHit = hit.getDisplacedTriangleHit();
        instanceID = displacedTriangleHit.instanceID.index;
        geometryID = gScene.getGeometryInstance(displacedTriangleHit.instanceID).geometryID;
        blasID = meshToBlasID[geometryID];
        materialID = gScene.getMaterialID(displacedTriangleHit.instanceID);

        // Load vertex attributes.
        v = gScene.getVertexData(displacedTriangleHit, -dir);
        break;
    }
    case HitType::Curve:
    {
        const CurveHit curveHit = hit.getCurveHit();
        instanceID = curveHit.instanceID.index;
        geometryID = gScene.getGeometryInstance(curveHit.instanceID).geometryID;
        materialID = gScene.getMaterialID(curveHit.instanceID);

        // Load vertex attributes.
        v = gScene.getVertexDataFromCurve(curveHit);
        break;
    }
    case HitType::SDFGrid:
    {
        const SDFGridHit sdfGridHit = hit.getSDFGridHit();
        instanceID = sdfGridHit.instanceID.index;
        geometryID = gScene.getGeometryInstance(sdfGridHit.instanceID).geometryID;
        materialID = gScene.getMaterialID(sdfGridHit.instanceID);

        // Load vertex attributes.
        v = gScene.getVertexDataFromSDFGrid(sdfGridHit, orig, dir);
        break;
    }
    default:
        // Should not happen. Return an error color.
        return float3(1, 0, 0);
    }

    // Load shading data.
    const ShadingData sd = gScene.materials.prepareShadingData(v, materialID, -dir);

    // Write pixel data for the selected pixel.
    if (all(pixel == params.selectedPixel))
    {
        PixelData d = {};

        // Geometry
        d.hitType = uint(hit.getType());
        d.instanceID = instanceID;
        d.materialID = materialID;
        d.geometryID = geometryID;
        d.blasID = blasID;

        // ShadingData
        d.posW = sd.posW;
        d.V = sd.V;
        d.N = sd.frame.N;
        d.T = sd.frame.T;
        d.B = sd.frame.B;
        d.uv = sd.uv;
        d.faceN = sd.faceN;
        d.tangentW = sd.tangentW;
        d.frontFacing = sd.frontFacing ? 1 : 0;
        d.curveRadius = sd.curveRadius;

        pixelData[0] = d;
    }

    // Profiling of secondary rays.
    if (params.profileSecondaryRays)
    {
        return traceSecondary(sd, pixel);
    }

    // Compute zebra stripes.
    const float z = ((pixel.x + pixel.y - params.frameCount) & 0x8) ? 1.f : 0.f;

    // Compute flat shading.
    const float3 frontN = faceforward(sd.faceN, dir, sd.faceN);
    const float flatShaded = 0.8f * saturate(dot(-frontN, dir)) + 0.2f;

    switch ((SceneDebuggerMode)params.mode)
    {
    case SceneDebuggerMode::FlatShaded:
        return float3(flatShaded);
    case SceneDebuggerMode::TriangleDensity:
        return colormapViridis(computeTriangleDensity(hit));
    // Geometry
    case SceneDebuggerMode::HitType:
        return pseudocolor(uint(hit.getType()));
    case SceneDebuggerMode::InstanceID:
        return pseudocolor(instanceID);
    case SceneDebuggerMode::MaterialID:
        return pseudocolor(materialID) * flatShaded;
    case SceneDebuggerMode::PrimitiveID:
        return pseudocolor(primitiveID);
    case SceneDebuggerMode::GeometryID:
        return pseudocolor(geometryID);
    case SceneDebuggerMode::BlasID:
        return pseudocolor(blasID);
    case SceneDebuggerMode::InstancedGeometry:
    {
        float v = 0.75f * luminance(abs(sd.faceN)) + 0.25f;
        if (hit.getType() == HitType::Triangle && instanceID != PixelData::kInvalidID)
        {
            bool isInstanced = (instanceInfo[instanceID].flags & (uint)InstanceInfoFlags::IsInstanced) != 0;
            return isInstanced ? float3(0, v, 0) : float3(v, 0, 0);
        }
        else
        {
            // For non-triangle geometry, return grayscale color to indicate instancing status is not available.
            return float3(v, v, v);
        }
    }
    case SceneDebuggerMode::MaterialType:
        return pseudocolor((uint)sd.mtl.getMaterialType()) * flatShaded;
    // Shading data
    case SceneDebuggerMode::FaceNormal:
        return remapVector(sd.faceN);
    case SceneDebuggerMode::ShadingNormal:
        return remapVector(sd.frame.N);
    case SceneDebuggerMode::ShadingTangent:
        return remapVector(sd.frame.T);
    case SceneDebuggerMode::ShadingBitangent:
        return remapVector(sd.frame.B);
    case SceneDebuggerMode::FrontFacingFlag:
    {
        float v = 0.75f * luminance(abs(sd.faceN)) + 0.25f;
        return sd.frontFacing ? float3(0, v, 0) : float3(v, 0, 0);
    }
    case SceneDebuggerMode::BackfacingShadingNormal:
    {
        float v = 0.75f * luminance(abs(sd.faceN)) + 0.25f;
        bool backFacing = dot(sd.frame.N, sd.V) <= 0.f;
        return backFacing ? float3(z, z, 0) : float3(v, v, v);
    }
    case SceneDebuggerMode::TexCoords:
        return float3(frac(remapVector(sd.uv)), 0.f);
    // Material properties
    case SceneDebuggerMode::BSDFProperties:
    {
        // Create material instance and query its properties.
        let lod = ExplicitLodTextureSampler(0.f);
        let mi = gScene.materials.getMaterialInstance(sd, lod);
        let bsdfProperties = mi.getProperties(sd);
        switch ((SceneDebuggerBSDFProperty)params.bsdfProperty)
        {
        case SceneDebuggerBSDFProperty::Emission:
            return bsdfProperties.emission;
        case SceneDebuggerBSDFProperty::Roughness:
            return float3(bsdfProperties.roughness);
        case SceneDebuggerBSDFProperty::GuideNormal:
            return remapVector(bsdfProperties.guideNormal);
        case SceneDebuggerBSDFProperty::DiffuseReflectionAlbedo:
            return bsdfProperties.diffuseReflectionAlbedo;
        case SceneDebuggerBSDFProperty::DiffuseTransmissionAlbedo:
            return bsdfProperties.diffuseTransmissionAlbedo;
        case SceneDebuggerBSDFProperty::SpecularReflectionAlbedo:
            return bsdfProperties.specularReflectionAlbedo;
        case SceneDebuggerBSDFProperty::SpecularTransmissionAlbedo:
            return bsdfProperties.specularTransmissionAlbedo;
        case SceneDebuggerBSDFProperty::SpecularReflectance:
            return bsdfProperties.specularReflectance;
        case SceneDebuggerBSDFProperty::IsTransmissive:
            return bsdfProperties.isTransmissive ? float3(0, 1, 0) : float3(1, 0, 0);
        }
    }

    default:
        // Should not happen.
        return float3(1, 0, 0);
    }
}
```

**Features**:
- Hit type handling (Triangle, DisplacedTriangle, Curve, SDFGrid)
- Instance ID, material ID, geometry ID, BLAS ID extraction
- Vertex data loading for each geometry type
- Shading data preparation
- Pixel data writing for selected pixel
- Secondary ray profiling (optional)
- Zebra stripe computation
- Flat shading computation
- 18 visualization modes with different color computations

### Handle Miss Function

**Implementation**:
```cpp
float3 handleMiss(const uint2 pixel, const float3 dir)
{
    // Draw a checkerboard pattern.
    return ((pixel.x ^ pixel.y) & 0x8) != 0 ? float3(1.f) : float3(0.5f);
}
```

**Features**:
- Checkerboard pattern for background pixels
- XOR-based checkerboard generation

### Handle Volumes Function

**Implementation**:
```cpp
float3 handleVolumes(const float3 color, const float3 pos, const float3 dir, const float hitT)
{
    float Tr = 1.f;
    for (uint i = 0; i < gScene.getGridVolumeCount(); ++i)
    {
        GridVolume gridVolume = gScene.getGridVolume(i);
        Tr *= evalGridVolumeTransmittance(gridVolume, pos, dir, 0.f, hitT);
    }

    return Tr * color;
}
```

**Features**:
- Grid volume iteration
- Volume transmittance evaluation per volume
- Transmittance accumulation

### Eval Grid Volume Transmittance Function

**Implementation**:
```cpp
float evalGridVolumeTransmittance(GridVolume gridVolume, const float3 pos, const float3 dir, const float minT, const float maxT)
{
    if (!gridVolume.hasDensityGrid())
        return 1.f;

    // Intersect with volume bounds and get intersection interval along the view ray.
    AABB bounds = gridVolume.getBounds();
    float2 nearFar;
    bool hit = intersectRayAABB(pos, dir, bounds.minPoint, bounds.maxPoint, nearFar);
    nearFar.x = max(nearFar.x, minT);
    nearFar.y = min(nearFar.y, maxT);
    if (nearFar.x >= nearFar.y)
        return 1.f;

    // Setup access to density grid.
    Grid densityGrid;
    gScene.getGrid(gridVolume.getDensityGrid(), densityGrid);
    Grid::Accessor accessor = densityGrid.createAccessor();

    // Transform to index-space.
    const float3 ipos = mul(gridVolume.data.invTransform, float4(pos, 1.f)).xyz;
    const float3 idir = mul(gridVolume.data.invTransform, float4(dir, 0.f)).xyz;

    // Evaluate transmittance using ray-marching.
    const uint kSteps = 500;
    float opticalDepth = 0.f;
    for (uint step = 0; step < kSteps; ++step)
    {
        float t = lerp(nearFar.x, nearFar.y, (step + 0.5f) / kSteps);
        float density = densityGrid.lookupIndex(int3(ipos + t * idir), accessor);
        opticalDepth += density;
    }
    opticalDepth *= (nearFar.y - nearFar.x) / kSteps * gridVolume.data.densityScale * params.volumeDensityScale;
    return exp(-opticalDepth);
}
```

**Features**:
- Volume bounds intersection
- Density grid access
- Index-space transformation
- Ray marching with 500 steps
- Optical depth computation
- Transmittance evaluation with exponential decay

### Trace Secondary Function

**Implementation**:
```cpp
float3 traceSecondary(const ShadingData sd, const uint2 pixel)
{
    SampleGenerator sg = SampleGenerator(pixel, params.frameCount);

    // Create frame centered around the face normal oriented towards the viewer.
    const float3 N = sd.getOrientedFaceNormal();
    bool valid;
    let sf = ShadingFrame::createSafe(N, sd.tangentW, valid);

    // Generate random direction in cone centered around +z axis.
    float c = cos(params.profileSecondaryConeAngle * (M_2PI / 360.f));
    float s = sqrt(1.f - c * c);
    float z = c + (1.f - c) * sampleNext1D(sg); // z uniform on [cos(a),1]
    float phi = sampleNext1D(sg) * M_2PI;
    float3 dir = float3(cos(phi) * s, sin(phi) * s, z);
    dir = sf.fromLocal(dir);

    // Trace secondary ray.
    const Ray ray = Ray(sd.computeRayOrigin(), dir);

    float hitT = 1e30f;
    SceneRayQuery<kUseAlphaTest> sceneRayQuery;
    HitInfo hit = sceneRayQuery.traceRay(ray, hitT);

    float3 color = {};

    if (params.profileSecondaryLoadHit)
    {
        if (hit.isValid())
        {
            let hitSD = loadHit(ray, hit);
            color = hitSD.frame.N;
        }
    }
    else
    {
        color = hit.isValid() ? float3(0, 1, 0) : float3(1, 0, 0);
    }

    return color;
}
```

**Features**:
- Sample generator for random direction sampling
- Shading frame creation
- Cone-based direction generation around +z axis
- Secondary ray tracing
- Hit info loading (optional)
- Color computation from hit normal or visibility

### Compute Triangle Density Function

**Implementation**:
```cpp
float computeTriangleDensity(const HitInfo hit)
{
    if (hit.getType() != HitType::Triangle)
        return 0.f;

    const TriangleHit triangleHit = hit.getTriangleHit();
    const float A = gScene.getFaceAreaW(triangleHit.instanceID, triangleHit.primitiveIndex);

    float density = 1.f / A; // Can be inf
    float logDensity = log2(density);

    return (logDensity - params.triangleDensityLogRange.x) / (params.triangleDensityLogRange.y - params.triangleDensityLogRange.x);
}
```

**Features**:
- Triangle face area retrieval
- Density computation (1/A)
- Log2 density computation
- Range remapping to [0,1]

### Remap Vector Function

**Implementation**:
```cpp
float3 remapVector(float3 v)
{
    if ((bool)params.flipSign)
        v = -v;
    if ((bool)params.remapRange)
        v = 0.5f * v + 0.5f;
    return v;
}
```

**Features**:
- Sign flipping
- Range remapping from [-1,1] to [0,1]

### Pseudocolor Function

**Implementation**:
```cpp
float3 pseudocolor(uint value)
{
    uint h = jenkinsHash(value);
    return (uint3(h, h >> 8, h >> 16) & 0xff) / 255.f;
}
```

**Features**:
- Jenkins hash for pseudocolor generation
- RGB color from hash components
- Normalization to [0,1]

## Architecture Patterns

### Render Pass Pattern

- Inherits from [`RenderPass`](Source/Falcor/RenderGraph/RenderPass.h:1) base class
- Implements standard render pass interface:
  - `reflect()` - Defines input/output resources
  - `execute()` - Executes scene debugging
  - `renderUI()` - Renders comprehensive UI
  - `getProperties()` - Serializes configuration
  - `setScene()` - Sets scene and initializes programs
  - `onMouseEvent()` - Handles mouse events for pixel selection
  - `onKeyEvent()` - Handles keyboard events (always returns false)
  - `compile()` - Stores frame dimensions

### Compute Pass Pattern

- Uses compute shader for scene debugging
- Thread group size: 16x16x1
- Single dispatch per frame

### PixelDebug Integration Pattern

- PixelDebug utility for shader-based data extraction
- Frame management (beginFrame, prepareProgram, endFrame)
- Shader printSetPixel for selected pixel initialization
- Readback with staging buffer and fence synchronization

### Scene Integration Pattern

- Scene reference management
- Scene change tracking with UpdateFlags signal
- Scene shader modules integration
- Type conformances from scene
- Scene defines for shader compilation
- Ray tracing integration with scene ray query
- Vertex data loading for multiple geometry types
- Material instance creation and BSDF property queries

### Volume Integration Pattern

- Grid volume iteration
- Volume transmittance evaluation
- Ray marching with density grid access
- Optical depth computation
- Transmittance accumulation

### Secondary Ray Profiling Pattern

- Cone-based direction generation
- Sample generator for random sampling
- Secondary ray tracing
- Hit info loading (optional)
- Color computation for profiling

### Visualization Mode Pattern

- 18 different visualization modes
- Mode-specific color computations
- Pseudocolor generation for IDs
- Vector remapping for normals/tangents
- Flat shading computation
- Triangle density computation
- Material property queries

### UI Pattern

- Comprehensive UI with multiple groups
- Mode selection dropdown
- Parameter controls (checkboxes, sliders)
- Pixel data display with detailed information
- Mesh, instance, material, shading data groups
- Logging and profiling groups
- PixelDebug UI delegation

## Code Patterns

### Property Parsing Pattern

```cpp
// Parse dictionary.
for (const auto& [key, value] : props)
{
    if (key == kMode)
        mParams.mode = (uint32_t)value.operator SceneDebuggerMode();
    else if (key == kShowVolumes)
        mParams.showVolumes = value;
    else if (key == kUseVBuffer)
        mParams.useVBuffer = static_cast<bool>(value);
    else
        logWarning("Unknown property '{}' in a SceneDebugger properties.", key);
}
```

### Property Serialization Pattern

```cpp
Properties SceneDebugger::getProperties() const
{
    Properties props;
    props[kMode] = SceneDebuggerMode(mParams.mode);
    props[kShowVolumes] = mParams.showVolumes;
    props[kUseVBuffer] = mParams.useVBuffer;
    return props;
}
```

### Scene Change Tracking Pattern

```cpp
mUpdateFlagsConnection = mpScene->getUpdateFlagsSignal().connect([&](IScene::UpdateFlags flags) { mUpdateFlags |= flags; });
```

### Program Creation Pattern

```cpp
ProgramDesc desc;
desc.addShaderModules(mpScene->getShaderModules());
desc.addShaderLibrary(kShaderFile).csEntry("main");
desc.addTypeConformances(mpScene->getTypeConformances());

DefineList defines = mpScene->getSceneDefines();
defines.add(mpSampleGenerator->getDefines());
mpDebugPass = ComputePass::create(mpDevice, desc, defines);
```

### Mesh to BLAS ID Lookup Table Pattern

```cpp
auto blasIDs = mpScene->getMeshBlasIDs();
if (!blasIDs.empty())
{
    mpMeshToBlasID = mpDevice->createStructuredBuffer(
                sizeof(uint32_t),
                (uint32_t)blasIDs.size(),
                ResourceBindFlags::ShaderResource,
                MemoryType::DeviceLocal,
                blasIDs.data(),
                false
            );
}
```

### Instance Metadata Initialization Pattern

```cpp
void initInstanceInfo()
{
    const uint32_t instanceCount = mpScene ? mpScene->getGeometryInstanceCount() : 0;

    if (instanceCount == 0)
    {
        mpInstanceInfo = nullptr;
        return;
    }

    std::vector<std::vector<uint32_t>> instanceCounts((size_t)GeometryType::Count);
    for (auto& counts : instanceCounts)
        counts.resize(mpScene->getGeometryCount());

    for (uint32_t instanceID = 0; instanceID < instanceCount; instanceID++)
    {
        const auto& instance = mpScene->getGeometryInstance(instanceID);
        instanceCounts[(size_t)instance.getType()][instance.geometryID]++;
    }

    std::vector<InstanceInfo> instanceInfo(instanceCount);
    for (uint32_t instanceID = 0; instanceID < instanceCount; instanceID++)
    {
        const auto& instance = mpScene->getGeometryInstance(instanceID);
        auto& info = instanceInfo[instanceID];
        if (instanceCounts[(size_t)instance.getType()][instance.geometryID] > 1)
        {
            info.flags |= (uint32_t)InstanceInfoFlags::IsInstanced;
        }
    }

    mpInstanceInfo = mpDevice->createStructuredBuffer(
        sizeof(InstanceInfo),
        (uint32_t)instanceInfo.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        instanceInfo.data(),
        false
    );
}
```

### Pixel Data Readback Pattern

```cpp
mpPixelDebug->beginFrame(pRenderContext, renderData.getDefaultTextureDims());
mpPixelDebug->prepareProgram(mpDebugPass->getProgram(), mpDebugPass->getRootVar());
mpSampleGenerator->bindShaderData(mpDebugPass->getRootVar());

mpDebugPass->execute(pRenderContext, uint3(mParams.frameDim, 1));

pRenderContext->copyResource(mpPixelDataStaging.get(), mpPixelData.get());
pRenderContext->submit(false);
pRenderContext->signal(mpFence.get());

mPixelDataAvailable = true;
mParams.frameCount++;

mpPixelDebug->endFrame(pRenderContext);
```

### Hit Type Handling Pattern

```cpp
switch (hit.getType())
{
case HitType::Triangle:
    const TriangleHit triangleHit = hit.getTriangleHit();
    instanceID = triangleHit.instanceID.index;
    geometryID = gScene.getGeometryInstance(triangleHit.instanceID).geometryID;
    primitiveID = triangleHit.primitiveIndex;
    blasID = meshToBlasID[geometryID];
    materialID = gScene.getMaterialID(triangleHit.instanceID);
    v = gScene.getVertexData(triangleHit);
    break;
case HitType::DisplacedTriangle:
    const DisplacedTriangleHit displacedTriangleHit = hit.getDisplacedTriangleHit();
    instanceID = displacedTriangleHit.instanceID.index;
    geometryID = gScene.getGeometryInstance(displacedTriangleHit.instanceID).geometryID;
    blasID = meshToBlasID[geometryID];
    materialID = gScene.getMaterialID(displacedTriangleHit.instanceID);
    v = gScene.getVertexData(displacedTriangleHit, -dir);
    break;
case HitType::Curve:
    const CurveHit curveHit = hit.getCurveHit();
    instanceID = curveHit.instanceID.index;
    geometryID = gScene.getGeometryInstance(curveHit.instanceID).geometryID;
    materialID = gScene.getMaterialID(curveHit.instanceID);
    v = gScene.getVertexDataFromCurve(curveHit);
    break;
case HitType::SDFGrid:
    const SDFGridHit sdfGridHit = hit.getSDFGridHit();
    instanceID = sdfGridHit.instanceID.index;
    geometryID = gScene.getGeometryInstance(sdfGridHit.instanceID).geometryID;
    materialID = gScene.getMaterialID(sdfGridHit.instanceID);
    v = gScene.getVertexDataFromSDFGrid(sdfGridHit, orig, dir);
    break;
}
```

### Shading Data Preparation Pattern

```cpp
const ShadingData sd = gScene.materials.prepareShadingData(v, materialID, -dir);
```

### Material Instance Creation Pattern

```cpp
let lod = ExplicitLodTextureSampler(0.f);
let mi = gScene.materials.getMaterialInstance(sd, lod);
let bsdfProperties = mi.getProperties(sd);
```

### Visualization Mode Switch Pattern

```cpp
switch ((SceneDebuggerMode)params.mode)
{
case SceneDebuggerMode::FlatShaded:
    return float3(flatShaded);
case SceneDebuggerMode::TriangleDensity:
    return colormapViridis(computeTriangleDensity(hit));
case SceneDebuggerMode::HitType:
    return pseudocolor(uint(hit.getType()));
case SceneDebuggerMode::InstanceID:
    return pseudocolor(instanceID);
case SceneDebuggerMode::MaterialID:
    return pseudocolor(materialID) * flatShaded;
case SceneDebuggerMode::PrimitiveID:
    return pseudocolor(primitiveID);
case SceneDebuggerMode::GeometryID:
    return pseudocolor(geometryID);
case SceneDebuggerMode::BlasID:
    return pseudocolor(blasID);
case SceneDebuggerMode::InstancedGeometry:
    // Instanced geometry visualization
    break;
case SceneDebuggerMode::MaterialType:
    return pseudocolor((uint)sd.mtl.getMaterialType()) * flatShaded;
case SceneDebuggerMode::FaceNormal:
    return remapVector(sd.faceN);
case SceneDebuggerMode::ShadingNormal:
    return remapVector(sd.frame.N);
case SceneDebuggerMode::ShadingTangent:
    return remapVector(sd.frame.T);
case SceneDebuggerMode::ShadingBitangent:
    return remapVector(sd.frame.B);
case SceneDebuggerMode::FrontFacingFlag:
    // Front facing flag visualization
    break;
case SceneDebuggerMode::BackfacingShadingNormal:
    // Back-facing shading normal visualization
    break;
case SceneDebuggerMode::TexCoords:
    return float3(frac(remapVector(sd.uv)), 0.f);
case SceneDebuggerMode::BSDFProperties:
    // BSDF properties visualization
    break;
}
```

### Volume Transmittance Evaluation Pattern

```cpp
float Tr = 1.f;
for (uint i = 0; i < gScene.getGridVolumeCount(); ++i)
{
    GridVolume gridVolume = gScene.getGridVolume(i);
    Tr *= evalGridVolumeTransmittance(gridVolume, pos, dir, 0.f, hitT);
}

return Tr * color;
```

### Ray Marching Pattern

```cpp
const uint kSteps = 500;
float opticalDepth = 0.f;
for (uint step = 0; step < kSteps; ++step)
{
    float t = lerp(nearFar.x, nearFar.y, (step + 0.5f) / kSteps);
    float density = densityGrid.lookupIndex(int3(ipos + t * idir), accessor);
    opticalDepth += density;
}
opticalDepth *= (nearFar.y - nearFar.x) / kSteps * gridVolume.data.densityScale * params.volumeDensityScale;
return exp(-opticalDepth);
```

### Secondary Ray Profiling Pattern

```cpp
SampleGenerator sg = SampleGenerator(pixel, params.frameCount);

const float3 N = sd.getOrientedFaceNormal();
bool valid;
let sf = ShadingFrame::createSafe(N, sd.tangentW, valid);

float c = cos(params.profileSecondaryConeAngle * (M_2PI / 360.f));
float s = sqrt(1.f - c * c);
float z = c + (1.f - c) * sampleNext1D(sg);
float phi = sampleNext1D(sg) * M_2PI;
float3 dir = float3(cos(phi) * s, sin(phi) * s, z);
dir = sf.fromLocal(dir);

const Ray ray = Ray(sd.computeRayOrigin(), dir);

float hitT = 1e30f;
SceneRayQuery<kUseAlphaTest> sceneRayQuery;
HitInfo hit = sceneRayQuery.traceRay(ray, hitT);

float3 color = {};

if (params.profileSecondaryLoadHit)
{
    if (hit.isValid())
    {
        let hitSD = loadHit(ray, hit);
        color = hitSD.frame.N;
    }
}
else
{
    color = hit.isValid() ? float3(0, 1, 0) : float3(1, 0, 0);
}

return color;
```

### Plugin Registration Pattern

```cpp
extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, SceneDebugger>();
    Falcor::ScriptBindings::registerBinding(registerBindings);
}
```

### Scripting Registration Pattern

```cpp
void registerBindings(pybind11::module& m)
{
    pybind11::class_<SceneDebugger, RenderPass, ref<SceneDebugger>> pass(m, "SceneDebugger");
    pass.def_property(
        kMode,
        [](const SceneDebugger& self) { return enumToString(self.getMode()); },
        [](SceneDebugger& self, const std::string& value) { self.setMode(stringToEnum<SceneDebuggerMode>(value)); }
    );
}
```

## Integration Points

### Render Graph Integration

The pass integrates with render graph system:
- Input channels: 1 optional (vbuffer: Visibility buffer in packed format)
- Output channels: 1 required (output: Scene debugger output)
- Reflects input/output resources via `reflect()` method
- Executes scene debugging in `execute()` method
- Supports optional VBuffer input
- Scene change detection (throws if recompilation needed)

### Scene Integration

The pass integrates with scene system:
- Scene reference management
- Scene change tracking with UpdateFlags signal
- Scene shader modules integration
- Type conformances from scene
- Scene defines for shader compilation
- Ray tracing integration with scene ray query
- Vertex data loading for multiple geometry types (Triangle, DisplacedTriangle, Curve, SDFGrid)
- Material instance creation and BSDF property queries
- Mesh to BLAS ID lookup table
- Instance metadata generation
- Scene graph node hierarchy traversal
- Grid volume integration

### PixelDebug Integration

The pass integrates with PixelDebug utility:
- PixelDebug frame management (beginFrame, prepareProgram, endFrame)
- Shader printSetPixel for selected pixel initialization
- Readback with staging buffer and fence synchronization
- Pixel data extraction from shader
- UI delegation for logging controls

### SampleGenerator Integration

The pass integrates with SampleGenerator:
- Sample generator for secondary ray profiling
- Per-pixel random sampling
- Cone-based direction generation

### Volume Integration

The pass integrates with volume system:
- Grid volume iteration
- Volume transmittance evaluation
- Ray marching with density grid access
- Optical depth computation
- Transmittance accumulation

### UI Integration

The pass provides comprehensive UI:
- 18 visualization modes with dropdown
- Parameter controls (checkboxes, sliders)
- Pixel data display with detailed information
- Mesh info display (flags, vertexCount, indexCount, triangleCount, offsets)
- Mesh instance info display (flags, nodeID, meshID, materialID, isDynamic)
- Scene graph display (node hierarchy with local matrices)
- Curve info display (degree, vertexCount, indexCount, offsets)
- SDF grid info display (gridWidth)
- Shading data display (posW, V, N, T, B, uv, faceN, tangentW, frontFacing, curveRadius)
- Material info display (name, materialType, alphaMode, alphaThreshold, nestedPriority, activeLobes, defaultTextureSamplerID, doubleSided, thinSurface, emissive, basicMaterial, lightProfileEnabled, deltaSpecular)
- PixelDebug logging UI delegation
- Profiling group with secondary ray tracing options

### Event Integration

The pass handles events:
- Mouse events for pixel selection
- Keyboard events (always returns false)

## Use Cases

### Asset Issue Identification

Debugging applications:
- Identify incorrect normals
- Debug geometry problems
- Verify material assignments
- Check instance transformations
- Validate scene graph structure
- Analyze hit types
- Verify triangle density

### Geometry Debugging

Geometry debugging applications:
- Verify mesh structure
- Check vertex data
- Validate face normals
- Debug shading normals
- Analyze tangents and bitangents
- Verify front-facing flags
- Check texture coordinates

### Material System Debugging

Material debugging applications:
- Verify material assignments
- Check BSDF properties
- Analyze material types
- Validate material parameters
- Debug emission and roughness
- Check transmission properties

### Scene Graph Debugging

Scene graph debugging applications:
- Verify node hierarchy
- Check instance transformations
- Validate scene graph structure
- Analyze local matrices
- Debug instancing
- Check dynamic instances

### Volume Debugging

Volume debugging applications:
- Verify volume bounds
- Check density grids
- Analyze transmittance
- Debug ray marching
- Validate optical depth
- Check volume density scale

### Performance Profiling

Profiling applications:
- Profile secondary ray tracing
- Analyze cone angle distribution
- Measure hit rates
- Profile ray tracing overhead
- Analyze shading performance
- Benchmark material evaluation

### Research and Development

Research applications:
- Study visualization techniques
- Analyze rendering algorithms
- Research volume rendering
- Investigate material models
- Develop debugging tools
- Study scene graph structures

### Educational Use

Educational applications:
- Demonstrate scene structure
- Show rendering concepts
- Explain geometry types
- Teach material systems
- Visualize debugging techniques
- Explain ray tracing

## Performance Considerations

### GPU Computation

- Primary ray tracing: O(width × height) rays per frame
- Vertex data loading: O(1) per hit
- Shading data preparation: O(1) per hit
- Volume transmittance: O(width × height × volumeCount × steps) per frame
- Secondary ray profiling: O(width × height) rays per frame (optional)
- Total: O(width × height × (1 + volumeCount × steps)) per frame

### Memory Usage

- VBuffer input: O(width × height × sizeof(PackedHitInfo)) bytes (optional)
- Output texture: O(width × height × 16) bytes (RGBA32Float)
- Pixel data buffer: O(sizeof(PixelData)) bytes
- Pixel data staging buffer: O(sizeof(PixelData)) bytes
- Mesh to BLAS ID lookup: O(meshCount × 4) bytes
- Instance info buffer: O(instanceCount × 4) bytes
- Total memory: O(width × height × sizeof(PackedHitInfo) + sizeof(PixelData) + meshCount × 4 + instanceCount × 4) bytes

### Computational Complexity

- Primary ray tracing: O(width × height) per frame
- Hit handling: O(1) per pixel
- Shading data: O(1) per hit
- Volume transmittance: O(width × height × volumeCount × steps) per frame
- Secondary ray profiling: O(width × height) per frame (optional)
- Overall: O(width × height × (1 + volumeCount × steps)) per frame

### Readback Overhead

- Pixel data readback: O(1) per frame
- Fence synchronization: O(1) per frame
- Staging buffer copy: O(1) per frame
- Minimal overhead after synchronization

### Volume Rendering Overhead

- Ray marching steps: 500 steps per pixel
- Density grid lookups: 500 per pixel per volume
- Optical depth computation: 500 operations per pixel per volume
- Transmittance accumulation: O(volumeCount) per pixel

### Secondary Ray Profiling Overhead

- Secondary ray tracing: O(width × height) rays per frame (optional)
- Sample generator overhead: O(1) per pixel
- Shading frame creation: O(1) per pixel
- Hit info loading: O(1) per ray (optional)

## Limitations

### Feature Limitations

- No scene change support (throws if recompilation needed)
- No dynamic scene updates
- Limited to 18 visualization modes
- No custom visualization modes
- No preset management
- No undo/redo functionality
- No real-time preview

### Geometry Limitations

- Limited to triangle, displaced triangle, curve, and SDF grid geometry
- No support for other geometry types
- Triangle density only for triangles
- Instancing visualization only for triangles

### Material Limitations

- Limited to BSDF properties
- No custom material visualization
- No advanced material features
- No material parameter editing

### Volume Limitations

- Limited to grid volumes
- No volumetric primitives
- No volume editing
- Fixed 500 ray marching steps

### Performance Limitations

- High memory usage for buffers
- Volume rendering overhead
- Secondary ray profiling overhead
- Readback overhead
- No performance optimization

### Integration Limitations

- Limited to Falcor's scene system
- Requires VBuffer input (optional)
- Limited to Falcor's material system
- Limited to Falcor's volume system
- No external data sources

### UI Limitations

- Complex UI with many controls
- No preset management
- No undo/redo functionality
- Limited parameter validation
- No real-time preview

## Best Practices

### Visualization Mode Selection

- Choose appropriate mode for debugging
- Use FlatShaded for overall shading
- Use TriangleDensity for triangle analysis
- Use HitType for hit type debugging
- Use InstanceID for instance debugging
- Use MaterialID for material debugging
- Use Shading data modes for normal/tangent analysis
- Use BSDFProperties for material property debugging

### Geometry Debugging

- Verify mesh structure
- Check vertex data
- Validate face normals
- Debug shading normals
- Analyze tangents and bitangents
- Verify front-facing flags
- Check texture coordinates

### Material System Debugging

- Verify material assignments
- Check BSDF properties
- Analyze material types
- Validate material parameters
- Debug emission and roughness
- Check transmission properties

### Scene Graph Debugging

- Verify node hierarchy
- Check instance transformations
- Validate scene graph structure
- Analyze local matrices
- Debug instancing
- Check dynamic instances

### Volume Debugging

- Verify volume bounds
- Check density grids
- Analyze transmittance
- Debug ray marching
- Validate optical depth
- Check volume density scale

### Performance Profiling

- Profile secondary ray tracing
- Analyze cone angle distribution
- Measure hit rates
- Profile ray tracing overhead
- Analyze shading performance
- Benchmark material evaluation

### Research and Development

- Study visualization techniques
- Analyze rendering algorithms
- Research volume rendering
- Investigate material models
- Develop debugging tools
- Study scene graph structures

### Educational Use

- Demonstrate scene structure
- Show rendering concepts
- Explain geometry types
- Teach material systems
- Visualize debugging techniques
- Explain ray tracing

### Integration Best Practices

- Connect to appropriate render graph inputs
- Use correct texture formats
- Handle optional inputs
- Test with different render graphs
- Validate compatibility
- Ensure proper synchronization
- Use PixelDebug for data extraction
- Configure volume density scale appropriately

## Related Components

### RenderGraph/RenderPass

Render pass base class:
- `RenderPass::reflect()` - Define input/output resources
- `RenderPass::execute()` - Execute render pass
- `RenderPass::renderUI()` - Render UI controls
- `RenderPass::getProperties()` - Serialize configuration
- `RenderPass::setScene()` - Set scene reference
- `RenderPass::onMouseEvent()` - Handle mouse events
- `RenderPass::onKeyEvent()` - Handle keyboard events
- `RenderPass::compile()` - Compile pass

### RenderGraph/RenderPassHelpers

Render pass helper utilities:
- `addRenderPassInputs()` - Add input channels
- `addRenderPassOutputs()` - Add output channels

### Core/Program/ComputePass

Compute pass management:
- `ComputePass::create()` - Create compute pass
- `ComputePass::getRootVar()` - Get root variable
- `ComputePass::execute()` - Execute compute pass

### Utils/Debug/PixelDebug

Pixel debug utility:
- `PixelDebug::beginFrame()` - Begin pixel debug frame
- `PixelDebug::prepareProgram()` - Prepare program
- `PixelDebug::endFrame()` - End pixel debug frame
- `PixelDebug::renderUI()` - Render UI

### Utils/Sampling/SampleGenerator

Sample generator for random sampling:
- Provides per-pixel random sampling
- Supports multiple sampling methods

### Scene/Scene

Scene system:
- `Scene::bindShaderDataForRaytracing()` - Bind shader data for ray tracing
- `Scene::getShaderModules()` - Get shader modules
- `Scene::getTypeConformances()` - Get type conformances
- `Scene::getSceneDefines()` - Get scene defines
- `Scene::getUpdateFlagsSignal()` - Get update flags signal
- `Scene::getMeshBlasIDs()` - Get mesh to BLAS IDs
- `Scene::getGeometryInstanceCount()` - Get geometry instance count
- `Scene::getGeometryInstance()` - Get geometry instance
- `Scene::getMesh()` - Get mesh
- `Scene::getCurve()` - Get curve
- `Scene::getSDFGrid()` - Get SDF grid
- `Scene::getGridVolumeCount()` - Get grid volume count
- `Scene::getGridVolume()` - Get grid volume
- `Scene::getMaterial()` - Get material
- `Scene::getAnimationController()` - Get animation controller

### Scene/Materials

Material system:
- `Material::getHeader()` - Get material header
- `Material::getName()` - Get material name
- `MaterialInstance::getProperties()` - Get BSDF properties

### Utils/Math/HashUtils

Hash utilities:
- `jenkinsHash()` - Jenkins hash for pseudocolor

### Utils/Math/ShadingFrame

Shading frame utilities:
- `ShadingFrame::createSafe()` - Create shading frame
- `faceforward()` - Compute forward-facing normal

### Utils/Color/ColorMap

Color map utilities:
- `colormapViridis()` - Viridis color map for triangle density

### Utils/UI/Gui

UI framework for rendering controls:
- Provides checkbox, var, group, text, dropdown, dummy controls
- Simple UI widget interface
- Real-time control updates
