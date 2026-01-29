#include "Testing/UnitTest.h"
#include "Utils/Algorithm/BitonicSort.h"
#include <random>

namespace Falcor
{
namespace
{
// Sort the 'data' array in ascending order within chunks of 'chunkSize' elements.
void bitonicSortRef(std::vector<uint32_t>& data, const uint32_t chunkSize)
{
    if (chunkSize <= 1)
        return;
    for (size_t first = 0; first < data.size(); first += chunkSize)
    {
        size_t last = std::min(first + chunkSize, data.size());
        std::sort(data.begin() + first, data.begin() + last);
    }
}

void testGpuSort(GPUUnitTestContext& ctx, BitonicSort& bitonicSort, const uint32_t n, const uint32_t chunkSize)
{
    ref<Device> pDevice = ctx.getDevice();

    // Create a buffer of random data to use as test data.
    std::vector<uint32_t> testData(n);
    std::mt19937 r;
    for (auto& it : testData)
        it = r();

    ref<Buffer> pTestDataBuffer =
        pDevice->createBuffer(n * sizeof(uint32_t), ResourceBindFlags::UnorderedAccess, MemoryType::DeviceLocal, testData.data());

    // Execute sort on the GPU.
    uint32_t groupSize = std::max(chunkSize, 256u);
    bool retval = bitonicSort.execute(ctx.getRenderContext(), pTestDataBuffer, n, chunkSize, groupSize);
    EXPECT_EQ(retval, true);

    // Sort the test data on the CPU for comparison.
    bitonicSortRef(testData, chunkSize);

    // Compare results.
    std::vector<uint32_t> result = pTestDataBuffer->getElements<uint32_t>(0);
    for (uint32_t i = 0; i < n; i++)
    {
        EXPECT_EQ(testData[i], result[i]) << "i = " << i;
    }
}
} // namespace

#if FALCOR_NVAPI_AVAILABLE
GPU_TEST(BitonicSort, Device::Type::D3D12)
#else
GPU_TEST(BitonicSort, Device::Type::D3D12, "Requires NVAPI")
#endif
{
    // Create utility class for sorting.
    BitonicSort bitonicSort(ctx.getDevice());

    // Test different parameters.
    // The chunk size(last param) must  be a pow-of-two <= 1024.
    testGpuSort(ctx, bitonicSort, 100, 1);
    testGpuSort(ctx, bitonicSort, 19, 2);
    testGpuSort(ctx, bitonicSort, 1024, 4);
    testGpuSort(ctx, bitonicSort, 11025, 8);
    testGpuSort(ctx, bitonicSort, 290, 16);
    testGpuSort(ctx, bitonicSort, 1500, 32);
    testGpuSort(ctx, bitonicSort, 20000, 64);
    testGpuSort(ctx, bitonicSort, 2001, 128);
    testGpuSort(ctx, bitonicSort, 16384, 256);
    testGpuSort(ctx, bitonicSort, 3103, 1024);
}
} // namespace Falcor
