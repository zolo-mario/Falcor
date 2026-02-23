#include "Niagara.h"
#include "Scene/Scene.h"
#include "Scene/SceneBuilder.h"
#include "Utils/Math/Matrix.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

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
    mTotalMeshletCount = 0;

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

    std::vector<NiagaraFormat::MeshTaskCommand> taskCommands;
    std::vector<uint32_t> clusterIndices;

    for (uint32_t drawId = 0; drawId < draws.size(); ++drawId)
    {
        const auto& draw = draws[drawId];
        if (draw.meshIndex >= geom.meshes.size())
            continue;
        const auto& mesh = geom.meshes[draw.meshIndex];
        if (mesh.lodCount == 0)
            continue;

        const auto& lod0 = mesh.lods[0];
        uint32_t taskOffset = lod0.meshletOffset;
        uint32_t taskCount = lod0.meshletCount;

        NiagaraFormat::MeshTaskCommand cmd = {};
        cmd.drawId = drawId;
        cmd.taskOffset = taskOffset;
        cmd.taskCount = taskCount;
        cmd.meshletVisibilityOffset = 0;
        taskCommands.push_back(cmd);

        uint32_t commandId = drawId * kTaskStride;
        for (uint32_t mgi = 0; mgi < taskCount; ++mgi)
        {
            clusterIndices.push_back(commandId | (mgi << 24));
        }
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

    ProgramDesc desc;
    desc.addShaderLibrary(kMeshShaderFile)
        .amplificationEntry("ampMain")
        .meshEntry("meshMain")
        .psEntry("psMain");
    desc.setShaderModel(ShaderModel::SM6_5);

    mpMeshletProgram = Program::create(pDevice, desc);
    mpMeshletVars = ProgramVars::create(pDevice, mpMeshletProgram.get());

    mpRasterState = GraphicsState::create(pDevice);
    mpRasterState->setProgram(mpMeshletProgram);
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
        pDevice->createTexture2D(width, height, ResourceFormat::D32Float, 1, 1, nullptr, ResourceBindFlags::DepthStencil));
}

void Niagara::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    const float4 clearColor(0.38f, 0.52f, 0.10f, 1);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);

    if (mpScene)
        mpScene->update(pRenderContext, getGlobalClock().getTime());

    if (!mpMeshletProgram || !mpMeshletVars || mTotalMeshletCount == 0)
        return;

    onResize(pTargetFbo->getWidth(), pTargetFbo->getHeight());

    float aspect = (float)pTargetFbo->getWidth() / (float)pTargetFbo->getHeight();
    float fovY;
    float znear;
    float4x4 view;
    float4x4 projection;
    if (mpScene && !mpScene->getCameras().empty())
    {
        const auto& pCam = mpScene->getCamera();
        view = pCam->getViewMatrix();
        projection = pCam->getData().projMatNoJitter;
    }
    else
    {
        fovY = mResult.camera.fovY;
        znear = mResult.camera.znear;
        view = mResult.camera.viewMatrix;
        projection = math::perspective(fovY, aspect, znear, 1e6f);
    }

    struct NiagaraGlobals
    {
        float4x4 projection;
        float4x4 view;
        uint32_t meshletCount;
    };
    NiagaraGlobals globals = {};
    globals.projection = projection;
    globals.view = view;
    globals.meshletCount = mTotalMeshletCount;

    auto var = mpMeshletVars->getRootVar();
    var["CB"]["gGlobals"].setBlob(&globals, sizeof(globals));
    var["gTaskCommands"] = mpDcb;
    var["gDraws"] = mpDb;
    var["gMeshlets"] = mpMlb;
    var["gMeshletData"] = mpMdb;
    var["gVertices"] = mpVb;
    var["gClusterIndices"] = mpCib;

    mpRasterState->setFbo(mpFbo);
    pRenderContext->clearFbo(mpFbo.get(), float4(0, 0, 0, 0), 1.0f, 0, FboAttachmentType::All);

    const uint32_t asGroupCount = (mTotalMeshletCount + kASGroupSize - 1) / kASGroupSize;
    pRenderContext->drawMeshTasks(mpRasterState.get(), mpMeshletVars.get(), asGroupCount, 1, 1);

    pRenderContext->blit(mpFbo->getColorTexture(0)->getSRV(), pTargetFbo->getRenderTargetView(0));
}

void Niagara::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Niagara", {250, 200});
    renderGlobalUI(pGui);
    if (w.dropdown("Scene", kSceneDropdownList, mSceneIndex))
        loadSelectedScene();
    w.text("AS 1:1 DispatchMesh, PS output wpos");
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
