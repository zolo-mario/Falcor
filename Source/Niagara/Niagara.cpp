#include "Niagara.h"
#include "NiagaraConfig.h"
#include "Utils/CrashHandler.h"
#include "Scene/SceneBuilder.h"
#include "Core/API/Buffer.h"
#include "Core/API/Device.h"
#include "Core/API/Texture.h"
#include "Utils/Math/Matrix.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

static const char kMeshShaderFile[] = "Niagara/shaders/NiagaraMeshlet.ms.slang";
static const uint32_t kMaxTextures = 64;

static const std::vector<std::string> kScenePaths = {
    "test_scenes/bunny.pyscene",
    "Arcade/Arcade.pyscene",
    "test_scenes/cornell_box.pyscene",
    "test_scenes/cesium_man/CesiumMan.pyscene",
};

static const Gui::DropdownList kSceneDropdownList = {
    {0, "Bunny"},
    {1, "Arcade"},
    {2, "Cornell Box"},
    {3, "Cesium Man"},
};

uint32_t mSampleGuiWidth = 250;
uint32_t mSampleGuiHeight = 200;
uint32_t mSampleGuiPositionX = 20;
uint32_t mSampleGuiPositionY = 40;

Niagara::Niagara(const SampleAppConfig& config) : SampleApp(config) {}

Niagara::~Niagara() = default;

void Niagara::loadScene(RenderContext* pRenderContext, const std::filesystem::path& path, SceneBuilder::Flags buildFlags)
{
    ref<Scene> pScene = SceneBuilder(getDevice(), path, getSettings(), buildFlags).getScene();
    if (pScene)
    {
        convertFalcorSceneToNiagaraScene(pScene.get(), mpNiagaraScene, true, false, false);
        uploadSceneBuffers(pRenderContext);
    }
}

void Niagara::uploadSceneBuffers(RenderContext* pRenderContext)
{
    auto& geom = mpNiagaraScene.geometry;
    auto& draws = mpNiagaraScene.draws;
    auto& materials = mpNiagaraScene.materials;
    auto& texturePaths = mpNiagaraScene.texturePaths;

    mpVb = nullptr;
    mpIb = nullptr;
    mpMlb = nullptr;
    mpMdb = nullptr;
    mpDb = nullptr;
    mpMb = nullptr;
    mpMtb = nullptr;
    mpDcb = nullptr;
    mpCib = nullptr;
    mpTextures.clear();
    mTotalMeshletCount = 0;

    if (geom.vertices.empty() || draws.empty())
        return;

    auto pDevice = getDevice();

    mpVb = pDevice->createStructuredBuffer(
        sizeof(NiagaraVertex),
        (uint32_t)geom.vertices.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        geom.vertices.data());

    mpIb = pDevice->createBuffer(
        geom.indices.size() * sizeof(uint32_t),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        geom.indices.data());

    mpMlb = pDevice->createStructuredBuffer(
        sizeof(NiagaraMeshlet),
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
        sizeof(NiagaraMeshDraw),
        (uint32_t)draws.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        draws.data());

    mpMb = pDevice->createStructuredBuffer(
        sizeof(NiagaraMesh),
        (uint32_t)geom.meshes.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        geom.meshes.data());

    mpMtb = pDevice->createStructuredBuffer(
        sizeof(NiagaraMaterial),
        (uint32_t)materials.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        materials.data());

    std::vector<NiagaraMeshTaskCommand> taskCommands;
    std::vector<uint32_t> clusterIndices;

    const uint32_t TASK_STRIDE = 64;
    for (uint32_t drawId = 0; drawId < draws.size(); ++drawId)
    {
        const auto& draw = draws[drawId];
        const auto& mesh = geom.meshes[draw.meshIndex];
        if (mesh.lodCount == 0)
            continue;

        const auto& lod0 = mesh.lods[0];
        uint32_t taskOffset = lod0.meshletOffset;
        uint32_t taskCount = lod0.meshletCount;

        NiagaraMeshTaskCommand cmd = {};
        cmd.drawId = drawId;
        cmd.taskOffset = taskOffset;
        cmd.taskCount = taskCount;
        cmd.lateDrawVisibility = 0;
        cmd.meshletVisibilityOffset = 0;
        taskCommands.push_back(cmd);

        uint32_t commandId = drawId * TASK_STRIDE;
        for (uint32_t mgi = 0; mgi < taskCount; ++mgi)
        {
            clusterIndices.push_back(commandId | (mgi << 24));
        }
    }

    mTotalMeshletCount = (uint32_t)clusterIndices.size();

    mpDcb = pDevice->createStructuredBuffer(
        sizeof(NiagaraMeshTaskCommand),
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

    uint32_t maxTexIndex = 0;
    for (const auto& m : materials)
    {
        maxTexIndex = std::max(maxTexIndex, (uint32_t)std::max({m.albedoTexture, m.normalTexture, m.specularTexture, m.emissiveTexture}));
    }

    uint32_t whitePixel = 0xFFFFFFFF;
    ref<Texture> pWhiteTex = pDevice->createTexture2D(1, 1, ResourceFormat::RGBA8UnormSrgb, 1, 1, &whitePixel, ResourceBindFlags::ShaderResource);

    mpTextures.resize(std::max(1u, maxTexIndex + 1), pWhiteTex);
    mpTextures.resize(kMaxTextures, pWhiteTex);
    for (uint32_t i = 0; i < texturePaths.size() && i + 1 < mpTextures.size(); ++i)
    {
        std::filesystem::path path(texturePaths[i]);
        if (!path.is_absolute())
        {
            auto rtPath = getRuntimeDirectory() / path;
            auto dataPath = getRuntimeDirectory() / "data" / path;
            if (std::filesystem::exists(rtPath))
                path = rtPath;
            else if (std::filesystem::exists(dataPath))
                path = dataPath;
        }
        if (std::filesystem::exists(path))
        {
            if (auto pTex = Texture::createFromFile(pDevice, path, false, true))
                mpTextures[i + 1] = pTex;
        }
    }

    if (!pDevice->isShaderModelSupported(ShaderModel::SM6_5))
    {
        logError("Niagara requires Shader Model 6.5 for mesh shader support.");
        return;
    }

    ProgramDesc desc;
    desc.addShaderLibrary(kMeshShaderFile).meshEntry("meshMain").psEntry("psMain");
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
    loadScene(pRenderContext, kScenePaths[mSceneIndex]);
}

void Niagara::onShutdown() {}

void Niagara::onResize(uint32_t width, uint32_t height)
{
    if (mpFbo && mpFbo->getWidth() == width && mpFbo->getHeight() == height)
        return;

    auto pDevice = getDevice();
    mpFbo = Fbo::create(pDevice);
    auto rtSrvFlags = ResourceBindFlags::RenderTarget | ResourceBindFlags::ShaderResource;
    mpFbo->attachColorTarget(
        pDevice->createTexture2D(width, height, ResourceFormat::RGBA8UnormSrgb, 1, 1, nullptr, rtSrvFlags),
        0);
    mpFbo->attachColorTarget(
        pDevice->createTexture2D(width, height, ResourceFormat::RGBA8UnormSrgb, 1, 1, nullptr, rtSrvFlags),
        1);
    mpFbo->attachColorTarget(
        pDevice->createTexture2D(width, height, ResourceFormat::R32Uint, 1, 1, nullptr, rtSrvFlags),
        2);
    mpFbo->attachDepthStencilTarget(
        pDevice->createTexture2D(width, height, ResourceFormat::D32Float, 1, 1, nullptr, ResourceBindFlags::DepthStencil));
}

void Niagara::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    const float4 clearColor(0.38f, 0.52f, 0.10f, 1);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);

    if (!mpMeshletProgram || !mpMeshletVars || mTotalMeshletCount == 0)
        return;

    onResize(pTargetFbo->getWidth(), pTargetFbo->getHeight());

    float aspect = (float)pTargetFbo->getWidth() / (float)pTargetFbo->getHeight();
    float fovY = mpNiagaraScene.camera.fovY;
    float znear = mpNiagaraScene.camera.znear;

    float4x4 view = mpNiagaraScene.camera.viewMatrix;

    float4x4 projection = math::perspective(fovY, aspect, znear, 1e6f);

    NiagaraGlobals globals = {};
    globals.projection = projection;
    globals.cullData.view = view;
    globals.cullData.znear = znear;
    globals.cullData.zfar = 1e6f;
    globals.cullData.drawCount = (uint32_t)mpNiagaraScene.draws.size();
    globals.screenWidth = (float)pTargetFbo->getWidth();
    globals.screenHeight = (float)pTargetFbo->getHeight();

    auto var = mpMeshletVars->getRootVar();
    var["CB"]["gGlobals"].setBlob(&globals, sizeof(globals));
    var["gTaskCommands"] = mpDcb;
    var["gDraws"] = mpDb;
    var["gMeshlets"] = mpMlb;
    var["gMeshletData"] = mpMdb;
    var["gVertices"] = mpVb;
    var["gClusterIndices"] = mpCib;
    var["gMaterials"] = mpMtb;
    var["gSampler"] = pRenderContext->getDevice()->getDefaultSampler();

    for (uint32_t i = 0; i < kMaxTextures; ++i)
    {
        var["gTextures"][i] = mpTextures[i];
    }

    mpRasterState->setFbo(mpFbo);
    pRenderContext->clearFbo(mpFbo.get(), float4(0, 0, 0, 0), 1.0f, 0, FboAttachmentType::All);

    pRenderContext->drawMeshTasks(mpRasterState.get(), mpMeshletVars.get(), mTotalMeshletCount, 1, 1);

    pRenderContext->blit(mpFbo->getColorTexture(0)->getSRV(), pTargetFbo->getRenderTargetView(0));
}

void Niagara::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Niagara", {250, 250});
    renderGlobalUI(pGui);
    if (w.dropdown("Scene", kSceneDropdownList, mSceneIndex))
    {
        loadScene(getRenderContext(), kScenePaths[mSceneIndex]);
    }
    w.text("Meshlet -> Mesh Shader -> PS (meshlet ID)");
    w.text(fmt::format("Meshlets: {}", mTotalMeshletCount));
}

bool Niagara::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return false;
}

bool Niagara::onMouseEvent(const MouseEvent& mouseEvent)
{
    return false;
}

void Niagara::onHotReload(HotReloadFlags reloaded) {}

int runMain(int argc, char** argv)
{
    SampleAppConfig config;
    config.windowDesc.title = "Niagara";
    config.windowDesc.width = 800;
    config.windowDesc.height = 600;
    config.windowDesc.resizableWindow = true;
    config.generateShaderDebugInfo = true;

    Niagara project(config);
    return project.run();
}

int main(int argc, char** argv)
{
    Falcor::CrashHandler::Install();
    return catchAndReportAllExceptions([&]() { return runMain(argc, argv); });
}
