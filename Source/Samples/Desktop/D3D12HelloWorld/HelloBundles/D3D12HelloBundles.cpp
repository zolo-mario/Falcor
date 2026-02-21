#include "D3D12HelloBundles.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

D3D12HelloBundles::D3D12HelloBundles(SampleApp* pHost) : SampleBase(pHost) {}

D3D12HelloBundles::~D3D12HelloBundles() {}

void D3D12HelloBundles::onLoad(RenderContext* pRenderContext)
{
    // Match D3D12 Hello Bundles: aspect ratio for vertex positions (default 1280/720)
    uint2 size = getWindow() ? getWindow()->getClientAreaSize() : uint2(1280, 720);
    const float aspectRatio = size.y > 0 ? (float)size.x / size.y : (1280.f / 720.f);

    // Match D3D12 Hello Bundles vertex data (1:1) - same as HelloTriangle
    // Top: red, Right: green, Left: blue
    const Vertex triangleVertices[] = {
        {{0.0f, 0.25f * aspectRatio, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{0.25f, -0.25f * aspectRatio, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{-0.25f, -0.25f * aspectRatio, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
    };

    const uint32_t vertexBufferSize = sizeof(triangleVertices);
    mpVertexBuffer = getDevice()->createBuffer(
        vertexBufferSize, ResourceBindFlags::Vertex, MemoryType::Upload, (void*)triangleVertices);

    // Vertex layout: POSITION (float3) + COLOR (float4) - match D3D12 input element desc
    auto pBufLayout = VertexBufferLayout::create();
    pBufLayout->addElement("POSITION", offsetof(Vertex, position), ResourceFormat::RGB32Float, 1, 0);
    pBufLayout->addElement("COLOR", offsetof(Vertex, color), ResourceFormat::RGBA32Float, 1, 1);
    auto pLayout = VertexLayout::create();
    pLayout->addBufferLayout(0, pBufLayout);

    Vao::BufferVec buffers{mpVertexBuffer};
    mpVao = Vao::create(Vao::Topology::TriangleList, pLayout, buffers);

    // Program: VS + PS - match D3D12 PSO (same as HelloTriangle)
    ProgramDesc desc;
    desc.addShaderLibrary("Samples/Desktop/D3D12HelloWorld/HelloBundles/HelloBundles.slang")
        .vsEntry("VSMain")
        .psEntry("PSMain");
    mpProgram = Program::create(getDevice(), desc);
    mpVars = ProgramVars::create(getDevice(), mpProgram.get());

    mpState = GraphicsState::create(getDevice());
    mpState->setVao(mpVao);
    mpState->setProgram(mpProgram);
    mpDepthStencilState = DepthStencilState::create(DepthStencilState::Desc().setDepthEnabled(false));
    mpState->setDepthStencilState(mpDepthStencilState);
    // Disable culling - vertices are CW from +Z, default would cull back face
    mpState->setRasterizerState(
        RasterizerState::create(RasterizerState::Desc().setCullMode(RasterizerState::CullMode::None)));
}

void D3D12HelloBundles::onShutdown() {}

void D3D12HelloBundles::onResize(uint32_t width, uint32_t height) {}

void D3D12HelloBundles::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    // Match D3D12 Hello Bundles clear color (0.0f, 0.2f, 0.4f, 1.0f)
    const float4 clearColor(0.0f, 0.2f, 0.4f, 1.0f);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);

    // Draw triangle - semantic equivalent to ExecuteBundle(m_bundle) where bundle contains
    // DrawInstanced(3, 1, 0, 0). Falcor does not expose D3D12 bundles; direct draw yields same output.
    mpState->setFbo(pTargetFbo, true);
    pRenderContext->draw(mpState.get(), mpVars.get(), 3, 0);
}

void D3D12HelloBundles::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "D3D12 Hello Bundles", {250, 200});
    renderGlobalUI(pGui);
    w.text("D3D12 Hello Bundles - migrated to Falcor");
}

bool D3D12HelloBundles::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return false;
}

bool D3D12HelloBundles::onMouseEvent(const MouseEvent& mouseEvent)
{
    return false;
}

void D3D12HelloBundles::onHotReload(HotReloadFlags reloaded)
{
    //
}

SampleBase* D3D12HelloBundles::create(SampleApp* pHost)
{
    return new D3D12HelloBundles(pHost);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<SampleBase, D3D12HelloBundles>();
}
