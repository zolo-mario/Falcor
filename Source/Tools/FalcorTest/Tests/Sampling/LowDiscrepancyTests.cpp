#include "Testing/UnitTest.h"
#include <random>

namespace Falcor
{
// Just check the first four values.
GPU_TEST(RadicalInverse)
{
    ctx.createProgram("Tests/Sampling/LowDiscrepancyTests.cs.slang", "testRadicalInverse");
    ctx.allocateStructuredBuffer("result", 4);
    ctx["TestCB"]["resultSize"] = 4;
    ctx.runProgram();

    std::vector<float> s = ctx.readBuffer<float>("result");
    EXPECT_EQ(s[0], 0.f);
    EXPECT_EQ(s[1], 0.5f);
    EXPECT_EQ(s[2], 0.25f);
    EXPECT_EQ(s[3], 0.75f);
}

} // namespace Falcor
