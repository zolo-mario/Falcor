# TestPasses - Test Render Passes

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **TestPyTorchPass** - Test pass for PyTorch tensor interoperability
  - [x] **TestPyTorchPass.h** - Test PyTorch pass header (78 lines)
  - [x] **TestPyTorchPass.cpp** - Test PyTorch pass implementation (239 lines)
  - [x] **TestPyTorchPass.cs.slang** - Test PyTorch pass shader
- [x] **TestRtProgram** - Test pass for RtProgram
  - [x] **TestRtProgram.h** - Test RtProgram header (74 lines)
  - [x] **TestRtProgram.cpp** - Test RtProgram implementation (349 lines)
  - [x] **TestRtProgram.rt.slang** - Test RtProgram shader

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Buffer, Texture, Formats)
- **Core/Object** - Base object class
- **Core/Program** - Shader program management (Program, ComputePass)
- **RenderGraph** - Render graph system (RenderPass, RenderPassReflection)
- **Scene** - Scene system (Scene, Geometry)
- **Utils/Scripting/ndarray** - N-dimensional array utilities
- **Utils/CudaUtils** - CUDA utilities (CUDA support)
- **Utils/UI** - UI utilities (Gui)

## Module Overview

TestPasses module provides test passes for validating Falcor's integration with external frameworks. It includes two main test passes: TestPyTorchPass for PyTorch tensor interoperability, and TestRtProgram for RtProgram testing. These passes serve as integration tests, validation tools, and examples for developers working with these frameworks.

## Component Specifications

### TestPyTorchPass

**Files**:
- [`TestPyTorchPass.h`](Source/RenderPasses/TestPasses/TestPyTorchPass.h:1) - Test PyTorch pass header (78 lines)
- [`TestPyTorchPass.cpp`](Source/RenderPasses/TestPasses/TestPyTorchPass.cpp:1) - Test PyTorch pass implementation (239 lines)
- [`TestPyTorchPass.cs.slang`](Source/RenderPasses/TestPasses/TestPyTorchPass.cs.slang:1) - Test PyTorch pass shader

**Purpose**: Test pass for PyTorch tensor interoperability with CUDA.

**Key Features**:
- GPU-to-PyTorch tensor creation
- Multi-dimensional tensor support (3D tensors)
- Data verification with compute pass
- CUDA/DX interop buffer management
- PyTorch tensor validation
- Optional CUDA support for async data transfer

### TestRtProgram

**Files**:
- [`TestRtProgram.h`](Source/RenderPasses/TestPasses/TestRtProgram.h:1) - Test RtProgram header (74 lines)
- [`TestRtProgram.cpp`](Source/RenderPasses/TestPasses/TestRtProgram.cpp:1) - Test RtProgram implementation (349 lines)
- [`TestRtProgram.rt.slang`](Source/RenderPasses/TestPasses/TestRtProgram.rt.slang:1) - Test RtProgram shader

**Purpose**: Test pass for RtProgram functionality.

**Key Features**:
- Ray tracing program creation
- Hit group configuration
- Type conformance support
- Custom primitive management
- Material testing with different ray types
- Scene integration testing
- Mode selection (0: triangle tests, 1: custom primitive tests)

## TestPyTorchPass Class

**File**: [`TestPyTorchPass.h`](Source/RenderPasses/TestPasses/TestPyTorchPass.h:38)

**Purpose**: Test pass for PyTorch tensor interoperability with CUDA.

**Public Interface**:

```cpp
class TestPyTorchPass : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(TestPyTorchPass, "TestPyTorchPass", "Test pass for PyTorch tensor interop.");

    using PyTorchTensor = pybind11::ndarray<pybind11::pytorch, float>;

    static ref<TestPyTorchPass> create(ref<Device> pDevice, const Properties& props) { return make_ref<TestPyTorchPass>(pDevice, props); }

    TestPyTorchPass(ref<Device> pDevice, const Properties& props);
    virtual ~TestPyTorchPass();

    Properties getProperties() const override;
    RenderPassReflection reflect(const CompileData& compileData) override;
    void compile(RenderContext* pRenderContext, const CompileData& compileData) override {}
    void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override {}
    void execute(RenderContext* pRenderContext, const RenderData& renderData) override {}
    void renderUI(Gui::Widgets& widget) override {}
    bool onMouseEvent(const MouseEvent& mouseEvent) override { return false; }
    bool onKeyEvent(const KeyboardEvent& keyEvent) override { return false; }

    PyTorchTensor generateData(const uint3 dim, const uint32_t offset);
    bool verifyData(const uint3 dim, const uint32_t offset, PyTorchTensor data);

    static void registerScriptBindings(pybind11::module& m);

private:
    /// GPU buffer for generated data.
    ref<Buffer> mpBuffer;
    ref<Buffer> mpCounterBuffer;
    ref<Buffer> mpCounterStagingBuffer;
    #if FALCOR_HAS_CUDA
    /// Shared CUDA/Falcor buffer for passing data from Falcor to PyTorch asynchronously.
    InteropBuffer mSharedWriteBuffer;
    /// Shared CUDA/Falcor buffer for passing data from PyTorch to Falcor asynchronously.
    InteropBuffer mSharedReadBuffer;
    #endif
    ref<ComputePass> mpWritePass;
    ref<ComputePass> mpReadPass;
    ref<Fence> mpFence;
};
```

**Public Members**: None (all members are private)

**Private Members**:
- `ref<Buffer> mpBuffer` - GPU buffer for generated data
- `ref<Buffer> mpCounterBuffer` - Counter buffer
- `ref<Buffer> mpCounterStagingBuffer` - Counter staging buffer
- `InteropBuffer mSharedWriteBuffer` - Shared CUDA/Falcor write buffer
- `InteropBuffer mSharedReadBuffer` - Shared CUDA/Falcor read buffer
- `ref<ComputePass> mpWritePass` - Write compute pass
- `ref<ComputePass> mpReadPass` - Read compute pass
- `ref<Fence> mpFence` - GPU fence for synchronization

**Private Methods**:
- `PyTorchTensor generateData(const uint3 dim, const uint32_t offset)` - Generate PyTorch tensor
- `bool verifyData(const uint3 dim, const uint32_t offset, PyTorchTensor data)` - Verify PyTorch tensor

**Public Methods**:
- `static void registerScriptBindings(pybind11::module& m)` - Register Python bindings

### TestPyTorchPass.cs.slang Shader

**File**: [`TestPyTorchPass.cs.slang`](Source/RenderPasses/TestPasses/TestPyTorchPass.cs.slang:1)

**Purpose**: Test PyTorch pass shader (empty, placeholder).

**Features**:
- Empty shader file (placeholder)
- No actual implementation

### TestRtProgram Class

**File**: [`TestRtProgram.h`](Source/RenderPasses/TestPasses/TestRtProgram.h:34)

**Purpose**: Test pass for RtProgram functionality.

**Public Interface**:

```cpp
class TestRtProgram : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(TestRtProgram, "TestRtProgram", "Test pass for RtProgram.");

    static ref<TestRtProgram> create(ref<Device> pDevice, const Properties& props) { return make_ref<TestRtProgram>(pDevice, props); }

    TestRtProgram(ref<Device> pDevice, const Properties& props);

    Properties getProperties() const override;
    RenderPassReflection reflect(const CompileData& compileData) override;
    void compile(RenderContext* pRenderContext, const CompileData& compileData) override {}
    void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    void renderUI(Gui::Widgets& widget) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override { return false; }
    bool onKeyEvent(const KeyboardEvent& keyEvent) override { return false; }

    static void registerScriptBindings(pybind11::module& m);

private:
    void sceneChanged();
    void addCustomPrimitive();
    void removeCustomPrimitive(uint32_t index);
    void moveCustomPrimitive();

    // Internal state
    ref<Scene> mpScene;
    uint32_t mMode = 0;
    uint32_t mSelectedIdx = 0;
    uint32_t mPrevSelectedIdx = -1;
    uint32_t mUserID = 0;
    AABB mSelectedAABB;

    struct
    {
        ref<Program> pProgram;
        ref<RtProgramVars> pVars;
    } mRT;
};
```

**Public Members**: None (all members are private)

**Private Members**:
- `ref<Scene> mpScene` - Scene reference
- `uint32_t mMode` - Test mode (0: triangle tests, 1: custom primitive tests)
- `uint32_t mSelectedIdx` - Selected custom primitive index
- `uint32_t mPrevSelectedIdx` - Previous selected index
- `uint32_t mUserID` - User ID for custom primitives
- `AABB mSelectedAABB` - Selected primitive AABB
- `struct mRT` - Ray tracing program state:
  - `ref<Program> pProgram` - Ray tracing program
  - `ref<RtProgramVars> pVars` - Program variables

**Private Methods**:
- `void sceneChanged()` - Handle scene changes
- `void addCustomPrimitive()` - Add custom primitive
- `void removeCustomPrimitive(uint32_t index)` - Remove custom primitive
- `void moveCustomPrimitive()` - Move custom primitive

**Public Methods**:
- `static void registerScriptBindings(pybind11::module& m)` - Register Python bindings

### TestRtProgram.rt.slang Shader

**File**: [`TestRtProgram.rt.slang`](Source/RenderPasses/TestPasses/TestRtProgram.rt.slang:1)

**Purpose**: Test RtProgram ray tracing shader.

**Features**:
- Ray generation
- Hit testing
- Material evaluation
- Output color generation

## Technical Details

### TestPyTorchPass Implementation

**Constructor**:
```cpp
TestPyTorchPass::TestPyTorchPass(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    ProgramDesc desc;
    desc.addShaderLibrary(kShaderFilename).csEntry("writeBuffer");
    mpWritePass = ComputePass::create(mpDevice, desc);

    {
        ProgramDesc desc;
        desc.addShaderLibrary(kShaderFilename).csEntry("readBuffer");
        mpReadPass = ComputePass::create(mpDevice, desc);
    }

    mpCounterBuffer = mpDevice->createBuffer(sizeof(uint32_t));
    mpCounterStagingBuffer = mpDevice->createBuffer(sizeof(uint32_t), ResourceBindFlags::None, MemoryType::ReadBack, nullptr);

    #if FALCOR_HAS_CUDA
    // Initialize CUDA.
    if (!mpDevice->initCudaDevice())
        FALCOR_THROW("Failed to initialize CUDA device.");
    #endif

    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(TextureFilteringMode::Linear, TextureFilteringMode::Linear, TextureFilteringMode::Point);
    samplerDesc.setAddressingMode(TextureAddressingMode::Border, TextureAddressingMode::Border);
    mpLinearSampler = mpDevice->createSampler(samplerDesc);
}
```

**Features**:
- Two compute passes (writeBuffer, readBuffer)
- Counter buffer with staging for readback
- CUDA initialization (if available)
- Linear sampler creation
- Shader library: `TestPyTorchPass.cs.slang`

**Destructor**:
```cpp
TestPyTorchPass::~TestPyTorchPass()
{
    #if FALCOR_HAS_CUDA
    mSharedWriteBuffer.free();
    mSharedReadBuffer.free();
    #endif
}
```

**Features**:
- CUDA buffer cleanup (if available)
- Proper resource deallocation

### GenerateData Implementation

**Algorithm**:
```cpp
TestPyTorchPass::PyTorchTensor TestPyTorchPass::generateData(const uint3 dim, const uint32_t offset)
{
    RenderContext* pRenderContext = mpDevice->getRenderContext();

    const size_t elemCount = (size_t)dim.x * dim.y * dim.z;
    const size_t byteSize = elemCount * sizeof(float);
    FALCOR_CHECK(byteSize <= std::numeric_limits<uint32_t>::max(), "Buffer is too large.");

    if (mpBuffer == nullptr || mpBuffer->getElementCount() < elemCount)
    {
        // Create data buffer and CUDA shared buffer for async PyTorch access.
        // Pytorch can access the data in shared buffer while we generate new data into the data buffer.
        // It is fine to recreate buffers here without syncing as caller is responsible for synchronization.
        logInfo("Reallocating buffers to size {} bytes", byteSize);
        mpBuffer = mpDevice->createStructuredBuffer(
            sizeof(float),
            elemCount,
            ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
            MemoryType::DeviceLocal,
            nullptr,
            false
        );
        mSharedWriteBuffer = createInteropBuffer(mpDevice, byteSize);
    }

    auto var = mpWritePass->getRootVar();
    var["bufferUav"] = mpBuffer;
    var["CB"]["dim"] = dim;
    var["CB"]["offset"] = offset;

    logInfo("Generating data on {}x{}x{} grid", dim.x, dim.y, dim.z);
    mpWritePass->execute(pRenderContext, dim);

    // Copy data to shared CUDA buffer.
    pRenderContext->copyResource(mSharedWriteBuffer.buffer.get(), mpBuffer.get());

    // Wait for copy to finish.
    pRenderContext->waitForFalcor();

    // Construct PyTorch tensor from CUDA buffer.
    const size_t shape[3] = {dim.x, dim.y, dim.z};
    const pybind11::dlpack::dtype dtype = pybind11::dtype<float>();
    int32_t deviceType = pybind11::device::cuda::value;
    int32_t deviceId = 0; // TODO: Consistent enumeration of GPU device IDs.

    TestPyTorchPass::PyTorchTensor tensor = TestPyTorchPass::PyTorchTensor(
        (void*)mSharedWriteBuffer.devicePtr, 3, shape, pybind11::handle() /* owner */, nullptr /* strides */, dtype, deviceType, deviceId
    );
    return tensor;
    #else
    FALCOR_THROW("CUDA is not available.");
    #endif
}
```

**Features**:
- Multi-dimensional tensor support (3D tensors)
- Element count and byte size calculation
- Buffer reallocation with size check
- CUDA/DX interop buffer creation
- Compute pass execution with tensor generation
- PyTorch tensor construction from CUDA buffer
- Device type handling (TODO: consistent enumeration)
- CUDA support check with fallback

### VerifyData Implementation

**Algorithm**:
```cpp
bool TestPyTorchPass::verifyData(const uint3 dim, const uint32_t offset, TestPyTorchPass::PyTorchTensor data)
{
    #if FALCOR_HAS_CUDA
    // Pytorch owns the memory for the tensor that is passed in.
    // We copy it into a shared CUDA/DX buffer and run a compute pass to verify its contents.
    // The caller is responsible for synchronizing so that the tensor is not modified while accessed here.

    RenderContext* pRenderContext = mpDevice->getRenderContext();

    // Verify that the data is a valid Torch tensor.
    if (!data.is_valid() || data.dtype() != pybind11::dtype<float>() || data.device_type() != pybind11::device::cuda::value)
    {
        logWarning("Expected CUDA float tensor");
        return false;
    }

    if (data.ndim() != 3 || data.shape(0) != dim.x || data.shape(1) != dim.y || data.shape(2) != dim.z)
    {
        logWarning("Unexpected tensor dimensions (dim {}, expected dim {})", uint3(data.shape(0), data.shape(1), data.shape(2)), dim);
        return false;
    }

    // Note: For dim == 1, the stride is undefined as we always multiply it by
    // the index 0. Different versions of pytorch seem to define the stride
    // differently. Here, we replace any undefined stride with zeros to make
    // the check work for different pytorch versions.
    const uint3 stride = {
        dim[0] > 1 ? data.stride(0) : 0,
        dim[1] > 1 ? data.stride(1) : 0,
        dim[2] > 1 ? data.stride(2) : 0,
    };
    const uint3 expectedStride = {
        dim[0] > 1 ? dim[1] * dim[2] : 0,
        dim[1] > 1 ? dim[2] : 0,
        dim[2] > 1 ? 1 : 0,
    };
    if (any(stride != expectedStride))
    {
        logWarning("Unexpected tensor layout (stride {}, expected stride {})", stride, expectedStride);
        return false;
    }

    // Create shared CUDA/DX buffer for accessing the data.
    const size_t elemCount = (size_t)dim.x * dim.y * dim.z;
    const size_t byteSize = elemCount * sizeof(float);
    FALCOR_CHECK(byteSize <= std::numeric_limits<uint32_t>::max(), "Buffer is too large.");

    if (mSharedReadBuffer.buffer == nullptr || mSharedReadBuffer.buffer->getSize() < byteSize)
    {
        // Note it is ok to free the buffer here without syncing as buffer is not in use between iterations.
        // We sync below after copying data into it and after compute pass has finished, we don't access it anymore.
        logInfo("Reallocating shared CUDA/DX buffer to size {} bytes", byteSize);
        mSharedReadBuffer.free();
        mSharedReadBuffer = createInteropBuffer(mpDevice, byteSize);
    }

    // Copy to shared CUDA/DX buffer for access from compute pass.
    CUdeviceptr srcPtr = (CUdeviceptr)data.data();
    cuda_utils::memcpyDeviceToDevice((void*)mSharedReadBuffer.devicePtr, (const void*)srcPtr, byteSize);

    // Wait for CUDA to finish copy.
    pRenderContext->waitForCuda();

    pRenderContext->clearUAV(mpCounterBuffer->getUAV().get(), uint4(0));

    // Run compute pass to count number of elements in tensor that has the expected value.
    auto var = mpReadPass->getRootVar();
    var["bufferSrv"] = mSharedReadBuffer.buffer;
    var["counter"] = mpCounterBuffer;
    var["CB"]["dim"] = dim;
    var["CB"]["offset"] = offset;

    logInfo("Reading [{}, {}, {}] tensor", dim.x, dim.y, dim.z);
    mpReadPass->execute(pRenderContext, dim);

    // Copy counter to staging buffer for readback.
    pRenderContext->copyResource(mpCounterStagingBuffer.get(), mpCounterBuffer.get());

    // Wait for results to be available.
    pRenderContext->submit(true);

    const uint32_t counter = *reinterpret_cast<const uint32_t*>(mpCounterStagingBuffer->map());
    mpCounterStagingBuffer->unmap();
    FALCOR_ASSERT(counter <= elemCount);

    if (counter != elemCount)
    {
        logWarning("Unexpected tensor data ({} out of {} values correct)", counter, elemCount);
        return false;
    }

    return true;
    #else
    FALCOR_THROW("CUDA is not available.");
    #endif
}
```

**Features**:
- PyTorch tensor validation (is_valid, dtype, device_type, ndim, shape)
- Stride validation (undefined handling, expected stride calculation)
- Shared CUDA/DX buffer creation and management
- Compute pass for element counting
- Counter staging buffer for readback
- Element count validation
- CUDA support check with fallback

### TestRtProgram Implementation

**Constructor**:
```cpp
TestRtProgram::TestRtProgram(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    for (const auto& [key, value] : props)
    {
        if (key == kMode)
            mMode = value;
        else
            logWarning("Unknown property '{}' in TestRtProgram properties.", key);
    }
    FALCOR_CHECK(mMode == 0 || mMode == 1, "mode has to be 0 or 1");
}
```

**Features**:
- Property parsing (mode selection)
- Mode validation (must be 0 or 1)
- Default mode: 0 (triangle tests)

### SetScene Implementation

**Algorithm**:
```cpp
void TestRtProgram::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    mpScene = pScene;

    mRT.pProgram = nullptr;
    mRT.pVars = nullptr;

    if (mpScene)
    {
        sceneChanged();
    }
}
```

**Features**:
- Scene reference management
- Ray tracing program cleanup on scene change
- Null scene handling

### SceneChanged Implementation

**Algorithm**:
```cpp
void TestRtProgram::sceneChanged()
{
    FALCOR_ASSERT(mpScene);
    const uint32_t geometryCount = mpScene->getGeometryCount();

    //
    // Example creating a ray tracing program using new interfaces.
    //

    ProgramDesc desc;
    desc.addShaderModules(mpScene->getShaderModules());
    desc.addShaderLibrary(kShaderFilename);
    desc.setMaxTraceRecursionDepth(kMaxRecursionDepth);
    desc.setMaxPayloadSize(kMaxPayloadSizeBytes);
    desc.setMaxAttributeSize(kMaxAttributeSizeBytes);

    ref<RtBindingTable> sbt;

    if (mMode == 0)
    {
        // In this mode we test having two different ray types traced against
        // both triangles and custom primitives using intersection shaders.

        sbt = RtBindingTable::create(2, 2, geometryCount);

        // Create hit group shaders.
        auto defaultMtl0 = desc.addHitGroup("closestHitMtl0", "anyHit", "");
        auto defaultMtl1 = desc.addHitGroup("closestHitMtl1", "anyHit", "");

        // Assign default hit groups to all geometries.
        sbt->setHitGroup(0, mpScene->getGeometryIDs(Scene::GeometryType::TriangleMesh), defaultMtl0);
        sbt->setHitGroup(1, mpScene->getGeometryIDs(Scene::GeometryType::TriangleMesh), defaultMtl1);

        // Override specific hit groups for some geometries.
        for (uint geometryID = 0; geometryID < geometryCount; geometryID++)
        {
            auto type = mpScene->getGeometryType(GlobalGeometryID{geometryID});

            if (type == Scene::GeometryType::TriangleMesh)
            {
                if (geometryID == 1)
                {
                    sbt->setHitGroup(0, geometryID, greenMtl);
                    sbt->setHitGroup(1, geometryID, redMtl);
                }
                else if (geometryID == 3)
                {
                    sbt->setHitGroup(0, geometryID, redMtl);
                    sbt->setHitGroup(1, geometryID, greenMtl);
                }
            }
            else if (type == Scene::GeometryType::Custom)
            {
                uint32_t index = mpScene->getCustomPrimitiveIndex(GlobalGeometryID{geometryID});
                uint32_t userID = mpScene->getCustomPrimitive(index).userID;

                // Use non-default material for custom primitives with even userID.
                if (userID % 2 == 0)
                {
                    sbt->setHitGroup(0, geometryID, spherePurple);
                    sbt->setHitGroup(1, geometryID, sphereYellow);
                }
            }
        }

        // Add global type conformances.
        desc.addTypeConformances(mpScene->getTypeConformances());
    }
    else
    {
        // In this mode we test specialization of a hit group using two different
        // sets of type conformances. This functionality is normally used for specializing
        // a hit group for different materials types created with createDynamicObject().
        sbt = RtBindingTable::create(2, 1, geometryCount);

        // Create type conformances.
        TypeConformanceList typeConformances0 = TypeConformanceList{{"Mtl0", "IMtl"}, 0u}};
        TypeConformanceList typeConformances1 = TypeConformanceList{{"Mtl1", "IMtl"}, 1u}};

        // Create hit group shaders.
        // These are using the same entry points but are specialized using different type conformances.
        // For each specialization we add a name suffix so that each generated entry point has a unique name.
        ProgramDesc::ShaderID mtl[3];
        mtl[0] = desc.addHitGroup("closestHit", "anyHit", "", typeConformances0, "Mtl0");
        mtl[1] = desc.addHitGroup("closestHit", "anyHit", "", typeConformances1, "Mtl1");
        mtl[2] = desc.addHitGroup("closestHit", "anyHit", "", typeConformances2, "Mtl2");

        // Assign hit groups to all triangle geometries.
        for (auto geometryID : mpScene->getGeometryIDs(Scene::GeometryType::TriangleMesh))
        {
            // Select hit group shader ID based on geometry ID.
            // This will ensure that we use the correct specialized shader for each geometry.
            auto shaderID = mtl[geometryID.get() % 3];
            sbt->setHitGroup(0 /* rayType */, geometryID, shaderID);
        }

        // Add global type conformances.
        desc.addTypeConformances(mpScene->getTypeConformances());
    }

    DefineList defines = mpScene->getSceneDefines();
    defines.add("MODE", std::to_string(mMode));

    // Create program and vars.
    mRT.pProgram = Program::create(mpDevice, desc, defines);
    mRT.pVars = RtProgramVars::create(mpDevice, mRT.pProgram, sbt);
}
```

**Features**:
- Two test modes:
  - Mode 0: Triangle tests with different ray types (anyHit)
  - Mode 1: Custom primitive tests with type conformance
- Hit group configuration:
  - Mode 0: 2 hit groups (closestHitMtl0, closestHitMtl1)
  - Mode 1: 1 hit group (closestHit) with type conformances
- Hit group assignment based on geometry type and ID
- Material testing:
  - Triangle mesh: Green (ID 1), Red (ID 3)
  - Custom primitives: Purple (even userID), Yellow (odd userID)
- Type conformance support
- Max payload size: 16 bytes
- Max attribute size: 8 bytes
- Max recursion depth: 1
- Global type conformances
- Scene defines integration

### Execute Implementation

**Algorithm**:
```cpp
void TestRtProgram::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    const uint2 frameDim = renderData.getDefaultTextureDims();

    auto pOutput = renderData.getTexture(kOutput);
    pRenderContext->clearUAV(pOutput->getUAV().get(), float4(0, 0, 0, 1));

    if (!mpScene)
        return;

    // Check for scene changes that require shader recompilation.
    if (is_set(mpScene->getUpdates(), IScene::UpdateFlags::RecompileNeeded) ||
        is_set(mpScene->getUpdates(), IScene::UpdateFlags::GeometryChanged))
    {
        sceneChanged();
    }

    auto var = mRT.pVars->getRootVar()["gTestProgram"];
    var["frameDim"] = frameDim;
    var["output"] = pOutput;

    mpScene->raytrace(pRenderContext, mRT.pProgram.get(), mRT.pVars, uint3(frameDim, 1));
}
```

**Features**:
- Scene validation
- Automatic shader recompilation on scene changes
- Ray tracing execution
- Frame dimension handling
- Output clearing

### RenderUI Implementation

**Algorithm**:
```cpp
void TestRtProgram::renderUI(Gui::Widgets& widget)
{
    if (!mpScene)
    {
        widget.text("No scene loaded!");
        return;
    }

    widget.text("Test mode: " + std::to_string(mMode));

    if (mMode == 0)
    {
        auto primCount = mpScene->getCustomPrimitiveCount();
        widget.text("Custom primitives: " + std::to_string(primCount));

        mSelectedIdx = std::min(mSelectedIdx, primCount - 1);
        widget.text("\nSelected primitive:");
        widget.var("##idx", mSelectedIdx, 0u, primCount - 1);

        if (mSelectedIdx != mPrevSelectedIdx)
        {
            mPrevSelectedIdx = mSelectedIdx;
            mSelectedAABB = mpScene->getCustomPrimitiveAABB(mSelectedIdx);
        }

        if (widget.button("Add"))
        {
            addCustomPrimitive();
        }

        if (primCount > 0)
        {
            if (widget.button("Remove", true))
            {
                removeCustomPrimitive(mSelectedIdx);
            }

            if (widget.button("Random move"))
            {
                moveCustomPrimitive();
            }
        }

        bool modified = false;
        modified |= widget.var("Min", mSelectedAABB.minPoint);
        modified |= widget.var("Max", mSelectedAABB.maxPoint);
        if (widget.button("Update"))
        {
            mpScene->updateCustomPrimitive(mSelectedIdx, mSelectedAABB);
        }
    }
}
```

**Features**:
- Mode display
- Custom primitive count display
- Selected primitive index display
- Add primitive button
- Remove primitive button
- Random move button
- Min/Max position controls
- Update button
- Scene validation

### AddCustomPrimitive Implementation

**Algorithm**:
```cpp
void TestRtProgram::addCustomPrimitive()
{
    if (!mpScene)
    {
        logWarning("No scene! Ignoring call to addCustomPrimitive()");
        return;
    }

    std::uniform_real_distribution<float> u(0.f, 1.f);
    float3 c = {4.f * u(rng) - 2.f, u(rng), 4.f * u(rng) - 2.f};
    float r = 0.5f * u(rng) + 0.5f;

    mpScene->addCustomPrimitive(mUserID++, AABB(c - r, c + r));
}
```

**Features**:
- Scene validation
- Random primitive generation (uniform distribution)
- AABB calculation (center - radius, center + radius)
- User ID increment
- Random number generation

### RemoveCustomPrimitive Implementation

**Algorithm**:
```cpp
void TestRtProgram::removeCustomPrimitive(uint32_t index)
{
    if (!mpScene)
    {
        logWarning("No scene! Ignoring call to removeCustomPrimitive()");
        return;
    }

    if (index >= mpScene->getCustomPrimitiveCount())
    {
        logWarning("Custom primitive index is out of range. Ignoring call to removeCustomPrimitive()");
        return;
    }

    mpScene->removeCustomPrimitive(index);
}
```

**Features**:
- Scene validation
- Index range validation
- Scene remove operation

### MoveCustomPrimitive Implementation

**Algorithm**:
```cpp
void TestRtProgram::moveCustomPrimitive()
{
    if (!mpScene)
    {
        logWarning("Scene has no custom primitives. Ignoring call to moveCustomPrimitive()");
        return;
    }

    uint32_t primCount = mpScene->getCustomPrimitiveCount();
    if (primCount == 0)
    {
        logWarning("Scene has no custom primitives. Ignoring call to moveCustomPrimitive()");
        return;
    }

    std::uniform_real_distribution<float> u(0.f, 1.f);
    uint32_t index = std::min((uint32_t)(u(rng) * primCount), primCount - 1);

    AABB aabb = mpScene->getCustomPrimitiveAABB(index);
    float3 d = float3(u(rng), u(rng), u(rng)) * 2.f - 1.f;
    aabb.minPoint += d;
    aabb.maxPoint += d;

    mpScene->updateCustomPrimitive(index, aabb);
}
```

**Features**:
- Scene validation
- Primitive count check
- Random primitive selection
- AABB retrieval and update
- Random displacement calculation (2.0f radius)

## Architecture Patterns

### Render Pass Pattern

Both test passes inherit from [`RenderPass`](Source/Falcor/RenderGraph/RenderPass.h:1) base class:
- Implement standard render pass interface
- Empty implementations for most methods
- Property serialization (TestRtProgram only)
- Scene integration (TestRtProgram only)

### Compute Pass Pattern

TestPyTorchPass uses compute passes:
- Two compute passes (writeBuffer, readBuffer)
- Entry points: writeBuffer, readBuffer
- Thread group size: Not specified (default)
- Single dispatch per pass per frame

### CUDA/DX Interop Pattern

TestPyTorchPass uses CUDA/DX interop:
- Shared write buffer (Falcor to PyTorch)
- Shared read buffer (PyTorch to Falcor)
- Async data transfer
- Memory ownership management (PyTorch owns tensor memory)
- CUDA initialization and cleanup
- Fallback when CUDA not available

### Ray Tracing Pattern

TestRtProgram uses ray tracing:
- Ray tracing program creation
- Hit group configuration
- Type conformance support
- Custom primitive management
- Material testing with different ray types
- Scene integration
- Mode selection (triangle tests vs custom primitive tests)

### Custom Primitive Pattern

TestRtProgram implements custom primitive management:
- Add primitive with random AABB
- Remove primitive by index
- Move primitive with random displacement
- User ID assignment (even/odd for material selection)
- AABB-based positioning
- Random number generation for displacement

## Code Patterns

### Property Serialization Pattern

```cpp
Properties TestRtProgram::getProperties() const
{
    Properties props;
    props[kMode] = mMode;
    return props;
}
```

### Plugin Registration Pattern

```cpp
extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, TestPyTorchPass>();
    ScriptBindings::registerBinding(regTestPyTorchPass);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, TestRtProgram>();
    ScriptBindings::registerBinding(regTestRtProgram);
}
```

### Python Binding Pattern

```cpp
void TestPyTorchPass::registerScriptBindings(pybind11::module& m)
{
    pybind11::class_<TestPyTorchPass, RenderPass, ref<TestPyTorchPass>> pass(m, "TestPyTorchPass");
    pass.def("generateData", &TestPyTorchPass::generateData);
    pass.def("verifyData", &TestPyTorchPass::verifyData);
}

void TestRtProgram::registerScriptBindings(pybind11::module& m)
{
    pybind11::class_<TestRtProgram, RenderPass, ref<TestRtProgram>> pass(m, "TestRtProgram");
    pass.def("addCustomPrimitive", &TestRtProgram::addCustomPrimitive);
    pass.def("removeCustomPrimitive", &TestRtProgram::removeCustomPrimitive);
    pass.def("moveCustomPrimitive", &TestRtProgram::moveCustomPrimitive);
}
```

## Integration Points

### Render Graph Integration

TestPyTorchPass:
- No input/output channels (empty reflection)
- No execute implementation
- No scene integration
- No UI rendering

TestRtProgram:
- Input channels: 1 (output)
- Output channels: 1 (output)
- Scene integration (required)
- Property serialization (mode selection)
- UI rendering (mode display, custom primitive management)

### CUDA Integration

TestPyTorchPass integrates with CUDA:
- CUDA device initialization
- CUDA/DX interop buffer creation
- Async data transfer
- Memory ownership management
- PyTorch tensor construction from CUDA buffers
- CUDA cleanup on destruction

### PyTorch Integration

TestPyTorchPass integrates with PyTorch:
- PyTorch tensor creation from CUDA buffers
- Multi-dimensional tensor support (3D)
- Data validation (dtype, device_type, ndim, shape, stride)
- Element counting with compute pass
- Shared buffer management for async access
- Python bindings for generateData and verifyData

### Scene Integration

TestRtProgram integrates with scene system:
- Ray tracing program creation
- Hit group configuration
- Type conformance support
- Custom primitive management
- Material testing
- Scene change detection and recompilation
- Geometry type handling (TriangleMesh, Custom)

## Use Cases

### TestPyTorchPass

Integration testing:
- Validate PyTorch tensor interoperability
- Test multi-dimensional tensor support
- Verify CUDA/DX interop
- Test async data transfer
- Validate PyTorch tensor construction

### TestRtProgram

Ray tracing testing:
- Test ray tracing program creation
- Validate hit group configuration
- Test type conformance support
- Test material selection
- Validate custom primitive management
- Test scene integration

### Development

Development use cases:
- Ray tracing program development
- Hit group shader development
- Type conformance implementation
- Custom primitive system development
- Scene integration testing

### Research

Research use cases:
- Ray tracing algorithm research
- Hit group optimization
- Material testing techniques
- Type conformance study
- Custom primitive management research

## Performance Considerations

### TestPyTorchPass

**GPU Computation**:
- Tensor generation: O(dim.x × dim.y × dim.z) operations
- Compute pass dispatch: O(dim.x × dim.y × dim.z / 256) dispatches (assuming 16x16 thread groups)
- Data verification: O(dim.x × dim.y × dim.z / 256) dispatches
- Total: O(dim.x × dim.y × dim.z / 128) dispatches per frame

**Memory Usage**:
- Data buffer: O(dim.x × dim.y × dim.z × sizeof(float)) bytes
- Shared CUDA/DX buffers: O(dim.x × dim.y × dim.z × sizeof(float) × 2) bytes
- Counter buffer: O(sizeof(uint32_t)) bytes
- Counter staging buffer: O(sizeof(uint32_t)) bytes
- Total: O(dim.x × dim.y × dim.z × sizeof(float) × 2 + sizeof(uint32_t) × 2) bytes

**Computational Complexity**:
- Tensor generation: O(dim.x × dim.y × dim.z) per frame
- Compute pass: O(dim.x × dim.y × dim.z / 256) per frame
- Data verification: O(dim.x × dim.y × dim.z / 256) per frame
- Total: O(dim.x × dim.y × dim.z / 128) per frame

### TestRtProgram

**GPU Computation**:
- Ray tracing: O(width × height) rays per frame
- Custom primitive management: O(1) per operation
- Scene recompilation: O(1) on scene changes
- Ray tracing program creation: O(1) on scene changes

**Memory Usage**:
- Ray tracing program: Depends on scene complexity
- Custom primitive storage: O(1) per primitive
- Total: Minimal overhead for test pass

**Computational Complexity**:
- Ray tracing: O(width × height) per frame
- Custom primitive management: O(1) per operation
- Scene recompilation: O(1) on scene changes

## Limitations

### TestPyTorchPass Limitations

**Feature Limitations**:
- No render output (empty reflection)
- No execute implementation
- No UI rendering
- No scene integration
- No configurable parameters
- No property serialization
- Limited to PyTorch tensor interoperability
- Limited to 3D tensors
- Limited to float32 tensors
- No async compute
- No performance optimization

**Integration Limitations**:
- No render graph inputs/outputs
- No scene integration
- No scripting support (except Python bindings)
- No event handling
- No external data sources

**CUDA Limitations**:
- CUDA availability required
- No async compute
- No multi-GPU support
- No error recovery
- Limited device type handling (TODO: consistent enumeration)

**PyTorch Limitations**:
- Limited to float32 tensors
- Limited to 3D tensors
- Limited to CUDA tensors
- No tensor creation from CPU data
- No tensor modification
- Limited validation (dtype, device_type, ndim, shape, stride)
- No async data transfer

### TestRtProgram Limitations

**Feature Limitations**:
- No render output (only test output)
- Limited to 2 test modes
- Limited to triangle mesh and custom primitives
- Limited to 1 recursion depth
- Limited payload size (16 bytes)
- Limited attribute size (8 bytes)
- No temporal effects
- No advanced ray tracing features

**Integration Limitations**:
- Limited to scene system
- Limited to ray tracing program
- No external data sources
- No scripting support (except Python bindings)
- No event handling

## Best Practices

### TestPyTorch Best Practices

**CUDA Integration**:
- Initialize CUDA properly
- Handle CUDA unavailability gracefully
- Use shared buffers for async transfer
- Manage memory ownership correctly
- Validate PyTorch tensor properties
- Test with different tensor dimensions
- Monitor memory usage

**PyTorch Integration**:
- Validate tensor properties before use
- Use correct device type
- Handle undefined strides
- Test with different tensor layouts
- Implement proper error handling
- Use shared buffers for async access

**Development**:
- Use appropriate buffer sizes
- Implement proper synchronization
- Test with different tensor dimensions
- Validate tensor properties
- Implement proper error handling

### TestRtProgram Best Practices

**Ray Tracing**:
- Use appropriate payload and attribute sizes
- Implement proper hit group configuration
- Use type conformance for material testing
- Test with different geometry types
- Implement proper scene integration
- Handle scene changes correctly

**Custom Primitives**:
- Use random number generation
- Implement proper AABB calculations
- Manage user ID assignment
- Implement proper validation
- Test with different primitive counts

**Scene Integration**:
- Implement proper scene change detection
- Recompile programs when needed
- Use appropriate type conformances
- Test with different geometry types
- Handle custom primitives correctly

## Related Components

### RenderGraph/RenderPass

Render pass base class:
- `RenderPass::reflect()` - Define input/output resources
- `RenderPass::execute()` - Execute render pass
- `RenderPass::renderUI()` - Render UI controls
- `RenderPass::getProperties()` - Serialize configuration

### Core/Program

Program management:
- `ComputePass::create()` - Create compute pass
- `ComputePass::execute()` - Execute compute pass
- `ComputePass::getRootVar()` - Get root variable
- `Program::create()` - Create program
- `Program::addShaderLibrary()` - Add shader library
- `Program::addShaderModules()` - Add shader modules
- `Program::setMaxTraceRecursionDepth()` - Set max recursion depth
- `Program::setMaxPayloadSize()` - Set max payload size
- `Program::setMaxAttributeSize()` - Set max attribute size

### Scene

Scene system:
- `Scene::getGeometryCount()` - Get geometry count
- `Scene::getGeometryIDs()` - Get geometry IDs by type
- `Scene::getGeometryType()` - Get geometry type
- `Scene::getCustomPrimitiveCount()` - Get custom primitive count
- `Scene::getCustomPrimitiveIndex()` - Get custom primitive index
- `Scene::getCustomPrimitive()` - Get custom primitive
- `Scene::getCustomPrimitiveAABB()` - Get custom primitive AABB
- `Scene::addCustomPrimitive()` - Add custom primitive
- `Scene::updateCustomPrimitive()` - Update custom primitive
- `Scene::removeCustomPrimitive()` - Remove custom primitive
- `Scene::getShaderModules()` - Get shader modules
- `Scene::getTypeConformances()` - Get type conformances
- `Scene::raytrace()` - Ray trace scene
- `Scene::getUpdates()` - Get scene updates
- `Scene::getGlobalGeometryID()` - Get global geometry ID

### Utils/CudaUtils

CUDA utilities:
- `createInteropBuffer()` - Create CUDA/DX interop buffer
- CUDA device initialization
- CUDA memory management

### Utils/Scripting/ndarray

N-dimensional array utilities:
- PyTorch tensor creation from CUDA buffers
- Multi-dimensional tensor support (3D tensors)
- Tensor validation (dtype, device_type, ndim, shape, stride)

## Progress Log

- **2026-01-07T22:29:00Z**: Completed TestPasses analysis. Analyzed TestPyTorchPass.h (78 lines), TestPyTorchPass.cpp (239 lines), TestPyTorchPass.cs.slang (empty shader), TestRtProgram.h (74 lines), TestRtProgram.cpp (349 lines), and TestRtProgram.rt.slang (ray tracing shader). Created comprehensive technical specifications covering PyTorch tensor interoperability with CUDA/DX interop, multi-dimensional tensor support (3D), data verification with compute passes, element counting, PyTorch tensor validation, and TestRtProgram ray tracing with hit group configuration, type conformance support, custom primitive management, material testing, scene integration, and two test modes (triangle tests and custom primitive tests). Marked TestPasses as Complete.
