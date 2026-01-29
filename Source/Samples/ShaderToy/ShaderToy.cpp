#include "ShaderToy.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

ShaderToy::ShaderToy(const SampleAppConfig& config) : SampleApp(config) {}

ShaderToy::~ShaderToy() {}

void ShaderToy::onLoad(RenderContext* pRenderContext)
{
    // create rasterizer state
    RasterizerState::Desc rsDesc;
    mpNoCullRastState = RasterizerState::create(rsDesc);

    // Depth test
    DepthStencilState::Desc dsDesc;
    dsDesc.setDepthEnabled(false);
    mpNoDepthDS = DepthStencilState::create(dsDesc);

    // Blend state
    BlendState::Desc blendDesc;
    mpOpaqueBS = BlendState::create(blendDesc);

    // Texture sampler
    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(TextureFilteringMode::Linear, TextureFilteringMode::Linear, TextureFilteringMode::Linear).setMaxAnisotropy(8);
    mpLinearSampler = getDevice()->createSampler(samplerDesc);

    // Load shaders
    mpMainPass = FullScreenPass::create(getDevice(), "Samples/ShaderToy/Toy.ps.slang");
}

void ShaderToy::onResize(uint32_t width, uint32_t height)
{
    mAspectRatio = (float(width) / float(height));
}

void ShaderToy::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    // iResolution
    float width = (float)pTargetFbo->getWidth();
    float height = (float)pTargetFbo->getHeight();
    auto var = mpMainPass->getRootVar()["ToyCB"];
    var["iResolution"] = float2(width, height);
    var["iGlobalTime"] = (float)getGlobalClock().getTime();

    // run final pass
    mpMainPass->execute(pRenderContext, pTargetFbo);
}

int runMain(int argc, char** argv)
{
    SampleAppConfig config;
    config.windowDesc.width = 1280;
    config.windowDesc.height = 720;
    config.windowDesc.resizableWindow = true;
    config.windowDesc.enableVSync = true;
    config.windowDesc.title = "Falcor Shader Toy";

    ShaderToy shaderToy(config);
    return shaderToy.run();
}

int main(int argc, char** argv)
{
    return catchAndReportAllExceptions([&]() { return runMain(argc, argv); });
}
