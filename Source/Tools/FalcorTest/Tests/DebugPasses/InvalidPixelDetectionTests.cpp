#include "Core/Plugin.h"
#include "Testing/UnitTest.h"
#include "RenderGraph/RenderGraph.h"

namespace Falcor
{
GPU_TEST(InvalidPixelDetectionPass)
{
    PluginManager::instance().loadPluginByName("DebugPasses");

    ref<Device> pDevice = ctx.getDevice();

    float pInitData[8] = {
        std::numeric_limits<float>::quiet_NaN(),
        std::numeric_limits<float>::signaling_NaN(),
        std::numeric_limits<float>::infinity(),
        -1 * std::numeric_limits<float>::infinity(),
        0.0f,
        255.0f,
        125.8f,
        1.0f,
    };

    RenderContext* pRenderContext = ctx.getRenderContext();
    ref<Fbo> pTargetFbo = Fbo::create2D(pDevice, 2, 4, ResourceFormat::BGRA8UnormSrgb);
    ref<Texture> pInput = pDevice->createTexture2D(2, 4, ResourceFormat::R32Float, 1, Resource::kMaxPossible, pInitData);
    ref<RenderGraph> pGraph = RenderGraph::create(ctx.getDevice(), "Invalid Pixel Detection");
    ref<RenderPass> pPass = RenderPass::create("InvalidPixelDetectionPass", ctx.getDevice());
    if (!pPass)
        FALCOR_THROW("Could not create render pass 'InvalidPixelDetectionPass'");
    pGraph->addPass(pPass, "InvalidPixelDetectionPass");
    pGraph->setInput("InvalidPixelDetectionPass.src", pInput);
    pGraph->markOutput("InvalidPixelDetectionPass.dst");
    pGraph->onResize(pTargetFbo.get());
    pGraph->execute(pRenderContext);
    ref<Resource> pOutput = pGraph->getOutput("InvalidPixelDetectionPass.dst");
    std::vector<uint8_t> color = pRenderContext->readTextureSubresource(pOutput->asTexture().get(), 0);
    uint32_t* output = (uint32_t*)color.data();

    for (uint32_t i = 0; i < 8; ++i)
    {
        uint32_t expected;
        switch (i)
        {
        case 0:
        case 1:
            expected = 0xFFFF0000;
            break;
        case 2:
        case 3:
            expected = 0xFF00FF00;
            break;
        default:
            expected = 0xFF000000;
            break;
        }
        EXPECT_EQ(output[i], expected);
    }
}
} // namespace Falcor
