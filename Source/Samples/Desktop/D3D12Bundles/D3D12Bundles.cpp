#include "D3D12Bundles.h"
#include "Core/Platform/OS.h"
#include "Utils/Math/FalcorMath.h"
#include <fstream>

FALCOR_EXPORT_D3D12_AGILITY_SDK

D3D12Bundles::D3D12Bundles(const SampleAppConfig& config) : SampleApp(config) {}

D3D12Bundles::~D3D12Bundles() {}

std::filesystem::path D3D12Bundles::findOcctityBin()
{
    // Search order: exe dir, project sample dir, sibling DirectX-Graphics-Samples
    std::vector<std::filesystem::path> searchPaths = {
        getRuntimeDirectory() / "occcity.bin",
        getProjectDirectory() / "Source" / "Samples" / "Desktop" / "D3D12Bundles" / "data" / "occcity.bin",
        getProjectDirectory() / "Source" / "Samples" / "Desktop" / "D3D12Bundles" / "occcity.bin",
        getProjectDirectory() / ".." / "DirectX-Graphics-Samples" / "Samples" / "Desktop" / "D3D12Bundles" / "src" / "occcity.bin",
    };
    for (const auto& p : searchPaths)
    {
        auto canonical = std::filesystem::weakly_canonical(p);
        if (std::filesystem::exists(canonical))
            return canonical;
    }
    return {};
}

void D3D12Bundles::loadOcctityAssets(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file)
        FALCOR_THROW("Failed to open occcity.bin: {}", path.string());
    size_t size = file.tellg();
    file.seekg(0);
    if (size < kIndexDataOffset + kIndexDataSize)
        FALCOR_THROW("occcity.bin too small: {} bytes", size);

    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);

    // Create vertex buffer
    mpVertexBuffer = getDevice()->createBuffer(
        kVertexDataSize, ResourceBindFlags::Vertex, MemoryType::Upload, data.data() + kVertexDataOffset);

    // Create index buffer
    mpIndexBuffer = getDevice()->createBuffer(
        kIndexDataSize, ResourceBindFlags::Index, MemoryType::Upload, data.data() + kIndexDataOffset);

    // Create texture (BC1 at start of file)
    mpTexture = getDevice()->createTexture2D(
        kTextureWidth, kTextureHeight, ResourceFormat::BC1Unorm, 1, 1,
        data.data() + kTextureOffset, ResourceBindFlags::ShaderResource);

    // Vertex layout: POSITION(12) NORMAL(12) TEXCOORD(8) TANGENT(12) = 44 bytes
    auto pBufLayout = VertexBufferLayout::create();
    pBufLayout->addElement("POSITION", 0, ResourceFormat::RGB32Float, 1, 0);
    pBufLayout->addElement("NORMAL", 12, ResourceFormat::RGB32Float, 1, 1);
    pBufLayout->addElement("TEXCOORD", 24, ResourceFormat::RG32Float, 1, 2);
    pBufLayout->addElement("TANGENT", 32, ResourceFormat::RGB32Float, 1, 3);
    auto pLayout = VertexLayout::create();
    pLayout->addBufferLayout(0, pBufLayout);

    Vao::BufferVec buffers{mpVertexBuffer};
    mpVao = Vao::create(Vao::Topology::TriangleList, pLayout, buffers, mpIndexBuffer, ResourceFormat::R32Uint);

    // City model matrices - match FrameResource::SetCityPositions(8.0f, -8.0f)
    mModelMatrices.resize(kCityCount);
    for (uint32_t i = 0; i < kCityRowCount; i++)
    {
        float cityOffsetZ = i * -8.0f;
        for (uint32_t j = 0; j < kCityColumnCount; j++)
        {
            float cityOffsetX = j * 8.0f;
            float y = 0.02f * (i * kCityColumnCount + j);
            mModelMatrices[i * kCityColumnCount + j] = math::matrixFromTranslation(float3(cityOffsetX, y, cityOffsetZ));
        }
    }

    // Sampler: linear, wrap - match D3D12 sample
    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(TextureFilteringMode::Linear, TextureFilteringMode::Linear, TextureFilteringMode::Linear)
        .setAddressingMode(TextureAddressingMode::Wrap, TextureAddressingMode::Wrap, TextureAddressingMode::Wrap);
    mpSampler = getDevice()->createSampler(samplerDesc);

    // Programs - two PSOs (simple and alt pixel shader)
    ProgramDesc descSimple;
    descSimple.addShaderLibrary("Samples/Desktop/D3D12Bundles/MeshSimple.slang").vsEntry("VSMain").psEntry("PSMain");
    mpProgramSimple = Program::create(getDevice(), descSimple);

    ProgramDesc descAlt;
    descAlt.addShaderLibrary("Samples/Desktop/D3D12Bundles/MeshAlt.slang").vsEntry("VSMain").psEntry("PSMain");
    mpProgramAlt = Program::create(getDevice(), descAlt);

    mpVarsSimple = ProgramVars::create(getDevice(), mpProgramSimple.get());
    mpVarsAlt = ProgramVars::create(getDevice(), mpProgramAlt.get());

    mpDepthStencilState = DepthStencilState::create(DepthStencilState::Desc().setDepthEnabled(true));
    mpRasterizerState = RasterizerState::create(RasterizerState::Desc().setCullMode(RasterizerState::CullMode::None));

    mpStateSimple = GraphicsState::create(getDevice());
    mpStateSimple->setVao(mpVao);
    mpStateSimple->setProgram(mpProgramSimple);
    mpStateSimple->setDepthStencilState(mpDepthStencilState);
    mpStateSimple->setRasterizerState(mpRasterizerState);

    mpStateAlt = GraphicsState::create(getDevice());
    mpStateAlt->setVao(mpVao);
    mpStateAlt->setProgram(mpProgramAlt);
    mpStateAlt->setDepthStencilState(mpDepthStencilState);
    mpStateAlt->setRasterizerState(mpRasterizerState);
}

void D3D12Bundles::onLoad(RenderContext* pRenderContext)
{
    auto path = findOcctityBin();
    if (path.empty())
        FALCOR_THROW(
            "occcity.bin not found. Copy from DirectX-Graphics-Samples/Samples/Desktop/D3D12Bundles/src/occcity.bin "
            "to the executable directory or Source/Samples/Desktop/D3D12Bundles/data/");

    loadOcctityAssets(path);

    // Camera - match SimpleCamera::Init({8, 8, 30})
    mpCamera = Camera::create("MainCamera");
    mpCamera->setPosition(float3(8.f, 8.f, 30.f));
    mpCamera->setTarget(float3(0.f, 0.f, 0.f));
    mpCamera->setUpVector(float3(0.f, 1.f, 0.f));

    mpCameraController = std::make_unique<OrbiterCameraController>(mpCamera);
    mpCameraController->setModelParams(float3(0.f, 0.f, 0.f), 20.f, 2.f);
    mpCameraController->setCameraSpeed(20.f);
}

void D3D12Bundles::onShutdown() {}

void D3D12Bundles::onResize(uint32_t width, uint32_t height)
{
    if (mpCamera)
        mpCamera->setAspectRatio((float)width / (float)height);
}

void D3D12Bundles::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    // Update camera
    if (mpCameraController && mpCameraController->update())
        mpCamera->beginFrame(false);

    // Match D3D12 Bundles clear color (0.0f, 0.2f, 0.4f, 1.0f)
    const float4 clearColor(0.0f, 0.2f, 0.4f, 1.0f);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);

    float4x4 view = mpCamera->getViewMatrix();
    float4x4 proj = mpCamera->getProjMatrix();

    // Draw 30 cities - alternate PSO (match D3D12 UseBundles draw loop)
    bool usePso1 = true;
    for (uint32_t i = 0; i < kCityCount; i++)
    {
        float4x4 mvp = math::transpose(math::mul(mModelMatrices[i], math::mul(view, proj)));

        ref<ProgramVars> pVars = usePso1 ? mpVarsSimple : mpVarsAlt;
        auto var = pVars->getRootVar();
        var["g_txDiffuse"] = mpTexture;
        var["g_sampler"] = mpSampler;
        var["cb0"]["g_mWorldViewProj"] = mvp;

        ref<GraphicsState> pState = usePso1 ? mpStateSimple : mpStateAlt;
        pState->setFbo(pTargetFbo, true);
        pRenderContext->drawIndexed(pState.get(), pVars.get(), kNumIndices, 0, 0);
        usePso1 = !usePso1;
    }
}

void D3D12Bundles::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "D3D12 Bundles", {250, 200});
    renderGlobalUI(pGui);
    w.text("D3D12 Bundles - migrated to Falcor");
    w.text("30 city instances, alternating PSO (bundle semantic)");
}

bool D3D12Bundles::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return mpCameraController && mpCameraController->onKeyEvent(keyEvent);
}

bool D3D12Bundles::onMouseEvent(const MouseEvent& mouseEvent)
{
    return mpCameraController && mpCameraController->onMouseEvent(mouseEvent);
}

void D3D12Bundles::onHotReload(HotReloadFlags reloaded) {}

int runMain(int argc, char** argv)
{
    SampleAppConfig config;
    config.windowDesc.title = "D3D12 Bundles";
    config.windowDesc.resizableWindow = true;

    D3D12Bundles project(config);
    return project.run();
}

int main(int argc, char** argv)
{
    return catchAndReportAllExceptions([&]() { return runMain(argc, argv); });
}
