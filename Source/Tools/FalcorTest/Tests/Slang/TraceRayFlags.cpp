#include "Testing/UnitTest.h"

namespace Falcor
{
namespace
{
void testRayFlags(GPUUnitTestContext& ctx, bool useDXR_1_1)
{
    std::vector<uint32_t> expected = {
        (uint32_t)RayFlags::None,
        (uint32_t)RayFlags::ForceOpaque,
        (uint32_t)RayFlags::ForceNonOpaque,
        (uint32_t)RayFlags::AcceptFirstHitAndEndSearch,
        (uint32_t)RayFlags::SkipClosestHitShader,
        (uint32_t)RayFlags::CullBackFacingTriangles,
        (uint32_t)RayFlags::CullFrontFacingTriangles,
        (uint32_t)RayFlags::CullOpaque,
        (uint32_t)RayFlags::CullNonOpaque,
    };

    DefineList defines;
    ShaderModel shaderModel = ShaderModel::SM6_3;

    if (useDXR_1_1)
    {
        expected.push_back((uint32_t)RayFlags::SkipTriangles);
        expected.push_back((uint32_t)RayFlags::SkipProceduralPrimitives);
        defines.add("DXR_1_1");
        shaderModel = ShaderModel::SM6_5;
    }

    ctx.createProgram("Tests/Slang/TraceRayFlags.cs.slang", "testRayFlags", defines, SlangCompilerFlags::None, shaderModel);
    ctx.allocateStructuredBuffer("result", (uint32_t)expected.size());
    ctx.runProgram(1, 1, 1);

    std::vector<uint32_t> result = ctx.readBuffer<uint32_t>("result");
    for (size_t i = 0; i < expected.size(); ++i)
    {
        EXPECT_EQ(result[i], expected[i]);
    }
}
} // namespace

GPU_TEST(TraceRayFlagsDXR1_0)
{
    testRayFlags(ctx, false);
}

GPU_TEST(TraceRayFlagsDXR1_1)
{
    testRayFlags(ctx, true);
}
} // namespace Falcor
