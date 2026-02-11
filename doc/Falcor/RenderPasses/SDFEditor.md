# SDFEditor - Signed Distance Function Editor

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **SDFEditor** - SDF editor pass
  - [x] **SDFEditor.h** - SDF editor header (253 lines)
  - [x] **SDFEditor.cpp** - SDF editor implementation (1736 lines)

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Texture, Buffer, Fbo, Fence)
- **Core/Object** - Base object class
- **Core/Pass/FullScreenPass** - Full screen pass for 2D GUI rendering
- **Core/Program** - Shader program management (Program, ProgramVars)
- **RenderGraph** - Render graph system (RenderPass, RenderPassReflection, RenderPassHelpers)
- **Scene** - Scene system (Scene, Camera, GeometryInstance, AnimationController, SDFGrid)
- **Scene/SDFs** - SDF grid system (SDF3DPrimitiveFactory, SDFGrid)
- **Utils/UI** - UI utilities (Gui)
- **Utils/Math** - Mathematical utilities (math, quaternion, matrix)
- **Utils/Sampling** - Sampling utilities (SampleGenerator)
- **Utils/Debug** - Debug utilities (PixelDebug)

## Module Overview

The SDFEditor is a comprehensive 2D editor for creating and editing signed distance function (SDF) grids. It provides a full-featured 2D GUI for visualizing and manipulating SDF primitives, with support for multiple shape types, transformations, grid plane manipulation, and symmetry operations. The editor includes features like picking, selection, undo/redo, baking, and export capabilities. It integrates with the scene's SDFGrid system and provides extensive keyboard shortcuts and UI controls for efficient editing workflows.

## Component Specifications

### SDFEditor Class

**File**: [`SDFEditor.h`](Source/RenderPasses/SDFEditor/SDFEditor.h:38)

**Purpose**: Signed distance function (SDF) editor.

**Public Interface**:

```cpp
class SDFEditor : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(SDFEditor, "SDFEditor", "Signed distance function (SDF) editor");

    static ref<SDFEditor> create(ref<Device> pDevice, const Properties& props) { return make_ref<SDFEditor>(pDevice, props); }

    SDFEditor(ref<Device> pDevice, const Properties& props);

    Properties getProperties() const override;
    RenderPassReflection reflect(const CompileData& compileData) override;
    void compile(RenderContext* pRenderContext, const CompileData& compileData) override {}
    void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    void renderUI(RenderContext* pRenderContext, Gui::Widgets& widget) override;
    void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;

    // Scripting functions
    static void registerBindings(pybind11::module& m);

private:
    enum class TransformationState
    {
        None,
        Translating,
        Rotating,
        Scaling
    };

    struct KeyboardButtonsPressed
    {
        bool undo = false;
        bool redo = false;
        bool shift = false;
        bool control = false;
        bool prevShift = false;
        bool prevControl = false;

        void registerCurrentStatesIntoPrevious()
        {
            prevShift = shift;
            prevControl = control;
        }
    };

    struct UI2D
    {
        bool recordStartingMousePos = false;
        float scrollDelta = 0.0f;
        KeyboardButtonsPressed keyboardButtonsPressed;
        float2 startMousePosition = {0.0f, 0.0f};
        float2 currentMousePosition = {0.0f, 0.0f};
        float2 prevMousePosition = {0.0f, 0.0f};
        CpuTimer timer;
        CpuTimer::TimePoint timeOfReleaseMainGUIKey;
        bool fadeAwayGUI = false;
        bool drawCurrentModes = true;
        std::unique_ptr<Marker2DSet> pMarker2DSet;
        std::unique_ptr<SelectionWheel> pSelectionWheel;
        float currentBlobbing = 0.0f;
        SDF3DShapeType currentEditingShape = SDF3DShapeType::Sphere;
        SDFOperationType currentEditingOperator = SDFOperationType::Union;
        SDFBBRenderSettings bbRenderSettings;
        SDFGridPlane previousGridPlane;
        SDFGridPlane gridPlane;
        SDFGridPlane symmetryPlane;
    };

    struct CurrentEdit
    {
        uint32_t instanceID;
        SdfGridID gridID;
        ref<SDFGrid> pSDFGrid;
        SDF3DPrimitive primitive;
        SDF3DPrimitive symmetryPrimitive;
        uint32_t primitiveID = UINT32_MAX;
        uint32_t symmetryPrimitiveID = UINT32_MAX;
    };

    struct SDFEdit
    {
        SdfGridID gridID;
        uint32_t primitiveID;
    };

    struct UndoneSDFEdit
    {
        SdfGridID gridID;
        SDF3DPrimitive primitive;
    };

    // 2D GUI functions.
    void setup2DGUI();
    bool isMainGUIKeyDown() const;
    void setupPrimitiveAndOperation(
        const float2& center,
        const float markerSize,
        const SDF3DShapeType editingPrimitive,
        const SDFOperationType editingOperator,
        const float4& color,
        const float alpha = 1.0f
    );
    void setupCurrentModes2D();
    void manipulateGridPlane(
        SDFGridPlane& gridPlane,
        SDFGridPlane& previousGridPlane,
        bool isTranslationKeyDown,
        bool isConstrainedManipulationKeyDown
    );
    void rotateGridPlane(
        const float mouseDiff,
        const float3& rotationVector,
        const float3& inNormal,
        const float3& inRightVector,
        float3& outNormal,
        float3& outRightVector,
        const bool fromPreviousMouse = true
    );
    void translateGridPlane(const float mouseDiff, const float3& translationVector, const float3& inPosition, float3& outPosition);
    bool gridPlaneManipulated() const;
    bool symmetryPlaneManipulated() const;

    // Editing functions
    void updateEditShapeType();
    void updateEditOperationType();
    void updateSymmetryPrimitive();
    void addEditPrimitive(bool addToCurrentEdit, bool addToHistory);
    void removeEditPrimitives();
    void updateEditPrimitives();

    // Input and actions
    void handleActions();
    void handleToggleSymmetryPlane();
    void handleToggleEditing();
    void handleEditMovement();
    void handleAddPrimitive();
    bool handlePicking(const float2& currentMousePos, float3& p);
    uint32_t calcPrimitivesAffectedCount(uint32_t keyPressedCount);
    void handleUndo();
    void handleRedo();

    // Baking
    void bakePrimitives();

    // Input and actions
    void bindShaderData(const ShaderVar& var, const ref<Texture>& pInputColor, const ref<Texture>& pVBuffer);
    void fetchPreviousVBufferAndZBuffer(RenderContext* pRenderContext, ref<Texture>& pVBuffer, ref<Texture>& pDepth);

private:
    // Internal state

    ref<Scene> mpScene;                  ///< The current scene.
    ref<Camera> mpCamera;                ///< The camera.
    ref<FullScreenPass> mpGUIPass;       ///< A full screen pass drawing the 2D GUI.
    ref<Fbo> mpFbo;                      ///< Frame buffer object.
    ref<Texture> mpEditingVBuffer;       ///< A copy of VBuffer used while moving/adding a primitive.
    ref<Texture> mpEditingLinearZBuffer; ///< A copy of linear Z buffer used while moving/adding a primitive.
    ref<Buffer> mpSDFEditingDataBuffer;  ///< A buffer that contain current Edit data for GUI visualization.
    ref<Buffer> mpPickingInfo;         ///< Buffer for reading back picking info from GPU.
    ref<Buffer> mpPickingInfoReadBack; ///< Staging buffer for reading back picking info from the GPU.
    ref<Fence> mpReadbackFence;        ///< GPU fence for synchronizing picking info readback.

    SDFPickingInfo mPickingInfo;

    ref<Buffer> mpGridInstanceIDsBuffer;     ///< Buffer for grid instance IDs.
    uint32_t mGridInstanceCount = 0;

    uint32_t mNonBakedPrimitiveCount = 0;
    uint32_t mBakePrimitivesBatchSize = 5; ///< The number of primitives to bake at a time.
    uint32_t mPreservedHistoryCount = 100; ///< Primitives that should not be baked.

    // Undo/Redo
    uint32_t mUndoPressedCount = 0;
    uint32_t mRedoPressedCount = 0;

    bool mLMBDown = false;
    bool mRMBDown = false;
    bool mMMBDown = false;

    bool mEditingKeyDown = false;
    bool mGUIKeyDown = false;
    bool mPreviewEnabled = true;
    bool mAllowEditingOnOtherSurfaces = false;
    bool mAutoBakingEnabled = true;

    uint2 mFrameDim = {0, 0};
    UI2D mUI2D;

    CurrentEdit mCurrentEdit;
    std::vector<SDFEdit> mPerformedSDFEdits;
    std::vector<UndoneSDFEdit> mUndoneSDFEdits;

    SDFEditingData mGPUEditingData;

    struct
    {
        TransformationState prevState = TransformationState::None;
        TransformationState state = TransformationState::None;
        Transform startInstanceTransform;
        Transform startPrimitiveTransform;
        SDF3DPrimitive startPrimitive;
        float3 startPlanePos = {0.0f, 0.0f, 0.0f};
        float3 referencePlaneDir = {0.0f, 0.0f, 0.0f};
        SDFEditorAxis axis = SDFEditorAxis::All;
        SDFEditorAxis prevAxis = SDFEditorAxis::All;
        float2 startMousePos = {0.0f, 0.0f};
    } mPrimitiveTransformationEdit;

    struct
    {
        TransformationState prevState = TransformationState::None;
        TransformationState state = TransformationState::None;
        Transform startTransform;
        float3 startPlanePos = {0.0f, 0.0f, 0.0f};
        float3 referencePlaneDir = {0.0f, 0.0f, 0.0f};
        float prevScrollTotal = 0.0f;
        float scrollTotal = 0.0f;
        float2 startMousePos = {0.0f, 0.0f};
    } mInstanceTransformationEdit;
};
```

**Public Members**: None (all members are private)

**Private Members**:
- `ref<Scene> mpScene` - Scene reference
- `ref<Camera> mpCamera` - Camera reference
- `ref<FullScreenPass> mpGUIPass` - Full screen pass for 2D GUI rendering
- `ref<Fbo> mpFbo` - Frame buffer object
- `ref<Texture> mpEditingVBuffer` - Copy of VBuffer used while moving/adding primitives
- `ref<Texture> mpEditingLinearZBuffer` - Copy of linear Z buffer used while moving/adding primitives
- `ref<Buffer> mpSDFEditingDataBuffer` - Buffer for current Edit data for GUI visualization
- `ref<Buffer> mpPickingInfo` - Buffer for reading back picking info from GPU
- `ref<Buffer> mpPickingInfoReadBack` - Staging buffer for reading back picking info
- `ref<Fence> mpReadbackFence` - GPU fence for synchronizing picking info readback
- `ref<Buffer> mpGridInstanceIDsBuffer` - Buffer for grid instance IDs
- `uint32_t mGridInstanceCount` - Grid instance count
- `uint32_t mNonBakedPrimitiveCount` - Non-baked primitive count
- `uint32_t mBakePrimitivesBatchSize` - Bake primitives batch size
- `uint32_t mPreservedHistoryCount` - Preserved history count
- `uint32_t mUndoPressedCount` - Undo pressed count
- `uint32_t mRedoPressedCount` - Redo pressed count
- `bool mLMBDown` - Left mouse button down flag
- `bool mRMBDown` - Right mouse button down flag
- `bool mMMBDown` - Middle mouse button down flag
- `bool mEditingKeyDown` - Editing key down flag
- `bool mGUIKeyDown` - GUI key down flag
- `bool mPreviewEnabled` - Preview enabled flag
- `bool mAllowEditingOnOtherSurfaces` - Allow editing on other surfaces flag
- `bool mAutoBakingEnabled` - Auto baking enabled flag
- `uint2 mFrameDim` - Frame dimensions
- `UI2D mUI2D` - 2D UI state
- `CurrentEdit mCurrentEdit` - Current edit state
- `std::vector<SDFEdit> mPerformedSDFEdits` - Performed SDF edits history
- `std::vector<UndoneSDFEdit> mUndoneSDFEdits` - Undone SDF edits history
- `SDFEditingData mGPUEditingData` - GPU editing data for shader

**Private Methods**:
- `void bindShaderData(const ShaderVar& var, const ref<Texture>& pInputColor, const ref<Texture>& pVBuffer)` - Bind shader data
- `void fetchPreviousVBufferAndZBuffer(RenderContext* pRenderContext, ref<Texture>& pVBuffer, ref<Texture>& pDepth)` - Fetch previous VBuffer and Z buffer
- `void setup2DGUI()` - Setup 2D GUI
- `bool isMainGUIKeyDown() const` - Check if main GUI key is down
- `void setupPrimitiveAndOperation(...)` - Setup primitive and operation for editing
- `void setupCurrentModes2D()` - Setup current editing modes
- `void manipulateGridPlane(...)` - Manipulate grid plane (translate/rotate)
- `void rotateGridPlane(...)` - Rotate grid plane around axis
- `void translateGridPlane(...)` - Translate grid plane
- `void updateEditShapeType()` - Update editing shape type
- `void updateEditOperationType()` - Update editing operation type
- `void updateSymmetryPrimitive()` - Update symmetry primitive
- `void addEditPrimitive(...)` - Add/edit primitive
- `void removeEditPrimitives()` - Remove primitives
- `void updateEditPrimitives()` - Update edit primitives
- `void handleActions()` - Handle input actions
- `void handleToggleSymmetryPlane()` - Handle symmetry plane toggle
- `void handleToggleEditing()` - Handle editing toggle
- `void handleEditMovement()` - Handle edit movement
- `void handleAddPrimitive()` - Handle primitive addition
- `bool handlePicking(...)` - Handle picking
- `void handleUndo()` - Handle undo
- `void handleRedo()` - Handle redo
- `void bakePrimitives()` - Bake primitives to grid

**Public Methods**:
- `static void registerBindings(pybind11::module& m)` - Register Python bindings

## Technical Details

### Render Graph Integration

**Input Channels**:
- `inputColor` (required): Input image for 2D GUI drawing on top (RGBA32Float)
- `vbuffer` (optional): Visibility buffer in packed format (RGBA32Uint)
- `linearZ` (optional): Linear Z and slope (RG32Float)

**Output Channels**:
- `output` (required): Input image with 2D GUI drawn on top (RGBA32Float)

**Reflection**:
```cpp
RenderPassReflection reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    addRenderPassInputs(reflector, kInputChannels);
    reflector.addOutput(kOutputChannel, "Input image with 2D GUI drawn on top");
    return reflector;
}
```

**Features**:
- Input color channel for 2D GUI overlay
- Optional VBuffer and linear Z channels for depth testing
- Output channel for 2D GUI rendering
- No configurable parameters

### Execute Implementation

**Implementation**:
```cpp
void SDFEditor::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    if (!mpScene || !mpGUIPass)
        return;

    auto pOutput = renderData.getTexture(kOutputChannel);
    auto pInputColor = renderData.getTexture(kInputColorChannel);
    FALCOR_ASSERT(pOutput && pInputColor);

    mFrameDim = uint2(pOutput->getWidth(), pOutput->getHeight());
    mpFbo->attachColorTarget(pOutput, 0);

    // Make a copy of the vBuffer and the linear z-buffer before editing.
    ref<Texture> pVBuffer = renderData.getTexture(kInputVBuffer);
    ref<Texture> pDepth = renderData.getTexture(kInputDepth);
    if (!mEditingKeyDown)
    {
        fetchPreviousVBufferAndZBuffer(pRenderContext, pVBuffer, pDepth);
    }
    else // When editing, use the copies instead to not add SDF primitives on the current edits.
    {
        pVBuffer = mpEditingVBuffer;
    }

    // Wait for the picking info from the previous frame.
    mpReadbackFence->wait();
    mPickingInfo = *reinterpret_cast<const SDFPickingInfo*>(mpPickingInfoReadBack->map());
    mpPickingInfoReadBack->unmap();

    setup2DGUI();
    handleActions();

    // Set shader data.
    auto rootVar = mpGUIPass->getRootVar();
    bindShaderData(rootVar, pInputColor, pVBuffer);

    mpGUIPass->execute(pRenderContext, mpFbo);

    // Copy picking info into a staging buffer.
    pRenderContext->copyResource(mpPickingInfoReadBack.get(), mpPickingInfo.get());
    pRenderContext->submit(false);
    pRenderContext->signal(mpReadbackFence.get());

    // Prepare next frame.
    {
        mUI2D.keyboardButtonsPressed.registerCurrentStatesIntoPrevious();
        mUI2D.prevMousePosition = mUI2D.currentMousePosition;
        mUI2D.scrollDelta = 0.0f;
        mInstanceTransformationEdit.prevState = mInstanceTransformationEdit.state;
        mInstanceTransformationEdit.prevScrollTotal = mInstanceTransformationEdit.scrollTotal;
        mPrimitiveTransformationEdit.prevState = mPrimitiveTransformationEdit.state;
        mPrimitiveTransformationEdit.prevAxis = mPrimitiveTransformationEdit.axis;
    }
}
```

**Features**:
- Scene validation (returns early if no scene)
- Frame dimension tracking
- VBuffer and linear Z buffer copying for editing
- Picking info readback with fence synchronization
- 2D GUI setup and action handling
- Shader data binding
- Full-screen pass execution for 2D GUI overlay
- Picking info staging buffer copy
- State preparation for next frame

### Set Scene Implementation

**Implementation**:
```cpp
void SDFEditor::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    mpScene = pScene;

    if (!mpScene)
        return;

    mpCamera = mpScene->getCamera();
    mpGUIPass = FullScreenPass::create(
        mpDevice, ProgramDesc().addShaderLibrary(kGUIPassShaderFilename).psEntry("psMain"), mpScene->getSceneDefines()
    );

    // Initialize editing primitive.
    {
        const SDF3DShapeType kDefaultShapeType = SDF3DShapeType::Sphere;
        const float3 kDefaultShapeData = float3(0.01f);
        const float kDefaultShapeBlobbing = 0.0f;
        const float kDefaultOperationSmoothing = 0.0f;
        const SDFOperationType kDefaultOperationType = SDFOperationType::Union;
        const Transform kDefaultTransform = Transform();

        mUI2D.currentEditingShape = kDefaultShapeType;
        mUI2D.currentEditingOperator = kDefaultOperationType;

        if (mpScene->getSDFGridCount() > 0)
        {
            std::vector<uint32_t> instanceIDs = mpScene->getGeometryInstanceIDsByType(Scene::GeometryType::SDFGrid);
            if (instanceIDs.empty())
                FALCOR_THROW("Scene missing SDFGrid object!");

            mCurrentEdit.instanceID = instanceIDs[0];
            GeometryInstanceData instance = mpScene->getGeometryInstance(mCurrentEdit.instanceID);
            SdfGridID sdfGridID = mpScene->findSDFGridIDFromGeometryInstanceID(mCurrentEdit.instanceID);
            FALCOR_ASSERT(sdfGridID != SdfGridID::Invalid());
            mCurrentEdit.gridID = sdfGridID;
            mCurrentEdit.pSDFGrid = mpScene->getSDFGrid(sdfGridID);
            mCurrentEdit.primitive = SDF3DPrimitiveFactory::initCommon(
                kDefaultShapeType,
                kDefaultShapeData,
                kDefaultShapeBlobbing,
                kDefaultOperationSmoothing,
                kDefaultOperationType,
                kDefaultTransform
            );

            const AnimationController* pAnimationController = mpScene->getAnimationController();
            const float4x4& transform = pAnimationController->getGlobalMatrices()[instance.globalMatrixID];

            // Update GUI variables
            mUI2D.bbRenderSettings.selectedInstanceID = mCurrentEdit.instanceID;
            mUI2D.gridPlane.position = transform.getCol(3).xyz();
            mUI2D.previousGridPlane.position = transform.getCol(3).xyz();

            updateSymmetryPrimitive();
        }
    }

    mNonBakedPrimitiveCount = mCurrentEdit.pSDFGrid->getPrimitiveCount();
}
```

**Features**:
- Scene reference and camera integration
- Full-screen pass creation for 2D GUI overlay
- SDFGrid validation and instance ID lookup
- Animation controller integration for instance transformations
- Default primitive initialization (Sphere)
- GUI variable setup for editing primitive display
- Non-baked primitive count initialization

### Property Serialization

**Implementation**:
```cpp
Properties SDFEditor::getProperties() const
{
    return {};
}
```

**Features**:
- Empty properties (no configurable parameters)
- No serialization needed

### UI Rendering

**Implementation**:
```cpp
void SDFEditor::renderUI(RenderContext* pRenderContext, Gui::Widgets& widget)
{
    widget.text("Help:");
    widget.tooltip(
        "Abbreviations: Prim=Primitive, Op=Operation, MB = mouse button, LMB = left MB, RMB = right MB, MMB = middle MB\n"
        "\n"
        "To create an SDF from an empty grid, use either the grid plane to place the SDF on, or intersect the grid instance with other "
        "geometry and toggle Editing on Other surfaces by pressing 'C'.\n"
        "\n"
        "* Bring up prim type / op selection: Hold Tab\n"
        "* Select prim type / op in selection : LMB\n"
        "* Rotate prim: CTRL+R, move mouse, LMB to confirm\n"
        "* Scale prim: Ctrl+S, move mouse, LMB to confirm\n"
        "* During scaling of prim: \n"
        "   Key 1: scale only X\n"
        "   Key 2: scale only Y\n"
        "   Key 3: scale only Z\n"
        "   Key 4: change op smoothing (no visualization).\n"
        "   Pressing the same key again goes back to scaling all (except Operator Smoothing).\n"
        "\n"
        "Show true primitive preview : Hold Alt + move mouse\n"
        "Add Primitive : Hold Alt + LMB\n"
        "Undo added prim : Ctrl + Z\n"
        "Redo added prim : Ctrl + Y\n"
        "\n"
        "Translate instance : Shift + T, then move mouse or scroll wheel\n"
        "Rotate instance : Shift + R, then move mouse\n"
        "Scale instance : Shift + S, then move mouse\n"
        "\n"
        "Toggle Gridplane : G\n"
        "Toggle Symmetry : H\n"
        "Toggle Primitive Preview : X\n"
        "Toggle Editing on Other Surfaces : C\n"
        "Change Bounding Box Vis : B\n"
        "\n"
        "Rotate grid plane : Hold RMB + move mouse\n"
        "Rotate grid plane around primary axis : Hold CTRL + RMB + move mouse\n"
        "Translate grid plane : Hold Shift + RMB + move mouse\n"
        "\n"
        "Rotate symmetry plane : Hold MMB + move mouse\n"
        "Rotate symmetry plane around primary axis : Hold CTRL + MMB + move mouse\n"
        "Translate symmetry plane : Hold Shift + MMB  + move mouse"
    );

    if (auto group = widget.group("IO", true))
    {
        if (mCurrentEdit.pSDFGrid->wasInitializedWithPrimitives())
        {
            if (group.button("Save SDF primitives", false))
            {
                std::filesystem::path filePath = "sdfGrid.sdf";
                if (saveFileDialog(kSDFFileExtensionFilters, filePath))
                {
                    mCurrentEdit.pSDFGrid->writePrimitivesToFile(filePath);
                }
            }
        }

        if (group.button("Save SDF grid"))
        {
            std::filesystem::path filePath = "sdfGrid.sdfg";
            if (saveFileDialog(kSDFGridFileExtensionFilters, filePath))
            {
                mCurrentEdit.pSDFGrid->writeValuesFromPrimitivesToFile(filePath, pRenderContext);
            }
        }
    }

    if (auto nodeGroup = widget.group("Grid", true))
    {
        bool gridActive = mUI2D.gridPlane.active > 0 ? true : false;
        widget.checkbox("Show/use grid plane", gridActive);
        mUI2D.gridPlane.active = uint32_t(gridActive);

        if (mUI2D.gridPlane.active)
        {
            widget.var("Plane center", mUI2D.gridPlane.position, -10.0f, 10.0f, 0.05f);
            widget.direction("Plane normal", mUI2D.gridPlane.normal);
            widget.direction("Plane right vector", mUI2D.gridPlane.rightVector);
            widget.slider("Plane size", mUI2D.gridPlane.planeSize, 0.01f, 2.0f, false, "%2.2f");
            widget.slider("Grid line width", mUI2D.gridPlane.gridLineWidth, 0.01f, 0.1f, false, "%2.2f");
            widget.slider("Grid scale", mUI2D.gridPlane.gridScale, 0.01f, 50.0f, false, "%2.2f");
            widget.rgbaColor("Grid color", mUI2D.gridPlane.color);
        }
    }

    if (auto nodeGroup = widget.group("Bounding Boxes", true))
    {
        static Gui::DropdownList sdfBoundingBoxRenderModes = {
            {uint32_t(SDFBBRenderMode::Disabled), "Disabled"},
            {uint32_t(SDFBBRenderMode::RenderAll), "Render All"},
            {uint32_t(SDFBBRenderMode::RenderSelectedOnly), "Render Selected"},
        };

        widget.dropdown("Render Mode", sdfBoundingBoxRenderModes, mUI2D.bbRenderSettings.renderMode);
        widget.slider("Edge Thickness", mUI2D.bbRenderSettings.edgeThickness, 0.00001f, 0.0005f);
    }

    if (auto group = widget.group("Edit", true))
    {
        if (group.slider<float>("Blobbing", mUI2D.currentBlobbing, kMinShapeBlobbyness, kMaxShapeBlobbyness))
        {
            mCurrentEdit.primitive.shapeBlobbing = mUI2D.currentBlobbing;
        }

        group.var<uint32_t>("Preserved primitives: ", mPreservedHistoryCount, 1);
        group.tooltip("Number of primitives that cannot be baked.");
        group.var<uint32_t>("Batch Size: ", mBakePrimitivesBatchSize, 1);
        group.tooltip("Number of primitives to bake at a time.");
        if (group.button("Bake primitives"))
        {
            mCurrentEdit.pSDFGrid->bakePrimitives(mBakePrimitivesBatchSize);
        }

        group.checkbox("Auto baking", mAutoBakingEnabled);
        group.tooltip("Enable baking at each edit depending on the batch size");

        if (auto innerGroup = group.group("Statistics", true))
        {
            innerGroup.text("#Primitives: " + std::to_string(mCurrentEdit.pSDFGrid->getPrimitiveCount()));
            innerGroup.text("#Baked primitives: " + std::to_string(mCurrentEdit.pSDFGrid->getBakedPrimitiveCount()));
        }
    }
}
```

**Features**:
- Help text with keyboard shortcuts
- IO group for saving SDF primitives and grids
- Grid group for grid plane manipulation
- Bounding boxes group for rendering mode selection
- Edit group for blobbing, baking, and statistics

### Keyboard Event Handling

**Implementation**:
```cpp
bool SDFEditor::onKeyEvent(const KeyboardEvent& keyEvent)
{
    mUI2D.keyboardButtonsPressed.registerCurrentStatesIntoPrevious();

    if (keyEvent.type == KeyboardEvent::Type::KeyPressed)
    {
        switch (keyEvent.key)
        {
        case Input::Key::LeftControl:
        case Input::Key::RightControl:
            mUI2D.keyboardButtonsPressed.control = true;
            mpScene->setCameraControlsEnabled(false);
            return true;
        case Input::Key::LeftAlt:
        case Input::Key::RightAlt:
            mEditingKeyDown = true;
            mInstanceTransformationEdit.state = TransformationState::None;
            mPrimitiveTransformationEdit.state = TransformationState::None;
            mpScene->setCameraControlsEnabled(false);
            handleToggleEditing();
            return true;
        case Input::Key::LeftShift:
        case Input::Key::RightShift:
            mUI2D.keyboardButtonsPressed.shift = true;
            mPrimitiveTransformationEdit.state = TransformationState::None;
            mpScene->setCameraControlsEnabled(false);
            return true;
        case Input::Key::Tab:
            mGUIKeyDown = true;
            mUI2D.recordStartingMousePos = true;
            mInstanceTransformationEdit.state = TransformationState::None;
            mpScene->setCameraControlsEnabled(false);
            return true;
        case Input::Key::G:
            mUI2D.gridPlane.active = !mUI2D.gridPlane.active;
            return true;
        case Input::Key::H:
            handleToggleSymmetryPlane();
            return true;
        case Input::Key::B:
            mUI2D.bbRenderSettings.renderMode = (mUI2D.bbRenderSettings.renderMode + 1) % uint32_t(SDFBBRenderMode::Count);
            return true;
        case Input::Key::C:
            if (!mEditingKeyDown && !keyEvent.hasModifier(Input::Modifier::Ctrl) && !keyEvent.hasModifier(Input::Modifier::Shift))
            {
                mAllowEditingOnOtherSurfaces = !mAllowEditingOnOtherSurfaces;
                return true;
            }
            break;
        case Input::Key::X:
            if (!mEditingKeyDown && !keyEvent.hasModifier(Input::Modifier::Ctrl) && !keyEvent.hasModifier(Input::Modifier::Shift))
            {
                mPreviewEnabled = !mPreviewEnabled;
                return true;
            }
            break;
        case Input::Key::Z:
            if (keyEvent.hasModifier(Input::Modifier::Ctrl) && !mEditingKeyDown)
            {
                mUndoPressedCount = 0;
                mUI2D.keyboardButtonsPressed.undo = true;
                mUI2D.keyboardButtonsPressed.redo = false;
                handleUndo();
                return true;
            }
            break;
        case Input::Key::Y:
            if (keyEvent.hasModifier(Input::Modifier::Ctrl) && !mEditingKeyDown)
            {
                mRedoPressedCount = 0;
                mUI2D.keyboardButtonsPressed.redo = true;
                mUI2D.keyboardButtonsPressed.undo = false;
                handleRedo();
                return true;
            }
            break;
        case Input::Key::T:
            if (!mEditingKeyDown && keyEvent.hasModifier(Input::Modifier::Shift))
            {
                mInstanceTransformationEdit.state = TransformationState::Translating;
                return true;
            }
            break;
        case Input::Key::R:
            if (!mEditingKeyDown)
            {
                if (keyEvent.hasModifier(Input::Modifier::Shift))
                {
                    mInstanceTransformationEdit.state = TransformationState::Rotating;
                    return true;
                }
                else if (keyEvent.hasModifier(Input::Modifier::Ctrl))
                {
                    mPrimitiveTransformationEdit.state = TransformationState::Rotating;
                    return true;
                }
            }
            break;
        case Input::Key::S:
            if (!mEditingKeyDown)
            {
                if (keyEvent.hasModifier(Input::Modifier::Shift))
                {
                    mInstanceTransformationEdit.state = TransformationState::Scaling;
                    return true;
                }
                else if (keyEvent.hasModifier(Input::Modifier::Ctrl))
                {
                    mPrimitiveTransformationEdit.state = TransformationState::Scaling;
                    return true;
                }
            }
            break;
        case Input::Key::Key1:
        case Input::Key::Key2:
        case Input::Key::Key3:
        case Input::Key::Key4:
            if (mPrimitiveTransformationEdit.state == TransformationState::Scaling)
            {
                // Handle axis selection for scaling
                return true;
            }
            break;
        default:
            break;
        }
    }
    else if (keyEvent.type == KeyboardEvent::Type::KeyReleased)
    {
        // Handle key release events
        // ...
    }
    else if (keyEvent.type == KeyboardEvent::Type::KeyRepeated)
    {
        // Handle key repeat events
        // ...
    }

    return false;
}
```

**Features**:
- Main GUI key detection (Tab)
- Modifier key handling (Ctrl, Shift, Alt)
- Transformation mode state management
- Undo/Redo with pressed count tracking
- Preview toggle (X)
- Editing on other surfaces toggle (C)
- Auto baking toggle
- Bounding box visibility mode selection (B)
- Grid plane toggle (G)
- Symmetry plane toggle (H)
- Instance transformation modes (Shift+T, Shift+R, Shift+S)
- Primitive transformation modes (Ctrl+R, Ctrl+S)
- Axis selection for scaling (1, 2, 3, 4)

### Mouse Event Handling

**Implementation**:
```cpp
bool SDFEditor::onMouseEvent(const MouseEvent& mouseEvent)
{
    float2 currentMousePos = mouseEvent.pos * float2(mFrameDim);
    mUI2D.currentMousePosition = currentMousePos;

    if (mouseEvent.button == Input::MouseButton::Left)
    {
        mLMBDown = mouseEvent.type == MouseEvent::Type::ButtonDown;
    }

    if (mouseEvent.button == Input::MouseButton::Right)
    {
        mRMBDown = mouseEvent.type == MouseEvent::Type::ButtonDown;
    }

    if (mouseEvent.button == Input::MouseButton::Middle)
    {
        mMMBDown = mouseEvent.type == MouseEvent::Type::ButtonDown;
    }

    bool handled = false;

    if (isMainGUIKeyDown())
    {
        if (mUI2D.recordStartingMousePos)
        {
            mUI2D.recordStartingMousePos = false;
            mUI2D.startMousePosition = currentMousePos;
        }
    }
    else if (mEditingKeyDown && !gridPlaneManipulated())
    {
        if (!mLMBDown)
        {
            if (mouseEvent.type == MouseEvent::Type::Move)
            {
                handleEditMovement();
                handled = true;
            }
        }
        else if ((mouseEvent.type == MouseEvent::Type::ButtonDown && mouseEvent.button == Input::MouseButton::Left) || mouseEvent.type == MouseEvent::Type::Move)
        {
            handleAddPrimitive();
            handled = true;
        }
    }
    else if (gridPlaneManipulated())
    {
        if (mouseEvent.type == MouseEvent::Type::ButtonDown && mouseEvent.button == Input::MouseButton::Right)
        {
            mUI2D.startMousePosition = currentMousePos;
            mUI2D.previousGridPlane = mUI2D.gridPlane;
        }
        else if (mouseEvent.type == MouseEvent::Type::Move)
        {
            manipulateGridPlane(
                mUI2D.gridPlane, mUI2D.previousGridPlane, mUI2D.keyboardButtonsPressed.shift, mUI2D.keyboardButtonsPressed.control
            );
        }
    }
    else if (symmetryPlaneManipulated())
    {
        if (mouseEvent.type == MouseEvent::Type::ButtonDown && mouseEvent.button == Input::MouseButton::Middle)
        {
            mUI2D.startMousePosition = currentMousePos;
            mUI2D.previousSymmetryPlane = mUI2D.symmetryPlane;
        }
        else if (mouseEvent.type == MouseEvent::Type::Move)
        {
            manipulateGridPlane(
                mUI2D.symmetryPlane, mUI2D.previousSymmetryPlane, mUI2D.keyboardButtonsPressed.shift, mUI2D.keyboardButtonsPressed.control
            );
        }
    }

    if (mLMBDown)
    {
        mInstanceTransformationEdit.state = TransformationState::None;
        mPrimitiveTransformationEdit.state = TransformationState::None;
    }
    else if (mouseEvent.type == MouseEvent::Type::ButtonUp && mouseEvent.button == Input::MouseButton::Right)
    {
        mUI2D.previousGridPlane = mUI2D.gridPlane;
    }
    else if (mouseEvent.type == MouseEvent::Type::ButtonUp && mouseEvent.button == Input::MouseButton::Middle)
    {
        mUI2D.previousSymmetryPlane = mUI2D.symmetryPlane;
    }
    else if (mouseEvent.type == MouseEvent::Type::Wheel)
    {
        mUI2D.scrollDelta += mouseEvent.wheelDelta.y;

        if (mInstanceTransformationEdit.state != TransformationState::None)
        {
            mInstanceTransformationEdit.scrollTotal += mouseEvent.wheelDelta.y;
        }
    }

    return handled;
}
```

**Features**:
- Mouse button detection (Left, Right, Middle)
- Mouse wheel handling for operation smoothing
- Grid plane manipulation (rotate, translate)
- Symmetry plane manipulation (rotate, translate)
- Picking for primitive selection
- Mouse movement tracking for transformation
- Starting mouse position recording

### Picking

**Implementation**:
```cpp
bool SDFEditor::handlePicking(const float2& currentMousePos, float3& localPos)
{
    if (!mpScene)
        return false;

    // Create picking ray.
    float3 rayOrigin = mpCamera->getPosition();

    const CameraData& cameraData = mpCamera->getData();
    float2 ndc = float2(-1.0f, 1.0f) + float2(2.0f, -2.0f) * (currentMousePos + float2(0.5f, 0.5f)) / float2(mFrameDim);
    float3 rayDir = normalize(ndc.x * cameraData.cameraU + ndc.y * cameraData.cameraV + cameraData.cameraW);

    float3 iSectPosition;
    if (mUI2D.gridPlane.active) // Grid is on, so we pick on the grid.
    {
        float t = mUI2D.gridPlane.intersect(rayOrigin, rayDir);
        iSectPosition = rayOrigin + rayDir * t;
    }
    else if (mAllowEditingOnOtherSurfaces || mPickingInfo.instanceID == mCurrentEdit.instanceID)
    {
        iSectPosition = rayOrigin + rayDir * mPickingInfo.distance;
    }
    else
    {
        return false;
    }

    const GeometryInstanceData& instance = mpScene->getGeometryInstance(mCurrentEdit.instanceID);
    const AnimationController* pAnimationController = mpScene->getAnimationController();
    const float4x4& invTransposeInstanceTransform = pAnimationController->getInvTransposeGlobalMatrices()[instance.globalMatrixID];
    localPos = transformPoint(transpose(invTransposeInstanceTransform), iSectPosition);
    return true;
}
```

**Features**:
- Camera ray tracing through scene
- Grid plane intersection
- Surface intersection (if editing on other surfaces)
- Local position calculation
- Instance transformation integration

## Architecture Patterns

### Render Pass Pattern

- Inherits from [`RenderPass`](Source/Falcor/RenderGraph/RenderPass.h:1) base class
- Implements standard render pass interface:
  - `reflect()` - Defines input/output resources
  - `execute()` - Renders 2D GUI overlay
  - `renderUI()` - Renders comprehensive UI
  - `getProperties()` - Serializes configuration (empty)
  - `setScene()` - Sets scene and initializes programs
  - `onMouseEvent()` - Handles mouse events for picking
  - `onKeyEvent()` - Handles keyboard events
  - `compile()` - Stores frame dimensions

### Full Screen Pass Pattern

- Uses full-screen pass for 2D GUI rendering
- Pixel shader: `GUIPass.ps.slang`
- Entry point: `psMain`
- Single dispatch per frame
- No complex rendering logic

### Scene Integration Pattern

- Scene reference management
- SDFGrid validation and instance ID lookup
- Animation controller integration for instance transformations
- Camera integration for perspective transformations
- Grid instance buffer creation
- Scene defines for shader compilation

### 2D GUI Pattern

- Full-featured 2D GUI system:
  - 2D overlay rendering
  - Marker system for visualizing primitives
  - Selection wheel for different shape/operation types
  - Keyboard shortcuts for all operations
  - Transformation state management
  - Edit history tracking
  - Baking system for SDF grid export

### Picking Pattern

- Ray tracing through scene
- Grid plane intersection
- Surface intersection (if editing on other surfaces)
- Local position calculation
- Instance transformation integration

### Transformation Pattern

- Multiple transformation modes:
  - None, Translating, Rotating, Scaling
- State machine with previous/current state
- Transformation matrices for instance transforms
- Constraint-based rotation (around single axis)

### Grid Plane Pattern

- Grid plane manipulation:
  - Translation, rotation around axis
  - Symmetry plane support (Normal, Right, Up)
  - Constrained rotation mode (around primary axis)
  - Speed factor based on diagonal

### Bounding Box Pattern

- Multiple rendering modes:
  - Disabled, RenderAll, RenderSelected
- Edge thickness configuration
- Wireframe or solid rendering

### Marker System Pattern

- Multiple marker types:
  - Simple markers (Circle, Square)
  - Color-coded by operation type
  - Size-based rendering
  - Sector-based selection wheel

### Selection Wheel Pattern

- Sector-based operation selection
- Multiple shape/operation types per sector
- Visual feedback with colors
- Smooth operation smoothing

### Undo/Redo Pattern

- Edit history tracking
- Per-SDF edit tracking
- Primitive removal from SDF grid
- Re-addition of removed primitives

### Baking Pattern

- Batch-based primitive baking
- Configurable batch size
- Preserved history count
- Non-baked primitive count tracking
- File I/O for saving/loading SDF grids

## Code Patterns

### Property Serialization Pattern

```cpp
Properties SDFEditor::getProperties() const
{
    return {};
}
```

### Shader Data Binding Pattern

```cpp
void SDFEditor::bindShaderData(const ShaderVar& var, const ref<Texture>& pInputColor, const ref<Texture>& pVBuffer)
{
    mGPUEditingData.editing = mEditingKeyDown;
    mGPUEditingData.previewEnabled = mPreviewEnabled;
    mGPUEditingData.instanceID = mCurrentEdit.instanceID;
    mGPUEditingData.scalingAxis = uint32_t(
        mPrimitiveTransformationEdit.state != TransformationState::Scaling ? SDFEditorAxis::Count : mPrimitiveTransformationEdit.axis
    );
    mGPUEditingData.primitive = mCurrentEdit.primitive;
    mGPUEditingData.primitiveBB = SDF3DPrimitiveFactory::computeAABB(mCurrentEdit.primitive);
    mpSDFEditingDataBuffer->setBlob(&mGPUEditingData, 0, sizeof(SDFEditingData));
    mpScene->bindShaderData(rootVar["gScene"]);
}
```

### 2D GUI Setup Pattern

```cpp
void SDFEditor::setup2DGUI()
{
    const float minSide = float(std::min(mFrameDim.x, mFrameDim.y));
    const float radius = minSide * 0.5f;
    const float markerSize = radius * 0.2f;
    const float roundedRadius = markerSize * 0.1f;

    mUI2D.pMarker2DSet->clear();
    mUI2D.pSelectionWheel->update(mUI2D.currentMousePosition);

    setupCurrentModes2D();
}
```

### Grid Plane Manipulation Pattern

```cpp
void SDFEditor::manipulateGridPlane(
    SDFGridPlane& gridPlane,
    SDFGridPlane& previousGridPlane,
    bool isTranslationKeyDown,
    bool isConstrainedManipulationKeyDown
)
{
    float2 diffPrev = mUI2D.currentMousePosition - mUI2D.prevMousePosition;
    float2 diffStart = mUI2D.currentMousePosition - mUI2D.startMousePosition;
    float3 up = mpCamera->getUpVector();
    float3 view = normalize(mpScene->getCamera()->getTarget() - mpScene->getCamera()->getPosition());
    float3 right = cross(view, up);

    if (!isTranslationKeyDown)
    {
        if (!isConstrainedManipulationKeyDown)
        {
            // Rotate plane arbitrarily along right and up vectors.
            rotateGridPlane(diffPrev.y, right, previousGridPlane.normal, previousGridPlane.rightVector, gridPlane.normal, gridPlane.rightVector);
            rotateGridPlane(diffPrev.x, up, gridPlane.normal, gridPlane.rightVector, gridPlane.normal, gridPlane.rightVector);
            previousGridPlane = gridPlane;
        }
        else
        {
            // Constrained rotation to the axis with most movement since shift was pressed.
            if (std::abs(diffStart.y) > std::abs(diffStart.x))
            {
                rotateGridPlane(diffStart.y, right, previousGridPlane.normal, previousGridPlane.rightVector, gridPlane.normal, gridPlane.rightVector, false);
            }
            else
            {
                rotateGridPlane(diffStart.x, up, previousGridPlane.normal, previousGridPlane.rightVector, gridPlane.normal, gridPlane.rightVector, false);
            }
        }
    }
    else
    {
        if (!isConstrainedManipulationKeyDown)
        {
            translateGridPlane(diffPrev.x, right, previousGridPlane.position, gridPlane.position);
            translateGridPlane(-diffPrev.y, up, gridPlane.position, gridPlane.position);
            previousGridPlane = gridPlane;
        }
        else
        {
            if (std::abs(diffStart.y) > std::abs(diffStart.x))
            {
                translateGridPlane(-diffStart.y, up, previousGridPlane.position, gridPlane.position);
            }
            else
            {
                translateGridPlane(diffStart.x, right, previousGridPlane.position, gridPlane.position);
            }
        }
    }
}
```

### Grid Plane Rotation Pattern

```cpp
void SDFEditor::rotateGridPlane(
    const float mouseDiff,
    const float3& rotationVector,
    const float3& inNormal,
    const float3& inRightVector,
    float3& outNormal,
    float3& outRightVector,
    const bool fromPreviousMouse
)
{
    const float diagonal = length(float2(mFrameDim));
    const float maxAngle = float(M_PI) * 0.05f;
    float angle;

    if (fromPreviousMouse)
    {
        const float speedFactor = 2.0f * float(M_PI) * 0.075f / diagonal;
        angle = mouseDiff * std::abs(mouseDiff) * speedFactor;
        angle = std::clamp(angle, -maxAngle, maxAngle);
    }
    else
    {
        const float speedFactor = 2.0f * float(M_PI) * 0.5f / diagonal;
        angle = mouseDiff * speedFactor;
    }

    float4x4 rotationMatrix = math::rotate(float4x4::identity(), angle, rotationVector);

    outNormal = normalize(transformVector(rotationMatrix, inNormal));
    outRightVector = normalize(transformVector(rotationMatrix, inRightVector));
}
```

### Grid Plane Translation Pattern

```cpp
void SDFEditor::translateGridPlane(const float mouseDiff, const float3& translationVector, const float3& inPosition, float3& outPosition)
{
    const float diagonal = length(float2(mFrameDim));
    const float speedFactor = 0.5f / diagonal;
    float translation = mouseDiff * std::abs(mouseDiff) * speedFactor;
    outPosition = outPosition + translation * translationVector;
}
```

### Symmetry Plane Pattern

```cpp
void SDFEditor::handleToggleSymmetryPlane()
{
    mUI2D.symmetryPlane.active = !mUI2D.symmetryPlane.active;

    if (mUI2D.symmetryPlane.active)
    {
        updateSymmetryPrimitive();
    }

    if (mEditingKeyDown)
    {
        if (mUI2D.symmetryPlane.active)
        {
            uint32_t basePrimitiveID;
            basePrimitiveID = mCurrentEdit.pSDFGrid->addPrimitives({mCurrentEdit.symmetryPrimitive});
            mCurrentEdit.symmetryPrimitiveID = basePrimitiveID;
        }
        else if (mCurrentEdit.primitiveID != kInvalidPrimitiveID)
        {
            mCurrentEdit.pSDFGrid->removePrimitives({mCurrentEdit.symmetryPrimitiveID});
            mCurrentEdit.symmetryPrimitiveID = kInvalidPrimitiveID;
        }
    }
}
```

### Transformation State Machine

```cpp
enum class TransformationState
{
    None,
    Translating,
    Rotating,
    Scaling
};
```

### Edit History Pattern

```cpp
std::vector<SDFEdit> mPerformedSDFEdits;
std::vector<UndoneSDFEdit> mUndoneSDFEdits;
```

### Baking Pattern

```cpp
void SDFEditor::bakePrimitives()
{
    uint32_t batchCount = (mNonBakedPrimitiveCount - mPreservedHistoryCount) / mBakePrimitivesBatchSize;
    uint32_t bakePrimitivesCount = mBakePrimitivesBatchSize * batchCount;
    mCurrentEdit.pSDFGrid->bakePrimitives(bakePrimitivesCount);
    mNonBakedPrimitiveCount -= bakePrimitivesCount;
}
```

## Integration Points

### Render Graph Integration

The pass integrates with render graph system:
- Input channels: 3 (1 required: inputColor, 2 optional: vbuffer, linearZ)
- Output channels: 1 required: output (2D GUI overlay)
- Reflects input/output resources via `reflect()` method
- Executes 2D GUI overlay in `execute()` method
- No configurable parameters (empty properties)

### Scene Integration

The pass integrates with scene system:
- Scene reference management
- SDFGrid validation and instance ID lookup
- Animation controller integration for instance transformations
- Camera integration for perspective transformations
- Grid instance buffer creation
- Scene defines for shader compilation

### Full Screen Pass Integration

The pass integrates with full-screen pass system:
- 2D GUI overlay rendering via full-screen pass
- Pixel shader: `GUIPass.ps.slang`
- Single dispatch per frame
- No complex rendering logic

### 2D GUI Integration

The pass provides comprehensive 2D GUI:
- Marker system for visualizing primitives
- Selection wheel for different shape/operation types
- Keyboard shortcuts for all operations
- Transformation state management
- Edit history tracking
- Baking system for SDF grid export
- Help text with keyboard shortcuts

### Picking Integration

The pass integrates with picking system:
- Camera ray tracing through scene
- Grid plane intersection
- Surface intersection (if editing on other surfaces)
- Local position calculation
- Instance transformation integration

### Scene/SDFGrid Integration

The pass integrates with SDFGrid system:
- SDF3DPrimitiveFactory for primitive creation
- SDFGrid lookup for grid instance IDs
- Instance metadata generation
- Geometry type filtering
- Instance count tracking
- Primitive count tracking

### Utils/Math Integration

The pass integrates with mathematical utilities:
- Matrix operations for transformations
- Quaternion operations for rotations
- Vector operations for translations
- Normalize and cross product operations
- Matrix composition for complex transforms

## Use Cases

### SDF Grid Editing

Editing applications:
- Create SDF grids from primitives
- Edit primitive shapes and transformations
- Apply symmetry operations
- Adjust grid plane orientation
- Manage instance metadata
- Export/import SDF grids

### Scene Debugging

Debugging applications:
- Verify SDF grid structure
- Check instance assignments
- Validate instance transformations
- Analyze scene graph structure
- Debug geometry issues
- Verify material assignments
- Check SDF primitive types

### Baking

Baking applications:
- Export SDF grids to file
- Create optimized SDF representations
- Batch processing for large grids
- Preserve history count for primitives

### Research and Development

Research applications:
- Study SDF representation techniques
- Analyze grid optimization strategies
- Investigate symmetry operations
- Research transformation algorithms
- Develop new editing workflows
- Study baking optimization

### Educational Use

Educational applications:
- Demonstrate SDF editing concepts
- Show transformation techniques
- Explain symmetry operations
- Teach grid plane manipulation
- Visualize primitive operations

## Performance Considerations

### GPU Computation

- 2D GUI rendering: O(width × height) pixels per frame
- Picking ray tracing: O(width × height) rays per frame (optional)
- Transformation calculations: O(1) per operation
- Grid plane manipulation: O(1) per operation
- Baking: O(primitiveCount × gridResolution) per bake

### Memory Usage

- FBO: O(width × height × 16) bytes (RGBA32Float)
- Editing VBuffer: O(width × height × 4) bytes (RGBA32Uint)
- Linear Z buffer: O(width × height × 8) bytes (RG32Float)
- SDF editing data buffer: O(sizeof(SDFEditingData)) bytes
- Picking info buffer: O(sizeof(SDFPickingInfo)) bytes
- Picking info readback: O(sizeof(SDFPickingInfo)) bytes
- Grid instance IDs buffer: O(instanceCount × 4) bytes
- Total memory: O(width × height × (16 + 4 + 8 + sizeof(SDFEditingData) + sizeof(SDFPickingInfo) + instanceCount × 4) bytes

### Computational Complexity

- 2D GUI: O(width × height) per frame
- Picking: O(width × height) rays per frame (optional)
- Transformations: O(1) per operation
- Grid plane: O(1) per operation
- Baking: O(primitiveCount × gridResolution) per bake

## Limitations

### Feature Limitations

- No configurable parameters
- No property serialization
- No preset management
- No undo/redo persistence
- Limited to SDFGrid geometry type
- Limited to sphere and box shapes
- No custom primitive types
- Limited to smooth operations
- Limited to grid plane manipulation
- Limited to symmetry operations
- No bounding box editing modes
- No real-time preview

### Scene Limitations

- Requires SDFGrid geometry type
- Limited to scene's SDFGrid system
- No support for other geometry types
- No support for procedural geometry

### Performance Limitations

- High memory usage for buffers
- Picking ray tracing overhead
- Transformation calculation overhead
- Baking overhead
- 2D GUI rendering overhead
- No performance optimization

### UI Limitations

- Complex UI with many controls
- No preset management
- No undo/redo functionality
- No real-time preview
- Limited parameter validation
- No help system integration

## Best Practices

### SDF Grid Editing

- Use appropriate shape types for geometry
- Apply transformations correctly
- Use symmetry operations efficiently
- Manage instance metadata
- Export grids for use in rendering
- Test with different baking settings

### Scene Debugging

- Verify SDF grid structure
- Check instance assignments
- Validate instance transformations
- Analyze scene graph structure

### Baking

- Use appropriate batch size
- Manage preserved history count
- Test with different export formats

### Research and Development

- Study SDF representation techniques
- Analyze grid optimization strategies
- Research transformation algorithms
- Develop new editing workflows
- Study baking optimization

### Educational Use

- Demonstrate SDF editing concepts
- Show transformation techniques
- Explain symmetry operations
- Teach grid plane manipulation
- Visualize primitive operations

## Related Components

### RenderGraph/RenderPass

Render pass base class:
- `RenderPass::reflect()` - Define input/output resources
- `RenderPass::execute()` - Execute render pass
- `RenderPass::renderUI()` - Render UI controls
- `RenderPass::getProperties()` - Serialize configuration
- `RenderPass::setScene()` - Set scene reference

### Core/Pass/FullScreenPass

Full-screen pass management:
- `FullScreenPass::create()` - Create full-screen pass
- `FullScreenPass::execute()` - Execute full-screen pass
- `FullScreenPass::getRootVar()` - Get root variable

### Scene/Scene

Scene system:
- `Scene::getCamera()` - Get camera
- `Scene::getAnimationController()` - Get animation controller
- `Scene::getSDFGridCount()` - Get SDF grid count
- `Scene::getGeometryInstanceCount()` - Get geometry instance count
- `Scene::getGeometryInstance()` - Get geometry instance
- `Scene::getSDFGrid()` - Get SDF grid
- `Scene::findSDFGridIDFromGeometryInstanceID()` - Find SDF grid ID
- `Scene::getGeometryInstanceIDsByType()` - Get geometry instance IDs by type

### Scene/SDFs/SDF3DPrimitiveFactory

SDF primitive factory:
- `SDF3DPrimitiveFactory::initCommon()` - Initialize common primitive data
- `SDF3DPrimitiveFactory::computeAABB()` - Compute axis-aligned bounding box

### Utils/Math

Matrix operations:
- `math::rotate()` - Matrix rotation
- `math::transpose()` - Matrix transpose
- `math::compose()` - Matrix composition
- `math::inverse()` - Matrix inversion
- `math::decompose()` - Matrix decomposition
- `math::quatFromAngleAxis()` - Quaternion from angle axis

### Utils/UI/Gui

UI framework for rendering controls:
- Provides checkbox, var, group, text, dropdown, dummy controls
- Simple UI widget interface
- Real-time control updates

## Progress Log

- **2026-01-07T22:10:00Z**: Completed SDFEditor analysis. Analyzed SDFEditor.h (253 lines) and SDFEditor.cpp (1736 lines). Created comprehensive technical specification covering 2D GUI system, marker system, selection wheel, grid plane manipulation, symmetry operations, transformation state machine, edit history tracking, baking system, picking, keyboard/mouse event handling, and integration with scene/SDFGrid systems.
