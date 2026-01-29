#include "Testing/UnitTest.h"
#include <random>

namespace Falcor
{
namespace
{
// Reference function to interleave m bits from x and y.
// The result is a bit sequence: 0 ... 0 ym xm ... y1 x1 y0 x0.
uint32_t referenceBitInterleave(uint32_t x, uint32_t y, uint32_t m)
{
    uint32_t result = 0;
    for (uint32_t i = 0; i < m; i++)
    {
        result |= ((x >> i) & 1) << (2 * i);
        result |= ((y >> i) & 1) << (2 * i + 1);
    }
    return result;
}
} // namespace

GPU_TEST(BitInterleave)
{
    ref<Device> pDevice = ctx.getDevice();

    const uint32_t tests = 5;
    const uint32_t n = 1 << 16;

    // First test the reference function itself against a manually constructed example.
    EXPECT_EQ(referenceBitInterleave(0xe38e, 0xbe8b, 16), 0xdeadc0de);
    EXPECT_EQ(referenceBitInterleave(0xe38e, 0xbe8b, 12), 0x00adc0de);

    // Create a buffer of random bits to use as test data.
    std::vector<uint32_t> testData(n);
    std::mt19937 r;
    for (auto& it : testData)
        it = r();

    ref<Buffer> pTestDataBuffer =
        pDevice->createBuffer(n * sizeof(uint32_t), ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, testData.data());

    // Setup and run GPU test.
    ctx.createProgram("Tests/Utils/BitTricksTests.cs.slang", "testBitInterleave");
    ctx.allocateStructuredBuffer("result", n * tests);
    ctx["testData"] = pTestDataBuffer;
    ctx.runProgram(n);

    // Verify results.
    std::vector<uint32_t> result = ctx.readBuffer<uint32_t>("result");
    for (uint32_t i = 0; i < n; i++)
    {
        const uint32_t bits = testData[i];
        const uint32_t interleavedBits = referenceBitInterleave(bits, bits >> 16, 16);

        // Check result of interleave functions.
        EXPECT_EQ(result[tests * i + 0], interleavedBits);
        EXPECT_EQ(result[tests * i + 1], (interleavedBits & 0xffff));

        // Check result of de-interleave functions.
        EXPECT_EQ(result[tests * i + 2], (bits & 0x00ff00ff));
        EXPECT_EQ(result[tests * i + 3], (bits & 0x000f000f));
        EXPECT_EQ(result[tests * i + 4], (bits & 0x0f0f0f0f));
    }
}
} // namespace Falcor
