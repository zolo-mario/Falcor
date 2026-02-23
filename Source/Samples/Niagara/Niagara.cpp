#include "Niagara.h"
#include "Scene/Scene.h"
#include "Scene/SceneBuilder.h"
#include "Utils/Math/Matrix.h"
#include "Utils/Math/VectorMath.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

namespace
{
float4 normalizePlane(const float4& p)
{
    float len = math::length(float3(p.x, p.y, p.z));
    return len > 1e-8f ? p / len : p;
}

uint32_t previousPow2(uint32_t v)
{
    uint32_t r = 1;
    while (r * 2 < v)
        r *= 2;
    return r;
}

uint32_t getMipLevels(uint32_t width, uint32_t height)
{
    uint32_t m = std::max(width, height);
    uint32_t levels = 1;
    while (m > 1)
    {
        m >>= 1;
        ++levels;
    }
    return levels;
}
} // namespace

static const char kMeshShaderFile[] = "Samples/Niagara/shaders/NiagaraMeshlet.slang";

const Gui::DropdownList Niagara::kSceneDropdownList = {
    {0, "Bunny"},
    {1, "Arcade"},
};

Niagara::Niagara(SampleApp* pHost) : SampleBase(pHost)
{
}

Niagara::~Niagara()
{
}

void Niagara::loadSelectedScene()
{
    const char* path = (mSceneIndex == 0) ? "test_scenes/bunny.pyscene" : "Arcade/Arcade.pyscene";
    mpScene = SceneBuilder(getDevice(), path, Settings(), SceneBuilder::Flags::Default).getScene();
    mConvertOk = mpScene && convertFalcorSceneToNiagaraScene(mpScene.get(), mResult);
    if (mConvertOk)
        uploadSceneBuffers();
    if (mpScene)
    {
        float radius = mpScene->getSceneBounds().radius();
        mpScene->setCameraSpeed(radius * 0.25f);
        const Fbo* pFbo = getTargetFbo().get();
        if (pFbo && pFbo->getWidth() > 0 && pFbo->getHeight() > 0)
        {
            float nearZ = std::max(0.1f, radius / 750.0f);
            float farZ = radius * 10;
            mpScene->getCamera()->setDepthRange(nearZ, farZ);
            mpScene->getCamera()->setAspectRatio((float)pFbo->getWidth() / (float)pFbo->getHeight());
        }
    }
}

void Niagara::uploadSceneBuffers()
{
    auto& geom = mResult.geometry;
    auto& draws = mResult.draws;

    mpVb = nullptr;
    mpMlb = nullptr;
    mpMdb = nullptr;
    mpDb = nullptr;
    mpDcb = nullptr;
    mpCib = nullptr;
    mpDrawVisibility = nullptr;
    mpMeshletVisibility = nullptr;
    mTotalMeshletCount = 0;
    mDvbCleared = false;
    mMvbCleared = false;

    if (geom.vertices.empty() || draws.empty())
        return;

    auto pDevice = getDevice();
    if (!pDevice->isShaderModelSupported(ShaderModel::SM6_5))
    {
        logError("Niagara requires Shader Model 6.5 for mesh shader support.");
        return;
    }

    mpVb = pDevice->createStructuredBuffer(
        sizeof(NiagaraFormat::Vertex),
        (uint32_t)geom.vertices.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        geom.vertices.data());

    mpMlb = pDevice->createStructuredBuffer(
        sizeof(NiagaraFormat::Meshlet),
        (uint32_t)geom.meshlets.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        geom.meshlets.data());

    mpMdb = pDevice->createBuffer(
        geom.meshletdata.size() * sizeof(uint32_t),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        geom.meshletdata.data());

    mpDb = pDevice->createStructuredBuffer(
        sizeof(NiagaraFormat::MeshDraw),
        (uint32_t)draws.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        draws.data());

    std::vector<NiagaraFormat::MeshTaskCommand> taskCommands(draws.size());
    std::vector<uint32_t> clusterIndices;

    for (uint32_t drawId = 0; drawId < draws.size(); ++drawId)
    {
        NiagaraFormat::MeshTaskCommand cmd = {};
        cmd.drawId = drawId;
        cmd.taskOffset = 0;
        cmd.taskCount = 0;
        cmd.meshletVisibilityOffset = draws[drawId].meshletVisibilityOffset;

        const auto& draw = draws[drawId];
        if (draw.meshIndex < geom.meshes.size())
        {
            const auto& mesh = geom.meshes[draw.meshIndex];
            if (mesh.lodCount > 0)
            {
                const auto& lod0 = mesh.lods[0];
                cmd.taskOffset = lod0.meshletOffset;
                cmd.taskCount = lod0.meshletCount;
                uint32_t commandId = drawId * kTaskStride;
                for (uint32_t mgi = 0; mgi < cmd.taskCount; ++mgi)
                    clusterIndices.push_back(commandId | (mgi << 24));
            }
        }
        taskCommands[drawId] = cmd;
    }

    mTotalMeshletCount = (uint32_t)clusterIndices.size();
    if (mTotalMeshletCount == 0)
        return;

    mpDcb = pDevice->createStructuredBuffer(
        sizeof(NiagaraFormat::MeshTaskCommand),
        (uint32_t)taskCommands.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        taskCommands.data());

    mpCib = pDevice->createStructuredBuffer(
        sizeof(uint32_t),
        (uint32_t)clusterIndices.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        clusterIndices.data());

    auto dvbFlags = ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess;
    mpDrawVisibility = pDevice->createStructuredBuffer(
        sizeof(uint32_t),
        (uint32_t)draws.size(),
        dvbFlags,
        MemoryType::DeviceLocal,
        nullptr);
    mpMeshletVisibility = pDevice->createStructuredBuffer(
        sizeof(uint32_t),
        (mTotalMeshletCount + 31) / 32,
        dvbFlags,
        MemoryType::DeviceLocal,
        nullptr);

    ProgramDesc desc;
    desc.addShaderLibrary(kMeshShaderFile)
        .amplificationEntry("ampMain")
        .meshEntry("meshMain")
        .psEntry("psMain");
    desc.setShaderModel(ShaderModel::SM6_5);

    DefineList earlyDefines = {{"LATE", "0"}};
    DefineList lateDefines = {{"LATE", "1"}};
    mpMeshletProgramEarly = Program::create(pDevice, desc, earlyDefines);
    mpMeshletProgramLate = Program::create(pDevice, desc, lateDefines);
    mpMeshletVarsEarly = ProgramVars::create(pDevice, mpMeshletProgramEarly.get());
    mpMeshletVarsLate = ProgramVars::create(pDevice, mpMeshletProgramLate.get());

    ProgramDesc reduceDesc;
    reduceDesc.addShaderLibrary("Samples/Niagara/shaders/NiagaraDepthReduce.slang").csEntry("main");
    mpDepthReducePass = ComputePass::create(pDevice, reduceDesc, DefineList(), true);

    ProgramDesc updateDvbDesc;
    updateDvbDesc.addShaderLibrary("Samples/Niagara/shaders/NiagaraUpdateDvb.slang").csEntry("main");
    mpUpdateDvbPass = ComputePass::create(pDevice, updateDvbDesc, DefineList(), true);

    mpRasterState = GraphicsState::create(pDevice);
    mpRasterState->setProgram(mpMeshletProgramEarly);
    mpRasterState->setVao(nullptr);

    DepthStencilState::Desc dsDesc;
    dsDesc.setDepthFunc(ComparisonFunc::LessEqual).setDepthWriteMask(true);
    mpRasterState->setDepthStencilState(DepthStencilState::create(dsDesc));
}

void Niagara::onLoad(RenderContext* pRenderContext)
{
    loadSelectedScene();
}

void Niagara::onShutdown()
{
}

void Niagara::onResize(uint32_t width, uint32_t height)
{
    if (mpFbo && mpFbo->getWidth() == width && mpFbo->getHeight() == height)
        return;

    if (mpScene && mpScene->getCameras().size() > 0)
    {
        float aspect = (float)width / (float)height;
        mpScene->getCamera()->setAspectRatio(aspect);
    }

    auto pDevice = getDevice();
    mpFbo = Fbo::create(pDevice);
    auto rtFlags = ResourceBindFlags::RenderTarget | ResourceBindFlags::ShaderResource;
    mpFbo->attachColorTarget(
        pDevice->createTexture2D(width, height, ResourceFormat::RGBA32Float, 1, 1, nullptr, rtFlags),
        0);
    mpFbo->attachDepthStencilTarget(
        pDevice->createTexture2D(width, height, ResourceFormat::D32Float, 1, 1, nullptr, ResourceBindFlags::DepthStencil | ResourceBindFlags::ShaderResource));

    mDepthPyramidWidth = previousPow2(width);
    mDepthPyramidHeight = previousPow2(height);
    uint32_t pyrBaseW = std::max(1u, mDepthPyramidWidth / 2);
    uint32_t pyrBaseH = std::max(1u, mDepthPyramidHeight / 2);
    mDepthPyramidLevels = getMipLevels(pyrBaseW, pyrBaseH);
    auto pyramidFlags = ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess;
    mpDepthPyramid = pDevice->createTexture2D(
        pyrBaseW,
        pyrBaseH,
        ResourceFormat::R32Float,
        1,
        mDepthPyramidLevels,
        nullptr,
        pyramidFlags);
}

void Niagara::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    const float4 clearColor(0.38f, 0.52f, 0.10f, 1);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);

    if (mpScene)
        mpScene->update(pRenderContext, getGlobalClock().getTime());

    if (!mpMeshletProgramEarly || !mpMeshletVarsEarly || mTotalMeshletCount == 0)
        return;

    onResize(pTargetFbo->getWidth(), pTargetFbo->getHeight());

    if (!mDvbCleared && mpDrawVisibility)
    {
        pRenderContext->clearUAV(mpDrawVisibility->getUAV().get(), uint4(0));
        mDvbCleared = true;
    }
    if (!mMvbCleared && mpMeshletVisibility)
    {
        pRenderContext->clearUAV(mpMeshletVisibility->getUAV().get(), uint4(0));
        mMvbCleared = true;
    }

    float aspect = (float)pTargetFbo->getWidth() / (float)pTargetFbo->getHeight();
    float fovY;
    float znear;
    float zfar;
    float4x4 view;
    float4x4 projection;
    if (mpScene && !mpScene->getCameras().empty())
    {
        const auto& pCam = mpScene->getCamera();
        view = pCam->getViewMatrix();
        projection = pCam->getData().projMatNoJitter;
        znear = pCam->getNearPlane();
        zfar = pCam->getFarPlane();
    }
    else
    {
        fovY = mResult.camera.fovY;
        znear = mResult.camera.znear;
        zfar = 1e6f;
        view = mResult.camera.viewMatrix;
        projection = math::perspective(fovY, aspect, znear, zfar);
    }

    // frustum planes: row3 + row0 = left plane, row3 + row1 = top plane (GLM convention)
    float4 frustumX = normalizePlane(projection.getRow(3) + projection.getRow(0));
    float4 frustumY = normalizePlane(projection.getRow(3) + projection.getRow(1));

    uint32_t pyrBaseW = std::max(1u, mDepthPyramidWidth / 2);
    uint32_t pyrBaseH = std::max(1u, mDepthPyramidHeight / 2);
    float P00 = projection[0][0];
    float P11 = projection[1][1];

    struct NiagaraGlobals
    {
        float4x4 projection;
        float4x4 view;
        uint32_t meshletCount;
        float znear;
        float zfar;
        float frustum[4];
        uint32_t clusterBackfaceEnabled;
        float pyramidWidth;
        float pyramidHeight;
        float P00;
        float P11;
        uint32_t occlusionEnabled;
        uint32_t clusterOcclusionEnabled;
    };

    auto bindCommonVars = [&](ProgramVars* pVars, const NiagaraGlobals& g) {
        auto var = pVars->getRootVar();
        var["CB"]["gGlobals"].setBlob(&g, sizeof(g));
        var["gTaskCommands"] = mpDcb;
        var["gDraws"] = mpDb;
        var["gMeshlets"] = mpMlb;
        var["gMeshletData"] = mpMdb;
        var["gVertices"] = mpVb;
        var["gClusterIndices"] = mpCib;
        var["gDrawVisibility"] = mpDrawVisibility;
        var["gMeshletVisibility"] = mpMeshletVisibility;
    };

    mpRasterState->setFbo(mpFbo);
    pRenderContext->clearFbo(mpFbo.get(), float4(0, 0, 0, 0), 1.0f, 0, FboAttachmentType::All);

    const uint32_t asGroupCount = (mTotalMeshletCount + kASGroupSize - 1) / kASGroupSize;

    NiagaraGlobals globalsBase = {};
    globalsBase.projection = projection;
    globalsBase.view = view;
    globalsBase.meshletCount = mTotalMeshletCount;
    globalsBase.znear = znear;
    globalsBase.zfar = zfar;
    globalsBase.frustum[0] = frustumX.x;
    globalsBase.frustum[1] = frustumX.z;
    globalsBase.frustum[2] = frustumY.y;
    globalsBase.frustum[3] = frustumY.z;
    globalsBase.clusterBackfaceEnabled = mTwoPhaseEnabled ? 1u : 1u;
    globalsBase.pyramidWidth = (float)pyrBaseW;
    globalsBase.pyramidHeight = (float)pyrBaseH;
    globalsBase.P00 = P00;
    globalsBase.P11 = P11;
    globalsBase.occlusionEnabled = mOcclusionEnabled ? 1u : 0u;
    globalsBase.clusterOcclusionEnabled = mTwoPhaseEnabled && mClusterOcclusionEnabled ? 1u : 0u;

    if (mTwoPhaseEnabled)
    {
        // Early pass: render previously visible meshlets
        bindCommonVars(mpMeshletVarsEarly.get(), globalsBase);
        mpRasterState->setProgram(mpMeshletProgramEarly);
        pRenderContext->drawMeshTasks(mpRasterState.get(), mpMeshletVarsEarly.get(), asGroupCount, 1, 1);

        // Build depth pyramid
        if (mpDepthReducePass && mpDepthPyramid && mpFbo->getDepthStencilTexture())
        {
            auto pDepth = mpFbo->getDepthStencilTexture();
            pRenderContext->resourceBarrier(pDepth.get(), Resource::State::ShaderResource);
            uint32_t inW = mDepthPyramidWidth;
            uint32_t inH = mDepthPyramidHeight;
            for (uint32_t i = 0; i < mDepthPyramidLevels; ++i)
            {
                uint32_t outW = std::max(1u, mpDepthPyramid->getWidth(i));
                uint32_t outH = std::max(1u, mpDepthPyramid->getHeight(i));
                if (i > 0)
                {
                    inW = std::max(1u, mpDepthPyramid->getWidth(i - 1));
                    inH = std::max(1u, mpDepthPyramid->getHeight(i - 1));
                }
                pRenderContext->resourceBarrier(mpDepthPyramid.get(), Resource::State::UnorderedAccess);
                auto var = mpDepthReducePass->getRootVar();
                var["gInputDepth"].setSrv(i == 0 ? pDepth->getSRV() : mpDepthPyramid->getSRV(i - 1, 1));
                var["gOutputPyramid"].setUav(mpDepthPyramid->getUAV(i));
                var["CB"]["gInputSize"] = uint2(inW, inH);
                var["CB"]["gOutputSize"] = uint2(outW, outH);
                mpDepthReducePass->execute(pRenderContext, outW, outH, 1);
                pRenderContext->uavBarrier(mpDepthPyramid.get());
            }
            pRenderContext->resourceBarrier(mpDepthPyramid.get(), Resource::State::ShaderResource);
        }

        // Late pass: occlusion test, render newly visible, update mvb
        bindCommonVars(mpMeshletVarsLate.get(), globalsBase);
        mpMeshletVarsLate->getRootVar()["gDepthPyramid"] = mpDepthPyramid;
        mpMeshletVarsLate->getRootVar()["gDepthPyramid_sampler"] = getDevice()->getDefaultSampler();
        mpRasterState->setProgram(mpMeshletProgramLate);
        pRenderContext->drawMeshTasks(mpRasterState.get(), mpMeshletVarsLate.get(), asGroupCount, 1, 1);

        // Update draw visibility from meshlet visibility
        if (mpUpdateDvbPass && mpDrawVisibility)
        {
            pRenderContext->uavBarrier(mpMeshletVisibility.get());
            auto var = mpUpdateDvbPass->getRootVar();
            var["gTaskCommands"] = mpDcb;
            var["gMeshletVisibility"] = mpMeshletVisibility;
            var["gDrawVisibility"] = mpDrawVisibility;
            var["CB"]["gDrawCount"] = (uint32_t)mResult.draws.size();
            mpUpdateDvbPass->execute(pRenderContext, (uint32_t)mResult.draws.size(), 1, 1);
        }
    }
    else
    {
        // Single pass: frustum + cone only
        bindCommonVars(mpMeshletVarsEarly.get(), globalsBase);
        mpRasterState->setProgram(mpMeshletProgramEarly);
        pRenderContext->drawMeshTasks(mpRasterState.get(), mpMeshletVarsEarly.get(), asGroupCount, 1, 1);
    }

    pRenderContext->blit(mpFbo->getColorTexture(0)->getSRV(), pTargetFbo->getRenderTargetView(0));
}

void Niagara::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Niagara", {250, 280});
    renderGlobalUI(pGui);
    if (w.dropdown("Scene", kSceneDropdownList, mSceneIndex))
        loadSelectedScene();
    w.checkbox("Two-phase (Early/Late)", mTwoPhaseEnabled);
    w.checkbox("Occlusion culling", mOcclusionEnabled);
    w.checkbox("Cluster occlusion (mvb)", mClusterOcclusionEnabled);
    w.text("AS frustum+cone cull, PS output wpos");
    if (mConvertOk)
        w.text(fmt::format("{} meshes, {} draws, {} meshlets", mResult.geometry.meshes.size(), mResult.draws.size(), mTotalMeshletCount));
    else
        w.text("failed");
    if (mpScene)
        mpScene->renderUI(w);
}

bool Niagara::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return mpScene && mpScene->onKeyEvent(keyEvent);
}

bool Niagara::onMouseEvent(const MouseEvent& mouseEvent)
{
    return mpScene && mpScene->onMouseEvent(mouseEvent);
}

void Niagara::onHotReload(HotReloadFlags reloaded)
{
}

SampleBase* Niagara::create(SampleApp* pHost)
{
    return new Niagara(pHost);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<SampleBase, Niagara>();
}
