#include "Testing/UnitTest.h"
#include "DiffRendering/SceneGradients.h"

namespace Falcor
{
namespace
{
const char kShaderFile[] = "Tests/DiffRendering/SceneGradientsTest.cs.slang";

void testAggregateGradients(GPUUnitTestContext& ctx, const uint32_t hashSize)
{
    // We create a gradient vector with dimension = 3.
    // We add 10^i to the i-th element for 1024 times (using atomic add).
    // So the expected value of the i-th element is 1024 * (10^i).

    const uint32_t gradDim = 3;
    const uint32_t elemCount = 1024;

    ref<Device> pDevice = ctx.getDevice();
    RenderContext* pRenderContext = pDevice->getRenderContext();

    std::unique_ptr<SceneGradients> pSceneGradients =
        std::make_unique<SceneGradients>(pDevice, std::vector<SceneGradients::GradConfig>({{GradientType::Material, gradDim, hashSize}}));
    pSceneGradients->clearGrads(pRenderContext, GradientType::Material);

    ctx.createProgram(kShaderFile, "atomicAdd");
    ctx["CB"]["sz"] = uint2(gradDim, elemCount);
    ctx["CB"]["hashSize"] = hashSize;
    pSceneGradients->bindShaderData(ctx["gSceneGradients"]);
    ctx.runProgram(gradDim, elemCount, 1);

    pSceneGradients->aggregateGrads(pRenderContext, GradientType::Material);

    ctx.createProgram(kShaderFile, "testAggregateGradients");
    ctx["CB"]["sz"] = uint2(gradDim, elemCount);
    ctx["grads"] = pSceneGradients->getGradsBuffer(GradientType::Material);
    ctx.allocateStructuredBuffer("result", gradDim);
    ctx.runProgram(gradDim, 1, 1);

    std::vector<float> result = ctx.readBuffer<float>("result");
    for (uint32_t i = 0; i < gradDim; ++i)
    {
        float refValue = elemCount * std::pow(10.f, i);
        float relAbsDiff = std::abs(result[i] - refValue) / refValue;
        EXPECT_LE(relAbsDiff, 1e-6f);
    }
}
} // namespace

// Disabled on Vulkan for now as the compiler generates invalid code.
GPU_TEST(AggregateGradients, Device::Type::D3D12)
{
    testAggregateGradients(ctx, 64);
}
} // namespace Falcor
