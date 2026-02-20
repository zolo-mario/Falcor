#include "D3D12HelloTexture.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

// Match D3D12 Hello Texture constants
static constexpr uint32_t kTextureWidth = 256;
static constexpr uint32_t kTextureHeight = 256;
static constexpr uint32_t kTexturePixelSize = 4;

D3D12HelloTexture::D3D12HelloTexture(SampleApp* pHost) : SampleBase(pHost) {}

D3D12HelloTexture::~D3D12HelloTexture() {}

static std::vector<uint8_t> generateTextureData()
{
    const uint32_t rowPitch = kTextureWidth * kTexturePixelSize;
    const uint32_t cellPitch = rowPitch >> 3;
    const uint32_t cellHeight = kTextureWidth >> 3;
    const uint32_t textureSize = rowPitch * kTextureHeight;

    std::vector<uint8_t> data(textureSize);
    uint8_t* pData = data.data();

    for (uint32_t n = 0; n < textureSize; n += kTexturePixelSize)
    {
        uint32_t x = n % rowPitch;
        uint32_t y = n / rowPitch;
        uint32_t i = x / cellPitch;
        uint32_t j = y / cellHeight;

        if (i % 2 == j % 2)
        {
            pData[n] = 0x00;
            pData[n + 1] = 0x00;
            pData[n + 2] = 0x00;
            pData[n + 3] = 0xff;
        }
        else
        {
            pData[n] = 0xff;
            pData[n + 1] = 0xff;
            pData[n + 2] = 0xff;
            pData[n + 3] = 0xff;
        }
    }
    return data;
}

void D3D12HelloTexture::onLoad(RenderContext* pRenderContext)
{
    // Match D3D12 Hello Texture: aspect ratio for vertex positions (default 1280/720)
    uint2 size = getWindow() ? getWindow()->getClientAreaSize() : uint2(1280, 720);
    const float aspectRatio = size.y > 0 ? (float)size.x / size.y : (1280.f / 720.f);

    // Match D3D12 Hello Texture vertex data (1:1) - POSITION + TEXCOORD
    const Vertex triangleVertices[] = {
        {{0.0f, 0.25f * aspectRatio, 0.0f}, {0.5f, 0.0f}},
        {{0.25f, -0.25f * aspectRatio, 0.0f}, {1.0f, 1.0f}},
        {{-0.25f, -0.25f * aspectRatio, 0.0f}, {0.0f, 1.0f}},
    };

    const uint32_t vertexBufferSize = sizeof(triangleVertices);
    mpVertexBuffer = getDevice()->createBuffer(
        vertexBufferSize, ResourceBindFlags::Vertex, MemoryType::Upload, (void*)triangleVertices);

    // Vertex layout: POSITION (float3) + TEXCOORD (float2) - match D3D12 input element desc
    auto pBufLayout = VertexBufferLayout::create();
    pBufLayout->addElement("POSITION", offsetof(Vertex, position), ResourceFormat::RGB32Float, 1, 0);
    pBufLayout->addElement("TEXCOORD", offsetof(Vertex, uv), ResourceFormat::RG32Float, 1, 1);
    auto pLayout = VertexLayout::create();
    pLayout->addBufferLayout(0, pBufLayout);

    Vao::BufferVec buffers{mpVertexBuffer};
    mpVao = Vao::create(Vao::Topology::TriangleList, pLayout, buffers);

    // Texture: 256x256 checkerboard - match D3D12 CreateCommittedResource + GenerateTextureData
    auto textureData = generateTextureData();
    mpTexture = getDevice()->createTexture2D(
        kTextureWidth, kTextureHeight, ResourceFormat::RGBA8Unorm, 1, 1, textureData.data(),
        ResourceBindFlags::ShaderResource);

    // Sampler: Point filter, Border addressing - match D3D12_STATIC_SAMPLER_DESC
    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(TextureFilteringMode::Point, TextureFilteringMode::Point, TextureFilteringMode::Point)
        .setAddressingMode(TextureAddressingMode::Border, TextureAddressingMode::Border, TextureAddressingMode::Border);
    mpSampler = getDevice()->createSampler(samplerDesc);

    // Program: VS + PS - match D3D12 PSO
    ProgramDesc desc;
    desc.addShaderLibrary("Samples/Desktop/D3D12HelloTexture/HelloTexture.slang")
        .vsEntry("VSMain")
        .psEntry("PSMain");
    mpProgram = Program::create(getDevice(), desc);
    mpVars = ProgramVars::create(getDevice(), mpProgram.get());

    // Bind texture and sampler - match SetGraphicsRootDescriptorTable + SetDescriptorHeaps
    auto var = mpVars->getRootVar();
    var["g_texture"] = mpTexture;
    var["g_sampler"] = mpSampler;

    mpState = GraphicsState::create(getDevice());
    mpState->setVao(mpVao);
    mpState->setProgram(mpProgram);
    mpDepthStencilState = DepthStencilState::create(DepthStencilState::Desc().setDepthEnabled(false));
    mpState->setDepthStencilState(mpDepthStencilState);
    // Disable culling - vertices are CW from +Z, default would cull back face
    mpState->setRasterizerState(RasterizerState::create(RasterizerState::Desc().setCullMode(RasterizerState::CullMode::None)));
}

void D3D12HelloTexture::onShutdown() {}

void D3D12HelloTexture::onResize(uint32_t width, uint32_t height) {}

void D3D12HelloTexture::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    // Match D3D12 Hello Texture clear color (0.0f, 0.2f, 0.4f, 1.0f)
    const float4 clearColor(0.0f, 0.2f, 0.4f, 1.0f);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);

    // Draw triangle with texture - match DrawInstanced(3, 1, 0, 0)
    mpState->setFbo(pTargetFbo, true);
    pRenderContext->draw(mpState.get(), mpVars.get(), 3, 0);
}

void D3D12HelloTexture::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "D3D12 Hello Texture", {250, 200});
    renderGlobalUI(pGui);
    w.text("D3D12 Hello Texture - migrated to Falcor");
}

bool D3D12HelloTexture::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return false;
}

bool D3D12HelloTexture::onMouseEvent(const MouseEvent& mouseEvent)
{
    return false;
}

void D3D12HelloTexture::onHotReload(HotReloadFlags reloaded)
{
    //
}

SampleBase* D3D12HelloTexture::create(SampleApp* pHost)
{
    return new D3D12HelloTexture(pHost);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<SampleBase, D3D12HelloTexture>();
}
