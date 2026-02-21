#include "D3D12HelloConstantBuffers.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

static const float4 kClearColor(0.0f, 0.2f, 0.4f, 1.0f);
static const float kTranslationSpeed = 0.005f;
static const float kOffsetBounds = 1.25f;

D3D12HelloConstantBuffers::D3D12HelloConstantBuffers(SampleApp* pHost) : SampleBase(pHost) {}

D3D12HelloConstantBuffers::~D3D12HelloConstantBuffers() {}

void D3D12HelloConstantBuffers::onLoad(RenderContext* pRenderContext)
{
    uint2 size = getWindow() ? getWindow()->getClientAreaSize() : uint2(1280, 720);
    float aspectRatio = size.y > 0 ? (float)size.x / size.y : (1280.f / 720.f);

    const Vertex vertices[] = {
        {{0.0f, 0.25f * aspectRatio, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{0.25f, -0.25f * aspectRatio, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{-0.25f, -0.25f * aspectRatio, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
    };
    mpVertexBuffer = getDevice()->createBuffer(sizeof(vertices), ResourceBindFlags::Vertex, MemoryType::Upload, (void*)vertices);

    auto pBufLayout = VertexBufferLayout::create();
    pBufLayout->addElement("POSITION", offsetof(Vertex, position), ResourceFormat::RGB32Float, 1, 0);
    pBufLayout->addElement("COLOR", offsetof(Vertex, color), ResourceFormat::RGBA32Float, 1, 1);
    auto pLayout = VertexLayout::create();
    pLayout->addBufferLayout(0, pBufLayout);
    mpVao = Vao::create(Vao::Topology::TriangleList, pLayout, {mpVertexBuffer});

    ProgramDesc desc;
    desc.addShaderLibrary("Samples/Desktop/D3D12HelloWorld/HelloConstantBuffers/HelloConstantBuffers.slang")
        .vsEntry("VSMain")
        .psEntry("PSMain");
    mpProgram = Program::create(getDevice(), desc);
    mpVars = ProgramVars::create(getDevice(), mpProgram.get());

    mpState = GraphicsState::create(getDevice());
    mpState->setVao(mpVao);
    mpState->setProgram(mpProgram);
    mpState->setDepthStencilState(DepthStencilState::create(DepthStencilState::Desc().setDepthEnabled(false)));
    mpState->setRasterizerState(RasterizerState::create(RasterizerState::Desc().setCullMode(RasterizerState::CullMode::None)));
}

void D3D12HelloConstantBuffers::onShutdown() {}

void D3D12HelloConstantBuffers::onResize(uint32_t width, uint32_t height) {}

void D3D12HelloConstantBuffers::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    mConstantBufferData.offset.x += kTranslationSpeed;
    if (mConstantBufferData.offset.x > kOffsetBounds)
        mConstantBufferData.offset.x = -kOffsetBounds;
    mpVars->getRootVar()["SceneConstantBuffer"].setBlob(&mConstantBufferData, sizeof(mConstantBufferData));

    pRenderContext->clearFbo(pTargetFbo.get(), kClearColor, 1.0f, 0, FboAttachmentType::All);
    mpState->setFbo(pTargetFbo, true);
    pRenderContext->draw(mpState.get(), mpVars.get(), 3, 0);
}

void D3D12HelloConstantBuffers::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Hello Constant Buffers", {250, 200});
    renderGlobalUI(pGui);
    w.text("Triangle with animated constant buffer offset.");
}

bool D3D12HelloConstantBuffers::onKeyEvent(const KeyboardEvent& keyEvent) { return false; }

bool D3D12HelloConstantBuffers::onMouseEvent(const MouseEvent& mouseEvent) { return false; }

void D3D12HelloConstantBuffers::onHotReload(HotReloadFlags reloaded) {}

SampleBase* D3D12HelloConstantBuffers::create(SampleApp* pHost)
{
    return new D3D12HelloConstantBuffers(pHost);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<SampleBase, D3D12HelloConstantBuffers>();
}
