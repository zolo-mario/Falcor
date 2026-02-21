#include "MeshletCull.h"
#include "Scene/SceneBuilder.h"
#include "Scene/SceneMeshletData.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

static const char kMeshletShaderFile[] = "Samples/Desktop/D3D12MeshShaders/MeshletCull/MeshletCull.slang";
static const float4 kClearColor(0.0f, 0.2f, 0.4f, 1.0f);

MeshletCull::MeshletCull(SampleApp* pHost) : SampleBase(pHost) {}

MeshletCull::~MeshletCull() {}

void MeshletCull::onLoad(RenderContext* pRenderContext)
{
    if (!getDevice()->isShaderModelSupported(ShaderModel::SM6_5))
    {
        throw RuntimeError("MeshletCull requires Shader Model 6.5 for mesh shader support.");
    }

    mpScene = SceneBuilder(getDevice(), "test_scenes/bunny.pyscene", Settings(), SceneBuilder::Flags::Default).getScene();
    if (!mpScene || mpScene->getGeometryInstanceCount() == 0)
    {
        throw RuntimeError("MeshletCull: Failed to load test_scenes/bunny.pyscene. Check FALCOR_MEDIA_FOLDERS.");
    }

    mpScene->setCameraController(Scene::CameraControllerType::Orbiter);
    mpScene->setCameraSpeed(25.f);
    mpScene->setCameraAspectRatio(16.f / 9.f);

    ProgramDesc desc;
    desc.addShaderModules(mpScene->getShaderModules());
    desc.addShaderLibrary(kMeshletShaderFile)
        .amplificationEntry("ampMain")
        .meshEntry("meshMain")
        .psEntry("psMain");
    desc.addTypeConformances(mpScene->getTypeConformances());
    desc.setShaderModel(ShaderModel::SM6_5);

    DefineList defines;
    defines.add(mpScene->getSceneDefines());

    mpMeshletProgram = Program::create(getDevice(), desc, defines);
    mpMeshletVars = ProgramVars::create(getDevice(), mpMeshletProgram.get());

    mpMeshletState = GraphicsState::create(getDevice());
    mpMeshletState->setProgram(mpMeshletProgram);
    mpMeshletState->setVao(nullptr);

    mpDepthStencilState = DepthStencilState::create(
        DepthStencilState::Desc().setDepthFunc(ComparisonFunc::LessEqual).setDepthWriteMask(true));
    mpMeshletState->setDepthStencilState(mpDepthStencilState);

    mpRasterizerState = RasterizerState::create(RasterizerState::Desc().setCullMode(RasterizerState::CullMode::None));
    mpMeshletState->setRasterizerState(mpRasterizerState);

    mpConstantsBuffer = getDevice()->createStructuredBuffer(
        sizeof(MeshletCullBunnyCB), 1, ResourceBindFlags::ShaderResource, MemoryType::Upload, nullptr, false);
}

void MeshletCull::updateConstants(RenderContext* pRenderContext)
{
    const Camera* pCamera = mpScene->getCamera().get();
    float4x4 viewProj = pCamera->getViewProjMatrix();
    float3 viewPosition = pCamera->getPosition();

    float4x4 vp = transpose(viewProj);
    auto normalizePlane = [](float4 p) {
        float len = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
        return float4(p.x / len, p.y / len, p.z / len, p.w / len);
    };

    MeshletCullBunnyCB cb;
    cb.viewProj = viewProj;
    cb.planes[0] = normalizePlane(vp[3] + vp[0]);
    cb.planes[1] = normalizePlane(vp[3] - vp[0]);
    cb.planes[2] = normalizePlane(vp[3] + vp[1]);
    cb.planes[3] = normalizePlane(vp[3] - vp[1]);
    cb.planes[4] = normalizePlane(vp[2]);
    cb.planes[5] = normalizePlane(vp[3] - vp[2]);
    cb.viewPosition = viewPosition;
    cb.drawMeshlets = mDrawMeshlets ? 1u : 0u;
    cb.meshletCount = mMeshletCount;

    mpConstantsBuffer->setBlob(&cb, 0, sizeof(cb));
}

void MeshletCull::onShutdown()
{
    mpScene.reset();
}

void MeshletCull::onResize(uint32_t width, uint32_t height)
{
    if (mpScene)
        mpScene->setCameraAspectRatio((float)width / (float)height);
}

void MeshletCull::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    pRenderContext->clearFbo(pTargetFbo.get(), kClearColor, 1.0f, 0, FboAttachmentType::All);

    if (!mpScene || !mpMeshletProgram || !mpMeshletVars)
        return;

    mpScene->update(pRenderContext, (float)getFrameRate().getLastFrameTime());

    SceneMeshletData* pMeshletData = mpScene->getMeshletData(pRenderContext);
    if (!pMeshletData || !pMeshletData->isValid())
        return;

    mMeshletCount = pMeshletData->getMeshletCount();

    uint32_t width = pTargetFbo->getWidth();
    uint32_t height = pTargetFbo->getHeight();
    if (!mpFbo || mpFbo->getWidth() != width || mpFbo->getHeight() != height)
    {
        auto pDevice = getDevice();
        mpFbo = Fbo::create(pDevice);
        auto rtFlags = ResourceBindFlags::RenderTarget | ResourceBindFlags::ShaderResource;
        mpFbo->attachColorTarget(
            pDevice->createTexture2D(width, height, ResourceFormat::RGBA8UnormSrgb, 1, 1, nullptr, rtFlags), 0);
        mpFbo->attachDepthStencilTarget(
            pDevice->createTexture2D(width, height, ResourceFormat::D32Float, 1, 1, nullptr, ResourceBindFlags::DepthStencil));
    }

    updateConstants(pRenderContext);

    auto var = mpMeshletVars->getRootVar();
    var["gCB"] = mpConstantsBuffer;
    var["gMeshlets"] = pMeshletData->getMeshletBuffer();
    var["gMeshletVertices"] = pMeshletData->getMeshletVerticesBuffer();
    var["gMeshletTriangles"] = pMeshletData->getMeshletTrianglesBuffer();
    mpScene->bindShaderDataForRaytracing(pRenderContext, var["gScene"]);

    mpMeshletState->setFbo(mpFbo);
    pRenderContext->clearFbo(mpFbo.get(), kClearColor, 1.0f, 0, FboAttachmentType::All);

    const uint32_t asGroupCount = (mMeshletCount + kASGroupSize - 1) / kASGroupSize;
    pRenderContext->drawMeshTasks(mpMeshletState.get(), mpMeshletVars.get(), asGroupCount, 1, 1);

    pRenderContext->blit(mpFbo->getColorTexture(0)->getSRV(), pTargetFbo->getRenderTargetView(0));
}

void MeshletCull::setProperties(const Properties& props)
{
    if (props.has("draw-meshlets"))
        mDrawMeshlets = (props.get<double>("draw-meshlets", 1.0) != 0.0);
}

Properties MeshletCull::getProperties() const
{
    Properties p;
    p["draw-meshlets"] = mDrawMeshlets ? 1.0 : 0.0;
    return p;
}

void MeshletCull::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "D3D12 Meshlet Cull", {250, 200});
    renderGlobalUI(pGui);
    w.text("MeshletCull - AS culls meshlests before MS dispatch");
    w.text("Bunny model via SceneMeshletData");
    w.text(fmt::format("Meshlets: {}", mMeshletCount));
    w.checkbox("Draw Meshlets", mDrawMeshlets);
}

bool MeshletCull::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return mpScene && mpScene->onKeyEvent(keyEvent);
}

bool MeshletCull::onMouseEvent(const MouseEvent& mouseEvent)
{
    return mpScene && mpScene->onMouseEvent(mouseEvent);
}

void MeshletCull::onHotReload(HotReloadFlags reloaded)
{
    if (is_set(reloaded, HotReloadFlags::Program) && mpScene)
    {
        ProgramDesc desc;
        desc.addShaderModules(mpScene->getShaderModules());
        desc.addShaderLibrary(kMeshletShaderFile)
            .amplificationEntry("ampMain")
            .meshEntry("meshMain")
            .psEntry("psMain");
        desc.addTypeConformances(mpScene->getTypeConformances());
        desc.setShaderModel(ShaderModel::SM6_5);
        DefineList defines;
        defines.add(mpScene->getSceneDefines());
        mpMeshletProgram = Program::create(getDevice(), desc, defines);
        mpMeshletVars = ProgramVars::create(getDevice(), mpMeshletProgram.get());
        mpMeshletState->setProgram(mpMeshletProgram);
    }
}

SampleBase* MeshletCull::create(SampleApp* pHost)
{
    return new MeshletCull(pHost);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<SampleBase, MeshletCull>();
}
