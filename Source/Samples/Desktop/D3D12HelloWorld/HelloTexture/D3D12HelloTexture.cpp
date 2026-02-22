#include "D3D12HelloTexture.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

static const float4 kClearColor(0.0f, 0.2f, 0.4f, 1.0f);
static const uint32_t kTextureSize = 256;

// 8x8 checkerboard: black/white cells, 256x256 RGBA8
static std::vector<uint8_t> generateCheckerboard()
{
    const uint32_t pixelSize = 4;
    const uint32_t cellSize = 32; // 256/8
    std::vector<uint8_t> data(kTextureSize * kTextureSize * pixelSize);
    for (uint32_t y = 0; y < kTextureSize; ++y)
    {
        for (uint32_t x = 0; x < kTextureSize; ++x)
        {
            bool black = ((x / cellSize) + (y / cellSize)) % 2 == 0;
            uint32_t i = (y * kTextureSize + x) * pixelSize;
            data[i + 0] = data[i + 1] = data[i + 2] = black ? 0x00 : 0xff;
            data[i + 3] = 0xff;
        }
    }
    return data;
}

D3D12HelloTexture::D3D12HelloTexture(SampleApp* pHost) : SampleBase(pHost) {}

D3D12HelloTexture::~D3D12HelloTexture() {}

void D3D12HelloTexture::onLoad(RenderContext* pRenderContext)
{
    uint2 size = getWindow() ? getWindow()->getClientAreaSize() : uint2(1280, 720);
    float aspectRatio = size.y > 0 ? (float)size.x / size.y : (1280.f / 720.f);

    const Vertex vertices[] = {
        {{0.0f, 0.25f * aspectRatio, 0.0f}, {0.5f, 0.0f}},
        {{0.25f, -0.25f * aspectRatio, 0.0f}, {1.0f, 1.0f}},
        {{-0.25f, -0.25f * aspectRatio, 0.0f}, {0.0f, 1.0f}},
    };
    mpVertexBuffer = getDevice()->createBuffer(sizeof(vertices), ResourceBindFlags::Vertex, MemoryType::Upload, (void*)vertices);

    auto pBufLayout = VertexBufferLayout::create();
    pBufLayout->addElement("POSITION", offsetof(Vertex, position), ResourceFormat::RGB32Float, 1, 0);
    pBufLayout->addElement("TEXCOORD", offsetof(Vertex, uv), ResourceFormat::RG32Float, 1, 1);
    auto pLayout = VertexLayout::create();
    pLayout->addBufferLayout(0, pBufLayout);
    mpVao = Vao::create(Vao::Topology::TriangleList, pLayout, {mpVertexBuffer});

    auto textureData = generateCheckerboard();
    mpTexture = getDevice()->createTexture2D(
        kTextureSize, kTextureSize, ResourceFormat::RGBA8Unorm, 1, 1, textureData.data(),
        ResourceBindFlags::ShaderResource);

    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(TextureFilteringMode::Point, TextureFilteringMode::Point, TextureFilteringMode::Point)
        .setAddressingMode(TextureAddressingMode::Border, TextureAddressingMode::Border, TextureAddressingMode::Border);
    mpSampler = getDevice()->createSampler(samplerDesc);

    ProgramDesc desc;
    desc.addShaderLibrary("Samples/Desktop/D3D12HelloWorld/HelloTexture/HelloTexture.slang").vsEntry("VSMain").psEntry("PSMain");
    mpProgram = Program::create(getDevice(), desc);
    mpVars = ProgramVars::create(getDevice(), mpProgram.get());
    mpVars->getRootVar()["g_texture"] = mpTexture;
    mpVars->getRootVar()["g_sampler"] = mpSampler;

    mpState = GraphicsState::create(getDevice());
    mpState->setVao(mpVao);
    mpState->setProgram(mpProgram);
    mpState->setDepthStencilState(DepthStencilState::create(DepthStencilState::Desc().setDepthEnabled(false)));
    mpState->setRasterizerState(RasterizerState::create(RasterizerState::Desc().setCullMode(RasterizerState::CullMode::None)));
}

void D3D12HelloTexture::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    pRenderContext->clearFbo(pTargetFbo.get(), kClearColor, 1.0f, 0, FboAttachmentType::All);
    mpState->setFbo(pTargetFbo, true);
    pRenderContext->draw(mpState.get(), mpVars.get(), 3, 0);
}

void D3D12HelloTexture::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Hello Texture", {250, 200});
    renderGlobalUI(pGui);
    w.text("Triangle with checkerboard texture.");
}

SampleBase* D3D12HelloTexture::create(SampleApp* pHost)
{
    return new D3D12HelloTexture(pHost);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<SampleBase, D3D12HelloTexture>();
}
