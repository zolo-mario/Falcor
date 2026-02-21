#include "D3D12HelloTriangle.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

static const float4 kClearColor(0.0f, 0.2f, 0.4f, 1.0f);

D3D12HelloTriangle::D3D12HelloTriangle(SampleApp* pHost) : SampleBase(pHost) {}

D3D12HelloTriangle::~D3D12HelloTriangle() {}

void D3D12HelloTriangle::onLoad(RenderContext* pRenderContext)
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
    desc.addShaderLibrary("Samples/Desktop/D3D12HelloWorld/HelloTriangle/HelloTriangle.slang").vsEntry("VSMain").psEntry("PSMain");
    mpProgram = Program::create(getDevice(), desc);
    mpVars = ProgramVars::create(getDevice(), mpProgram.get());

    mpState = GraphicsState::create(getDevice());
    mpState->setVao(mpVao);
    mpState->setProgram(mpProgram);
    mpState->setDepthStencilState(DepthStencilState::create(DepthStencilState::Desc().setDepthEnabled(false)));
    mpState->setRasterizerState(RasterizerState::create(RasterizerState::Desc().setCullMode(RasterizerState::CullMode::None)));
}

void D3D12HelloTriangle::onShutdown() {}

void D3D12HelloTriangle::onResize(uint32_t width, uint32_t height) {}

void D3D12HelloTriangle::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    pRenderContext->clearFbo(pTargetFbo.get(), kClearColor, 1.0f, 0, FboAttachmentType::All);
    mpState->setFbo(pTargetFbo, true);
    pRenderContext->draw(mpState.get(), mpVars.get(), 3, 0);
}

void D3D12HelloTriangle::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Hello Triangle", {250, 200});
    renderGlobalUI(pGui);
    w.text("Single triangle, vertex colors.");
}

bool D3D12HelloTriangle::onKeyEvent(const KeyboardEvent& keyEvent) { return false; }

bool D3D12HelloTriangle::onMouseEvent(const MouseEvent& mouseEvent) { return false; }

void D3D12HelloTriangle::onHotReload(HotReloadFlags reloaded) {}

SampleBase* D3D12HelloTriangle::create(SampleApp* pHost)
{
    return new D3D12HelloTriangle(pHost);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<SampleBase, D3D12HelloTriangle>();
}
