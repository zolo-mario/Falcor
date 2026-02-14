#include "VBufferMeshletRaster.h"
#include "Scene/SceneMeshletData.h"
#include "Scene/HitInfo.h"
#include "RenderGraph/RenderPassStandardFlags.h"
#include "RenderGraph/RenderPassHelpers.h"

namespace
{
    const char kMeshShaderFile[] = "RenderPasses/GBuffer/VBuffer/MeshletRaster.ms.slang";

    const std::string kVBufferName = "vbuffer";
    const std::string kVBufferDesc = "V-buffer in packed format (indices + barycentrics)";
    const std::string kDepthName = "depthStencil";

    // Extra channels - not used for now
    // const ChannelList kVBufferExtraChannels = {
    //     { "viewW",  "gViewW",        "View direction in world space", true /* optional */, ResourceFormat::RGBA32Float },
    //     { "depth",  "gDepth",        "Depth buffer (NDC)",  true /* optional */, ResourceFormat::R32Float  },
    //     { "mvec",   "gMotionVector", "Motion vector",       true /* optional */, ResourceFormat::RG32Float },
    // };
}

VBufferMeshletRaster::VBufferMeshletRaster(ref<Device> pDevice, const Properties& props)
    : GBufferBase(pDevice)
{
    parseProperties(props);

    if (!pDevice->isShaderModelSupported(ShaderModel::SM6_5))
        FALCOR_THROW("VBufferMeshletRaster requires Shader Model 6.5 for mesh shader support.");
    if (!pDevice->isFeatureSupported(Device::SupportedFeatures::Barycentrics))
        FALCOR_THROW("VBufferMeshletRaster requires pixel shader barycentrics support.");

    mRaster.pState = GraphicsState::create(pDevice);
    DepthStencilState::Desc dsDesc;
    dsDesc.setDepthFunc(ComparisonFunc::LessEqual).setDepthWriteMask(true);
    mRaster.pState->setDepthStencilState(DepthStencilState::create(dsDesc));
    mRaster.pState->setVao(nullptr); // Mesh pipeline has no VAO
    mpFbo = Fbo::create(pDevice);
}

Properties VBufferMeshletRaster::getProperties() const
{
    return GBufferBase::getProperties();
}

RenderPassReflection VBufferMeshletRaster::reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    const uint2 sz = RenderPassHelpers::calculateIOSize(mOutputSizeSelection, mFixedOutputSize, compileData.defaultTexDims);

    reflector.addOutput(kDepthName, "Depth buffer")
        .format(ResourceFormat::D32Float)
        .bindFlags(ResourceBindFlags::DepthStencil)
        .texture2D(sz.x, sz.y);
    reflector.addOutput(kVBufferName, kVBufferDesc)
        .bindFlags(ResourceBindFlags::RenderTarget | ResourceBindFlags::UnorderedAccess)
        .format(mVBufferFormat)
        .texture2D(sz.x, sz.y);

    // addRenderPassOutputs(reflector, kVBufferExtraChannels, ResourceBindFlags::UnorderedAccess, sz);

    return reflector;
}

void VBufferMeshletRaster::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    GBufferBase::setScene(pRenderContext, pScene);

    mMeshletCount = 0;
    mRaster.pProgram = nullptr;
    mRaster.pVars = nullptr;

    if (!mpScene || mpScene->getGeometryInstanceCount() == 0)
        return;

    SceneMeshletData* pMeshletData = mpScene->getMeshletData(pRenderContext);
    if (pMeshletData && pMeshletData->isValid())
    {
        mMeshletCount = pMeshletData->getMeshletCount();

        ProgramDesc desc;
        desc.addShaderModules(mpScene->getShaderModules());
        desc.addShaderLibrary(kMeshShaderFile).meshEntry("meshMain").psEntry("psMain");
        desc.addTypeConformances(mpScene->getTypeConformances());
        desc.setShaderModel(ShaderModel::SM6_5);

        DefineList defines;
        defines.add(mpScene->getSceneDefines());

        mRaster.pProgram = Program::create(mpDevice, desc, defines);
        mRaster.pState->setProgram(mRaster.pProgram);
        mRaster.pVars = ProgramVars::create(mpDevice, mRaster.pProgram.get());
    }

    logInfo("VBufferMeshletRaster: Using {} meshlets from Scene ({} instances)",
            mMeshletCount, mpScene->getGeometryInstanceCount());
}

void VBufferMeshletRaster::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    GBufferBase::execute(pRenderContext, renderData);

    auto pVBuffer = renderData.getTexture(kVBufferName);
    auto pDepth = renderData.getTexture(kDepthName);
    FALCOR_ASSERT(pVBuffer);
    FALCOR_ASSERT(pDepth);
    updateFrameDim(uint2(pVBuffer->getWidth(), pVBuffer->getHeight()));

    // Clear outputs
    pRenderContext->clearUAV(pVBuffer->getUAV().get(), uint4(0));
    pRenderContext->clearDsv(pDepth->getDSV().get(), 1.f, 0);
    // clearRenderPassChannels(pRenderContext, kVBufferExtraChannels, renderData);

    if (!mpScene || mMeshletCount == 0 || !mRaster.pProgram || !mRaster.pVars)
        return;

    SceneMeshletData* pMeshletData = mpScene->getMeshletData(pRenderContext);
    if (!pMeshletData || !pMeshletData->isValid())
        return;

    // Setup FBO
    mpFbo->attachColorTarget(pVBuffer, 0);
    mpFbo->attachDepthStencilTarget(pDepth);
    mRaster.pState->setFbo(mpFbo);

    // Bind resources (from Scene meshlet data)
    auto var = mRaster.pVars->getRootVar();
    // var["CB"]["gFrameDim"] = mFrameDim;
    var["CB"]["gMeshletCount"] = mMeshletCount;
    var["gMeshlets"] = pMeshletData->getMeshletBuffer();
    var["gMeshletVertices"] = pMeshletData->getMeshletVerticesBuffer();
    var["gMeshletTriangles"] = pMeshletData->getMeshletTrianglesBuffer();
    mpScene->bindShaderDataForRaytracing(pRenderContext, var["gScene"]);

    // Draw mesh tasks (one group per meshlet)
    pRenderContext->drawMeshTasks(mRaster.pState.get(), mRaster.pVars.get(), mMeshletCount, 1, 1);

    mFrameCount++;
}

void VBufferMeshletRaster::renderUI(Gui::Widgets& widget)
{
    GBufferBase::renderUI(widget);

    widget.separator();
    widget.text("Meshlet Statistics:");
    widget.text(fmt::format("  Total Meshlets: {}", mMeshletCount));
}
