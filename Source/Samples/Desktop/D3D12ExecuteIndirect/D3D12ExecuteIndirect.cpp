#include "D3D12ExecuteIndirect.h"
#include "Utils/Math/Matrix.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

uint32_t mSampleGuiWidth = 250;
uint32_t mSampleGuiHeight = 200;
uint32_t mSampleGuiPositionX = 20;
uint32_t mSampleGuiPositionY = 40;

D3D12ExecuteIndirect::D3D12ExecuteIndirect(const SampleAppConfig& config) : SampleApp(config) {}

D3D12ExecuteIndirect::~D3D12ExecuteIndirect() {}

float D3D12ExecuteIndirect::getRandomFloat(float min, float max)
{
    float scale = (float)rand() / RAND_MAX;
    return scale * (max - min) + min;
}

void D3D12ExecuteIndirect::onLoad(RenderContext* pRenderContext)
{
    // Match D3D12 ExecuteIndirect: aspect ratio for projection
    uint2 size = getWindow() ? getWindow()->getClientAreaSize() : uint2(1280, 720);
    const float aspectRatio = size.y > 0 ? (float)size.x / size.y : (1280.f / 720.f);

    // Vertex buffer - single triangle geometry (1:1 from original)
    const Vertex triangleVertices[] = {
        {{0.0f, kTriangleHalfWidth, kTriangleDepth}},
        {{kTriangleHalfWidth, -kTriangleHalfWidth, kTriangleDepth}},
        {{-kTriangleHalfWidth, -kTriangleHalfWidth, kTriangleDepth}},
    };
    mpVertexBuffer = getDevice()->createBuffer(
        sizeof(triangleVertices), ResourceBindFlags::Vertex, MemoryType::Upload, (void*)triangleVertices);

    auto pBufLayout = VertexBufferLayout::create();
    pBufLayout->addElement("POSITION", offsetof(Vertex, position), ResourceFormat::RGB32Float, 1, 0);
    auto pLayout = VertexLayout::create();
    pLayout->addBufferLayout(0, pBufLayout);
    Vao::BufferVec buffers{mpVertexBuffer};
    mpVao = Vao::create(Vao::Topology::TriangleList, pLayout, buffers);

    // Constant buffer: TriangleCount * FrameCount for triple buffering (match original)
    const size_t cbSize = kTriangleCount * kFrameCount * sizeof(SceneConstantBuffer);
    mpConstantBuffer = getDevice()->createStructuredBuffer(
        sizeof(SceneConstantBuffer), kTriangleCount * kFrameCount,
        ResourceBindFlags::ShaderResource, MemoryType::Upload, nullptr, false);

    mConstantBufferData.resize(kTriangleCount);
    for (uint32_t n = 0; n < kTriangleCount; n++)
    {
        mConstantBufferData[n].velocity = float4(getRandomFloat(0.01f, 0.02f), 0.0f, 0.0f, 0.0f);
        mConstantBufferData[n].offset =
            float4(getRandomFloat(-5.0f, -1.5f), getRandomFloat(-1.0f, 1.0f), getRandomFloat(0.0f, 2.0f), 0.0f);
        mConstantBufferData[n].color =
            float4(getRandomFloat(0.5f, 1.0f), getRandomFloat(0.5f, 1.0f), getRandomFloat(0.5f, 1.0f), 1.0f);
        float4x4 proj = math::matrixFromPerspectiveFov(math::radians(45.f), aspectRatio, 0.01f, 20.0f);
        mConstantBufferData[n].projection = math::transpose(proj);
    }
    mpConstantBuffer->setBlob(mConstantBufferData.data(), 0, kTriangleCount * sizeof(SceneConstantBuffer));

    // Command buffer: 1024 draw commands (3, 1, 0, n) for non-culling path
    std::vector<DrawIndirectCommand> commands(kTriangleCount);
    for (uint32_t n = 0; n < kTriangleCount; n++)
    {
        commands[n].vertexCount = 3;
        commands[n].instanceCount = 1;
        commands[n].firstVertex = 0;
        commands[n].firstInstance = n;
    }
    mpCommandBuffer = getDevice()->createStructuredBuffer(
        sizeof(DrawIndirectCommand), kTriangleCount,
        ResourceBindFlags::ShaderResource | ResourceBindFlags::IndirectArg,
        MemoryType::DeviceLocal, commands.data(), false);

    // Processed command buffer: AppendStructuredBuffer output (with UAV counter for drawIndirect count)
    mpProcessedCommandBuffer = getDevice()->createStructuredBuffer(
        sizeof(DrawIndirectCommand), kTriangleCount,
        ResourceBindFlags::UnorderedAccess | ResourceBindFlags::IndirectArg,
        MemoryType::DeviceLocal, nullptr, true);

    // Graphics program
    ProgramDesc graphicsDesc;
    graphicsDesc.addShaderLibrary("Samples/Desktop/D3D12ExecuteIndirect/Shaders.slang")
        .vsEntry("VSMain")
        .psEntry("PSMain");
    mpGraphicsProgram = Program::create(getDevice(), graphicsDesc);
    mpGraphicsVars = ProgramVars::create(getDevice(), mpGraphicsProgram.get());

    mpGraphicsState = GraphicsState::create(getDevice());
    mpGraphicsState->setVao(mpVao);
    mpGraphicsState->setProgram(mpGraphicsProgram);
    mpDepthStencilState = DepthStencilState::create(DepthStencilState::Desc().setDepthEnabled(true).setDepthFunc(ComparisonFunc::Less));
    mpGraphicsState->setDepthStencilState(mpDepthStencilState);

    // Compute pass for culling
    ProgramDesc computeDesc;
    computeDesc.addShaderLibrary("Samples/Desktop/D3D12ExecuteIndirect/Compute.slang").csEntry("CSMain");
    mpCullPass = ComputePass::create(getDevice(), computeDesc);
    mpCullVars = ProgramVars::create(getDevice(), mpCullPass->getProgram().get());
}

void D3D12ExecuteIndirect::onShutdown() {}

void D3D12ExecuteIndirect::onResize(uint32_t width, uint32_t height)
{
    // Update projection matrices when aspect ratio changes
    uint2 size = getWindow() ? getWindow()->getClientAreaSize() : uint2(width, height);
    const float aspectRatio = size.y > 0 ? (float)size.x / size.y : (1280.f / 720.f);
    for (uint32_t n = 0; n < kTriangleCount; n++)
    {
        mConstantBufferData[n].projection =
            math::transpose(math::matrixFromPerspectiveFov(math::radians(45.f), aspectRatio, 0.01f, 20.0f));
    }
}

void D3D12ExecuteIndirect::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    // Update constant buffer data (match OnUpdate)
    const float offsetBounds = 2.5f;
    for (uint32_t n = 0; n < kTriangleCount; n++)
    {
        mConstantBufferData[n].offset.x += mConstantBufferData[n].velocity.x;
        if (mConstantBufferData[n].offset.x > offsetBounds)
        {
            mConstantBufferData[n].velocity.x = getRandomFloat(0.01f, 0.02f);
            mConstantBufferData[n].offset.x = -offsetBounds;
        }
    }
    uint32_t frameIdx = mFrameIndex % kFrameCount;
    mpConstantBuffer->setBlob(
        mConstantBufferData.data(), frameIdx * kTriangleCount * sizeof(SceneConstantBuffer),
        kTriangleCount * sizeof(SceneConstantBuffer));

    // Match D3D12 ExecuteIndirect clear color (0.0f, 0.2f, 0.4f, 1.0f)
    const float4 clearColor(0.0f, 0.2f, 0.4f, 1.0f);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);

    // Bind constant buffer (frame slice) for graphics
    mpGraphicsVars->getRootVar()["gSceneCB"] =
        mpConstantBuffer->getSRV(frameIdx * kTriangleCount * sizeof(SceneConstantBuffer),
                                 kTriangleCount * sizeof(SceneConstantBuffer));

    mpGraphicsState->setFbo(pTargetFbo, true);

    if (mEnableCulling)
    {
        // 1. Reset UAV counter to 0
        pRenderContext->clearUAVCounter(mpProcessedCommandBuffer, 0);

        // 2. Bind resources for compute
        mpCullVars->getRootVar()["RootConstants"]["xOffset"] = kTriangleHalfWidth;
        mpCullVars->getRootVar()["RootConstants"]["zOffset"] = kTriangleDepth;
        mpCullVars->getRootVar()["RootConstants"]["cullOffset"] = kCullingCutoff;
        mpCullVars->getRootVar()["RootConstants"]["commandCount"] = (float)kTriangleCount;
        mpCullVars->getRootVar()["cbv"] =
            mpConstantBuffer->getSRV(frameIdx * kTriangleCount * sizeof(SceneConstantBuffer),
                                     kTriangleCount * sizeof(SceneConstantBuffer));
        mpCullVars->getRootVar()["outputCommands"] = mpProcessedCommandBuffer;

        // 3. Dispatch compute
        uint32_t groupCount = (kTriangleCount + kComputeThreadBlockSize - 1) / kComputeThreadBlockSize;
        mpCullPass->execute(pRenderContext, groupCount, 1, 1);

        // 4. DrawIndirect with count from UAV counter
        pRenderContext->drawIndirect(
            mpGraphicsState.get(), mpGraphicsVars.get(), kTriangleCount,
            mpProcessedCommandBuffer.get(), 0,
            mpProcessedCommandBuffer->getUAVCounter().get(), 0);
    }
    else
    {
        // Draw all triangles without culling
        pRenderContext->drawIndirect(
            mpGraphicsState.get(), mpGraphicsVars.get(), kTriangleCount,
            mpCommandBuffer.get(), 0, nullptr, 0);
    }

    mFrameIndex++;
}

void D3D12ExecuteIndirect::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "D3D12 Execute Indirect", {250, 200});
    renderGlobalUI(pGui);
    w.text("SPACE: toggle compute culling");
    w.text(mEnableCulling ? "Culling: ON" : "Culling: OFF");
}

bool D3D12ExecuteIndirect::onKeyEvent(const KeyboardEvent& keyEvent)
{
    if (keyEvent.type == KeyboardEvent::Type::KeyPressed && keyEvent.key == Input::Key::Space)
    {
        mEnableCulling = !mEnableCulling;
        return true;
    }
    return false;
}

bool D3D12ExecuteIndirect::onMouseEvent(const MouseEvent& mouseEvent)
{
    return false;
}

void D3D12ExecuteIndirect::onHotReload(HotReloadFlags reloaded)
{
    //
}

int runMain(int argc, char** argv)
{
    SampleAppConfig config;
    config.windowDesc.title = "D3D12 Execute Indirect";
    config.windowDesc.resizableWindow = true;

    D3D12ExecuteIndirect project(config);
    return project.run();
}

int main(int argc, char** argv)
{
    return catchAndReportAllExceptions([&]() { return runMain(argc, argv); });
}
