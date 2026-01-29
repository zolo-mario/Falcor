#include "Testing/UnitTest.h"

namespace Falcor
{
/** GPU test for builtin constant buffer using cbuffer syntax.
 */
GPU_TEST(BuiltinConstantBuffer1)
{
    ctx.createProgram("Tests/Core/ConstantBufferTests.cs.slang", "testCbuffer1");
    ctx.allocateStructuredBuffer("result", 3);
    ctx["CB"]["params1"]["a"] = 1;
    ctx["CB"]["params1"]["b"] = 3u;
    ctx["CB"]["params1"]["c"] = 5.5f;
    ctx.runProgram(1, 1, 1);

    std::vector<float> result = ctx.readBuffer<float>("result");
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[1], 3);
    EXPECT_EQ(result[2], 5.5f);
}

/** GPU test for builtin constant buffer using ConstantBuffer<> syntax.
 */
GPU_TEST(BuiltinConstantBuffer2)
{
    ctx.createProgram("Tests/Core/ConstantBufferTests.cs.slang", "testCbuffer2");
    ctx.allocateStructuredBuffer("result", 3);
    ctx["params2"]["a"] = 1;
    ctx["params2"]["b"] = 3u;
    ctx["params2"]["c"] = 5.5f;
    ctx.runProgram(1, 1, 1);

    std::vector<float> result = ctx.readBuffer<float>("result");
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[1], 3);
    EXPECT_EQ(result[2], 5.5f);
}
} // namespace Falcor
