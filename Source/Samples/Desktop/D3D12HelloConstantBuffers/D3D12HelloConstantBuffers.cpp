#include "D3D12HelloConstantBuffers.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

D3D12HelloConstantBuffers::D3D12HelloConstantBuffers(const SampleAppConfig& config) : SampleApp(config)
{
    mConstantBufferData = {};
}

D3D12HelloConstantBuffers::~D3D12HelloConstantBuffers() {}

void D3D12HelloConstantBuffers::onLoad(RenderContext* pRenderContext)
{
    // Match D3D12 Hello Constant Buffers: aspect ratio for vertex positions (default 1280/720)
    uint2 size = getWindow() ? getWindow()->getClientAreaSize() : uint2(1280, 720);
    const float aspectRatio = size.y > 0 ? (float)size.x / size.y : (1280.f / 720.f);

    // Match D3D12 Hello Const Buffers vertex data (1:1)
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

    // Program: VS + PS - match D3D12 PSO
    ProgramDesc desc;
    desc.addShaderLibrary("Samples/Desktop/D3D12HelloConstantBuffers/HelloConstantBuffers.slang")
        .vsEntry("VSMain")
        .psEntry("PSMain");
    mpProgram = Program::create(getDevice(), desc);
    mpVars = ProgramVars::create(getDevice(), mpProgram.get());

    mpState = GraphicsState::create(getDevice());
    mpState->setVao(mpVao);
    mpState->setProgram(mpProgram);
    mpDepthStencilState = DepthStencilState::create(DepthStencilState::Desc().setDepthEnabled(false));
    mpState->setDepthStencilState(mpDepthStencilState);
    // Disable culling - D3D12 Hello Triangle vertices are CW from +Z, default would cull back face
    mpRasterizerState = RasterizerState::create(RasterizerState::Desc().setCullMode(RasterizerState::CullMode::None));
    mpState->setRasterizerState(mpRasterizerState);
}

void D3D12HelloConstantBuffers::onShutdown() {}

void D3D12HelloConstantBuffers::onResize(uint32_t width, uint32_t height) {}

void D3D12HelloConstantBuffers::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    // Match D3D12 Hello Const Buffers OnUpdate: translationSpeed 0.005f, offsetBounds 1.25f
    const float translationSpeed = 0.005f;
    const float offsetBounds = 1.25f;

    mConstantBufferData.offset.x += translationSpeed;
    if (mConstantBufferData.offset.x > offsetBounds)
    {
        mConstantBufferData.offset.x = -offsetBounds;
    }
    mpVars->getRootVar()["SceneConstantBuffer"].setBlob(&mConstantBufferData, sizeof(mConstantBufferData));

    // Match D3D12 Hello Const Buffers clear color (0.0f, 0.2f, 0.4f, 1.0f)
    const float4 clearColor(0.0f, 0.2f, 0.4f, 1.0f);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);

    // Draw triangle - match DrawInstanced(3, 1, 0, 0)
    mpState->setFbo(pTargetFbo, true);
    pRenderContext->draw(mpState.get(), mpVars.get(), 3, 0);
}

void D3D12HelloConstantBuffers::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "D3D12 Hello Constant Buffers", {250, 200});
    renderGlobalUI(pGui);
    w.text("D3D12 Hello Constant Buffers - migrated to Falcor");
}

bool D3D12HelloConstantBuffers::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return false;
}

bool D3D12HelloConstantBuffers::onMouseEvent(const MouseEvent& mouseEvent)
{
    return false;
}

void D3D12HelloConstantBuffers::onHotReload(HotReloadFlags reloaded)
{
    //
}

int runMain(int argc, char** argv)
{
    SampleAppConfig config;
    config.windowDesc.title = "D3D12 Hello Constant Buffers";
    config.windowDesc.resizableWindow = true;

    D3D12HelloConstantBuffers project(config);
    return project.run();
}

int main(int argc, char** argv)
{
    return catchAndReportAllExceptions([&]() { return runMain(argc, argv); });
}
