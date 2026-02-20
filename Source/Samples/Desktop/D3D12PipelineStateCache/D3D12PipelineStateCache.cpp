#include "D3D12PipelineStateCache.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

static float getRandomColor()
{
    return (float)(rand() % 100) / 100.0f;
}

D3D12PipelineStateCache::D3D12PipelineStateCache(const SampleAppConfig& config) : SampleApp(config)
{
    for (uint32_t i = 0; i < EffectPipelineTypeCount; i++)
        mEnabledEffects[i] = true;
}

D3D12PipelineStateCache::~D3D12PipelineStateCache() {}

void D3D12PipelineStateCache::onLoad(RenderContext* pRenderContext)
{
    uint2 size = getWindow() ? getWindow()->getClientAreaSize() : uint2(1280, 720);
    const float aspectRatio = size.y > 0 ? (float)size.x / size.y : (1280.f / 720.f);

    // Match D3D12 sample: camera at (0,0,5), fov 0.8
    mProjectionMatrix = math::perspective(0.8f, aspectRatio, 1.0f, 1000.0f);

    // Cube vertices - match D3D12 VertexPositionColor (1:1)
    struct VertexPositionColor
    {
        float4 position;
        float3 color;
    };
    const VertexPositionColor cubeVertices[] = {
        {{-1.0f, 1.0f, -1.0f, 1.0f}, {getRandomColor(), getRandomColor(), getRandomColor()}},
        {{1.0f, 1.0f, -1.0f, 1.0f}, {getRandomColor(), getRandomColor(), getRandomColor()}},
        {{1.0f, 1.0f, 1.0f, 1.0f}, {getRandomColor(), getRandomColor(), getRandomColor()}},
        {{-1.0f, 1.0f, 1.0f, 1.0f}, {getRandomColor(), getRandomColor(), getRandomColor()}},
        {{-1.0f, -1.0f, -1.0f, 1.0f}, {getRandomColor(), getRandomColor(), getRandomColor()}},
        {{1.0f, -1.0f, -1.0f, 1.0f}, {getRandomColor(), getRandomColor(), getRandomColor()}},
        {{1.0f, -1.0f, 1.0f, 1.0f}, {getRandomColor(), getRandomColor(), getRandomColor()}},
        {{-1.0f, -1.0f, 1.0f, 1.0f}, {getRandomColor(), getRandomColor(), getRandomColor()}},
    };
    const uint32_t cubeIndices[] = {
        0, 1, 3, 1, 2, 3, 3, 2, 7, 6, 7, 2, 2, 1, 6, 5, 6, 1,
        1, 0, 5, 4, 5, 0, 0, 3, 4, 7, 4, 3, 7, 6, 4, 5, 4, 6,
    };

    mpCubeVertexBuffer = getDevice()->createBuffer(
        sizeof(cubeVertices), ResourceBindFlags::Vertex, MemoryType::Upload, (void*)cubeVertices);
    mpCubeIndexBuffer = getDevice()->createBuffer(
        sizeof(cubeIndices), ResourceBindFlags::Index, MemoryType::Upload, (void*)cubeIndices);

    auto pCubeBufLayout = VertexBufferLayout::create();
    pCubeBufLayout->addElement("POSITION", 0, ResourceFormat::RGBA32Float, 1, 0);
    pCubeBufLayout->addElement("COLOR", 16, ResourceFormat::RGB32Float, 1, 1);
    auto pCubeLayout = VertexLayout::create();
    pCubeLayout->addBufferLayout(0, pCubeBufLayout);
    Vao::BufferVec cubeBuffers{mpCubeVertexBuffer};
    mpCubeVao = Vao::create(Vao::Topology::TriangleList, pCubeLayout, cubeBuffers, mpCubeIndexBuffer, ResourceFormat::R32Uint);

    // Quad vertices - match D3D12 VertexPositionUV (1:1)
    struct VertexPositionUV
    {
        float4 position;
        float2 uv;
    };
    const VertexPositionUV quadVertices[] = {
        {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
    };
    mpQuadVertexBuffer = getDevice()->createBuffer(
        sizeof(quadVertices), ResourceBindFlags::Vertex, MemoryType::Upload, (void*)quadVertices);
    auto pQuadBufLayout = VertexBufferLayout::create();
    pQuadBufLayout->addElement("POSITION", 0, ResourceFormat::RGBA32Float, 1, 0);
    pQuadBufLayout->addElement("TEXCOORD", 16, ResourceFormat::RG32Float, 1, 1);
    auto pQuadLayout = VertexLayout::create();
    pQuadLayout->addBufferLayout(0, pQuadBufLayout);
    Vao::BufferVec quadBuffers{mpQuadVertexBuffer};
    mpQuadVao = Vao::create(Vao::Topology::TriangleStrip, pQuadLayout, quadBuffers);

    // Constant buffer for cube (worldViewProjection)
    mpConstantBuffer = getDevice()->createBuffer(sizeof(float4x4), ResourceBindFlags::Constant, MemoryType::Upload, nullptr);

    // Programs
    ProgramDesc cubeDesc;
    cubeDesc.addShaderLibrary("Samples/Desktop/D3D12PipelineStateCache/Cube.slang")
        .vsEntry("VSMain")
        .psEntry("PSMain");
    mpCubeProgram = Program::create(getDevice(), cubeDesc);
    mpCubeVars = ProgramVars::create(getDevice(), mpCubeProgram.get());

    ProgramDesc effectDesc;
    effectDesc.addShaderLibrary("Samples/Desktop/D3D12PipelineStateCache/PostEffect.slang")
        .vsEntry("VSMain")
        .psEntry("PSMain");
    mpEffectProgram = Program::create(getDevice(), effectDesc);
    mpEffectVars = ProgramVars::create(getDevice(), mpEffectProgram.get());

    // Sampler - match D3D12: Point filter, Border
    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(TextureFilteringMode::Point, TextureFilteringMode::Point, TextureFilteringMode::Point)
        .setAddressingMode(TextureAddressingMode::Border, TextureAddressingMode::Border, TextureAddressingMode::Border);
    mpSampler = getDevice()->createSampler(samplerDesc);

    mpCubeState = GraphicsState::create(getDevice());
    mpCubeState->setVao(mpCubeVao);
    mpCubeState->setProgram(mpCubeProgram);
    mpCubeState->setDepthStencilState(DepthStencilState::create(DepthStencilState::Desc().setDepthEnabled(false)));
    mpCubeState->setRasterizerState(RasterizerState::create(RasterizerState::Desc().setCullMode(RasterizerState::CullMode::None)));

    mpEffectState = GraphicsState::create(getDevice());
    mpEffectState->setVao(mpQuadVao);
    mpEffectState->setProgram(mpEffectProgram);
    mpEffectState->setDepthStencilState(DepthStencilState::create(DepthStencilState::Desc().setDepthEnabled(false)));
}

void D3D12PipelineStateCache::onShutdown() {}

void D3D12PipelineStateCache::onResize(uint32_t width, uint32_t height)
{
    const float aspectRatio = height > 0 ? (float)width / height : (1280.f / 720.f);
    mProjectionMatrix = math::perspective(0.8f, aspectRatio, 1.0f, 1000.0f);

    mpIntermediateFbo = nullptr;
}

void D3D12PipelineStateCache::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    uint2 targetSize = pTargetFbo->getWidth(), pTargetFbo->getHeight();
    if (targetSize.x == 0 || targetSize.y == 0)
        return;

    if (!mpIntermediateFbo || mpIntermediateFbo->getWidth() != targetSize.x || mpIntermediateFbo->getHeight() != targetSize.y)
    {
        mpIntermediateFbo = Fbo::create2D(getDevice(), targetSize.x, targetSize.y, ResourceFormat::RGBA8Unorm);
    }

    // Pass 1: Draw cube to intermediate RT - match D3D12 IntermediateClearColor
    const float4 intermediateClearColor(kIntermediateClearColor[0], kIntermediateClearColor[1], kIntermediateClearColor[2], kIntermediateClearColor[3]);
    pRenderContext->clearFbo(mpIntermediateFbo.get(), intermediateClearColor, 1.0f, 0, FboAttachmentType::Color);

    float4x4 viewMatrix = math::matrixFromLookAt(float3(0, 0, 5), float3(0, 0, 0), float3(0, 1, 0));
    float4x4 worldMatrix = math::mul(math::matrixFromRotationY(mRotation), math::matrixFromRotationX(-mRotation));
    float4x4 wvp = math::transpose(math::mul(math::mul(worldMatrix, viewMatrix), mProjectionMatrix));
    mpConstantBuffer->setBlob(&wvp, 0, sizeof(float4x4));

    mpCubeVars->getRootVar()["PerDraw"]["worldViewProjection"] = mpConstantBuffer;
    mpCubeState->setFbo(mpIntermediateFbo, true);
    pRenderContext->drawIndexed(mpCubeState.get(), mpCubeVars.get(), 36, 0, 0);

    mRotation += 0.01f;

    // Pass 2: Post-process quads - match D3D12 3x3 grid
    pRenderContext->clearFbo(pTargetFbo.get(), float4(0, 0, 0, 0), 1.0f, 0, FboAttachmentType::Color);

    mpEffectVars->getRootVar()["g_tex"] = mpIntermediateFbo->getColorTexture(0);
    mpEffectVars->getRootVar()["g_samp"] = mpSampler;

    mpEffectState->setFbo(pTargetFbo, true);

    for (uint32_t i = PostBlit; i < EffectPipelineTypeCount; i++)
    {
        if (mEnabledEffects[i])
        {
            uint32_t quadCount = i - PostBlit;
            uint32_t qx = quadCount % kQuadsX;
            uint32_t qy = quadCount / kQuadsY;
            float quadW = (float)targetSize.x / kQuadsX;
            float quadH = (float)targetSize.y / kQuadsY;
            float vpX = qx * quadW;
            float vpY = qy * quadH;

            mpEffectState->setViewport(0, Viewport(vpX, vpY, quadW, quadH, 0.f, 1.f));
            mpEffectVars->getRootVar()["EffectCB"]["effectIndex"] = i;
            pRenderContext->draw(mpEffectState.get(), mpEffectVars.get(), 4, 0);
        }
    }
}

void D3D12PipelineStateCache::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "D3D12 Pipeline State Cache", {300, 280});
    renderGlobalUI(pGui);
    w.text("Keys 1-9: Toggle effects (Blit, Invert, GrayScale, EdgeDetect, Blur, Warp, Pixelate, Distort, Wave)");
    w.text("Note: PSO caching (C/U/L/M) is handled by Falcor/Slang internally.");
}

bool D3D12PipelineStateCache::onKeyEvent(const KeyboardEvent& keyEvent)
{
    if (keyEvent.type == KeyboardEvent::Type::KeyReleased)
    {
        switch (keyEvent.key)
        {
        case Input::Key::Key1: toggleEffect(PostBlit); return true;
        case Input::Key::Key2: toggleEffect(PostInvert); return true;
        case Input::Key::Key3: toggleEffect(PostGrayScale); return true;
        case Input::Key::Key4: toggleEffect(PostEdgeDetect); return true;
        case Input::Key::Key5: toggleEffect(PostBlur); return true;
        case Input::Key::Key6: toggleEffect(PostWarp); return true;
        case Input::Key::Key7: toggleEffect(PostPixelate); return true;
        case Input::Key::Key8: toggleEffect(PostDistort); return true;
        case Input::Key::Key9: toggleEffect(PostWave); return true;
        default: break;
        }
    }
    return false;
}

bool D3D12PipelineStateCache::onMouseEvent(const MouseEvent& mouseEvent)
{
    return false;
}

void D3D12PipelineStateCache::onHotReload(HotReloadFlags reloaded)
{
}

void D3D12PipelineStateCache::toggleEffect(EffectPipelineType type)
{
    if (type >= PostBlit && type < EffectPipelineTypeCount)
        mEnabledEffects[type] = !mEnabledEffects[type];
}

int runMain(int argc, char** argv)
{
    SampleAppConfig config;
    config.windowDesc.title = "D3D12 Pipeline State Object Cache Sample";
    config.windowDesc.resizableWindow = true;

    D3D12PipelineStateCache project(config);
    return project.run();
}

int main(int argc, char** argv)
{
    return catchAndReportAllExceptions([&]() { return runMain(argc, argv); });
}
