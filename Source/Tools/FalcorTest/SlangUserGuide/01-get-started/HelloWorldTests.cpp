/** Test for SlangUserGuide 01-get-started hello-world.slang.

    Verifies the compute shader correctly adds buffer0[i] + buffer1[i] -> result[i].
*/

#include "Testing/UnitTest.h"

namespace Falcor
{
GPU_TEST(HelloWorldAddBuffers)
{
    const uint32_t nElements = 64;

    std::vector<float> buffer0Data(nElements);
    std::vector<float> buffer1Data(nElements);
    for (uint32_t i = 0; i < nElements; ++i)
    {
        buffer0Data[i] = static_cast<float>(i);
        buffer1Data[i] = static_cast<float>(i * 2);
    }

    ctx.createProgram("SlangUserGuide/01-get-started/hello-world.slang", "computeMain");
    ctx.allocateStructuredBuffer("buffer0", nElements, buffer0Data.data(), nElements * sizeof(float));
    ctx.allocateStructuredBuffer("buffer1", nElements, buffer1Data.data(), nElements * sizeof(float));
    ctx.allocateStructuredBuffer("result", nElements);
    ctx.runProgram(nElements, 1, 1);

    std::vector<float> result = ctx.readBuffer<float>("result");
    EXPECT_EQ(result.size(), nElements);

    for (uint32_t i = 0; i < nElements; ++i)
    {
        float expected = buffer0Data[i] + buffer1Data[i];
        EXPECT_EQ(result[i], expected) << "i = " << i;
    }
}
} // namespace Falcor
