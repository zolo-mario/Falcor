#include "MultiSampling.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

namespace
{
const uint32_t kTriangleCount = 16;
const uint32_t kSampleCount = 8;
} // namespace

MultiSampling::MultiSampling(SampleApp* pHost) : SampleBase(pHost) {}

MultiSampling::~MultiSampling() {}

void MultiSampling::onLoad(RenderContext* pRenderContext)
{
    // Load program
    mpRasterPass = RasterPass::create(getDevice(), "Samples/MultiSampling/MultiSampling.3d.slang", "vsMain", "psMain");

    // Create disk triangles
    float2 vertices[kTriangleCount * 3];
    for (uint32_t i = 0; i < kTriangleCount; ++i)
    {
        float theta0 = float(i) / kTriangleCount * M_2PI;
        float theta1 = float(i + 1) / kTriangleCount * M_2PI;
        vertices[i * 3 + 0] = float2(0, 0);
        vertices[i * 3 + 1] = float2(cos(theta0), sin(theta0)) * 0.75f;
        vertices[i * 3 + 2] = float2(cos(theta1), sin(theta1)) * 0.75f;
    }
    auto vertexBuffer = getDevice()->createTypedBuffer<float2>(
        kTriangleCount * 3, ResourceBindFlags::ShaderResource | ResourceBindFlags::Vertex, MemoryType::DeviceLocal, vertices
    );

    // Create vertex layout
    auto bufferLayout = VertexBufferLayout::create();
    bufferLayout->addElement("POSITION", 0, ResourceFormat::RG32Float, 1, 0);
    auto layout = VertexLayout::create();
    layout->addBufferLayout(0, bufferLayout);

    // Create VAO
    mpVao = Vao::create(Vao::Topology::TriangleList, layout, {vertexBuffer});

    // Create FBO
    mpFbo = Fbo::create(getDevice());
    ref<Texture> tex = getDevice()->createTexture2DMS(
        128, 128, ResourceFormat::RGBA32Float, kSampleCount, 1, ResourceBindFlags::ShaderResource | ResourceBindFlags::RenderTarget
    );
    mpFbo->attachColorTarget(tex, 0);

    mpResolvedTexture = getDevice()->createTexture2D(128, 128, ResourceFormat::RGBA32Float, 1, 1);
}

void MultiSampling::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    pRenderContext->clearFbo(mpFbo.get(), float4(0.f), 0.f, 0);

    mpRasterPass->getState()->setFbo(mpFbo);
    mpRasterPass->getState()->setVao(mpVao);
    mpRasterPass->draw(pRenderContext, kTriangleCount * 3, 0);

    if (mFrame++ % 2 == 0)
    {
        // For even frames, resolve to texture and then blit.
        pRenderContext->resolveResource(mpFbo->getColorTexture(0), mpResolvedTexture);
        pRenderContext->blit(mpResolvedTexture->getSRV(), pTargetFbo->getRenderTargetView(0));
    }
    else
    {
        // For odd frames, blit directly.
        pRenderContext->blit(mpFbo->getColorTexture(0)->getSRV(), pTargetFbo->getRenderTargetView(0));
    }
}

SampleBase* MultiSampling::create(SampleApp* pHost)
{
    return new MultiSampling(pHost);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<SampleBase, MultiSampling>();
}
