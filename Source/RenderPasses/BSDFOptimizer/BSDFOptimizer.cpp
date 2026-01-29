#include "BSDFOptimizer.h"
#include "RenderGraph/RenderPassStandardFlags.h"
#include "Rendering/Materials/BSDFConfig.slangh"

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, BSDFOptimizer>();
    ScriptBindings::registerBinding(BSDFOptimizer::registerBindings);
}

namespace
{
const char kOptimizerPass[] = "RenderPasses/BSDFOptimizer/BSDFOptimizer.cs.slang";
const char kViewerPass[] = "RenderPasses/BSDFOptimizer/BSDFViewer.cs.slang";
const char kOutput[] = "output";

const char kViewerParameterBlockName[] = "gBSDFViewer";

// Scripting options.
const char kInitMaterialID[] = "initMaterialID";
const char kRefMaterialID[] = "refMaterialID";
} // namespace

BSDFOptimizer::BSDFOptimizer(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    parseProperties(props);

    mpSampleGenerator = SampleGenerator::create(mpDevice, SAMPLE_GENERATOR_UNIFORM);

    mpFence = mpDevice->createFence();
}

void BSDFOptimizer::parseProperties(const Properties& props)
{
    for (const auto& [key, value] : props)
    {
        if (key == kInitMaterialID)
            mParams.initMaterialID = value;
        else if (key == kRefMaterialID)
            mParams.refMaterialID = value;
        else
            logWarning("Unknown property '{}' in BSDFOptimizer properties.", key);
    }
}

Properties BSDFOptimizer::getProperties() const
{
    Properties props;
    props[kInitMaterialID] = mParams.initMaterialID;
    props[kRefMaterialID] = mParams.refMaterialID;
    return props;
}

RenderPassReflection BSDFOptimizer::reflect(const CompileData& compileData)
{
    RenderPassReflection r;
    r.addOutput(kOutput, "Output buffer").format(ResourceFormat::RGBA32Float).bindFlags(ResourceBindFlags::UnorderedAccess);
    return r;
}

void BSDFOptimizer::compile(RenderContext* pRenderContext, const CompileData& compileData)
{
    mParams.frameDim = compileData.defaultTexDims;

    // Set up viewports. Left: initial material, middle: absolute difference, right: reference material.
    uint32_t extent = std::min(mParams.frameDim.x / 3, mParams.frameDim.y);
    mParams.bsdfTableDim = uint2(extent, extent);

    uint32_t xOffset = (mParams.frameDim.x - extent * 3) / 2;
    uint32_t yOffset = (mParams.frameDim.y - extent) / 2;

    mParams.initViewPortOffset = float2(xOffset, yOffset);
    mParams.diffViewPortOffset = float2(xOffset + extent, yOffset);
    mParams.refViewPortOffset = float2(xOffset + extent * 2, yOffset);
    mParams.viewPortScale = float2(1.f / extent, 1.f / extent);
}

void BSDFOptimizer::initOptimization()
{
    static const std::map<MaterialType, std::map<std::string, float>> kLearningRates{
        {MaterialType::PBRTDiffuse,
         {
             {"diffuse", 1e-2f},
         }},
        {MaterialType::PBRTConductor,
         {
             {"eta", 1e-2f},
             {"k", 1e-2f},
             {"roughness", 1e-2f},
         }},
        {MaterialType::Standard,
         {
             {"base_color", 1e-2f},
             {"roughness", 3e-3f},
             {"metallic", 3e-3f},
         }},
    };

    // Reset BSDF parameters.
    mpScene->getMaterial(MaterialID(mParams.initMaterialID))->deserializeParams(mInitBSDFParams);
    mParams.frameCount = 0;

    // Initialize current BSDF parameters.
    mCurBSDFParams = mInitBSDFParams;

    // Set learning rates and adam optimizer.
    std::vector<float> lr(mCurBSDFParams.size(), 0.f);
    const auto& pMaterial = mpScene->getMaterial(MaterialID{mParams.initMaterialID});

    auto learningRateMap = kLearningRates.find(pMaterial->getType());
    if (learningRateMap != kLearningRates.end())
    {
        for (const auto& param : pMaterial->getParamLayout())
        {
            auto learningRate = learningRateMap->second.find(param.pythonName);
            if (learningRate != learningRateMap->second.end())
            {
                for (uint32_t i = 0; i < param.size; ++i)
                    lr[param.offset + i] = learningRate->second;
            }
        }
    }

    mAdam = AdamOptimizer(lr);
}

void BSDFOptimizer::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    mpScene = pScene;
    mMaterialList.clear();

    if (mpScene == nullptr)
        return;

    if (any(mParams.bsdfTableDim == uint2(0)))
        setBSDFSliceResolution(128);

    // Create optimizer program.
    {
        ProgramDesc desc;
        desc.addShaderModules(mpScene->getShaderModules());
        desc.addShaderLibrary(kOptimizerPass).csEntry("main");
        desc.addTypeConformances(mpScene->getTypeConformances());

        DefineList defines;
        defines.add(mpSampleGenerator->getDefines());
        defines.add(mpScene->getSceneDefines());

        mpOptimizerPass = ComputePass::create(mpDevice, desc, defines);
    }

    // Create viewer program.
    {
        ProgramDesc desc;
        desc.addShaderModules(mpScene->getShaderModules());
        desc.addShaderLibrary(kViewerPass).csEntry("main");
        desc.addTypeConformances(mpScene->getTypeConformances());

        DefineList defines;
        defines.add(mpSampleGenerator->getDefines());
        defines.add(mpScene->getSceneDefines());

        mpViewerPass = ComputePass::create(mpDevice, desc, defines);
    }

    mpSceneGradients = std::make_unique<SceneGradients>(
        mpDevice, std::vector<SceneGradients::GradConfig>({{GradientType::Material, SerializedMaterialParams::kParamCount, 64}})
    );

    // Prepare initial and reference BSDF parameters for optimization.
    mInitBSDFParams = mpScene->getMaterial(MaterialID(mParams.initMaterialID))->serializeParams();
    mRefBSDFParams = mpScene->getMaterial(MaterialID(mParams.refMaterialID))->serializeParams();

    initOptimization();

    // Prepare UI list of materials.
    mMaterialList.reserve(mpScene->getMaterialCount());
    for (uint32_t i = 0; i < mpScene->getMaterialCount(); i++)
    {
        auto mtl = mpScene->getMaterial(MaterialID{i});
        std::string name = std::to_string(i) + ": " + mtl->getName();
        mMaterialList.push_back({i, name});
    }
}

void BSDFOptimizer::executeOptimizerPass(RenderContext* pRenderContext)
{
    if (!mpScene)
        return;

    mpSceneGradients->clearGrads(pRenderContext, GradientType::Material);

    auto var = mpOptimizerPass->getRootVar();
    var["CB"]["params"].setBlob(mParams);
    mpSceneGradients->bindShaderData(var["gSceneGradients"]);
    mpScene->bindShaderData(var["gScene"]);

    mpOptimizerPass->execute(pRenderContext, uint3(mParams.bsdfTableDim, 1));

    mpSceneGradients->aggregateGrads(pRenderContext, GradientType::Material);
}

void BSDFOptimizer::step(RenderContext* pRenderContext)
{
    auto pBuffer = mpSceneGradients->getGradsBuffer(GradientType::Material);
    pBuffer->getBlob(mBSDFGrads.data(), 0, sizeof(float) * mBSDFGrads.size());

    // Update BSDF parameters.
    mAdam.step(mBSDFGrads, mCurBSDFParams);
    mpScene->getMaterial(MaterialID(mParams.initMaterialID))->deserializeParams(mCurBSDFParams);
}

void BSDFOptimizer::executeViewerPass(RenderContext* pRenderContext, const RenderData& renderData)
{
    auto pOutput = renderData.getTexture(kOutput);
    if (!mpScene || mpScene->getMaterialCount() == 0)
    {
        pRenderContext->clearUAV(pOutput->getUAV().get(), uint4(0));
        return;
    }

    auto var = mpViewerPass->getRootVar()[kViewerParameterBlockName];
    var["params"].setBlob(mParams);
    var["output"] = pOutput;
    mpScene->bindShaderData(mpViewerPass->getRootVar()["gScene"]);

    mpViewerPass->execute(pRenderContext, uint3(mParams.frameDim, 1));
}

void BSDFOptimizer::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    if (mpScene && is_set(mpScene->getUpdates(), IScene::UpdateFlags::RecompileNeeded))
    {
        FALCOR_THROW("This render pass does not support scene changes that require shader recompilation.");
    }

    if (mRunOptimization)
    {
        executeOptimizerPass(pRenderContext);
        step(pRenderContext);

        // Stop optimization if the error is small enough.
        float relL1Error = 0.f;
        for (size_t i = 0; i < mCurBSDFParams.size(); i++)
        {
            relL1Error += std::abs(mCurBSDFParams[i] - mRefBSDFParams[i]) / std::max(mRefBSDFParams[i], 1e-6f);
        }
        relL1Error /= mCurBSDFParams.size();
        if (relL1Error < 1e-3f)
        {
            mRunOptimization = false;
        }
    }
    executeViewerPass(pRenderContext, renderData);

    mParams.frameCount++;
}

void BSDFOptimizer::renderUI(Gui::Widgets& widget)
{
    if (!mpScene || mpScene->getMaterialCount() == 0)
    {
        widget.text("No scene/materials loaded.");
        return;
    }

    if (widget.button("Start optimization"))
    {
        if (mParams.frameCount > 0)
            initOptimization();
        mRunOptimization = true;
    }

    if (widget.button("Stop optimization"))
    {
        mRunOptimization = false;
    }

    if (widget.button("Reset optimization"))
    {
        initOptimization();
        mRunOptimization = false;
    }

    auto showMaterial = [&](uint32_t materialID)
    {
        const auto& pMaterial = mpScene->getMaterial(MaterialID{materialID});
        std::string label = std::to_string(materialID) + ": " + pMaterial->getName();
        if (auto materialGroup = widget.group(label))
        {
            pMaterial->renderUI(materialGroup);
        }
    };

    if (auto initMtlGroup = widget.group("Initial material", true))
    {
        initMtlGroup.tooltip("Choose initial material to optimize.\n");

        FALCOR_ASSERT(mMaterialList.size() > 0);
        if (initMtlGroup.dropdown("Materials", mMaterialList, mParams.initMaterialID))
        {
            mInitBSDFParams = mpScene->getMaterial(MaterialID(mParams.initMaterialID))->serializeParams();
        }
        showMaterial(mParams.initMaterialID);
    }

    if (auto refMtlGroup = widget.group("Reference material", true))
    {
        refMtlGroup.tooltip("Choose reference material.\n");

        FALCOR_ASSERT(mMaterialList.size() > 0);
        if (refMtlGroup.dropdown("Materials", mMaterialList, mParams.refMaterialID))
        {
            mRefBSDFParams = mpScene->getMaterial(MaterialID(mParams.refMaterialID))->serializeParams();
        }
        showMaterial(mParams.refMaterialID);
    }
}

void BSDFOptimizer::AdamOptimizer::step(fstd::span<float> dx, fstd::span<float> x)
{
    if (lr.size() != dx.size() || lr.size() != x.size())
    {
        logError("AdamOptimizer::step(): lr, dx, and x must have the same size.");
        return;
    }

    if (m.empty() || v.empty())
    {
        m.resize(dx.size(), 0.f);
        v.resize(dx.size(), 0.f);
    }

    steps++;
    for (size_t i = 0; i < dx.size(); i++)
    {
        if (lr[i] == 0.f)
            continue; // Skip parameters with zero learning rate.
        m[i] = beta1 * m[i] + (1.f - beta1) * dx[i];
        v[i] = beta2 * v[i] + (1.f - beta2) * dx[i] * dx[i];
        float mHat = m[i] / (1.f - std::pow(beta1, steps));
        float vHat = v[i] / (1.f - std::pow(beta2, steps));
        float delta = lr[i] * mHat / (std::sqrt(vHat) + epsilon);
        x[i] -= delta;
    }
}

// Python bindings.

uint32_t BSDFOptimizer::getBSDFSliceResolution() const
{
    FALCOR_ASSERT_EQ(mParams.bsdfTableDim.x, mParams.bsdfTableDim.y);
    return mParams.bsdfTableDim.x;
}

void BSDFOptimizer::setBSDFSliceResolution(uint32_t reso)
{
    mParams.bsdfTableDim = uint2(reso, reso);
    mParams.viewPortScale = float2(1.f / reso, 1.f / reso);
}

ref<Buffer> BSDFOptimizer::computeBSDFGrads()
{
    executeOptimizerPass(mpDevice->getRenderContext());
    return mpSceneGradients->getGradsBuffer(GradientType::Material);
}

void BSDFOptimizer::registerBindings(pybind11::module& m)
{
    pybind11::class_<BSDFOptimizer, RenderPass, ref<BSDFOptimizer>> pass(m, "BSDFOptimizer");
    pass.def_property_readonly("init_material_id", &BSDFOptimizer::getInitMaterialID);
    pass.def_property_readonly("ref_material_id", &BSDFOptimizer::getRefMaterialID);
    pass.def_property("bsdf_slice_resolution", &BSDFOptimizer::getBSDFSliceResolution, &BSDFOptimizer::setBSDFSliceResolution);

    pass.def("compute_bsdf_grads", &BSDFOptimizer::computeBSDFGrads);
}
