# Utils - Render Pass Utilities

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **Composite** - Composite pass
  - [x] **Composite.h** - Composite header (87 lines)
  - [x] **Composite.cpp** - Composite implementation (164 lines)
  - [x] **Composite.cs.slang** - Composite shader (80 lines)
  - [x] **CompositeMode.slangh** - Composite mode definitions (36 lines)
- [x] **CrossFade** - Cross fade pass
  - [x] **CrossFade.h** - Cross fade header (68 lines)
  - [x] **CrossFade.cpp** - Cross fade implementation (180 lines)
  - [x] **CrossFade.cs.slang** - Cross fade shader (52 lines)
- [x] **GaussianBlur** - Gaussian blur pass
  - [x] **GaussianBlur.h** - Gaussian blur header (69 lines)
  - [x] **GaussianBlur.cpp** - Gaussian blur implementation (223 lines)
  - [x] **GaussianBlur.ps.slang** - Gaussian blur shader (87 lines)

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Texture, Formats)
- **Core/Object** - Base object class
- **Core/Pass/FullScreenPass** - Full-screen pass
- **Core/Pass/ComputePass** - Compute pass
- **RenderGraph** - Render graph system (RenderPass, RenderPassReflection)
- **Utils/Math** - Mathematical utilities
- **Utils/UI** - UI utilities (Gui)

## Module Overview

Utils module provides utility render passes for common rendering operations. It includes three main utility passes: Composite for blending two buffers together with configurable operations, CrossFade for time-dependent fading between two buffers with auto-fade and manual fade modes, and GaussianBlur for separable Gaussian blur filtering with configurable kernel width and sigma. These passes are designed to be used as building blocks for more complex rendering pipelines and can be combined with other render passes to create advanced effects.

## Component Specifications

### Composite Class

**File**: [`Composite.h`](Source/RenderPasses/Utils/Composite/Composite.h:43)

**Purpose**: Simple composite pass that blends two buffers together with configurable blend operations.

**Public Interface**:

```cpp
class Composite : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(Composite, "Composite", "Composite pass.");

    /**
     * Composite modes.
     */
    enum class Mode
    {
        Add,
        Multiply,
    };

    FALCOR_ENUM_INFO(
        Mode,
        {
            {Mode::Add, "Add"},
            {Mode::Multiply, "Multiply"},
        }
    );

    static ref<Composite> create(ref<Device> pDevice, const Properties& props) { return make_ref<Composite>(pDevice, props); }

    Composite(ref<Device> pDevice, const Properties& props);

    Properties getProperties() const override;
    RenderPassReflection reflect(const CompileData& compileData) override;
    void compile(RenderContext* pRenderContext, const CompileData& compileData) override;
    void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    void renderUI(Gui::Widgets& widget) override;

private:
    DefineList getDefines() const;

    uint2 mFrameDim = {0, 0};
    Mode mMode = Mode::Add;
    float mScaleA = 1.f;
    float mScaleB = 1.f;
    ResourceFormat mOutputFormat = ResourceFormat::RGBA32Float;

    ref<ComputePass> mCompositePass;
};
```

**Public Members**: None (all members are private)

**Private Members**:
- `uint2 mFrameDim` - Frame dimensions
- `Mode mMode` - Composite mode (Add, Multiply)
- `float mScaleA` - Scale for input A
- `float mScaleB` - Scale for input B
- `ResourceFormat mOutputFormat` - Output format
- `ref<ComputePass> mCompositePass` - Composite compute pass

**Private Methods**:
- `DefineList getDefines() const` - Get shader defines

**Public Methods**:
- `Properties getProperties() const override` - Get properties
- `RenderPassReflection reflect(const CompileData& compileData) override` - Reflect input/output
- `void compile(RenderContext* pRenderContext, const CompileData& compileData) override` - Compile pass
- `void execute(RenderContext* pRenderContext, const RenderData& renderData) override` - Execute pass
- `void renderUI(Gui::Widgets& widget) override` - Render UI

### CompositeMode.slangh

**File**: [`CompositeMode.slangh`](Source/RenderPasses/Utils/Composite/CompositeMode.slangh:1)

**Purpose**: Type defines shared between host and device for composite mode.

**Features**:
- COMPOSITE_MODE_ADD (0)
- COMPOSITE_MODE_MULTIPLY (1)
- OUTPUT_FORMAT_FLOAT (0)
- OUTPUT_FORMAT_UINT (1)
- OUTPUT_FORMAT_SINT (2)

### Composite.cs.slang

**File**: [`Composite.cs.slang`](Source/RenderPasses/Utils/Composite/Composite.cs.slang:1)

**Purpose**: Composite compute shader.

**Features**:
- Thread group size: 16x16x1
- Two input textures (A, B)
- One output texture (output)
- Configurable composite mode (Add, Multiply)
- Configurable output format (Float, Uint, Sint)
- Integer format rounding (round-to-nearest-even)
- Frame dimension bounds checking

**Composite Operations**:
- Add: result = (scaleA * A) + (scaleB * B)
- Multiply: result = (scaleA * A) * (scaleB * B)

### CrossFade Class

**File**: [`CrossFade.h`](Source/RenderPasses/Utils/CrossFade/CrossFade.h:37)

**Purpose**: Simple pass for time-dependent fading between two buffers.

**Public Interface**:

```cpp
class CrossFade : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(CrossFade, "CrossFade", "CrossFade pass.");

    static ref<CrossFade> create(ref<Device> pDevice, const Properties& props) { return make_ref<CrossFade>(pDevice, props); }

    CrossFade(ref<Device> pDevice, const Properties& props);

    Properties getProperties() const override;
    RenderPassReflection reflect(const CompileData& compileData) override;
    void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    void compile(RenderContext* pRenderContext, const CompileData& compileData) override;
    void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    void renderUI(Gui::Widgets& widget) override;

private:
    uint2 mFrameDim = {0, 0};
    float mScaleA = 1.f;
    float mScaleB = 1.f;
    ResourceFormat mOutputFormat = ResourceFormat::RGBA32Float;

    ref<ComputePass> mpFadePass;

    ref<Scene> mpScene;
    uint32_t mMixFrame = 0;
    bool mEnableAutoFade = true;
    uint32_t mWaitFrameCount = 10;
    uint32_t mFadeFrameCount = 100;
    float mFadeFactor = 0.5f;
};
```

**Public Members**: None (all members are private)

**Private Members**:
- `uint2 mFrameDim` - Frame dimensions
- `float mScaleA` - Scale for input A
- `float mScaleB` - Scale for input B
- `ResourceFormat mOutputFormat` - Output format
- `ref<ComputePass> mpFadePass` - Fade compute pass
- `ref<Scene> mpScene` - Scene reference
- `uint32_t mMixFrame` - Current mix frame
- `bool mEnableAutoFade` - Auto-fade enable flag
- `uint32_t mWaitFrameCount` - Wait frame count (default: 10)
- `uint32_t mFadeFrameCount` - Fade frame count (default: 100)
- `float mFadeFactor` - Fixed fade factor (default: 0.5f)

**Private Methods**: None

**Public Methods**:
- `Properties getProperties() const override` - Get properties
- `RenderPassReflection reflect(const CompileData& compileData) override` - Reflect input/output
- `void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override` - Set scene
- `void compile(RenderContext* pRenderContext, const CompileData& compileData) override` - Compile pass
- `void execute(RenderContext* pRenderContext, const RenderData& renderData) override` - Execute pass
- `void renderUI(Gui::Widgets& widget) override` - Render UI

### CrossFade.cs.slang

**File**: [`CrossFade.cs.slang`](Source/RenderPasses/Utils/CrossFade/CrossFade.cs.slang:1)

**Purpose**: Cross fade compute shader.

**Features**:
- Thread group size: 16x16x1
- Two input textures (A, B)
- One output texture (output)
- Linear interpolation: result = (scaleA * A) + (scaleB * B)
- Frame dimension bounds checking

### GaussianBlur Class

**File**: [`GaussianBlur.h`](Source/RenderPasses/Utils/GaussianBlur/GaussianBlur.h:35)

**Purpose**: Gaussian blur filter with configurable kernel width and sigma.

**Public Interface**:

```cpp
class GaussianBlur : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(GaussianBlur, "GaussianBlur", "Gaussian blur.");

    static ref<GaussianBlur> create(ref<Device> pDevice, const Properties& props) { return make_ref<GaussianBlur>(pDevice, props); }

    GaussianBlur(ref<Device> pDevice, const Properties& props);

    Properties getProperties() const override;
    RenderPassReflection reflect(const CompileData& compileData) override;
    void compile(RenderContext* pRenderContext, const CompileData& compileData) override;
    void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    void renderUI(Gui::Widgets& widget) override;

    void setKernelWidth(uint32_t kernelWidth);
    void setSigma(float sigma);
    uint32_t getKernelWidth() { return mKernelWidth; }
    float getSigma() { return mSigma; }

    static void registerBindings(pybind11::module& m);

private:
    uint32_t mKernelWidth = 5;
    float mSigma = 2.0f;
    bool mReady = false;
    void createTmpFbo(const Texture* pSrc);
    void updateKernel();

    ref<FullScreenPass> mpHorizontalBlur;
    ref<FullScreenPass> mpVerticalBlur;
    ref<Fbo> mpFbo;
    ref<Fbo> mpTmpFbo;
    ref<Sampler> mpSampler;
};
```

**Public Members**: None (all members are private)

**Private Members**:
- `uint32_t mKernelWidth` - Kernel width (default: 5)
- `float mSigma` - Sigma (default: 2.0f)
- `bool mReady` - Ready flag
- `ref<FullScreenPass> mpHorizontalBlur` - Horizontal blur pass
- `ref<FullScreenPass> mpVerticalBlur` - Vertical blur pass
- `ref<Fbo> mpFbo` - Output FBO
- `ref<Fbo> mpTmpFbo` - Temporary FBO
- `ref<Sampler> mpSampler` - Sampler

**Private Methods**:
- `void createTmpFbo(const Texture* pSrc)` - Create temporary FBO
- `void updateKernel()` - Update kernel coefficients

**Public Methods**:
- `Properties getProperties() const override` - Get properties
- `RenderPassReflection reflect(const CompileData& compileData) override` - Reflect input/output
- `void compile(RenderContext* pRenderContext, const CompileData& compileData) override` - Compile pass
- `void execute(RenderContext* pRenderContext, const RenderData& renderData) override` - Execute pass
- `void renderUI(Gui::Widgets& widget) override` - Render UI
- `void setKernelWidth(uint32_t kernelWidth)` - Set kernel width
- `void setSigma(float sigma)` - Set sigma
- `uint32_t getKernelWidth()` - Get kernel width
- `float getSigma()` - Get sigma
- `static void registerBindings(pybind11::module& m)` - Register Python bindings

### GaussianBlur.ps.slang

**File**: [`GaussianBlur.ps.slang`](Source/RenderPasses/Utils/GaussianBlur/GaussianBlur.ps.slang:1)

**Purpose**: Gaussian blur pixel shader.

**Features**:
- Two-pass separable Gaussian blur (horizontal and vertical)
- Thread group size: 16x16x1
- Configurable kernel width (via define)
- Configurable sigma (via kernel coefficients)
- Texture array support (via define)
- Kernel coefficients buffer
- Linear filtering
- Clamp addressing mode

**Gaussian Blur Algorithm**:
- Horizontal pass: blur in horizontal direction
- Vertical pass: blur in vertical direction
- Separable kernel: 1D Gaussian kernel applied twice
- Kernel coefficients: computed on CPU, uploaded to GPU

## Technical Details

### Composite Implementation

**Constructor**:
```cpp
Composite::Composite(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    // Parse dictionary.
    for (const auto& [key, value] : props)
    {
        if (key == kMode)
            mMode = value;
        else if (key == kScaleA)
            mScaleA = value;
        else if (key == kScaleB)
            mScaleB = value;
        else if (key == kOutputFormat)
            mOutputFormat = value;
        else
            logWarning("Unknown property '{}' in Composite pass properties.", key);
    }

    // Create resources.
    mCompositePass = ComputePass::create(mpDevice, kShaderFile, "main", DefineList(), false);
}
```

**Features**:
- Property parsing (mode, scaleA, scaleB, outputFormat)
- Compute pass creation
- No defines initially

**GetProperties**:
```cpp
Properties Composite::getProperties() const
{
    Properties props;
    props[kMode] = mMode;
    props[kScaleA] = mScaleA;
    props[kScaleB] = mScaleB;
    if (mOutputFormat != ResourceFormat::Unknown)
        props[kOutputFormat] = mOutputFormat;
    return props;
}
```

**Features**:
- Mode serialization
- Scale A serialization
- Scale B serialization
- Output format serialization (conditional)

**Reflect**:
```cpp
RenderPassReflection Composite::reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    reflector.addInput(kInputA, "Input A").bindFlags(ResourceBindFlags::ShaderResource).flags(RenderPassReflection::Field::Flags::Optional);
    reflector.addInput(kInputB, "Input B").bindFlags(ResourceBindFlags::ShaderResource).flags(RenderPassReflection::Field::Flags::Optional);
    reflector.addOutput(kOutput, "Output").bindFlags(ResourceBindFlags::UnorderedAccess).format(mOutputFormat);
    return reflector;
}
```

**Features**:
- Input A (optional)
- Input B (optional)
- Output (unordered access)
- Output format specification

**Execute**:
```cpp
void Composite::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // Prepare program.
    const auto& pOutput = renderData.getTexture(kOutput);
    FALCOR_ASSERT(pOutput);
    mOutputFormat = pOutput->getFormat();

    if (mCompositePass->getProgram()->addDefines(getDefines()))
    {
        mCompositePass->setVars(nullptr);
    }

    // Bind resources.
    auto var = mCompositePass->getRootVar();
    var["CB"]["frameDim"] = mFrameDim;
    var["CB"]["scaleA"] = mScaleA;
    var["CB"]["scaleB"] = mScaleB;
    var["A"] = renderData.getTexture(kInputA); // Can be nullptr
    var["B"] = renderData.getTexture(kInputB); // Can be nullptr
    var["output"] = pOutput;
    mCompositePass->execute(pRenderContext, mFrameDim.x, mFrameDim.y);
}
```

**Features**:
- Output format detection
- Define update (conditional)
- Variable reset (conditional)
- Frame dimension binding
- Scale A binding
- Scale B binding
- Input A binding (can be nullptr)
- Input B binding (can be nullptr)
- Output binding
- Compute dispatch

**GetDefines**:
```cpp
DefineList Composite::getDefines() const
{
    uint32_t compositeMode = 0;
    switch (mMode)
    {
    case Mode::Add:
        compositeMode = COMPOSITE_MODE_ADD;
        break;
    case Mode::Multiply:
        compositeMode = COMPOSITE_MODE_MULTIPLY;
        break;
    default:
        FALCOR_UNREACHABLE();
        break;
    }

    FALCOR_ASSERT(mOutputFormat != ResourceFormat::Unknown);
    uint32_t outputFormat = 0;
    switch (getFormatType(mOutputFormat))
    {
    case FormatType::Uint:
        outputFormat = OUTPUT_FORMAT_UINT;
        break;
    case FormatType::Sint:
        outputFormat = OUTPUT_FORMAT_SINT;
        break;
    default:
        outputFormat = OUTPUT_FORMAT_FLOAT;
        break;
    }

    DefineList defines;
    defines.add("COMPOSITE_MODE", std::to_string(compositeMode));
    defines.add("OUTPUT_FORMAT", std::to_string(outputFormat));

    return defines;
}
```

**Features**:
- Composite mode define (COMPOSITE_MODE_ADD, COMPOSITE_MODE_MULTIPLY)
- Output format define (OUTPUT_FORMAT_FLOAT, OUTPUT_FORMAT_UINT, OUTPUT_FORMAT_SINT)
- Format type detection (Uint, Sint, Float)
- Unknown format handling (default to Float)

**RenderUI**:
```cpp
void Composite::renderUI(Gui::Widgets& widget)
{
    widget.text("This pass scales and composites inputs A and B together");
    widget.dropdown("Mode", mMode);
    widget.var("Scale A", mScaleA);
    widget.var("Scale B", mScaleB);
}
```

**Features**:
- Description text
- Mode dropdown (Add, Multiply)
- Scale A slider
- Scale B slider

### CrossFade Implementation

**Constructor**:
```cpp
CrossFade::CrossFade(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    // Parse dictionary.
    for (const auto& [key, value] : props)
    {
        if (key == kOutputFormat)
            mOutputFormat = value;
        else if (key == kEnableAutoFade)
            mEnableAutoFade = value;
        else if (key == kWaitFrameCount)
            mWaitFrameCount = value;
        else if (key == kFadeFrameCount)
            mFadeFrameCount = value;
        else if (key == kFadeFactor)
            mFadeFactor = value;
        else
            logWarning("Unknown property '{}' in CrossFade pass properties.", key);
    }

    // Create resources.
    mpFadePass = ComputePass::create(mpDevice, kShaderFile, "main");
}
```

**Features**:
- Property parsing (outputFormat, enableAutoFade, waitFrameCount, fadeFrameCount, fadeFactor)
- Compute pass creation

**GetProperties**:
```cpp
Properties CrossFade::getProperties() const
{
    Properties props;
    if (mOutputFormat != ResourceFormat::Unknown)
        props[kOutputFormat] = mOutputFormat;
    props[kEnableAutoFade] = mEnableAutoFade;
    props[kWaitFrameCount] = mWaitFrameCount;
    props[kFadeFrameCount] = mFadeFrameCount;
    props[kFadeFactor] = mFadeFactor;
    return props;
}
```

**Features**:
- Output format serialization (conditional)
- Auto-fade flag serialization
- Wait frame count serialization
- Fade frame count serialization
- Fade factor serialization

**Reflect**:
```cpp
RenderPassReflection CrossFade::reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    reflector.addInput(kInputA, "Input A").bindFlags(ResourceBindFlags::ShaderResource).flags(RenderPassReflection::Field::Flags::Optional);
    reflector.addInput(kInputB, "Input B").bindFlags(ResourceBindFlags::ShaderResource).flags(RenderPassReflection::Field::Flags::Optional);
    reflector.addOutput(kOutput, "Output").bindFlags(ResourceBindFlags::UnorderedAccess).format(mOutputFormat);
    return reflector;
}
```

**Features**:
- Input A (optional)
- Input B (optional)
- Output (unordered access)
- Output format specification

**SetScene**:
```cpp
void CrossFade::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    mpScene = pScene;
}
```

**Features**:
- Scene reference management

**Compile**:
```cpp
void CrossFade::compile(RenderContext* pRenderContext, const CompileData& compileData)
{
    mFrameDim = compileData.defaultTexDims;
}
```

**Features**:
- Frame dimension storage

**Execute**:
```cpp
void CrossFade::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    bool shouldReset = false;

    // Query refresh flags passed down from application and other passes.
    auto& dict = renderData.getDictionary();
    auto refreshFlags = dict.getValue(kRenderPassRefreshFlags, RenderPassRefreshFlags::None);

    // If any refresh flag is set, we reset frame accumulation.
    if (refreshFlags != RenderPassRefreshFlags::None)
        shouldReset = true;

    // Reset accumulation upon all scene changes, except camera jitter and history changes.
    // TODO: Add UI options to select which changes should trigger reset
    if (mpScene)
    {
        auto sceneUpdates = mpScene->getUpdates();
        if ((sceneUpdates & ~IScene::UpdateFlags::CameraPropertiesChanged) != IScene::UpdateFlags::None)
        {
            shouldReset = true;
        }

        if (is_set(sceneUpdates, IScene::UpdateFlags::CameraPropertiesChanged))
        {
            auto excluded = Camera::Changes::Jitter | Camera::Changes::History;
            auto cameraChanges = mpScene->getCamera()->getChanges();
            if ((cameraChanges & ~excluded) != Camera::Changes::None)
                shouldReset = true;
        }

        if (is_set(sceneUpdates, IScene::UpdateFlags::SDFGeometryChanged))
        {
            shouldReset = true;
        }
    }

    if (shouldReset)
    {
        mMixFrame = 0;
    }
    else
    {
        mMixFrame++;
    }

    float mix = mEnableAutoFade ? math::clamp((float(mMixFrame) - mWaitFrameCount) / mFadeFrameCount, 0.f, 1.f)
                                    : math::clamp(mFadeFactor, 0.f, 1.f);

    mScaleA = 1.f - mix;
    mScaleB = mix;

    // Prepare program.
    const auto& pOutput = renderData.getTexture(kOutput);
    FALCOR_ASSERT(pOutput);
    mOutputFormat = pOutput->getFormat();
    FALCOR_CHECK(!isIntegerFormat(mOutputFormat), "Output cannot be an integer format.");

    // Bind resources.
    auto var = mpFadePass->getRootVar();
    var["CB"]["frameDim"] = mFrameDim;
    var["CB"]["scaleA"] = mScaleA;
    var["CB"]["scaleB"] = mScaleB;

    var["A"] = renderData.getTexture(kInputA); // Can be nullptr
    var["B"] = renderData.getTexture(kInputB); // Can be nullptr
    var["output"] = pOutput;
    mpFadePass->execute(pRenderContext, mFrameDim.x, mFrameDim.y);
}
```

**Features**:
- Refresh flag detection
- Scene change detection (all changes except camera jitter and history)
- Camera change detection (excluding jitter and history)
- SDF geometry change detection
- Mix frame reset on changes
- Mix frame increment
- Mix factor calculation (auto-fade or manual)
- Scale A calculation (1 - mix)
- Scale B calculation (mix)
- Output format validation (must be float)
- Frame dimension binding
- Scale A binding
- Scale B binding
- Input A binding (can be nullptr)
- Input B binding (can be nullptr)
- Output binding
- Compute dispatch

**RenderUI**:
```cpp
void CrossFade::renderUI(Gui::Widgets& widget)
{
    widget.text("This pass fades between inputs A and B");
    widget.checkbox("Enable Auto Fade", mEnableAutoFade);
    if (mEnableAutoFade)
    {
        widget.var("Wait Frame Count", mWaitFrameCount);
        widget.var("Fade Frame Count", mFadeFrameCount);
    }
    else
    {
        widget.var("Fade Factor", mFadeFactor, 0.f, 1.f);
    }
}
```

**Features**:
- Description text
- Auto-fade checkbox
- Wait frame count slider (conditional)
- Fade frame count slider (conditional)
- Fade factor slider (conditional)

### GaussianBlur Implementation

**Constructor**:
```cpp
GaussianBlur::GaussianBlur(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    mpFbo = Fbo::create(mpDevice);
    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(TextureFilteringMode::Linear, TextureFilteringMode::Linear, TextureFilteringMode::Point)
        .setAddressingMode(TextureAddressingMode::Clamp, TextureAddressingMode::Clamp, TextureAddressingMode::Clamp);
    mpSampler = mpDevice->createSampler(samplerDesc);

    for (const auto& [key, value] : props)
    {
        if (key == kKernelWidth)
            mKernelWidth = value;
        else if (key == kSigma)
            mSigma = value;
        else
            logWarning("Unknown property '{}' in a GaussianBlur properties.", key);
    }
}
```

**Features**:
- FBO creation
- Linear sampler creation (min/mag: Linear, mip: Point)
- Clamp addressing mode
- Property parsing (kernelWidth, sigma)

**GetProperties**:
```cpp
Properties GaussianBlur::getProperties() const
{
    Properties props;
    props[kKernelWidth] = mKernelWidth;
    props[kSigma] = mSigma;
    return props;
}
```

**Features**:
- Kernel width serialization
- Sigma serialization

**Reflect**:
```cpp
RenderPassReflection GaussianBlur::reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    mReady = false;
    if (compileData.connectedResources.getFieldCount() > 0)
    {
        const RenderPassReflection::Field* edge = compileData.connectedResources.getField(kSrc);
        RenderPassReflection::Field::Type srcType = edge->getType();
        ResourceFormat srcFormat = edge->getFormat();
        uint32_t srcWidth = edge->getWidth();
        uint32_t srcHeight = edge->getHeight();
        uint32_t srcDepth = edge->getDepth();
        uint32_t srcSampleCount = edge->getSampleCount();
        uint32_t srcMipCount = edge->getMipCount();
        uint32_t srcArraySize = edge->getArraySize();

        auto formatField = [=](RenderPassReflection::Field& f)
        { return f.format(srcFormat).resourceType(srcType, srcWidth, srcHeight, srcDepth, srcSampleCount, srcMipCount, srcArraySize); };

        formatField(reflector.addInput(kSrc, "input image to be blurred"));
        formatField(reflector.addOutput(kDst, "output blurred image"));
        mReady = true;
    }
    else
    {
        reflector.addInput(kSrc, "input image to be blurred");
        reflector.addOutput(kDst, "output blurred image");
    }
    return reflector;
}
```

**Features**:
- Input format specification (conditional)
- Output format specification (conditional)
- Resource type specification (conditional)
- Ready flag

**Compile**:
```cpp
void GaussianBlur::compile(RenderContext* pRenderContext, const CompileData& compileData)
{
    FALCOR_CHECK(mReady, "GaussianBlur: Missing incoming reflection information");

    uint32_t arraySize = compileData.connectedResources.getField(kSrc)->getArraySize();
    DefineList defines;
    defines.add("_KERNEL_WIDTH", std::to_string(mKernelWidth));
    if (arraySize > 1)
        defines.add("_USE_TEX2D_ARRAY");

    uint32_t layerMask = (arraySize > 1) ? ((1 << arraySize) - 1) : 0;
    defines.add("_HORIZONTAL_BLUR");
    mpHorizontalBlur = FullScreenPass::create(mpDevice, kShaderFilename, defines, layerMask);
    defines.remove("_HORIZONTAL_BLUR");
    defines.add("_VERTICAL_BLUR");
    mpVerticalBlur = FullScreenPass::create(mpDevice, kShaderFilename, defines, layerMask);

    // Make programs share vars
    mpVerticalBlur->setVars(mpHorizontalBlur->getVars());

    updateKernel();
}
```

**Features**:
- Ready flag check
- Kernel width define
- Texture array define (conditional)
- Layer mask calculation (for texture arrays)
- Horizontal blur pass creation
- Vertical blur pass creation
- Variable sharing
- Kernel update

**Execute**:
```cpp
void GaussianBlur::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    auto pSrc = renderData.getTexture(kSrc);
    mpFbo->attachColorTarget(renderData.getTexture(kDst), 0);
    createTmpFbo(pSrc.get());

    // Horizontal pass
    {
        auto var = mpHorizontalBlur->getRootVar();
        var["gSampler"] = mpSampler;
        var["gSrcTex"] = pSrc;
        mpHorizontalBlur->execute(pRenderContext, mpTmpFbo);
    }

    // Vertical pass
    {
        auto var = mpVerticalBlur->getRootVar();
        var["gSrcTex"] = mpTmpFbo->getColorTexture(0);
        mpVerticalBlur->execute(pRenderContext, mpFbo);
    }
}
```

**Features**:
- Two-pass separable blur
- Horizontal pass (source to tmp)
- Vertical pass (tmp to destination)
- Temporary FBO creation (lazy)
- Sampler binding
- Source texture binding
- Full-screen pass execution

**CreateTmpFbo**:
```cpp
void GaussianBlur::createTmpFbo(const Texture* pSrc)
{
    bool createFbo = mpTmpFbo == nullptr;
    ResourceFormat srcFormat = pSrc->getFormat();

    if (createFbo == false)
    {
        createFbo = (pSrc->getWidth() != mpTmpFbo->getWidth()) || (pSrc->getHeight() != mpTmpFbo->getHeight()) ||
                    (srcFormat != mpTmpFbo->getColorTexture(0)->getFormat()) ||
                    pSrc->getArraySize() != mpTmpFbo->getColorTexture(0)->getArraySize());
    }

    if (createFbo)
    {
        Fbo::Desc fboDesc;
        fboDesc.setColorTarget(0, srcFormat);
        mpTmpFbo = Fbo::create2D(mpDevice, pSrc->getWidth(), pSrc->getHeight(), fboDesc, pSrc->getArraySize());
    }
}
```

**Features**:
- Lazy FBO creation
- Size change detection
- Format change detection
- Array size change detection
- Single color target

**SetKernelWidth**:
```cpp
void GaussianBlur::setKernelWidth(uint32_t kernelWidth)
{
    mKernelWidth = kernelWidth | 1; // Make sure to kernel width is an odd number
    requestRecompile();
}
```

**Features**:
- Odd number enforcement (bitwise OR with 1)
- Recompile request

**SetSigma**:
```cpp
void GaussianBlur::setSigma(float sigma)
{
    mSigma = sigma;
    requestRecompile();
}
```

**Features**:
- Sigma storage
- Recompile request

**UpdateKernel**:
```cpp
void GaussianBlur::updateKernel()
{
    uint32_t center = mKernelWidth / 2;
    float sum = 0;
    std::vector<float> weights(center + 1);
    for (uint32_t i = 0; i <= center; i++)
    {
        weights[i] = getCoefficient(mSigma, (float)mKernelWidth, (float)i);
        sum += (i == 0) ? weights[i] : 2 * weights[i];
    }

    ref<Buffer> pBuf = mpDevice->createTypedBuffer<float>(mKernelWidth, ResourceBindFlags::ShaderResource);

    for (uint32_t i = 0; i <= center; i++)
    {
        float w = weights[i] / sum;
        pBuf->setElement(center + i, w);
        pBuf->setElement(center - i, w);
    }

    mpHorizontalBlur->getRootVar()["weights"] = pBuf;
}
```

**Features**:
- Gaussian coefficient computation
- Kernel coefficient normalization
- Symmetric kernel storage
- Buffer creation
- Kernel buffer binding

**GetCoefficient**:
```cpp
float getCoefficient(float sigma, float kernelWidth, float x)
{
    float sigmaSquared = sigma * sigma;
    float p = -(x * x) / (2 * sigmaSquared);
    float e = std::exp(p);

    float a = 2 * (float)M_PI * sigmaSquared;
    return e / a;
}
```

**Features**:
- Gaussian function: e^(-(x²)/(2σ²)) / (2πσ²)
- Sigma squared computation
- Exponential computation
- Normalization factor

**RenderUI**:
```cpp
void GaussianBlur::renderUI(Gui::Widgets& widget)
{
    if (widget.var("Kernel Width", (int&)mKernelWidth, 1, 15, 2))
        setKernelWidth(mKernelWidth);
    if (widget.slider("Sigma", mSigma, 0.001f, mKernelWidth / 2.f))
        setSigma(mSigma);
}
```

**Features**:
- Kernel width slider (1-15, step 2)
- Sigma slider (0.001 to kernelWidth/2)

## Architecture Patterns

### Render Pass Pattern

All utility passes inherit from [`RenderPass`](Source/Falcor/RenderGraph/RenderPass.h:1) base class:
- Implement standard render pass interface
- Input/output specification (reflect)
- Property serialization (getProperties)
- Scene integration (CrossFade only)
- UI rendering (renderUI)

### Compute Pass Pattern

Composite and CrossFade use compute passes:
- Thread group size: 16x16x1
- Compute shader execution
- Direct dispatch (width, height)
- No FBO attachment

### Full-Screen Pass Pattern

GaussianBlur uses full-screen passes:
- Two-pass separable blur
- Thread group size: 16x16x1
- Pixel shader execution
- FBO attachment

### Two-Pass Pattern

GaussianBlur uses two-pass approach:
- Horizontal pass (source to tmp)
- Vertical pass (tmp to destination)
- Separable kernel (1D Gaussian)
- Temporary FBO for intermediate results

### Auto-Fade Pattern

CrossFade implements auto-fade:
- Mix frame accumulation
- Wait frame count (delay before fade)
- Fade frame count (fade duration)
- Mix factor calculation: clamp((mixFrame - waitFrameCount) / fadeFrameCount, 0, 1)
- Scene change detection (reset on changes)
- Refresh flag detection (reset on refresh)

### Separable Blur Pattern

GaussianBlur implements separable blur:
- 1D Gaussian kernel (horizontal)
- 1D Gaussian kernel (vertical)
- Two passes (horizontal + vertical)
- O(n²) → O(2n) complexity reduction
- Symmetric kernel storage

## Code Patterns

### Property Serialization Pattern

```cpp
Properties Composite::getProperties() const
{
    Properties props;
    props[kMode] = mMode;
    props[kScaleA] = mScaleA;
    props[kScaleB] = mScaleB;
    if (mOutputFormat != ResourceFormat::Unknown)
        props[kOutputFormat] = mOutputFormat;
    return props;
}
```

### Property Parsing Pattern

```cpp
Composite::Composite(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    for (const auto& [key, value] : props)
    {
        if (key == kMode)
            mMode = value;
        else if (key == kScaleA)
            mScaleA = value;
        else if (key == kScaleB)
            mScaleB = value;
        else if (key == kOutputFormat)
            mOutputFormat = value;
        else
            logWarning("Unknown property '{}' in Composite pass properties.", key);
    }
}
```

### Lazy Resource Creation Pattern

```cpp
void GaussianBlur::createTmpFbo(const Texture* pSrc)
{
    bool createFbo = mpTmpFbo == nullptr;
    ResourceFormat srcFormat = pSrc->getFormat();

    if (createFbo == false)
    {
        createFbo = (pSrc->getWidth() != mpTmpFbo->getWidth()) || (pSrc->getHeight() != mpTmpFbo->getHeight()) ||
                    (srcFormat != mpTmpFbo->getColorTexture(0)->getFormat()) ||
                    pSrc->getArraySize() != mpTmpFbo->getColorTexture(0)->getArraySize());
    }

    if (createFbo)
    {
        Fbo::Desc fboDesc;
        fboDesc.setColorTarget(0, srcFormat);
        mpTmpFbo = Fbo::create2D(mpDevice, pSrc->getWidth(), pSrc->getHeight(), fboDesc, pSrc->getArraySize());
    }
}
```

### Shader Recompile Pattern

```cpp
void GaussianBlur::setKernelWidth(uint32_t kernelWidth)
{
    mKernelWidth = kernelWidth | 1; // Make sure to kernel width is an odd number
    requestRecompile();
}
```

### Plugin Registration Pattern

```cpp
extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, CrossFade>();
    registry.registerClass<RenderPass, Composite>();
    registry.registerClass<RenderPass, GaussianBlur>();
    ScriptBindings::registerBinding(GaussianBlur::registerBindings);
}
```

### Scene Change Detection Pattern

```cpp
void CrossFade::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    bool shouldReset = false;

    // Query refresh flags passed down from application and other passes.
    auto& dict = renderData.getDictionary();
    auto refreshFlags = dict.getValue(kRenderPassRefreshFlags, RenderPassRefreshFlags::None);

    // If any refresh flag is set, we reset frame accumulation.
    if (refreshFlags != RenderPassRefreshFlags::None)
        shouldReset = true;

    // Reset accumulation upon all scene changes, except camera jitter and history changes.
    if (mpScene)
    {
        auto sceneUpdates = mpScene->getUpdates();
        if ((sceneUpdates & ~IScene::UpdateFlags::CameraPropertiesChanged) != IScene::UpdateFlags::None)
        {
            shouldReset = true;
        }

        if (is_set(sceneUpdates, IScene::UpdateFlags::CameraPropertiesChanged))
        {
            auto excluded = Camera::Changes::Jitter | Camera::Changes::History;
            auto cameraChanges = mpScene->getCamera()->getChanges();
            if ((cameraChanges & ~excluded) != Camera::Changes::None)
                shouldReset = true;
        }

        if (is_set(sceneUpdates, IScene::UpdateFlags::SDFGeometryChanged))
        {
            shouldReset = true;
        }
    }

    if (shouldReset)
    {
        mMixFrame = 0;
    }
    else
    {
        mMixFrame++;
    }
}
```

## Integration Points

### Render Graph Integration

All utility passes integrate with render graph system:
- Composite: 2 optional inputs (A, B), 1 output (out)
- CrossFade: 2 optional inputs (A, B), 1 output (out)
- GaussianBlur: 1 input (src), 1 output (dst)

### Scene Integration

CrossFade integrates with scene system:
- Scene reference management
- Scene change detection
- Camera change detection (excluding jitter and history)
- SDF geometry change detection
- Refresh flag detection

### Scripting Integration

GaussianBlur integrates with scripting:
- Python bindings for kernelWidth and sigma
- Property serialization

### UI Integration

All utility passes integrate with UI system:
- Composite: Mode dropdown, Scale A slider, Scale B slider
- CrossFade: Auto-fade checkbox, Wait frame count slider, Fade frame count slider, Fade factor slider
- GaussianBlur: Kernel width slider, Sigma slider

## Use Cases

### Rendering Applications

Composite blending:
- Additive blending for light accumulation
- Multiplicative blending for light modulation
- Independent scaling for each input
- Integer format support for mask blending

Cross-fading:
- Time-dependent transitions between scenes
- Auto-fade for smooth transitions
- Manual fade for fixed transitions
- Scene change detection for reset

Gaussian blurring:
- Image smoothing
- Noise reduction
- Depth of field simulation
- Bloom preprocessing
- Anti-aliasing

### Real-Time Rendering

Real-time rendering applications:
- Real-time composite operations
- Real-time cross-fading
- Real-time Gaussian blur
- Interactive parameter adjustment
- Performance optimization

### Post-Processing

Post-processing applications:
- Composite operations for multi-pass effects
- Cross-fading for transitions
- Gaussian blur for smoothing
- Multi-pass pipeline building

### Debugging and Analysis

Debugging and analysis use cases:
- Composite operation testing
- Cross-fade behavior analysis
- Gaussian blur quality analysis
- Kernel coefficient inspection
- Mix factor visualization

### Research and Development

Research and development use cases:
- Composite algorithm research
- Cross-fade algorithm development
- Gaussian blur optimization
- Kernel coefficient study
- Separable blur research

### Educational Use

Educational use cases:
- Composite operation demonstration
- Cross-fade explanation
- Gaussian blur teaching
- Kernel coefficient visualization
- Mix factor analysis

## Performance Considerations

### GPU Computation

**Composite Pass**:
- Compute shader: O(width × height) operations
- Composite operation: O(1) per pixel
- Scale multiplication: O(2) per pixel
- Total: O(width × height) per frame

**CrossFade Pass**:
- Compute shader: O(width × height) operations
- Linear interpolation: O(2) per pixel
- Scale multiplication: O(2) per pixel
- Total: O(width × height) per frame

**GaussianBlur Pass**:
- Horizontal pass: O(width × height × kernelWidth) operations
- Vertical pass: O(width × height × kernelWidth) operations
- Total: O(2 × width × height × kernelWidth) per frame

**Total GPU Computation**:
- Composite: O(width × height) per frame
- CrossFade: O(width × height) per frame
- GaussianBlur: O(2 × width × height × kernelWidth) per frame

### Memory Usage

**Composite Pass**:
- Input textures: O(width × height × sizeof(float4) × 2) bytes
- Output texture: O(width × height × sizeof(float4/uint4/int4)) bytes
- Total: O(width × height × sizeof(float4) × 3) bytes

**CrossFade Pass**:
- Input textures: O(width × height × sizeof(float4) × 2) bytes
- Output texture: O(width × height × sizeof(float4)) bytes
- Total: O(width × height × sizeof(float4) × 3) bytes

**GaussianBlur Pass**:
- Source texture: O(width × height × sizeof(float4)) bytes
- Temporary FBO: O(width × height × sizeof(float4)) bytes
- Output FBO: O(width × height × sizeof(float4)) bytes
- Kernel buffer: O(kernelWidth × sizeof(float)) bytes
- Total: O(width × height × sizeof(float4) × 3 + kernelWidth × sizeof(float)) bytes

**Total Memory Usage**:
- Composite: O(width × height × sizeof(float4) × 3) bytes
- CrossFade: O(width × height × sizeof(float4) × 3) bytes
- GaussianBlur: O(width × height × sizeof(float4) × 3 + kernelWidth × sizeof(float)) bytes

### Computational Complexity

**Composite Pass**:
- Composite operation: O(1) per pixel
- Scale multiplication: O(2) per pixel
- Total: O(3) per pixel

**CrossFade Pass**:
- Linear interpolation: O(2) per pixel
- Scale multiplication: O(2) per pixel
- Total: O(4) per pixel

**GaussianBlur Pass**:
- Horizontal pass: O(kernelWidth) per pixel
- Vertical pass: O(kernelWidth) per pixel
- Total: O(2 × kernelWidth) per pixel

**Total Computational Complexity**:
- Composite: O(3) per pixel
- CrossFade: O(4) per pixel
- GaussianBlur: O(2 × kernelWidth) per pixel

### Program Management Overhead

**Shader Compilation**:
- Composite: O(1) on mode/outputFormat change
- CrossFade: O(1) on creation
- GaussianBlur: O(1) on kernelWidth/sigma change
- Total: O(1) on parameter changes

**FBO Creation**:
- Composite: No FBO (compute pass)
- CrossFade: No FBO (compute pass)
- GaussianBlur: O(1) on size/format change (tmp FBO)
- Total: O(1) on size/format changes

**Kernel Update**:
- Composite: No kernel
- CrossFade: No kernel
- GaussianBlur: O(kernelWidth) on sigma change
- Total: O(kernelWidth) on sigma change

## Limitations

### Composite Limitations

**Feature Limitations**:
- Limited to 2 composite modes (Add, Multiply)
- No custom composite operations
- No alpha blending
- No per-pixel operations
- No conditional compositing
- No masking
- No region-based compositing

**Integration Limitations**:
- Limited to Falcor's render graph system
- No external data sources
- No scripting support
- No event handling

**Performance Limitations**:
- No performance optimization
- No adaptive quality settings
- No performance profiling tools

### CrossFade Limitations

**Feature Limitations**:
- Limited to float output format
- No integer format support
- No custom fade curves
- No temporal smoothing
- No fade presets
- No fade direction control

**Integration Limitations**:
- Limited to Falcor's scene system
- No external data sources
- No scripting support
- Limited refresh flag support

**Performance Limitations**:
- No performance optimization
- No adaptive quality settings
- No performance profiling tools

### GaussianBlur Limitations

**Feature Limitations**:
- Limited to separable Gaussian blur
- No anisotropic blur
- No motion blur
- No bilateral filtering
- Limited kernel width (1-15)
- No kernel presets
- No multi-pass blur

**Integration Limitations**:
- Limited to Falcor's render graph system
- No external data sources
- Limited scripting support (kernelWidth, sigma only)

**Performance Limitations**:
- Two-pass overhead
- Temporary FBO overhead
- No adaptive kernel size
- No performance profiling tools

## Best Practices

### Composite Best Practices

**Configuration**:
- Use Add mode for light accumulation
- Use Multiply mode for light modulation
- Set appropriate scales for each input
- Use float format for intermediate results
- Use integer format for final output (if needed)
- Test with different composite modes

**Performance Optimization**:
- Use appropriate output format
- Test with different scales
- Profile GPU performance
- Optimize shader compilation

**Debugging**:
- Test with different composite modes
- Verify scale application
- Check output format conversion
- Test with null inputs
- Validate composite operations

### CrossFade Best Practices

**Configuration**:
- Use auto-fade for smooth transitions
- Set appropriate wait frame count (delay)
- Set appropriate fade frame count (duration)
- Use manual fade for fixed transitions
- Test with different fade parameters
- Monitor mix frame progression

**Scene Integration**:
- Use scene change detection for reset
- Exclude camera jitter and history changes
- Monitor refresh flags
- Test with different scene types

**Performance Optimization**:
- Use appropriate output format
- Test with different fade parameters
- Profile GPU performance
- Optimize shader compilation

**Debugging**:
- Test auto-fade behavior
- Verify manual fade behavior
- Check scene change detection
- Monitor mix frame progression
- Validate scale calculations

### GaussianBlur Best Practices

**Configuration**:
- Use odd kernel width (enforced)
- Set appropriate sigma for blur strength
- Use larger kernel width for stronger blur
- Use smaller kernel width for weaker blur
- Test with different kernel widths and sigmas
- Use appropriate output format

**Performance Optimization**:
- Use separable blur (already implemented)
- Use appropriate kernel width
- Test with different sigma values
- Profile GPU performance
- Optimize kernel coefficient computation

**Debugging**:
- Test with different kernel widths
- Verify sigma application
- Check kernel coefficients
- Validate two-pass execution
- Test with texture arrays
- Monitor FBO creation

### General Best Practices

**Pipeline Building**:
- Use Composite for blending multiple passes
- Use CrossFade for transitions
- Use GaussianBlur for smoothing
- Combine multiple utility passes
- Test pipeline performance

**Parameter Tuning**:
- Test with different parameter combinations
- Profile performance impact
- Monitor memory usage
- Validate output quality
- Use appropriate thread group sizes

**Integration**:
- Connect to appropriate render graph inputs
- Use correct texture formats
- Handle optional inputs
- Test with different render graphs
- Validate compatibility

## Related Components

### RenderGraph/RenderPass

Render pass base class:
- `RenderPass::reflect()` - Define input/output resources
- `RenderPass::execute()` - Execute render pass
- `RenderPass::renderUI()` - Render UI controls
- `RenderPass::getProperties()` - Serialize configuration
- `RenderPass::setScene()` - Set scene

### Core/Pass/ComputePass

Compute pass:
- `ComputePass::create()` - Create compute pass
- `ComputePass::execute()` - Execute compute pass
- `ComputePass::getRootVar()` - Get root variable

### Core/Pass/FullScreenPass

Full-screen pass:
- `FullScreenPass::create()` - Create full-screen pass
- `FullScreenPass::execute()` - Execute full-screen pass
- `FullScreenPass::getRootVar()` - Get root variable

### Scene

Scene system:
- `Scene::getUpdates()` - Get scene updates
- `Scene::getCamera()` - Get camera
- `Camera::getChanges()` - Get camera changes

## Progress Log

- **2026-01-07T22:35:00Z**: Completed Utils analysis. Analyzed Utils.cpp (40 lines), Composite.h (87 lines), Composite.cpp (164 lines), Composite.cs.slang (80 lines), CompositeMode.slangh (36 lines), CrossFade.h (68 lines), CrossFade.cpp (180 lines), CrossFade.cs.slang (52 lines), GaussianBlur.h (69 lines), GaussianBlur.cpp (223 lines), and GaussianBlur.ps.slang (87 lines). Created comprehensive technical specifications covering Composite (Add/Multiply modes, independent scaling, integer format support with round-to-nearest-even), CrossFade (time-dependent fading, auto-fade with wait/fade frame counts, manual fade with fixed factor, scene change detection with refresh flags), and GaussianBlur (separable two-pass Gaussian blur, configurable kernel width and sigma, kernel coefficient computation, texture array support, lazy FBO creation). Marked Utils as Complete.
