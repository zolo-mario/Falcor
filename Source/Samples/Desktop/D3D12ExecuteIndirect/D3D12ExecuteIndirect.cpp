#include "D3D12ExecuteIndirect.h"
#include "Utils/Math/Matrix.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

D3D12ExecuteIndirect::D3D12ExecuteIndirect(SampleApp* pHost) : SampleBase(pHost) {}

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

    // Constant buffer: per-frame for triple buffering (match original)
    mConstantBufferData.resize(kTriangleCount);
    for (uint32_t n = 0; n < kTriangleCount; n++)
    {
        mConstantBufferData[n].velocity = float4(getRandomFloat(0.01f, 0.02f), 0.0f, 0.0f, 0.0f);
        // offset.x: -1.5~1.5 使首帧即有三角形可见（原 -5~-1.5 全在视口左侧外）
        mConstantBufferData[n].offset =
            float4(getRandomFloat(-1.5f, 1.5f), getRandomFloat(-1.0f, 1.0f), getRandomFloat(0.0f, 2.0f), 0.0f);
        mConstantBufferData[n].color =
            float4(getRandomFloat(0.5f, 1.0f), getRandomFloat(0.5f, 1.0f), getRandomFloat(0.5f, 1.0f), 1.0f);
        mConstantBufferData[n].projection = math::perspective(math::radians(45.f), aspectRatio, 0.01f, 20.0f);
    }
    for (uint32_t f = 0; f < kFrameCount; f++)
    {
        mpConstantBuffers[f] = getDevice()->createStructuredBuffer(
            sizeof(SceneConstantBuffer), kTriangleCount,
            ResourceBindFlags::ShaderResource, MemoryType::Upload, nullptr, false);
        mpConstantBuffers[f]->setBlob(mConstantBufferData.data(), 0, kTriangleCount * sizeof(SceneConstantBuffer));
    }

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

    // Visible indices: AppendStructuredBuffer<uint> output (culling path)
    mpVisibleIndicesBuffer = getDevice()->createStructuredBuffer(
        sizeof(uint32_t), kTriangleCount,
        ResourceBindFlags::UnorderedAccess | ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal, nullptr, true);

    // Draw args buffer: (3, count, 0, 0) for single drawIndirect
    mpDrawArgsBuffer = getDevice()->createStructuredBuffer(
        sizeof(uint32_t), 4,
        ResourceBindFlags::UnorderedAccess | ResourceBindFlags::IndirectArg,
        MemoryType::DeviceLocal, nullptr, false);

    // Graphics program (non-culling)
    ProgramDesc graphicsDesc;
    graphicsDesc.addShaderLibrary("Samples/Desktop/D3D12ExecuteIndirect/Shaders.slang")
        .vsEntry("VSMain")
        .psEntry("PSMain");
    mpGraphicsProgram = Program::create(getDevice(), graphicsDesc);
    mpGraphicsVars = ProgramVars::create(getDevice(), mpGraphicsProgram.get());

    mpGraphicsState = GraphicsState::create(getDevice());
    mpGraphicsState->setVao(mpVao);
    mpGraphicsState->setProgram(mpGraphicsProgram);
    // 禁用深度：1024 三角形重叠可接受，避免 depth clear/format 导致无输出
    mpDepthStencilState = DepthStencilState::create(DepthStencilState::Desc().setDepthEnabled(false));
    mpGraphicsState->setDepthStencilState(mpDepthStencilState);
    // 禁用背面剔除：与 HelloTriangle 一致，三角形顶点为 CW，默认剔除会剔除全部
    mpGraphicsState->setRasterizerState(RasterizerState::create(RasterizerState::Desc().setCullMode(RasterizerState::CullMode::None)));

    // Compute pass for culling
    ProgramDesc computeDesc;
    computeDesc.addShaderLibrary("Samples/Desktop/D3D12ExecuteIndirect/Compute.slang").csEntry("CSMain");
    mpCullPass = ComputePass::create(getDevice(), computeDesc);
    mpCullVars = ProgramVars::create(getDevice(), mpCullPass->getProgram().get());
    mpCullPass->setVars(mpCullVars);

    // Build args compute: (3, count, 0, 0) from visible indices counter
    ProgramDesc buildArgsDesc;
    buildArgsDesc.addShaderLibrary("Samples/Desktop/D3D12ExecuteIndirect/BuildArgs.slang").csEntry("CSMain");
    mpBuildArgsPass = ComputePass::create(getDevice(), buildArgsDesc);
    mpBuildArgsVars = ProgramVars::create(getDevice(), mpBuildArgsPass->getProgram().get());
    mpBuildArgsPass->setVars(mpBuildArgsVars);

    // Graphics program for culling (uses gVisibleIndices[instanceId])
    ProgramDesc graphicsCullingDesc;
    graphicsCullingDesc.addShaderLibrary("Samples/Desktop/D3D12ExecuteIndirect/ShadersCulling.slang")
        .vsEntry("VSMain")
        .psEntry("PSMain");
    mpGraphicsProgramCulling = Program::create(getDevice(), graphicsCullingDesc);
    mpGraphicsVarsCulling = ProgramVars::create(getDevice(), mpGraphicsProgramCulling.get());
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
            math::perspective(math::radians(45.f), aspectRatio, 0.01f, 20.0f);
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
    mpConstantBuffers[frameIdx]->setBlob(
        mConstantBufferData.data(), 0, kTriangleCount * sizeof(SceneConstantBuffer));

    // Match D3D12 ExecuteIndirect clear color (0.0f, 0.2f, 0.4f, 1.0f)
    const float4 clearColor(0.0f, 0.2f, 0.4f, 1.0f);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);

    // Bind constant buffer for graphics
    mpGraphicsVars->getRootVar()["gSceneCB"] = mpConstantBuffers[frameIdx];

    mpGraphicsState->setFbo(pTargetFbo, true);

    if (mEnableCulling)
    {
        // 1. Reset UAV counter to 0
        pRenderContext->clearUAVCounter(mpVisibleIndicesBuffer, 0);

        // 2. Cull compute: append visible triangle indices
        mpCullVars->getRootVar()["RootConstants"]["xOffset"] = kTriangleHalfWidth;
        mpCullVars->getRootVar()["RootConstants"]["zOffset"] = kTriangleDepth;
        mpCullVars->getRootVar()["RootConstants"]["cullOffset"] = kCullingCutoff;
        mpCullVars->getRootVar()["RootConstants"]["commandCount"] = (float)kTriangleCount;
        mpCullVars->getRootVar()["cbv"] = mpConstantBuffers[frameIdx];
        mpCullVars->getRootVar()["outputVisibleIndices"] = mpVisibleIndicesBuffer;

        uint32_t groupCount = (kTriangleCount + kComputeThreadBlockSize - 1) / kComputeThreadBlockSize;
        mpCullPass->execute(pRenderContext, groupCount, 1, 1);

        // 3. Build args: (3, count, 0, 0) from counter
        mpBuildArgsVars->getRootVar()["counter"] = mpVisibleIndicesBuffer->getUAVCounter();
        mpBuildArgsVars->getRootVar()["drawArgs"] = mpDrawArgsBuffer;
        mpBuildArgsPass->execute(pRenderContext, 1, 1, 1);

        // 4. Draw with visible indices: single drawIndirect(3, count, 0, 0), VS uses gVisibleIndices[instanceId]
        mpGraphicsState->setProgram(mpGraphicsProgramCulling);
        mpGraphicsVarsCulling->getRootVar()["gSceneCB"] = mpConstantBuffers[frameIdx];
        mpGraphicsVarsCulling->getRootVar()["gVisibleIndices"] = mpVisibleIndicesBuffer;

        pRenderContext->drawIndirect(
            mpGraphicsState.get(), mpGraphicsVarsCulling.get(), 1,
            mpDrawArgsBuffer.get(), 0, nullptr, 0);
    }
    else
    {
        mpGraphicsState->setProgram(mpGraphicsProgram);
        // drawIndirect 多命令时每 draw 的 SV_InstanceID 均为 0，改用 drawInstanced 使 instanceId=0..1023
        pRenderContext->drawInstanced(
            mpGraphicsState.get(), mpGraphicsVars.get(),
            3, kTriangleCount, 0, 0);
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

SampleBase* D3D12ExecuteIndirect::create(SampleApp* pHost)
{
    return new D3D12ExecuteIndirect(pHost);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<SampleBase, D3D12ExecuteIndirect>();
}
