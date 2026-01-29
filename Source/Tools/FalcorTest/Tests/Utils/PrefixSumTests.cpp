#include "Testing/UnitTest.h"
#include "Utils/Algorithm/PrefixSum.h"
#include <random>

namespace Falcor
{
namespace
{
uint32_t prefixSumRef(std::vector<uint32_t>& elems)
{
    // Perform exclusive scan. Return sum of all elements.
    uint32_t sum = 0;
    for (auto& it : elems)
    {
        uint32_t tmp = it;
        it = sum;
        sum += tmp;
    }
    return sum;
}

void testPrefixSum(GPUUnitTestContext& ctx, PrefixSum& prefixSum, uint32_t numElems)
{
    ref<Device> pDevice = ctx.getDevice();

    // Create a buffer of random data to use as test data.
    // We make sure the total sum fits in 32 bits.
    FALCOR_ASSERT(numElems > 0);
    const uint32_t maxVal = std::numeric_limits<uint32_t>::max() / numElems;
    std::vector<uint32_t> testData(numElems);
    std::mt19937 r;
    for (auto& it : testData)
        it = r() % maxVal;

    ref<Buffer> pTestDataBuffer =
        pDevice->createBuffer(numElems * sizeof(uint32_t), ResourceBindFlags::UnorderedAccess, MemoryType::DeviceLocal, testData.data());

    // Allocate buffer for the total sum on the GPU.
    uint32_t nullValue = 0;
    ref<Buffer> pSumBuffer =
        pDevice->createBuffer(sizeof(uint32_t), ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, &nullValue);

    // Execute prefix sum on the GPU.
    uint32_t sum = 0;
    prefixSum.execute(ctx.getRenderContext(), pTestDataBuffer, numElems, &sum, pSumBuffer, 0);

    // Compute prefix sum on the CPU for comparison.
    const uint32_t refSum = prefixSumRef(testData);

    // Compare results.
    EXPECT_EQ(sum, refSum);

    uint32_t resultSum = pSumBuffer->getElement<uint32_t>(0);
    EXPECT_EQ(resultSum, refSum);

    std::vector<uint32_t> result = pTestDataBuffer->getElements<uint32_t>();
    for (uint32_t i = 0; i < numElems; i++)
    {
        EXPECT_EQ(testData[i], result[i]) << "i = " << i;
    }
}
} // namespace

GPU_TEST(PrefixSum)
{
    // Quick test of our reference function.
    std::vector<uint32_t> x({5, 17, 2, 9, 23});
    uint32_t sum = prefixSumRef(x);
    FALCOR_ASSERT(x[0] == 0 && x[1] == 5 && x[2] == 22 && x[3] == 24 && x[4] == 33);
    FALCOR_ASSERT(sum == 56);

    // Create helper class.
    PrefixSum prefixSum(ctx.getDevice());

    // Test prefix sums on varying size buffers.
    testPrefixSum(ctx, prefixSum, 1);
    testPrefixSum(ctx, prefixSum, 27);
    testPrefixSum(ctx, prefixSum, 64);
    testPrefixSum(ctx, prefixSum, 2049);
    testPrefixSum(ctx, prefixSum, 10201);
    testPrefixSum(ctx, prefixSum, 231917);
    testPrefixSum(ctx, prefixSum, 1088921);
    testPrefixSum(ctx, prefixSum, 13912615);
}
} // namespace Falcor
