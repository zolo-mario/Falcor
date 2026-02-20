#include "D3D12HDR.h"
#include "Utils/Math/Vector.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

D3D12HDR::D3D12HDR(SampleApp* pHost) : SampleBase(pHost) {}

D3D12HDR::~D3D12HDR() {}

void D3D12HDR::onLoad(RenderContext* pRenderContext)
{
    uint2 size = getWindow() ? getWindow()->getClientAreaSize() : uint2(1280, 720);
    mWidth = size.x;
    mHeight = size.y;
    mAspectRatio = mHeight > 0 ? (float)mWidth / mHeight : (1280.f / 720.f);

    // Gradient vertices (match D3D12HDR LoadAssets)
    const GradientVertex gradientVertices[] = {
        // SDR strip [0,1]
        {{-1.0f, 0.45f, 0.0f}, {0.0f, 0.0f, 0.0f}},
        {{-1.0f, 0.55f, 0.0f}, {0.0f, 0.0f, 0.0f}},
        {{0.0f, 0.45f, 0.0f}, {1.0f, 1.0f, 1.0f}},
        {{0.0f, 0.55f, 0.0f}, {1.0f, 1.0f, 1.0f}},
        // HDR strip [0,9] - 3.0 is ~3x brighter perceptually
        {{-1.0f, -0.55f, 0.0f}, {0.0f, 0.0f, 0.0f}},
        {{-1.0f, -0.45f, 0.0f}, {0.0f, 0.0f, 0.0f}},
        {{0.0f, -0.55f, 0.0f}, {3.0f, 3.0f, 3.0f}},
        {{0.0f, -0.45f, 0.0f}, {3.0f, 3.0f, 3.0f}},
    };
    mpGradientVB = getDevice()->createBuffer(
        sizeof(gradientVertices), ResourceBindFlags::Vertex, MemoryType::Upload, (void*)gradientVertices);

    // Present vertices - fullscreen triangle (match D3D12HDR)
    const PresentVertex presentVertices[] = {
        {{-1.0f, -3.0f, 0.0f}, {0.0f, 2.0f}},
        {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{3.0f, 1.0f, 0.0f}, {2.0f, 0.0f}},
    };
    mpPresentVB = getDevice()->createBuffer(
        sizeof(presentVertices), ResourceBindFlags::Vertex, MemoryType::Upload, (void*)presentVertices);

    // Palette vertices - updated in updatePaletteVertices (aspect-ratio dependent)
    mPaletteVertices.resize(18);
    mpPaletteVB = getDevice()->createBuffer(
        18 * sizeof(TrianglesVertex), ResourceBindFlags::Vertex, MemoryType::Upload, nullptr);

    // VAOs
    auto pGradientBufLayout = VertexBufferLayout::create();
    pGradientBufLayout->addElement("POSITION", 0, ResourceFormat::RGB32Float, 1, 0);
    pGradientBufLayout->addElement("COLOR", 12, ResourceFormat::RGB32Float, 1, 1);
    auto pGradientLayout = VertexLayout::create();
    pGradientLayout->addBufferLayout(0, pGradientBufLayout);
    mpGradientVao = Vao::create(Vao::Topology::TriangleStrip, pGradientLayout, {mpGradientVB});

    auto pPaletteBufLayout = VertexBufferLayout::create();
    pPaletteBufLayout->addElement("POSITION", 0, ResourceFormat::RGB32Float, 1, 0);
    pPaletteBufLayout->addElement("TEXCOORD", 12, ResourceFormat::RG32Float, 1, 1);
    auto pPaletteLayout = VertexLayout::create();
    pPaletteLayout->addBufferLayout(0, pPaletteBufLayout);
    mpPaletteVao = Vao::create(Vao::Topology::TriangleList, pPaletteLayout, {mpPaletteVB});

    auto pPresentBufLayout = VertexBufferLayout::create();
    pPresentBufLayout->addElement("POSITION", 0, ResourceFormat::RGB32Float, 1, 0);
    pPresentBufLayout->addElement("TEXCOORD", 12, ResourceFormat::RG32Float, 1, 1);
    auto pPresentLayout = VertexLayout::create();
    pPresentLayout->addBufferLayout(0, pPresentBufLayout);
    mpPresentVao = Vao::create(Vao::Topology::TriangleList, pPresentLayout, {mpPresentVB});

    // Intermediate FBO (R16G16B16A16_FLOAT, match original)
    mpIntermediateFbo = Fbo::create2D(getDevice(), mWidth, mHeight, ResourceFormat::RGBA16Float, ResourceFormat::Unknown);

    // Programs
    ProgramDesc gradientDesc;
    gradientDesc.addShaderLibrary("Samples/Desktop/D3D12HDR/Gradient.slang").vsEntry("VSMain").psEntry("PSMain");
    mpGradientProgram = Program::create(getDevice(), gradientDesc);
    mpGradientVars = ProgramVars::create(getDevice(), mpGradientProgram.get());

    ProgramDesc palette709Desc;
    palette709Desc.addShaderLibrary("Samples/Desktop/D3D12HDR/Palette709.slang").vsEntry("VSMain").psEntry("PSMain");
    mpPalette709Program = Program::create(getDevice(), palette709Desc);
    mpPalette709Vars = ProgramVars::create(getDevice(), mpPalette709Program.get());

    ProgramDesc palette2020Desc;
    palette2020Desc.addShaderLibrary("Samples/Desktop/D3D12HDR/Palette2020.slang").vsEntry("VSMain").psEntry("PSMain");
    mpPalette2020Program = Program::create(getDevice(), palette2020Desc);
    mpPalette2020Vars = ProgramVars::create(getDevice(), mpPalette2020Program.get());

    ProgramDesc presentDesc;
    presentDesc.addShaderLibrary("Samples/Desktop/D3D12HDR/Present.slang").vsEntry("VSMain").psEntry("PSMain");
    mpPresentProgram = Program::create(getDevice(), presentDesc);
    mpPresentVars = ProgramVars::create(getDevice(), mpPresentProgram.get());

    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(TextureFilteringMode::Point, TextureFilteringMode::Point, TextureFilteringMode::Point)
        .setAddressingMode(TextureAddressingMode::Border, TextureAddressingMode::Border, TextureAddressingMode::Border);
    mpPresentSampler = getDevice()->createSampler(samplerDesc);

    mpGraphicsState = GraphicsState::create(getDevice());
    mpDepthStencilState = DepthStencilState::create(DepthStencilState::Desc().setDepthEnabled(false));
    mpGraphicsState->setDepthStencilState(mpDepthStencilState);

    updatePaletteVertices();
}

void D3D12HDR::updatePaletteVertices()
{
    // Rec.709 and Rec.2020 primaries (match D3D12HDR UpdateVertexBuffer)
    const float2 primaries709[] = {{0.64f, 0.33f}, {0.30f, 0.60f}, {0.15f, 0.06f}, {0.3127f, 0.3290f}};
    const float2 primaries2020[] = {{0.708f, 0.292f}, {0.170f, 0.797f}, {0.131f, 0.046f}, {0.3127f, 0.3290f}};
    const float2 offset1 = {0.2f, 0.0f};
    const float2 offset2 = {0.2f, -1.0f};

    auto transformVertex = [this](float2 point, float2 offset) -> float3 {
        float2 scale = float2(std::min(1.0f, 1.0f / mAspectRatio), std::min(1.0f, mAspectRatio));
        float2 margin = float2(0.5f * (1.0f - scale.x), 0.5f * (1.0f - scale.y));
        float2 v = point * scale + margin + offset;
        return float3(v.x, v.y, 0.0f);
    };

    float3 tri709[4], tri2020[4];
    for (int i = 0; i < 4; i++)
    {
        tri709[i] = transformVertex(primaries709[i], offset1);
        tri2020[i] = transformVertex(primaries2020[i], offset2);
    }

    TrianglesVertex* p = mPaletteVertices.data();
    // Rec.709 triangles (3 per primary + white)
    auto addTri = [&p](float3 a, float3 b, float3 c, float2 uva, float2 uvb, float2 uvc) {
        p[0] = {a, uva};
        p[1] = {b, uvb};
        p[2] = {c, uvc};
        p += 3;
    };
    addTri(tri709[2], tri709[1], tri709[3], primaries709[2], primaries709[1], primaries709[3]);
    addTri(tri709[1], tri709[0], tri709[3], primaries709[1], primaries709[0], primaries709[3]);
    addTri(tri709[0], tri709[2], tri709[3], primaries709[0], primaries709[2], primaries709[3]);
    addTri(tri2020[2], tri2020[1], tri2020[3], primaries2020[2], primaries2020[1], primaries2020[3]);
    addTri(tri2020[1], tri2020[0], tri2020[3], primaries2020[1], primaries2020[0], primaries2020[3]);
    addTri(tri2020[0], tri2020[2], tri2020[3], primaries2020[0], primaries2020[2], primaries2020[3]);

    mpPaletteVB->setBlob(mPaletteVertices.data(), 0, 18 * sizeof(TrianglesVertex));
}

void D3D12HDR::onShutdown() {}

void D3D12HDR::onResize(uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0)
        return;
    mWidth = width;
    mHeight = height;
    mAspectRatio = (float)width / height;

    mpIntermediateFbo = Fbo::create2D(getDevice(), mWidth, mHeight, ResourceFormat::RGBA16Float, ResourceFormat::Unknown);
    updatePaletteVertices();
}

void D3D12HDR::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    // 1. Draw to intermediate (RGBA16Float)
    const float4 clearColor(0.0f, 0.0f, 0.0f, 0.0f);
    pRenderContext->clearFbo(mpIntermediateFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::Color);
    mpGraphicsState->setFbo(mpIntermediateFbo, true);
    mpGraphicsState->setDepthStencilState(mpDepthStencilState);

    // Gradient (no root constants needed)
    mpGraphicsState->setVao(mpGradientVao);
    mpGraphicsState->setProgram(mpGradientProgram);
    pRenderContext->draw(mpGraphicsState.get(), mpGradientVars.get(), 4, 0);  // SDR strip
    pRenderContext->draw(mpGraphicsState.get(), mpGradientVars.get(), 4, 4);  // HDR strip

    // Palette - Rec.709 then Rec.2020
    mpGraphicsState->setVao(mpPaletteVao);
    mpGraphicsState->setProgram(mpPalette709Program);
    pRenderContext->draw(mpGraphicsState.get(), mpPalette709Vars.get(), 3, 0);
    pRenderContext->draw(mpGraphicsState.get(), mpPalette709Vars.get(), 3, 3);
    pRenderContext->draw(mpGraphicsState.get(), mpPalette709Vars.get(), 3, 6);
    mpGraphicsState->setProgram(mpPalette2020Program);
    pRenderContext->draw(mpGraphicsState.get(), mpPalette2020Vars.get(), 3, 9);
    pRenderContext->draw(mpGraphicsState.get(), mpPalette2020Vars.get(), 3, 12);
    pRenderContext->draw(mpGraphicsState.get(), mpPalette2020Vars.get(), 3, 15);

    // 2. Present: composite to pTargetFbo with display curve
    mpGraphicsState->setFbo(pTargetFbo, true);
    mpGraphicsState->setVao(mpPresentVao);
    mpGraphicsState->setProgram(mpPresentProgram);
    mpPresentVars->getRootVar()["RootConstants"]["standardNits"] = mReferenceWhiteNits;
    mpPresentVars->getRootVar()["RootConstants"]["displayCurve"] = (uint32_t)mDisplayCurve;
    mpPresentVars->getRootVar()["g_scene"] = mpIntermediateFbo->getColorTexture(0);
    mpPresentVars->getRootVar()["g_sampler"] = mpPresentSampler;
    pRenderContext->draw(mpGraphicsState.get(), mpPresentVars.get(), 3, 0);
}

void D3D12HDR::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "D3D12 HDR", {280, 180});
    renderGlobalUI(pGui);
    const Gui::DropdownList curveList = {{sRGB, "sRGB"}, {ST2084, "ST.2084 (HDR10)"}, {Linear, "Linear"}};
    w.dropdown("Display curve", curveList, reinterpret_cast<uint32_t&>(mDisplayCurve));
    w.slider("Reference white (nits)", mReferenceWhiteNits, 1.0f, 400.0f);
    w.text("Simplified: no runtime swap chain format switch.");
}

bool D3D12HDR::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return false;
}

bool D3D12HDR::onMouseEvent(const MouseEvent& mouseEvent)
{
    return false;
}

void D3D12HDR::onHotReload(HotReloadFlags reloaded)
{
}

SampleBase* D3D12HDR::create(SampleApp* pHost)
{
    return new D3D12HDR(pHost);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<SampleBase, D3D12HDR>();
}
