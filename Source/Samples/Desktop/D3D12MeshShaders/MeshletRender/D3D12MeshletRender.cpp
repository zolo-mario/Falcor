#include "D3D12MeshletRender.h"
#include "Scene/SceneBuilder.h"
#include "Scene/SceneMeshletData.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

static const char kMeshShaderFile[] = "Samples/Desktop/D3D12MeshShaders/MeshletRender/MeshletRender.ms.slang";
static const float4 kClearColor(0.0f, 0.2f, 0.4f, 1.0f);

D3D12MeshletRender::D3D12MeshletRender(SampleApp* pHost) : SampleBase(pHost) {}

void D3D12MeshletRender::onLoad(RenderContext* pRenderContext)
{
    if (!getDevice()->isShaderModelSupported(ShaderModel::SM6_5))
    {
        logError("D3D12MeshletRender requires Shader Model 6.5 for mesh shader support.");
        return;
    }

    mpScene = SceneBuilder(getDevice(), "test_scenes/bunny.pyscene", Settings(), SceneBuilder::Flags::Default).getScene();
    if (!mpScene || mpScene->getGeometryInstanceCount() == 0)
    {
        logError("D3D12MeshletRender: Failed to load test_scenes/bunny.pyscene. Check FALCOR_MEDIA_FOLDERS.");
        return;
    }

    mpScene->setCameraController(Scene::CameraControllerType::Orbiter);
    mpScene->setCameraSpeed(25.f);
    mpScene->setCameraAspectRatio(16.f / 9.f);

    ProgramDesc desc;
    desc.addShaderModules(mpScene->getShaderModules());
    desc.addShaderLibrary(kMeshShaderFile).meshEntry("meshMain").psEntry("psMain");
    desc.addTypeConformances(mpScene->getTypeConformances());
    desc.setShaderModel(ShaderModel::SM6_5);

    DefineList defines;
    defines.add(mpScene->getSceneDefines());

    mpMeshletProgram = Program::create(getDevice(), desc, defines);
    mpMeshletVars = ProgramVars::create(getDevice(), mpMeshletProgram.get());

    mpGraphicsState = GraphicsState::create(getDevice());
    mpGraphicsState->setProgram(mpMeshletProgram);
    mpGraphicsState->setVao(nullptr);

    DepthStencilState::Desc dsDesc;
    dsDesc.setDepthFunc(ComparisonFunc::LessEqual).setDepthWriteMask(true);
    mpGraphicsState->setDepthStencilState(DepthStencilState::create(dsDesc));
}

void D3D12MeshletRender::onShutdown()
{
    mpScene.reset();
}

void D3D12MeshletRender::onResize(uint32_t width, uint32_t height)
{
    if (mpScene)
        mpScene->setCameraAspectRatio(width / (float)height);
}

void D3D12MeshletRender::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    pRenderContext->clearFbo(pTargetFbo.get(), kClearColor, 1.0f, 0, FboAttachmentType::All);

    if (!mpScene || !mpMeshletProgram || !mpMeshletVars)
        return;

    mpScene->update(pRenderContext, (float)getFrameRate().getLastFrameTime());

    SceneMeshletData* pMeshletData = mpScene->getMeshletData(pRenderContext);
    if (!pMeshletData || !pMeshletData->isValid())
        return;

    mMeshletCount = pMeshletData->getMeshletCount();

    auto var = mpMeshletVars->getRootVar();
    var["CB"]["gMeshletCount"] = mMeshletCount;
    var["CB"]["gDrawMeshlets"] = 1u;
    var["gMeshlets"] = pMeshletData->getMeshletBuffer();
    var["gMeshletVertices"] = pMeshletData->getMeshletVerticesBuffer();
    var["gMeshletTriangles"] = pMeshletData->getMeshletTrianglesBuffer();
    mpScene->bindShaderDataForRaytracing(pRenderContext, var["gScene"]);

    mpGraphicsState->setFbo(pTargetFbo, true);
    pRenderContext->drawMeshTasks(mpGraphicsState.get(), mpMeshletVars.get(), mMeshletCount, 1, 1);
}

void D3D12MeshletRender::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "D3D12 Meshlet Render", {250, 200});
    renderGlobalUI(pGui);
    w.text("MS + PS, bunny via SceneMeshletData. Mouse: orbit.");
    w.text(fmt::format("Meshlets: {}", mMeshletCount));
}

bool D3D12MeshletRender::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return mpScene && mpScene->onKeyEvent(keyEvent);
}

bool D3D12MeshletRender::onMouseEvent(const MouseEvent& mouseEvent)
{
    return mpScene && mpScene->onMouseEvent(mouseEvent);
}

SampleBase* D3D12MeshletRender::create(SampleApp* pHost)
{
    return new D3D12MeshletRender(pHost);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<SampleBase, D3D12MeshletRender>();
}
