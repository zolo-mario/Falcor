#include "TinyRaytracer.h"

#include "Utils/Math/FalcorMath.h"
#include "Utils/Math/MatrixMath.h"
#include "Utils/Math/VectorMath.h"

#include <algorithm>
#include <cmath>

FALCOR_EXPORT_D3D12_AGILITY_SDK

namespace
{
const char kPresentShader[] = "Samples/GraphicsGems/TinyRaytracer/TinyRaytracerPresent.slang";

struct PresentVertex
{
    float3 position;
    float2 uv;
};
} // namespace

TinyRaytracer::TinyRaytracer(SampleApp* pHost) : SampleBase(pHost) {}

TinyRaytracer::~TinyRaytracer() = default;

void TinyRaytracer::rebuildCpuFramebuffer()
{
    mCpuFb.resize(mCpuWidth, mCpuHeight);
    mpCpuResultTexture = getDevice()->createTexture2D(
        mCpuWidth, mCpuHeight, ResourceFormat::RGBA8Unorm, 1, 1, nullptr, ResourceBindFlags::ShaderResource
    );
    if (mpVars)
        mpVars->getRootVar()["g_texture"] = mpCpuResultTexture;
}

void TinyRaytracer::initOrbitCamera()
{
    mpRenderCamera = Camera::create();
    mpOrbitCam = std::make_unique<OrbiterCameraController>(mpRenderCamera);
    mpRenderCamera->setDepthRange(0.1f, 100.f);
    mpRenderCamera->setFocalLength(fovYToFocalLength(math::radians(45.f), Camera::kDefaultFrameHeight));
    syncOrbitCameraDistance();
}

void TinyRaytracer::syncOrbitCameraDistance()
{
    if (mpOrbitCam)
        mpOrbitCam->setModelParams(float3(0.f), mOrbitWorldRadius, mCameraDistance);
}

void TinyRaytracer::onLoad(RenderContext* /*pRenderContext*/)
{
    mScene.buildScene(mScenePreset);
    initOrbitCamera();

    mCpuFb.resize(mCpuWidth, mCpuHeight);
    mpCpuResultTexture = getDevice()->createTexture2D(
        mCpuWidth, mCpuHeight, ResourceFormat::RGBA8Unorm, 1, 1, nullptr, ResourceBindFlags::ShaderResource
    );

    const PresentVertex vertices[] = {
        {{-1.f, -1.f, 0.f}, {0.f, 1.f}},
        {{-1.f, 3.f, 0.f}, {0.f, -1.f}},
        {{3.f, -1.f, 0.f}, {2.f, 1.f}},
    };
    mpVertexBuffer = getDevice()->createBuffer(sizeof(vertices), ResourceBindFlags::Vertex, MemoryType::Upload, (void*)vertices);

    auto pBufLayout = VertexBufferLayout::create();
    pBufLayout->addElement("POSITION", offsetof(PresentVertex, position), ResourceFormat::RGB32Float, 1, 0);
    pBufLayout->addElement("TEXCOORD", offsetof(PresentVertex, uv), ResourceFormat::RG32Float, 1, 1);
    auto pLayout = VertexLayout::create();
    pLayout->addBufferLayout(0, pBufLayout);
    mpVao = Vao::create(Vao::Topology::TriangleList, pLayout, {mpVertexBuffer});

    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(TextureFilteringMode::Point, TextureFilteringMode::Point, TextureFilteringMode::Point)
        .setAddressingMode(TextureAddressingMode::Clamp, TextureAddressingMode::Clamp, TextureAddressingMode::Clamp);
    mpSampler = getDevice()->createSampler(samplerDesc);

    ProgramDesc desc;
    desc.addShaderLibrary(kPresentShader).vsEntry("VSMain").psEntry("PSMain");
    mpProgram = Program::create(getDevice(), desc);
    mpVars = ProgramVars::create(getDevice(), mpProgram.get());
    mpVars->getRootVar()["g_texture"] = mpCpuResultTexture;
    mpVars->getRootVar()["g_sampler"] = mpSampler;

    mpState = GraphicsState::create(getDevice());
    mpState->setVao(mpVao);
    mpState->setProgram(mpProgram);
    mpState->setDepthStencilState(DepthStencilState::create(DepthStencilState::Desc().setDepthEnabled(false)));
    mpState->setRasterizerState(RasterizerState::create(RasterizerState::Desc().setCullMode(RasterizerState::CullMode::None)));
}

void TinyRaytracer::onShutdown()
{
    mpOrbitCam.reset();
    mpRenderCamera.reset();
    mpState.reset();
    mpVars.reset();
    mpProgram.reset();
    mpSampler.reset();
    mpCpuResultTexture.reset();
    mpVao.reset();
    mpVertexBuffer.reset();
}

void TinyRaytracer::onResize(uint32_t /*width*/, uint32_t /*height*/) {}

void TinyRaytracer::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    const float aspect = float(mCpuWidth) / float(std::max(1u, mCpuHeight));
    float4x4 viewMat = math::matrixFromLookAt(float3(0.f, 0.6f, mCameraDistance), float3(0.f, 0.2f, 0.f), float3(0.f, 1.f, 0.f));
    float4x4 projMat = math::perspective(math::radians(45.f), aspect, 0.1f, 100.f);

    if (mpOrbitCam && mpRenderCamera)
    {
        mpRenderCamera->setAspectRatio(aspect);
        mpOrbitCam->update();
        mpRenderCamera->beginFrame();
        viewMat = mpRenderCamera->getViewMatrix();
        projMat = mpRenderCamera->getProjMatrix();
    }

    const float3 clearRgb(0.05f, 0.06f, 0.08f);
    mCpuFb.clear(clearRgb);

    for (uint32_t y = 0; y < mCpuHeight; ++y)
    {
        for (uint32_t x = 0; x < mCpuWidth; ++x)
        {
            const float2 mousePos(
                (float(x) + 0.5f) / float(mCpuWidth), (float(y) + 0.5f) / float(mCpuHeight)
            );
            const float3 dir = mousePosToWorldRay(mousePos, viewMat, projMat);
            const float3 origin = mpOrbitCam && mpRenderCamera ? mpRenderCamera->getPosition() : float3(0.f, 0.6f, mCameraDistance);
            tinyrt::Ray ray{origin, normalize(dir)};
            float3 c = mScene.castRay(ray, 0, mMaxDepth);
            c.x = std::clamp(c.x, 0.f, 1.f);
            c.y = std::clamp(c.y, 0.f, 1.f);
            c.z = std::clamp(c.z, 0.f, 1.f);
            c = float3(std::sqrt(c.x), std::sqrt(c.y), std::sqrt(c.z));
            mCpuFb.setPixel((int)x, (int)y, c);
        }
    }

    pRenderContext->updateTextureData(mpCpuResultTexture.get(), mCpuFb.color.data());

    const float4 kClear(0.05f, 0.05f, 0.08f, 1.f);
    pRenderContext->clearFbo(pTargetFbo.get(), kClear, 1.0f, 0, FboAttachmentType::All);
    mpState->setFbo(pTargetFbo, true);
    pRenderContext->draw(mpState.get(), mpVars.get(), 3, 0);
}

void TinyRaytracer::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "CPU TinyRaytracer (tinyraytracer-style)", {440, 380}, {10, 10});
    w.text("Primary rays -> sphere hit -> Phong + shadow + reflect/refract (glass).");
    w.text("Camera: left-drag orbit, mouse wheel zoom (or Camera Z slider).");
    w.separator();
    w.slider("CPU width", mCpuWidth, 128u, 1024u);
    w.slider("CPU height", mCpuHeight, 128u, 1024u);
    if (w.button("Apply CPU framebuffer size"))
        rebuildCpuFramebuffer();
    if (w.slider("Camera Z", mCameraDistance, 0.8f, 12.f))
        syncOrbitCameraDistance();
    w.slider("Max ray depth", mMaxDepth, 0, 8);
    Gui::DropdownList presetList = {
        {0u, "Classic (glass+mirror)"},
        {1u, "Simple"},
    };
    uint32_t preset = (uint32_t)mScenePreset;
    if (w.dropdown("Scene preset", presetList, preset))
    {
        mScenePreset = (int)preset;
        mScene.buildScene(mScenePreset);
    }
    renderGlobalUI(pGui);
}

bool TinyRaytracer::onKeyEvent(const KeyboardEvent& /*keyEvent*/)
{
    return false;
}

bool TinyRaytracer::onMouseEvent(const MouseEvent& mouseEvent)
{
    if (mpOrbitCam && mpOrbitCam->onMouseEvent(mouseEvent))
        return true;
    return false;
}

void TinyRaytracer::onHotReload(HotReloadFlags /*reloaded*/)
{}

void TinyRaytracer::setProperties(const Properties& props)
{
    const uint32_t prevW = mCpuWidth;
    const uint32_t prevH = mCpuHeight;

    if (props.has("cpu-width"))
        mCpuWidth = props.get<uint32_t>("cpu-width");
    if (props.has("cpu-height"))
        mCpuHeight = props.get<uint32_t>("cpu-height");
    if (props.has("camera-z"))
    {
        mCameraDistance = props.get<float>("camera-z");
        syncOrbitCameraDistance();
    }
    if (props.has("max-depth"))
        mMaxDepth = props.get<int>("max-depth");
    if (props.has("scene-preset"))
        mScenePreset = props.get<int>("scene-preset");

    if (props.has("scene-preset"))
        mScene.buildScene(mScenePreset);

    if (mpCpuResultTexture && (mCpuWidth != prevW || mCpuHeight != prevH))
        rebuildCpuFramebuffer();
}

Properties TinyRaytracer::getProperties() const
{
    Properties p;
    p.set("cpu-width", mCpuWidth);
    p.set("cpu-height", mCpuHeight);
    p.set("camera-z", mCameraDistance);
    p.set("max-depth", mMaxDepth);
    p.set("scene-preset", mScenePreset);
    return p;
}

SampleBase* TinyRaytracer::create(SampleApp* pHost)
{
    return new TinyRaytracer(pHost);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<SampleBase, TinyRaytracer>();
}
