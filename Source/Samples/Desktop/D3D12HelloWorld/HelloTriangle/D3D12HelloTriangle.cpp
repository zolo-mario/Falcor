#include "D3D12HelloTriangle.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

D3D12HelloTriangle::D3D12HelloTriangle(SampleApp* pHost) : SampleBase(pHost) {}

D3D12HelloTriangle::~D3D12HelloTriangle() {}

void D3D12HelloTriangle::onLoad(RenderContext* pRenderContext)
{
    // Match D3D12 Hello Triangle: aspect ratio for vertex positions (default 1280/720)
    uint2 size = getWindow() ? getWindow()->getClientAreaSize() : uint2(1280, 720);
    const float aspectRatio = size.y > 0 ? (float)size.x / size.y : (1280.f / 720.f);

    // Match D3D12 Hello Triangle vertex data (1:1)
    // Top: red, Right: green, Left: blue
    const Vertex triangleVertices[] = {
        {{0.0f, 0.25f * aspectRatio, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{0.25f, -0.25f * aspectRatio, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{-0.25f, -0.25f * aspectRatio, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
    };

    const uint32_t vertexBufferSize = sizeof(triangleVertices);
    mpVertexBuffer = getDevice()->createBuffer(vertexBufferSize, ResourceBindFlags::Vertex, MemoryType::Upload, (void*)triangleVertices);

    // Vertex layout: POSITION (float3) + COLOR (float4) - match D3D12 input element desc
    auto pBufLayout = VertexBufferLayout::create();
    pBufLayout->addElement("POSITION", offsetof(Vertex, position), ResourceFormat::RGB32Float, 1, 0);
    pBufLayout->addElement("COLOR", offsetof(Vertex, color), ResourceFormat::RGBA32Float, 1, 1);
    auto pLayout = VertexLayout::create();
    pLayout->addBufferLayout(0, pBufLayout);

    Vao::BufferVec buffers{mpVertexBuffer};
    mpVao = Vao::create(Vao::Topology::TriangleList, pLayout, buffers);

    // Program: VS + PS - match D3D12 PSO
    ProgramDesc desc;
    desc.addShaderLibrary("Samples/Desktop/D3D12HelloWorld/HelloTriangle/HelloTriangle.slang").vsEntry("VSMain").psEntry("PSMain");
    mpProgram = Program::create(getDevice(), desc);
    mpVars = ProgramVars::create(getDevice(), mpProgram.get());

    mpState = GraphicsState::create(getDevice());
    mpState->setVao(mpVao);
    mpState->setProgram(mpProgram);
    mpDepthStencilState = DepthStencilState::create(DepthStencilState::Desc().setDepthEnabled(false));
    mpState->setDepthStencilState(mpDepthStencilState);
    // Disable culling - D3D12 Hello Triangle vertices are CW from +Z, default would cull back face
    mpState->setRasterizerState(RasterizerState::create(RasterizerState::Desc().setCullMode(RasterizerState::CullMode::None)));
}

void D3D12HelloTriangle::onShutdown() {}

void D3D12HelloTriangle::onResize(uint32_t width, uint32_t height)
{
    // Original D3D12 sample doesn't update vertices on resize; viewport/scissor handled by Falcor
}

void D3D12HelloTriangle::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    // Match D3D12 Hello Triangle clear color (0.0f, 0.2f, 0.4f, 1.0f)
    const float4 clearColor(0.0f, 0.2f, 0.4f, 1.0f);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);

    // Draw triangle - match DrawInstanced(3, 1, 0, 0)
    mpState->setFbo(pTargetFbo, true);
    pRenderContext->draw(mpState.get(), mpVars.get(), 3, 0);
}

void D3D12HelloTriangle::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "D3D12 Hello Triangle", {250, 200});
    renderGlobalUI(pGui);
    w.text("D3D12 Hello Triangle - migrated to Falcor");
}

bool D3D12HelloTriangle::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return false;
}

bool D3D12HelloTriangle::onMouseEvent(const MouseEvent& mouseEvent)
{
    return false;
}

void D3D12HelloTriangle::onHotReload(HotReloadFlags reloaded)
{
    //
}

SampleBase* D3D12HelloTriangle::create(SampleApp* pHost)
{
    return new D3D12HelloTriangle(pHost);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<SampleBase, D3D12HelloTriangle>();
}
