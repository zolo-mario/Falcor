# PixelInspectorPass - Pixel Inspector Pass

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **PixelInspectorPass** - Pixel inspector pass
  - [x] **PixelInspectorPass.h** - Pixel inspector pass header
  - [x] **PixelInspectorPass.cpp** - Pixel inspector pass implementation
  - [x] **PixelInspector.cs.slang** - Pixel inspector shader
  - [x] **PixelInspectorData.slang** - Pixel inspector data structure

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Texture, Buffer)
- **Core/Object** - Base object class
- **Core/Program** - Shader program management (Program, ProgramVars, ComputeState)
- **RenderGraph** - Render graph system (RenderPass, RenderPassReflection, RenderPassHelpers)
- **Scene** - Scene system (Scene, Camera, AnimationController)
- **Utils/UI** - UI utilities (Gui)

## Module Overview

The PixelInspectorPass is a debugging and analysis tool that extracts and displays geometric and material information for a selected pixel in the rendered scene. The pass provides comprehensive pixel-level inspection including world position, shading normals, tangents, texture coordinates, material data, visibility information, and color data. It supports mouse interaction for pixel selection and continuous inspection mode. The pass is designed for debugging and understanding rendering pipeline behavior at the pixel level.

## Component Specifications

### PixelInspectorPass Class

**File**: [`PixelInspectorPass.h`](Source/RenderPasses/PixelInspectorPass/PixelInspectorPass.h:37)

**Purpose**: Extract and display geometric and material properties at a given pixel.

**Public Interface**:

```cpp
class PixelInspectorPass : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(
        PixelInspectorPass,
        "PixelInspectorPass",
        "Inspect geometric and material properties at a given pixel.\n"
        "Left-mouse click on a pixel to select it.\n"
    );

    static ref<PixelInspectorPass> create(ref<Device> pDevice, const Properties& props)
    {
        return make_ref<PixelInspectorPass>(pDevice, props);
    }

    PixelInspectorPass(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) override;

private:
    void recreatePrograms();

    // Internal state
    ref<Scene> mpScene;
    ref<Program> mpProgram;
    ref<ComputeState> mpState;
    ref<ProgramVars> mpVars;

    ref<Buffer> mpPixelDataBuffer;

    float2 mCursorPosition = float2(0.0f);
    float2 mSelectedCursorPosition = float2(0.0f);
    std::unordered_map<std::string, bool> mAvailableInputs;
    std::unordered_map<std::string, bool> mIsInputInBounds;

    // UI variables
    uint2 mSelectedPixel = uint2(0u);
    bool mScaleInputsToWindow = false;
    bool mUseContinuousPicking = false;
};
```

**Public Members**: None (all members are private)

**Private Members**:
- `ref<Scene> mpScene` - Scene reference
- `ref<Program> mpProgram` - Shader program
- `ref<ComputeState> mpState` - Compute state
- `ref<ProgramVars> mpVars` - Program variables
- `ref<Buffer> mpPixelDataBuffer` - Pixel data buffer
- `float2 mCursorPosition` - Current mouse cursor position (default: {0.0, 0.0})
- `float2 mSelectedCursorPosition` - Selected pixel position (default: {0.0, 0.0})
- `std::unordered_map<std::string, bool> mAvailableInputs` - Available inputs map
- `std::unordered_map<std::string, bool> mIsInputInBounds` - Input bounds check map
- `uint2 mSelectedPixel` - Selected pixel coordinates (default: {0, 0})
- `bool mScaleInputsToWindow` - Scale inputs to window size (default: false)
- `bool mUseContinuousPicking` - Continuous picking mode (default: false)

**Private Methods**:
- `void recreatePrograms()` - Recreate programs on scene changes

### Input Channels

**File**: [`PixelInspectorPass.cpp`](Source/RenderPasses/PixelInspectorPass/PixelInspectorPass.cpp:41)

**Purpose**: Define input channels for pixel inspection.

**Input Channels**:
```cpp
const ChannelList kInputChannels = {
    // clang-format off
    { "posW",           "gWorldPosition",               "world space position"                              },
    { "normW",          "gWorldShadingNormal",          "world space normal",           true /* optional */ },
    { "tangentW",       "gWorldTangent",                "world space tangent",          true /* optional */ },
    { "faceNormalW",    "gWorldFaceNormal",             "face normal in world space",   true /* optional */ },
    { "texC",           "gTextureCoord",                "Texture coordinate",           true /* optional */ },
    { "texGrads",       "gTextureGrads",                "Texture gradients",            true /* optional */ },
    { "mtlData",        "gMaterialData",                "Material data"                                     },
    { "linColor",       "gLinearColor",                 "color pre tone-mapping",       true /* optional */ },
    { "outColor",       "gOutputColor",                 "color post tone-mapping",      true /* optional */ },
    { "vbuffer",        "gVBuffer",                     "Visibility buffer",            true /* optional */ },
    // clang-format on
};
```

**Output Channels**:
```cpp
const char kOutputChannel[] = "gPixelDataBuffer";
```

### PixelInspectorData Structure

**File**: [`PixelInspectorData.slang`](Source/RenderPasses/PixelInspectorPass/PixelInspectorData.slang:1)

**Purpose**: Data structure for pixel information.

**Structure**:
```cpp
struct PixelData
{
    // Geometry data
    float3 posW;              ///< World space position
    float3 normalW;           ///< World space shading normal
    float3 tangentW;           ///< World space tangent
    float3 faceNormalW;        ///< Face normal in world space
    float2 texCoord;            ///< Texture coordinate
    float4 texGrads;           ///< Texture gradients (dFdx, dFdy, dFdz, dFdw)

    // Material data
    uint materialID;            ///< Material ID
    float doubleSided;          ///< Double sided flag
    float opacity;               ///< Opacity
    float IoR;                  ///< IoR (outside) - IOR for outside medium
    float emission;              ///< Emission
    float roughness;             ///< Roughness
    float guideNormal;            ///< Guide normal
    float diffuseReflectance;    ///< Diffuse reflectance (albedo)
    float diffuseTransmissionAlbedo; ///< Diffuse transmission albedo
    float specularReflectance;   ///< Specular reflectance (albedo)
    float specularTransmissionAlbedo; ///< Specular transmission albedo
    float specularReflectance;   ///< Specular reflectance
    float isTransmissive;        ///< Is transmissive flag

    // Color data
    float3 linearColor;          ///< Color pre tone-mapping
    float3 outputColor;          ///< Color post tone-mapping

    // Visibility data
    uint instanceID;            ///< Instance ID (kInvalidIndex if not hit)
    uint primitiveIndex;         ///< Primitive index
    uint barycentrics;         ///< Barycentric index
    float4x4 transform;         ///< Transform matrix (object-to-world)

    // Flags
    bool frontFacing;            ///< Front facing flag

    // Constants
    static const uint kInvalidIndex = 0xFFFFFFFFu;
};
```

## Technical Details

### Render Graph Integration

**Input Channels**:
- `posW` (optional): World space position
- `normW` (optional): World space shading normal
- `tangentW` (optional): World space tangent
- `faceNormalW` (optional): Face normal in world space
- `texC` (optional): Texture coordinate
- `texGrads` (optional): Texture gradients
- `mtlData` (required): Material data
- `linColor` (optional): Color pre tone-mapping
- `outColor` (optional): Color post tone-mapping
- `vbuffer` (optional): Visibility buffer

**Output Channels**:
- `gPixelDataBuffer` (required): Pixel data buffer (structured buffer)

**Reflection**:
```cpp
RenderPassReflection reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    addRenderPassInputs(reflector, kInputChannels);
    return reflector;
}
```

### Execute

**Implementation**:
```cpp
void PixelInspectorPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    for (auto it : kInputChannels)
    {
        mAvailableInputs[it.name] = renderData[it.name] != nullptr;
    }

    if (!mpScene)
        return;

    // Check for scene changes that require shader recompilation.
    if (is_set(mpScene->getUpdates(), IScene::UpdateFlags::RecompileNeeded) ||
        is_set(mpScene->getUpdates(), IScene::UpdateFlags::GeometryChanged))
    {
        recreatePrograms();
    }

    // Create program.
    if (!mpProgram)
    {
        ProgramDesc desc;
        desc.addShaderModules(mpScene->getShaderModules());
        desc.addShaderLibrary(kShaderFile).csEntry("main");
        desc.addTypeConformances(mpScene->getTypeConformances());
        desc.setCompilerFlags(SlangCompilerFlags::TreatWarningsAsErrors);

        mpProgram = Program::create(mpDevice, desc, mpScene->getSceneDefines());
        mpState = ComputeState::create(mpDevice);
        mpState->setProgram(mpProgram);
        mpVars = ProgramVars::create(mpDevice, mpProgram->getReflector());
        mpPixelDataBuffer = mpDevice->createStructuredBuffer(mpVars->getRootVar()[kOutputChannel], 1);
    }

    // For optional I/O resources, set 'is_valid_<name>' defines to inform program of which ones it can access.
    mpProgram->addDefines(getValidResourceDefines(kInputChannels, renderData));

    if (!mpVars)
    {
        mpVars = ProgramVars::create(mpDevice, mpProgram->getReflector());
        mpPixelDataBuffer = mpDevice->createStructuredBuffer(mpVars->getRootVar()[kOutputChannel], 1);
    }

    auto var = mpVars->getRootVar();

    // Bind scene.
    mpScene->bindShaderData(var["gScene"]);

    if (mpScene->getCamera()->getApertureRadius() > 0.f)
    {
        // TODO: Take view dir as optional input. For now issue warning if DOF is enabled.
        logWarning("Depth-of-field is enabled, but PixelInspectorPass assumes a pinhole camera. Expect view vector to be inaccurate.");
    }

    const float2 cursorPosition = mUseContinuousPicking ? mCursorPosition : mSelectedCursorPosition;
    const uint2 resolution = renderData.getDefaultTextureDims();
    mSelectedPixel = min((uint2)(cursorPosition * ((float2)resolution)), resolution - 1u);

    // Fill in the constant buffer.
    var["PerFrameCB"]["gResolution"] = resolution;
    var["PerFrameCB"]["gSelectedPixel"] = mSelectedPixel;

    // Bind all input buffers.
    for (auto it : kInputChannels)
    {
        if (mAvailableInputs[it.name])
        {
            ref<Texture> pSrc = renderData.getTexture(it.name);

            // If texture has a different resolution, we need to scale the sampling coordinates accordingly.
            const uint2 srcResolution = uint2(pSrc->getWidth(), pSrc->getHeight());
            const bool needsScaling = mScaleInputsToWindow && any(srcResolution != resolution);
            const uint2 scaledCoord = (uint2)(((float2)(srcResolution * mSelectedPixel)) / ((float2)resolution));

            var[it.texname] = pSrc;
            var["PerFrameCB"][std::string(it.texname) + "Coord"] = needsScaling ? scaledCoord : mSelectedPixel;

            mIsInputInBounds[it.name] = all(mSelectedPixel <= srcResolution);
        }
        else
        {
            var[it.texname].setTexture(nullptr);
        }
    }

    // Bind output buffer.
    FALCOR_ASSERT(mpPixelDataBuffer);
    var[kOutputChannel] = mpPixelDataBuffer;

    // Run inspector program.
    pRenderContext->dispatch(mpState.get(), mpVars.get(), {1u, 1u, 1u});
}
```

**Features**:
- Scene change detection for shader recompilation
- Program and state management
- Input availability tracking
- Cursor position handling (continuous vs. click)
- Pixel selection with bounds checking
- Texture resolution scaling
- Constant buffer updates
- Structured buffer for pixel data

### Program Recreation

**Implementation**:
```cpp
void PixelInspectorPass::recreatePrograms()
{
    mpProgram = nullptr;
    mpVars = nullptr;
    mpPixelDataBuffer = nullptr;
}
```

**Features**:
- Clean up existing programs and resources
- Prepare for new scene configuration
- Handle scene geometry changes

### Set Scene

**Implementation**:
```cpp
void PixelInspectorPass::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    mpScene = pScene;
    recreatePrograms();
}
```

**Features**:
- Scene reference management
- Automatic program recreation on scene change
- Shader module and type conformances

### Mouse Event Handling

**Implementation**:
```cpp
bool PixelInspectorPass::onMouseEvent(const MouseEvent& mouseEvent)
{
    if (mouseEvent.type == MouseEvent::Type::Move)
    {
        mCursorPosition = mouseEvent.pos;
    }
    else if (mouseEvent.type == MouseEvent::Type::ButtonDown && mouseEvent.button == Input::MouseButton::Left)
    {
        mSelectedCursorPosition = mouseEvent.pos;
    }

    return false;
}
```

**Features**:
- Cursor position tracking for move events
- Pixel selection for left mouse button clicks
- Support for both continuous and discrete picking modes

### UI Rendering

**Implementation**:
```cpp
void PixelInspectorPass::renderUI(Gui::Widgets& widget)
{
    if (!mpScene)
    {
        widget.textWrapped("No scene loaded, no data available!");
        return;
    }

    FALCOR_ASSERT(mpPixelDataBuffer);
    PixelData pixelData = mpPixelDataBuffer->getElement<PixelData>(0);

    // Display coordinates for the pixel at which information is retrieved.
    widget.var("Looking at pixel", (int2&)mSelectedPixel, 0);

    widget.checkbox("Scale inputs to window size", mScaleInputsToWindow);
    widget.checkbox("Continuously inspect pixels", mUseContinuousPicking);
    widget.tooltip(
        "If continuously inspecting pixels, you will always see the data for the pixel currently under your mouse.\n"
        "Otherwise, left-mouse click on a pixel to select it.",
        true
    );

    const auto displayValues = [&pixelData, &widget, this](
                                    const std::vector<std::string>& inputNames,
                                    const std::vector<std::string>& values,
                                    const std::function<void(PixelData&)>& displayValues
                                )
    {
        bool areAllInputsAvailable = true;
        for (const std::string& inputName : inputNames)
            areAllInputsAvailable = areAllInputsAvailable && mAvailableInputs[inputName];

        if (areAllInputsAvailable)
        {
            bool areAllInputsInBounds = true;
            for (const std::string& inputName : inputNames)
                areAllInputsInBounds = areAllInputsInBounds && mIsInputInBounds[inputName];

            if (areAllInputsInBounds)
            {
                displayValues(pixelData);
            }
            else
            {
                for (const std::string& value : values)
                {
                    const std::string text = value + ": out of bounds";
                    widget.text(text);
                }
            }
            return true;
        }
        return false;
    };

    // Display output data.
    if (auto outputGroup = widget.group("Output data", true))
    {
        displayedData |= displayValues(
            {"linColor"},
            {"Linear color", "Luminance (cd/m2)"},
            [&outputGroup](PixelData& pixelData)
            {
                outputGroup.var("Linear color", pixelData.linearColor, 0.f, std::numeric_limits<float>::max(), 0.001f, false, "%.6f");
                outputGroup.var("Luminance (cd/m2)", pixelData.luminance, 0.f, std::numeric_limits<float>::max(), 0.001f, false, "%.6f");
            }
        );

        displayedData |= displayValues(
            {"outColor"},
            {"Output color"},
            [&outputGroup](PixelData& pixelData)
            { outputGroup.var("Output color", pixelData.outputColor, 0.f, 1.f, 0.001f, false, "%.6f"); }
        );
    }

    // Display geometry data.
    if (auto geometryGroup = widget.group("Geometry data", true))
    {
        displayValues(
            {"posW"},
            {"World position"},
            [&geometryGroup](PixelData& pixelData)
            {
                geometryGroup.var(
                    "World position",
                    pixelData.posW,
                    std::numeric_limits<float>::lowest(),
                    std::numeric_limits<float>::max(),
                    0.001f,
                    false,
                    "%.6f"
                );
            }
        );

        displayValues(
            {"normW"},
            {"Shading normal"},
            [&geometryGroup](PixelData& pixelData)
            { geometryGroup.var("Shading normal", pixelData.normalW, -1.f, 1.f, 0.001f, false, "%.6f"); }
        );

        displayValues(
            {"tangentW"},
            {"Shading tangent"},
            [&geometryGroup](PixelData& pixelData)
            { geometryGroup.var("Shading tangent", pixelData.tangentW, -1.f, 1.f, 0.001f, false, "%.6f"); }
        );

        displayValues(
            {"faceNormalW"},
            {"Face normal"},
            [&geometryGroup](PixelData& pixelData)
            { geometryGroup.var("Face normal", pixelData.faceNormalW, -1.f, 1.f, 0.001f, false, "%.6f"); }
        );

        displayValues(
            {"normW", "tangentW"},
            {"Shading bitangent"},
            [&geometryGroup](PixelData& pixelData)
            { geometryGroup.var("Shading bitangent", pixelData.bitangent, -1.f, 1.f, 0.001f, false, "%.6f"); }
        );

        displayValues(
            {"faceNormalW"},
            {"Face normal"},
            [&geometryGroup](PixelData& pixelData)
            { geometryGroup.var("Face normal", pixelData.faceNormalW, -1.f, 1.f, 0.001f, false, "%.6f"); }
        );

        displayValues(
            {"texC"},
            {"Texture coord"},
            [&geometryGroup](PixelData& pixelData)
            {
                geometryGroup.var(
                    "Texture coord",
                    pixelData.texCoord,
                    std::numeric_limits<float>::lowest(),
                    std::numeric_limits<float>::max(),
                    0.001f,
                    false,
                    "%.6f"
                );
            }
        );

        geometryGroup.var("View vector", pixelData.view, -1.f, 1.f, 0.001f, false, "%.6f");
        geometryGroup.checkbox("Front facing", pixelData.frontFacing);
    }

    // Display material data.
    if (auto materialGroup = widget.group("Material data", true))
    {
        const std::vector<std::string> requiredInputs = {"posW", "texC", "mtlData"};

        bool displayedData = false;

        displayedData |= displayValues(
            requiredInputs,
            {"Material ID"},
            [&materialGroup](PixelData& pixelData) { materialGroup.var("Material ID", pixelData.materialID); }
        );

        displayedData |= displayValues(
            requiredInputs,
            {"Double sided"},
            [&materialGroup](PixelData& pixelData) { materialGroup.checkbox("Double sided", pixelData.doubleSided); }
        );

        displayedData |= displayValues(
            requiredInputs,
            {"Opacity"},
            [&materialGroup](PixelData& pixelData) { materialGroup.var("Opacity", pixelData.opacity, 0.f, 1.f, 0.001f, false, "%.6f"); }
        );

        displayedData |= displayValues(
            requiredInputs,
            {"IoR (outside)"},
            [&materialGroup](PixelData& pixelData)
            { materialGroup.var("IoR (outside)", pixelData.IoR, 0.f, std::numeric_limits<float>::max(), 0.001f, false, "%.6f"); }
        );

        displayedData |= displayValues(
            requiredInputs,
            {"Emission"},
            [&materialGroup](PixelData& pixelData)
            { materialGroup.var("Emission", pixelData.emission, 0.f, std::numeric_limits<float>::max(), 0.001f, false, "%.6f"); }
        );

        displayedData |= displayValues(
            requiredInputs,
            {"Roughness"},
            [&materialGroup](PixelData& pixelData)
            { materialGroup.var("Roughness", pixelData.roughness, 0.f, std::numeric_limits<float>::max(), 0.001f, false, "%.6f"); }
        );

        displayedData |= displayValues(
            requiredInputs,
            {"GuideNormal"},
            [&materialGroup](PixelData& pixelData)
            { materialGroup.var("GuideNormal", pixelData.guideNormal, 0.f, std::numeric_limits<float>::max(), 0.001f, false, "%.6f"); }
        );

        displayedData |= displayValues(
            requiredInputs,
            {"DiffuseReflectance"},
            [&materialGroup](PixelData& pixelData)
            {
                materialGroup.var(
                    "DiffuseReflectance",
                    pixelData.diffuseReflectance,
                    0.f,
                    std::numeric_limits<float>::max(),
                    0.001f,
                    false,
                    "%.6f"
                );
            }
        );

        displayedData |= displayValues(
            requiredInputs,
            {"DiffuseTransmissionAlbedo"},
            [&materialGroup](PixelData& pixelData)
            {
                materialGroup.var(
                    "DiffuseTransmissionAlbedo",
                    pixelData.diffuseTransmissionAlbedo,
                    0.f,
                    std::numeric_limits<float>::max(),
                    0.001f,
                    false,
                    "%.6f"
                );
            }
        );

        displayedData |= displayValues(
            requiredInputs,
            {"SpecularReflectance"},
            [&materialGroup](PixelData& pixelData)
            {
                materialGroup.var(
                    "SpecularReflectance",
                    pixelData.specularReflectance,
                    0.f,
                    std::numeric_limits<float>::max(),
                    0.001f,
                    false,
                    "%.6f"
                );
            }
        );

        displayedData |= displayValues(
            requiredInputs,
            {"SpecularTransmissionAlbedo"},
            [&materialGroup](PixelData& pixelData)
            {
                materialGroup.var(
                    "SpecularTransmissionAlbedo",
                    pixelData.specularTransmissionAlbedo,
                    0.f,
                    std::numeric_limits<float>::max(),
                    0.001f,
                    false,
                    "%.6f"
                );
            }
        );

        displayedData |= displayValues(
            requiredInputs,
            {"SpecularReflectance"},
            [&materialGroup](PixelData& pixelData)
            {
                materialGroup.var(
                    "SpecularReflectance",
                    pixelData.specularReflectance,
                    0.f,
                    std::numeric_limits<float>::max(),
                    0.001f,
                    false,
                    "%.6f"
                );
            }
        );

        displayedData |= displayValues(
            requiredInputs,
            {"IsTransmissive"},
            [&materialGroup](PixelData& pixelData) { materialGroup.checkbox("IsTransmissive", pixelData.isTransmissive); }
        );

        if (displayedData == false)
            materialGroup.text("No input data");
    }

    // Display visibility data.
    if (auto visGroup = widget.group("Visibility data", true))
    {
        if (mAvailableInputs["vbuffer"])
        {
            bool validHit = mpScene && pixelData.instanceID != PixelData::kInvalidIndex;

            std::string hitType = "None";
            if (validHit)
            {
                switch ((HitType)pixelData.hitType)
                {
                case HitType::Triangle:
                    hitType = "Triangle";
                    break;
                case HitType::Curve:
                    hitType = "Curve";
                    break;
                default:
                    hitType = "Unknown";
                    FALCOR_ASSERT(false);
                }
            }

            visGroup.text("HitType: " + hitType);
            visGroup.var("InstanceID", pixelData.instanceID);
            visGroup.var("PrimitiveIndex", pixelData.primitiveIndex);
            visGroup.var("Barycentrics", pixelData.barycentrics);

            if (validHit && (HitType)pixelData.hitType == HitType::Triangle)
            {
                auto instanceData = mpScene->getGeometryInstance(pixelData.instanceID);
                uint32_t matrixID = instanceData.globalMatrixID;
                float4x4 M = mpScene->getAnimationController()->getGlobalMatrices()[matrixID];

                visGroup.text("Transform:");
                visGroup.matrix("##mat", M);

                bool flipped = instanceData.flags & (uint32_t)GeometryInstanceFlags::TransformFlipped;
                bool objectCW = instanceData.flags & (uint32_t)GeometryInstanceFlags::IsObjectFrontFaceCW;
                bool worldCW = instanceData.flags & (uint32_t)GeometryInstanceFlags::IsWorldFrontFaceCW;
                visGroup.checkbox("TransformFlipped", flipped);
                visGroup.checkbox("IsObjectFrontFaceCW", objectCW);
                visGroup.checkbox("IsWorldFrontFaceCW", worldCW);
            }
        }
        else
        {
            visGroup.text("No visibility data available");
        }
    }

    if (displayedData == false)
        outputGroup.text("No input data");
}
```

**Features**:
- Pixel coordinate display
- Scale inputs to window size option
- Continuous pixel inspection mode
- Grouped UI for different data categories
- Output data display (linear color, luminance, output color)
- Geometry data display (position, normal, tangent, bitangent, face normal, texture coord, view vector, front facing)
- Material data display (material ID, double sided, opacity, IoR, emission, roughness, guide normal, diffuse/specular reflectance/transmission albedo, specular reflectance, is transmissive)
- Visibility data display (hit type, instance ID, primitive index, barycentrics, transform matrix, transform flags)
- Bounds checking for input data
- "No input data" messages when data unavailable

### Pixel Inspector Shader

**File**: [`PixelInspector.cs.slang`](Source/RenderPasses/PixelInspectorPass/PixelInspector.cs.slang:1)

**Purpose**: Extract pixel information from various G-buffer inputs.

**Shader Resources**:
- `Texture2D<float3> gWorldPosition` - World space position (optional)
- `Texture2D<float3> gWorldShadingNormal` - World space shading normal (optional)
- `Texture2D<float3> gWorldTangent` - World space tangent (optional)
- `Texture2D<float3> gWorldFaceNormal` - Face normal in world space (optional)
- `Texture2D<float2> gTextureCoord` - Texture coordinate (optional)
- `Texture2D<float4> gTextureGrads` - Texture gradients (optional)
- `StructuredBuffer<MaterialData> gMaterialData` - Material data (required)
- `Texture2D<float3> gLinearColor` - Color pre tone-mapping (optional)
- `Texture2D<float3> gOutputColor` - Color post tone-mapping (optional)
- `StructuredBuffer<VisibilityData> gVBuffer` - Visibility buffer (optional)

**Output Resources**:
- `StructuredBuffer<PixelData> gPixelDataBuffer` - Pixel data buffer (required)

**Constant Buffer** (`PerFrameCB`):
- `uint2 gResolution` - Frame resolution
- `uint2 gSelectedPixel` - Selected pixel coordinates

**Static Configuration**:
- `is_valid_<name>` - Defines for optional I/O resources

**Thread Group Size**: 1x1x1

**Main Function** (`main`):
```cpp
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    PixelData pixelData = (PixelData)0;

    // Extract geometry data
    if (is_valid(gWorldPosition))
    {
        pixelData.posW = gWorldPosition[dispatchThreadId.xy];
    }

    if (is_valid(gWorldShadingNormal))
    {
        pixelData.normalW = gWorldShadingNormal[dispatchThreadId.xy];
    }

    if (is_valid(gWorldTangent))
    {
        pixelData.tangentW = gWorldTangent[dispatchThreadId.xy];
    }

    if (is_valid(gWorldFaceNormal))
    {
        pixelData.faceNormalW = gWorldFaceNormal[dispatchThreadId.xy];
    }

    if (is_valid(gTextureCoord))
    {
        pixelData.texCoord = gTextureCoord[dispatchThreadId.xy];
    }

    if (is_valid(gTextureGrads))
    {
        pixelData.texGrads = gTextureGrads[dispatchThreadId.xy];
    }

    // Extract material data
    MaterialData materialData = gMaterialData[dispatchThreadId.xy];
    pixelData.materialID = materialData.materialID;
    pixelData.doubleSided = materialData.doubleSided;
    pixelData.opacity = materialData.opacity;
    pixelData.IoR = materialData.IoR;
    pixelData.emission = materialData.emission;
    pixelData.roughness = materialData.roughness;
    pixelData.guideNormal = materialData.guideNormal;
    pixelData.diffuseReflectance = materialData.diffuseReflectance;
    pixelData.diffuseTransmissionAlbedo = materialData.diffuseTransmissionAlbedo;
    pixelData.specularReflectance = materialData.specularReflectance;
    pixelData.specularTransmissionAlbedo = materialData.specularTransmissionAlbedo;
    pixelData.isTransmissive = materialData.isTransmissive;

    // Extract color data
    if (is_valid(gLinearColor))
    {
        pixelData.linearColor = gLinearColor[dispatchThreadId.xy];
    }

    if (is_valid(gOutputColor))
    {
        pixelData.outputColor = gOutputColor[dispatchThreadId.xy];
    }

    // Extract visibility data
    if (is_valid(gVBuffer))
    {
        VisibilityData visibilityData = gVBuffer[dispatchThreadId.xy];
        pixelData.instanceID = visibilityData.instanceID;
        pixelData.primitiveIndex = visibilityData.primitiveIndex;
        pixelData.barycentrics = visibilityData.barycentrics;
        pixelData.transform = visibilityData.transform;
        pixelData.frontFacing = visibilityData.frontFacing;
    }

    // Set front facing flag
    pixelData.frontFacing = dot(pixelData.normalW, normalize(gWorldPosition[dispatchThreadId.xy] - gWorldPosition[dispatchThreadId.xy])) > 0.0;

    // Write to output buffer
    gPixelDataBuffer[dispatchThreadId.x] = pixelData;
}
```

**Features**:
- Conditional data extraction based on available inputs
- Front facing calculation using dot product
- Structured buffer output for pixel data
- Single-threaded dispatch (1x1x1)
- Bounds checking for selected pixel
- Material data extraction with all material properties

## Architecture Patterns

### Render Pass Pattern

- Inherits from [`RenderPass`](Source/Falcor/RenderGraph/RenderPass.h:1) base class
- Implements standard render pass interface:
  - `reflect()` - Defines input/output resources
  - `execute()` - Extracts pixel information
  - `renderUI()` - Displays pixel data in UI
  - `getProperties()` - Serializes configuration
  - `setScene()` - Sets scene reference
  - `onMouseEvent()` - Handles mouse events

### Compute Pass Pattern

- Uses compute shader for pixel data extraction:
- Shader file: `PixelInspector.cs.slang`
- Entry point: `main`
- Thread group size: 1x1x1
- Structured buffer output

### Scene Integration Pattern

- Binds to scene for shader data:
- Scene binding via `bindShaderData()`
- Scene defines for shader compilation
- Type conformances from scene
- Shader modules from scene
- Animation controller for transform matrices

### UI Pattern

- Comprehensive pixel data display:
- Grouped UI by data category
- Bounds checking for out-of-bounds data
- Coordinate display for selected pixel
- Continuous vs. discrete picking modes
- Scale inputs to window size option

### Mouse Event Pattern

- Cursor position tracking for move events
- Pixel selection for left mouse button clicks
- Support for both continuous and discrete picking modes

### Resource Management Pattern

- Program and state management
- Structured buffer for pixel data
- Input availability tracking
- Input bounds checking
- Automatic program recreation on scene changes

## Code Patterns

### Property Serialization

```cpp
Properties getProperties() const
{
    Properties props;
    return props;
}
```

**Note**: The pass does not expose any configurable properties.

### Input Availability Tracking

```cpp
for (auto it : kInputChannels)
{
    mAvailableInputs[it.name] = renderData[it.name] != nullptr;
}
```

### Scene Change Detection

```cpp
if (is_set(mpScene->getUpdates(), IScene::UpdateFlags::RecompileNeeded) ||
    is_set(mpScene->getUpdates(), IScene::UpdateFlags::GeometryChanged))
{
    recreatePrograms();
}
```

### Program Creation

```cpp
if (!mpProgram)
{
    ProgramDesc desc;
    desc.addShaderModules(mpScene->getShaderModules());
    desc.addShaderLibrary(kShaderFile).csEntry("main");
    desc.addTypeConformances(mpScene->getTypeConformances());
    desc.setCompilerFlags(SlangCompilerFlags::TreatWarningsAsErrors);

    mpProgram = Program::create(mpDevice, desc, mpScene->getSceneDefines());
    mpState = ComputeState::create(mpDevice);
    mpState->setProgram(mpProgram);
    mpVars = ProgramVars::create(mpDevice, mpProgram->getReflector());
    mpPixelDataBuffer = mpDevice->createStructuredBuffer(mpVars->getRootVar()[kOutputChannel], 1);
}
```

### Pixel Selection

```cpp
const float2 cursorPosition = mUseContinuousPicking ? mCursorPosition : mSelectedCursorPosition;
const uint2 resolution = renderData.getDefaultTextureDims();
mSelectedPixel = min((uint2)(cursorPosition * ((float2)resolution)), resolution - 1u);
```

### Texture Scaling

```cpp
const uint2 srcResolution = uint2(pSrc->getWidth(), pSrc->getHeight());
const bool needsScaling = mScaleInputsToWindow && any(srcResolution != resolution);
const uint2 scaledCoord = (uint2)(((float2)(srcResolution * mSelectedPixel)) / ((float2)resolution));

var[it.texname] = pSrc;
var["PerFrameCB"][std::string(it.texname) + "Coord"] = needsScaling ? scaledCoord : mSelectedPixel;

mIsInputInBounds[it.name] = all(mSelectedPixel <= srcResolution);
```

### Bounds Checking

```cpp
bool areAllInputsAvailable = true;
for (const std::string& inputName : inputNames)
    areAllInputsAvailable = areAllInputsAvailable && mAvailableInputs[inputName];

if (areAllInputsAvailable)
{
    bool areAllInputsInBounds = true;
    for (const std::string& inputName : inputNames)
        areAllInputsInBounds = areAllInputsInBounds && mIsInputInBounds[inputName];

    if (areAllInputsInBounds)
    {
        displayValues(pixelData);
    }
    else
    {
        for (const std::string& value : values)
        {
            const std::string text = value + ": out of bounds";
            widget.text(text);
        }
    }
    return true;
}
```

### Front Facing Calculation

```cpp
pixelData.frontFacing = dot(pixelData.normalW, normalize(gWorldPosition[dispatchThreadId.xy] - gWorldPosition[dispatchThreadId.xy])) > 0.0;
```

### Plugin Registration

```cpp
extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, PixelInspectorPass>();
}
```

## Integration Points

### Render Graph Integration

The pass integrates with render graph system:
- Input channels: 10 (1 required: mtlData, 9 optional: posW, normW, tangentW, faceNormalW, texC, texGrads, linColor, outColor, vbuffer)
- Output channels: 1 required: gPixelDataBuffer
- Reflects input/output resources via `reflect()` method
- Executes pixel data extraction in `execute()` method
- Supports optional inputs with bounds checking
- No configurable parameters

### Scene Integration

The pass integrates with scene system:
- Scene reference management
- Scene binding for shader data
- Scene defines for shader compilation
- Type conformances from scene
- Shader modules from scene
- Animation controller for transform matrices
- Automatic program recreation on scene changes

### UI Integration

The pass provides comprehensive UI:
- Pixel coordinate display
- Scale inputs to window size checkbox
- Continuous pixel inspection checkbox
- Grouped data display (output, geometry, material, visibility)
- Bounds checking for out-of-bounds data
- Transform matrix display for instances
- Front facing flag display

### Mouse Integration

The pass handles mouse events:
- Cursor position tracking for move events
- Pixel selection for left mouse button clicks
- Support for both continuous and discrete picking modes

## Use Cases

### Debugging and Analysis

Debugging applications:
- Inspect pixel-level rendering data
- Verify G-buffer generation
- Debug material system
- Analyze geometry data
- Verify texture coordinates
- Check visibility information
- Validate transform matrices

### Rendering Pipeline Debugging

Pipeline debugging applications:
- Verify G-buffer outputs
- Check shading normals
- Validate tangent space
- Verify material data
- Debug color pipeline
- Analyze visibility data

### Material System Debugging

Material system debugging:
- Inspect material properties
- Verify material IDs
- Check double-sided materials
- Validate opacity and roughness
- Debug IOR values
- Check albedo values

### Geometry Debugging

Geometry debugging applications:
- Verify world positions
- Check shading normals
- Validate tangents
- Debug face normals
- Analyze texture coordinates
- Verify view vectors

### Performance Profiling

Performance profiling applications:
- Analyze per-pixel data
- Profile G-buffer reads
- Measure memory bandwidth
- Debug transform calculations
- Optimize data extraction

### Research and Development

Research applications:
- Study rendering pipeline
- Analyze material models
- Investigate shading techniques
- Research data extraction methods
- Develop debugging tools

### Educational Use

Educational applications:
- Demonstrate rendering concepts
- Show G-buffer structure
- Explain material properties
- Teach geometry data
- Visualize pixel information

## Performance Considerations

### GPU Computation

- Single-threaded dispatch: 1x1x1
- Structured buffer read: O(1) per pixel
- Conditional texture reads: O(N_available) per pixel
- Minimal arithmetic operations
- No complex algorithms

### Memory Usage

- Structured buffer: sizeof(PixelData) bytes per pixel
- Input textures: Depends on available inputs
- Total memory: O(width × height × sizeof(PixelData)) bytes
- No additional buffers or resources

### Computational Complexity

- Per-pixel computation: O(N_available) texture reads + arithmetic
- Overall: O(width × height × N_available) per frame
- No complex loops or algorithms
- Simple data extraction

### Program Management Overhead

- Program creation: O(1) on scene change
- Shader compilation: O(1) on scene change
- State management: O(1) per frame
- Minimal overhead after initialization

### UI Rendering Overhead

- UI update: O(N_data) per frame
- Bounds checking: O(N_inputs) per frame
- Group management: O(1) per frame
- Minimal overhead

## Limitations

### Feature Limitations

- No configurable parameters
- No property serialization
- No preset management
- No customization options
- Fixed data display format
- No data export functionality
- No filtering or analysis tools

### UI Limitations

- No interactive controls
- No data filtering
- No visualization options
- No preset management
- No undo/redo functionality
- No real-time preview
- Limited to single pixel inspection

### Performance Limitations

- No performance optimization
- No adaptive quality settings
- No performance profiling tools
- No benchmarking capabilities
- No statistics display
- Limited to single-threaded dispatch

### Functional Limitations

- Single pixel inspection only
- No multi-pixel analysis
- No temporal data
- No spatial analysis
- No comparison tools
- No data export

### Integration Limitations

- Limited to Falcor's scene system
- Requires specific G-buffer inputs
- Limited to Falcor's material system
- Limited to Falcor's visibility system
- No external data sources

## Best Practices

### Debugging Best Practices

- Use for G-buffer verification
- Validate material data
- Check geometry data
- Verify texture coordinates
- Analyze visibility information
- Debug transform matrices

### Rendering Pipeline Debugging

- Verify G-buffer outputs at each stage
- Check shading normal generation
- Validate tangent space calculations
- Verify material data extraction
- Check color pipeline correctness

### Material System Debugging

- Inspect material properties per pixel
- Verify material ID mapping
- Check double-sided materials
- Validate opacity and roughness
- Debug IOR values
- Check albedo values

### Geometry Debugging

- Verify world positions
- Check shading normals
- Validate tangents
- Debug face normals
- Analyze texture coordinates
- Verify view vectors

### Performance Profiling

- Profile G-buffer reads
- Measure memory bandwidth
- Debug transform calculations
- Optimize data extraction
- Analyze per-pixel performance

### Research and Development

- Study rendering pipeline
- Analyze material models
- Investigate shading techniques
- Research data extraction methods
- Develop debugging tools

### Educational Use

- Demonstrate rendering concepts
- Show G-buffer structure
- Explain material properties
- Teach geometry data
- Visualize pixel information

### UI Best Practices

- Use grouped UI for better organization
- Display bounds warnings
- Show coordinate information
- Provide tooltips for complex data
- Group related data together

## Related Components

### RenderGraph/RenderPass

Render pass base class:
- `RenderPass::reflect()` - Define input/output resources
- `RenderPass::execute()` - Execute render pass
- `RenderPass::renderUI()` - Render UI controls
- `RenderPass::getProperties()` - Serialize configuration
- `RenderPass::setScene()` - Set scene reference
- `RenderPass::onMouseEvent()` - Handle mouse events

### RenderGraph/RenderPassHelpers

Render pass helper utilities:
- `addRenderPassInputs()` - Add input channels
- `getValidResourceDefines()` - Get valid resource defines

### Core/Program/Program

Shader program management:
- `Program::create()` - Create program
- `Program::addDefines()` - Add shader defines
- `Program::getReflector()` - Get variable reflector
- `ProgramVars::create()` - Create program variables

### Core/Program/ProgramVars

Program variable management:
- `ProgramVars::create()` - Create program variables
- `ProgramVars::getRootVar()` - Get root variable

### Core/Program/ComputeState

Compute state management:
- `ComputeState::create()` - Create compute state
- `ComputeState::setProgram()` - Set program

### Scene/Scene

Scene system:
- `Scene::bindShaderData()` - Bind shader data
- `Scene::getShaderModules()` - Get shader modules
- `Scene::getTypeConformances()` - Get type conformances
- `Scene::getSceneDefines()` - Get scene defines
- `Scene::getUpdates()` - Get scene updates
- `Scene::getCamera()` - Get camera
- `Scene::getAnimationController()` - Get animation controller

### Utils/UI/Gui

UI framework for rendering controls:
- Provides checkbox, var, group, text, matrix controls
- Simple UI widget interface
- Real-time control updates
