#include "Testing/UnitTest.h"

namespace Falcor
{
/** Minimal GPU test for constant buffer in ParameterBlock.
 */
GPU_TEST(ParamBlockCB)
{
    ref<Device> pDevice = ctx.getDevice();

    ctx.createProgram("Tests/Core/ParamBlockCB.cs.slang", "main");
    ctx.allocateStructuredBuffer("result", 1);

    auto pBlockReflection = ctx.getProgram()->getReflector()->getParameterBlock("gParamBlock");
    auto pParamBlock = ParameterBlock::create(pDevice, pBlockReflection);
    pParamBlock->getRootVar()["a"] = 42.1f;

    ctx["gParamBlock"] = pParamBlock;
    ctx.runProgram(1, 1, 1);

    std::vector<float> result = ctx.readBuffer<float>("result");
    EXPECT_EQ(result[0], 42.1f);
}
} // namespace Falcor
