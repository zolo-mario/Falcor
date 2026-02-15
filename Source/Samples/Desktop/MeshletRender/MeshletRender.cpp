#include "MeshletRender.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

static const char kMeshShaderFile[] = "Samples/Desktop/MeshletRender/MeshletRender.slang";

// Match D3D12 MeshletRender clear color (0.0f, 0.2f, 0.4f, 1.0f)
static const float4 kClearColor(0.0f, 0.2f, 0.4f, 1.0f);

MeshletRender::MeshletRender(const SampleAppConfig& config) : SampleApp(config) {}

MeshletRender::~MeshletRender() {}

void MeshletRender::updateCamera(float elapsed)
{
    const float moveSpeed = 150.f;
    const float turnSpeed = 1.57f;

    float3 move(0, 0, 0);
    if (m_keysPressed[0]) move.z -= 1.f; // W
    if (m_keysPressed[1]) move.x -= 1.f; // A
    if (m_keysPressed[2]) move.z += 1.f; // S
    if (m_keysPressed[3]) move.x += 1.f; // D

    if (std::abs(move.x) > 0.1f || std::abs(move.z) > 0.1f)
    {
        move = normalize(move);
    }

    if (m_keysPressed[4]) m_cameraYaw += turnSpeed * elapsed;   // Left
    if (m_keysPressed[5]) m_cameraYaw -= turnSpeed * elapsed;   // Right
    if (m_keysPressed[6]) m_cameraPitch += turnSpeed * elapsed; // Up
    if (m_keysPressed[7]) m_cameraPitch -= turnSpeed * elapsed; // Down

    m_cameraPitch = std::clamp(m_cameraPitch, -0.785f, 0.785f);

    float c = std::cos(m_cameraYaw);
    float s = std::sin(m_cameraYaw);
    float x = move.x * (-c) - move.z * s;
    float z = move.x * s - move.z * c;
    m_cameraPosition.x += x * moveSpeed * elapsed;
    m_cameraPosition.z += z * moveSpeed * elapsed;
}

void MeshletRender::updateConstants()
{
    float3 lookDir(std::sin(m_cameraYaw) * std::cos(m_cameraPitch),
                  std::sin(m_cameraPitch),
                  std::cos(m_cameraYaw) * std::cos(m_cameraPitch));
    float3 up(0, 1, 0);
    float4x4 view = math::matrixFromLookAt(m_cameraPosition, m_cameraPosition + lookDir, up);

    uint2 size = mpFbo ? uint2(mpFbo->getWidth(), mpFbo->getHeight()) : uint2(1280, 720);
    float aspect = size.y > 0 ? (float)size.x / size.y : (1280.f / 720.f);
    float4x4 proj = math::perspective(3.14159f / 3.f, aspect, 1.f, 1000.f);

    float4x4 world = math::matrix4x4Identity();
    float4x4 worldView = mul(view, world);
    float4x4 worldViewProj = mul(proj, worldView);

    struct Constants
    {
        float4x4 World;
        float4x4 WorldView;
        float4x4 WorldViewProj;
        uint32_t DrawMeshlets;
    };
    Constants c;
    c.World = world;
    c.WorldView = worldView;
    c.WorldViewProj = worldViewProj;
    c.DrawMeshlets = 1;

    mpConstantBuffer->setBlob(&c, 0, sizeof(c));
}

void MeshletRender::onLoad(RenderContext* pRenderContext)
{
    if (!getDevice()->isShaderModelSupported(ShaderModel::SM6_5))
    {
        logError("MeshletRender requires Shader Model 6.5 for mesh shader support.");
        return;
    }

    ProgramDesc desc;
    desc.addShaderLibrary(kMeshShaderFile).meshEntry("meshMain").psEntry("psMain");
    desc.setShaderModel(ShaderModel::SM6_5);

    mpMeshletProgram = Program::create(getDevice(), desc);
    mpMeshletVars = ProgramVars::create(getDevice(), mpMeshletProgram.get());

    // Constant buffer: World, WorldView, WorldViewProj, DrawMeshlets (256-byte aligned for D3D12)
    mpConstantBuffer = getDevice()->createBuffer(
        256,
        ResourceBindFlags::Constant,
        MemoryType::Upload);

    struct MeshInfo
    {
        uint32_t IndexBytes;
        uint32_t MeshletOffset;
    };
    mpMeshInfoBuffer = getDevice()->createBuffer(
        sizeof(MeshInfo),
        ResourceBindFlags::Constant,
        MemoryType::Upload);

    mpGraphicsState = GraphicsState::create(getDevice());
    mpGraphicsState->setProgram(mpMeshletProgram);
    mpGraphicsState->setVao(nullptr);

    DepthStencilState::Desc dsDesc;
    dsDesc.setDepthFunc(ComparisonFunc::LessEqual).setDepthWriteMask(true);
    mpGraphicsState->setDepthStencilState(DepthStencilState::create(dsDesc));

    // Load mesh - try multiple paths
    std::vector<std::filesystem::path> searchPaths = {
        getRuntimeDirectory() / "Dragon_LOD0.bin",
        getRuntimeDirectory() / "data" / "Dragon_LOD0.bin",
    };

    bool loaded = false;
    for (const auto& p : searchPaths)
    {
        if (std::filesystem::exists(p) && m_model.loadFromFile(p))
        {
            loaded = true;
            break;
        }
    }

    if (!loaded)
    {
        logWarning("MeshletRender: Dragon_LOD0.bin not found. Run D3D12 WavefrontConverter to generate it from Dragon.obj.");
        return;
    }

    m_model.uploadGpuResources(getDevice(), pRenderContext);
}

void MeshletRender::onShutdown() {}

void MeshletRender::onResize(uint32_t width, uint32_t height)
{
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
}

void MeshletRender::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    pRenderContext->clearFbo(pTargetFbo.get(), kClearColor, 1.0f, 0, FboAttachmentType::All);

    if (!mpMeshletProgram || !mpMeshletVars || m_model.getMeshCount() == 0)
        return;

    onResize(pTargetFbo->getWidth(), pTargetFbo->getHeight());

    updateCamera((float)getFrameRate().getLastFrameTime());
    updateConstants();

    auto var = mpMeshletVars->getRootVar();
    var["CB"] = mpConstantBuffer;
    var["Vertices"] = nullptr;
    var["Meshlets"] = nullptr;
    var["UniqueVertexIndices"] = nullptr;
    var["PrimitiveIndices"] = nullptr;

    mpGraphicsState->setFbo(mpFbo);
    pRenderContext->clearFbo(mpFbo.get(), kClearColor, 1.0f, 0, FboAttachmentType::All);

    for (size_t mi = 0; mi < m_model.getMeshCount(); ++mi)
    {
        auto& mesh = m_model.getMesh(mi);
        var["Vertices"] = mesh.pVertexBuffer;
        var["Meshlets"] = mesh.pMeshletBuffer;
        var["UniqueVertexIndices"] = mesh.pUniqueVertexIndexBuffer;
        var["PrimitiveIndices"] = mesh.pPrimitiveIndexBuffer;

        struct MeshInfo
        {
            uint32_t IndexBytes;
            uint32_t MeshletOffset;
        };
        for (const auto& subset : mesh.MeshletSubsets)
        {
            MeshInfo info;
            info.IndexBytes = mesh.IndexSize;
            info.MeshletOffset = subset.Offset;
            mpMeshInfoBuffer->setBlob(&info, 0, sizeof(info));
            var["MeshInfoCB"] = mpMeshInfoBuffer;

            pRenderContext->drawMeshTasks(mpGraphicsState.get(), mpMeshletVars.get(), subset.Count, 1, 1);
        }
    }

    pRenderContext->blit(mpFbo->getColorTexture(0)->getSRV(), pTargetFbo->getRenderTargetView(0));
}

void MeshletRender::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "MeshletRender", {250, 200});
    renderGlobalUI(pGui);
    w.text("D3D12 MeshletRender - migrated to Falcor");
    w.text("WASD: move, Arrows: look");
}

bool MeshletRender::onKeyEvent(const KeyboardEvent& keyEvent)
{
    if (keyEvent.type == KeyboardEvent::Type::KeyPressed)
    {
        switch (keyEvent.key)
        {
        case Input::Key::W: m_keysPressed[0] = true; return true;
        case Input::Key::A: m_keysPressed[1] = true; return true;
        case Input::Key::S: m_keysPressed[2] = true; return true;
        case Input::Key::D: m_keysPressed[3] = true; return true;
        case Input::Key::Left: m_keysPressed[4] = true; return true;
        case Input::Key::Right: m_keysPressed[5] = true; return true;
        case Input::Key::Up: m_keysPressed[6] = true; return true;
        case Input::Key::Down: m_keysPressed[7] = true; return true;
        default: break;
        }
    }
    else if (keyEvent.type == KeyboardEvent::Type::KeyReleased)
    {
        switch (keyEvent.key)
        {
        case Input::Key::W: m_keysPressed[0] = false; return true;
        case Input::Key::A: m_keysPressed[1] = false; return true;
        case Input::Key::S: m_keysPressed[2] = false; return true;
        case Input::Key::D: m_keysPressed[3] = false; return true;
        case Input::Key::Left: m_keysPressed[4] = false; return true;
        case Input::Key::Right: m_keysPressed[5] = false; return true;
        case Input::Key::Up: m_keysPressed[6] = false; return true;
        case Input::Key::Down: m_keysPressed[7] = false; return true;
        default: break;
        }
    }
    return false;
}

bool MeshletRender::onMouseEvent(const MouseEvent& mouseEvent)
{
    return false;
}

void MeshletRender::onHotReload(HotReloadFlags reloaded) {}

int runMain(int argc, char** argv)
{
    SampleAppConfig config;
    config.windowDesc.title = "D3D12 MeshletRender";
    config.windowDesc.resizableWindow = true;

    MeshletRender project(config);
    return project.run();
}

int main(int argc, char** argv)
{
    return catchAndReportAllExceptions([&]() { return runMain(argc, argv); });
}
