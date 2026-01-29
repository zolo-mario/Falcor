#include "Testing/UnitTest.h"

namespace Falcor
{
namespace
{
const uint32_t kNumElems = 256;

void test(GPUUnitTestContext& ctx, ShaderModel shaderModel)
{
    ctx.createProgram("Tests/Slang/ShaderModel.cs.slang", "main", DefineList(), SlangCompilerFlags::None, shaderModel);
    ctx.allocateStructuredBuffer("result", kNumElems);
    ctx.runProgram(kNumElems, 1, 1);

    std::vector<uint32_t> result = ctx.readBuffer<uint32_t>("result");
    for (uint32_t i = 0; i < kNumElems; i++)
    {
        EXPECT_EQ(result[i], 3 * i);
    }
}
} // namespace

GPU_TEST(ShaderModel6_0)
{
    test(ctx, ShaderModel::SM6_0);
}

GPU_TEST(ShaderModel6_1)
{
    test(ctx, ShaderModel::SM6_1);
}

GPU_TEST(ShaderModel6_2)
{
    test(ctx, ShaderModel::SM6_2);
}

GPU_TEST(ShaderModel6_3)
{
    test(ctx, ShaderModel::SM6_3);
}

GPU_TEST(ShaderModel6_4)
{
    test(ctx, ShaderModel::SM6_4);
}

GPU_TEST(ShaderModel6_5)
{
    test(ctx, ShaderModel::SM6_5);
}

#if FALCOR_HAS_D3D12_AGILITY_SDK
GPU_TEST(ShaderModel6_6, Device::Type::D3D12)
{
    test(ctx, ShaderModel::SM6_6);
}
#endif
} // namespace Falcor
