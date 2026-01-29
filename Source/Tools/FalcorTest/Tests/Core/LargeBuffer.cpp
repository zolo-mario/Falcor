#include "Testing/UnitTest.h"
#include <random>

namespace Falcor
{
namespace
{
std::mt19937 r;

/** Test copying memory to/from the end of a large buffer.
 */
void testCopyRegion(GPUUnitTestContext& ctx, size_t bufferSize)
{
    ref<Device> pDevice = ctx.getDevice();

    std::vector<uint32_t> data(256);
    const size_t testSize = data.size() * sizeof(data[0]);

    // Initialize small buffers with known data.
    for (size_t i = 0; i < data.size(); i++)
        data[i] = 0xcdcdcdcd;
    auto pDefaultData = pDevice->createBuffer(testSize, ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, data.data());
    for (size_t i = 0; i < data.size(); i++)
        data[i] = r();
    auto pTestData = pDevice->createBuffer(testSize, ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, data.data());
    auto pReadback = pDevice->createBuffer(testSize, ResourceBindFlags::None, MemoryType::ReadBack, nullptr);

    // Create large buffer.
    auto pBuffer = pDevice->createBuffer(bufferSize, ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, nullptr);
    EXPECT(pBuffer);
    EXPECT_EQ(bufferSize, pBuffer->getSize());

    // Default initialize the end of the large buffer.
    const uint64_t dstOffset = pBuffer->getSize() - testSize;
    ctx.getRenderContext()->copyBufferRegion(pBuffer.get(), dstOffset, pDefaultData.get(), 0ull, testSize);
    ctx.getRenderContext()->submit(true); // For safety's sake

    // Copy the test data into the end of the large buffer.
    ctx.getRenderContext()->copyBufferRegion(pBuffer.get(), dstOffset, pTestData.get(), 0ull, testSize);
    ctx.getRenderContext()->submit(true); // For safety's sake

    // For >4GB buffers, also default initialize at the destination offset cast to 32-bit *after* the copy above.
    // This is to make sure that copyBufferRegion() aren't actually truncating the offset internally.
    if (dstOffset + testSize > (1ull << 32))
    {
        ctx.getRenderContext()->copyBufferRegion(pBuffer.get(), (uint32_t)dstOffset, pDefaultData.get(), 0ull, testSize);
        ctx.getRenderContext()->submit(true); // For safety's sake
    }

    // Copy the end of the large buffer into a readback buffer.
    ctx.getRenderContext()->copyBufferRegion(pReadback.get(), 0ull, pBuffer.get(), dstOffset, testSize);

    // Flush and wait for the result.
    ctx.getRenderContext()->submit(true);

    // Check the result.
    const uint32_t* result = static_cast<const uint32_t*>(pReadback->map());
    for (size_t i = 0; i < data.size(); i++)
    {
        EXPECT_EQ(result[i], data[i]) << "i = " << i;
    }
    pReadback->unmap();
}

/** Test reading from the end of a large raw buffer.
 */
void testReadRaw(GPUUnitTestContext& ctx, bool useRootDesc, size_t bufferSize)
{
    ref<Device> pDevice = ctx.getDevice();

    DefineList defines;
    defines.add("USE_ROOT_DESC", useRootDesc ? "1" : "0");

    size_t elemCount = bufferSize / sizeof(uint32_t);
    FALCOR_ASSERT(elemCount <= std::numeric_limits<uint32_t>::max());

    // Initialize small buffers with known data.
    std::vector<uint32_t> data(256);
    const size_t testSize = data.size() * sizeof(data[0]);
    for (size_t i = 0; i < data.size(); i++)
        data[i] = 0xcdcdcdcd;
    auto pDefaultData = pDevice->createBuffer(testSize, ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, data.data());
    for (size_t i = 0; i < data.size(); i++)
        data[i] = r();
    auto pTestData = pDevice->createBuffer(testSize, ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, data.data());

    // Create large buffer.
    auto pBuffer = pDevice->createBuffer(bufferSize, ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, nullptr);
    EXPECT(pBuffer);

    // Copy the test data into the end of the large buffer.
    const uint64_t dstOffset = pBuffer->getSize() - testSize;
    ctx.getRenderContext()->copyBufferRegion(pBuffer.get(), dstOffset, pTestData.get(), 0ull, testSize);
    ctx.getRenderContext()->submit(true); // For safety's sake

    // For >4GB buffers, also default initialize at the destination offset cast to 32-bit *after* the copy above.
    if (dstOffset + testSize > (1ull << 32))
    {
        ctx.getRenderContext()->copyBufferRegion(pBuffer.get(), (uint32_t)dstOffset, pDefaultData.get(), 0ull, testSize);
        ctx.getRenderContext()->submit(true); // For safety's sake
    }

    // Run compute program to read from the large buffer.
    ctx.createProgram("Tests/Core/LargeBuffer.cs.slang", "testReadRaw", defines);
    ctx.allocateStructuredBuffer("result", 256);
    auto var = ctx.vars().getRootVar();
    var["buffer"] = pBuffer;
    var["CB"]["elemCount"] = (uint32_t)elemCount;
    ctx.runProgram(256, 1, 1);

    // Check the result.
    std::vector<uint32_t> result = ctx.readBuffer<uint32_t>("result");
    for (size_t i = 0; i < data.size(); i++)
    {
        EXPECT_EQ(result[i], data[i]) << "i = " << i;
    }
}

/** Test reading from the end of a large structured buffer (stride 16B).
 */
void testReadStructured(GPUUnitTestContext& ctx, bool useRootDesc, size_t bufferSize)
{
    ref<Device> pDevice = ctx.getDevice();

    DefineList defines;
    defines.add("USE_ROOT_DESC", useRootDesc ? "1" : "0");

    size_t elemCount = bufferSize / sizeof(uint4);
    FALCOR_ASSERT(elemCount <= std::numeric_limits<uint32_t>::max());

    // Initialize small buffers with known data.
    std::vector<uint4> data(256);
    const size_t testSize = data.size() * sizeof(data[0]);
    for (size_t i = 0; i < data.size(); i++)
        data[i] = uint4(0xcdcdcdcd);
    auto pDefaultData = pDevice->createBuffer(testSize, ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, data.data());
    for (size_t i = 0; i < data.size(); i++)
        data[i] = uint4(r());
    auto pTestData = pDevice->createBuffer(testSize, ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, data.data());

    // Create large buffer.
    auto pBuffer = pDevice->createStructuredBuffer(
        sizeof(uint4), (uint32_t)elemCount, ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, nullptr, false
    );
    EXPECT(pBuffer);

    // Copy the test data into the end of the large buffer.
    const uint64_t dstOffset = pBuffer->getSize() - testSize;
    ctx.getRenderContext()->copyBufferRegion(pBuffer.get(), dstOffset, pTestData.get(), 0ull, testSize);
    ctx.getRenderContext()->submit(true); // For safety's sake

    // For >4GB buffers, also default initialize at the destination offset cast to 32-bit *after* the copy above.
    if (dstOffset + testSize > (1ull << 32))
    {
        ctx.getRenderContext()->copyBufferRegion(pBuffer.get(), (uint32_t)dstOffset, pDefaultData.get(), 0ull, testSize);
        ctx.getRenderContext()->submit(true); // For safety's sake
    }

    // Run compute program to read from the large buffer.
    ctx.createProgram("Tests/Core/LargeBuffer.cs.slang", "testReadStructured", defines);
    ctx.allocateStructuredBuffer("result", 256);
    auto var = ctx.vars().getRootVar();
    var["structuredBuffer"] = pBuffer;
    var["CB"]["elemCount"] = (uint32_t)elemCount;
    ctx.runProgram(256, 1, 1);

    // Check the result.
    std::vector<uint32_t> result = ctx.readBuffer<uint32_t>("result");
    for (size_t i = 0; i < data.size(); i++)
    {
        EXPECT_EQ(result[i], data[i].x) << "i = " << i;
    }
}

/** Test reading from the end of a large structured buffer (stride 4B).
 */
void testReadStructuredUint(GPUUnitTestContext& ctx, bool useRootDesc, size_t bufferSize)
{
    ref<Device> pDevice = ctx.getDevice();

    DefineList defines;
    defines.add("USE_ROOT_DESC", useRootDesc ? "1" : "0");

    size_t elemCount = bufferSize / sizeof(uint32_t);
    FALCOR_ASSERT(elemCount <= std::numeric_limits<uint32_t>::max());

    // Initialize small buffers with known data.
    std::vector<uint32_t> data(256);
    const size_t testSize = data.size() * sizeof(data[0]);
    for (size_t i = 0; i < data.size(); i++)
        data[i] = 0xcdcdcdcd;
    auto pDefaultData = pDevice->createBuffer(testSize, ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, data.data());
    for (size_t i = 0; i < data.size(); i++)
        data[i] = r();
    auto pTestData = pDevice->createBuffer(testSize, ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, data.data());

    // Create large buffer.
    auto pBuffer = pDevice->createStructuredBuffer(
        sizeof(uint32_t), (uint32_t)elemCount, ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, nullptr, false
    );
    EXPECT(pBuffer);

    // Copy the test data into the end of the large buffer.
    const uint64_t dstOffset = pBuffer->getSize() - testSize;
    ctx.getRenderContext()->copyBufferRegion(pBuffer.get(), dstOffset, pTestData.get(), 0ull, testSize);
    ctx.getRenderContext()->submit(true); // For safety's sake

    // For >4GB buffers, also default initialize at the destination offset cast to 32-bit *after* the copy above.
    if (dstOffset + testSize > (1ull << 32))
    {
        ctx.getRenderContext()->copyBufferRegion(pBuffer.get(), (uint32_t)dstOffset, pDefaultData.get(), 0ull, testSize);
        ctx.getRenderContext()->submit(true); // For safety's sake
    }

    // Run compute program to read from the large buffer.
    ctx.createProgram("Tests/Core/LargeBuffer.cs.slang", "testReadStructuredUint", defines);
    ctx.allocateStructuredBuffer("result", 256);
    auto var = ctx.vars().getRootVar();
    var["structuredBufferUint"] = pBuffer;
    var["CB"]["elemCount"] = (uint32_t)elemCount;
    ctx.runProgram(256, 1, 1);

    // Check the result.
    std::vector<uint32_t> result = ctx.readBuffer<uint32_t>("result");
    for (size_t i = 0; i < data.size(); i++)
    {
        EXPECT_EQ(result[i], data[i]) << "i = " << i;
    }
}
} // namespace

/** Tests copying a memory region into the high addresses of a GPU buffer.
    The data is then copied into a staging buffer and mapped to the CPU.

    The copy operations work with 64-bit addresses so should theoretically
    support >4GB buffers, but that does not currently seem to be the case.
*/

GPU_TEST(LargeBufferCopyRegion1)
{
    testCopyRegion(ctx, 3ull << 30); // 3GB
}

GPU_TEST(LargeBufferCopyRegion2)
{
    testCopyRegion(ctx, 4ull << 30); // 4GB
}

GPU_TEST(LargeBufferCopyRegion3, "Disabled due to 4GB buffer limit")
{
    testCopyRegion(ctx, 5ull << 30); // 5GB
}

/** Tests reading from raw buffer bound as root descriptor.
    Raw buffers are addressed using a 32-bit offset so cannot exceed 4GB.
*/

GPU_TEST(LargeBufferReadRawRoot1)
{
    testReadRaw(ctx, true, 3ull << 30); // 3GB
}

// Enabled for D3D12 only since Vulkan doesn't support buffers larger than 2^32-1.
GPU_TEST(LargeBufferReadRawRoot2, Device::Type::D3D12)
{
    testReadRaw(ctx, true, 4ull << 30); // 4GB
}

// Enabled for D3D12 only since Vulkan doesn't support buffers larger than 2^32-1.
GPU_TEST(LargeBufferReadRawRoot3, Device::Type::D3D12, "Disabled due to 4GB buffer limit")
{
    testReadRaw(ctx, true, 5ull << 30); // 5GB
}

/** Tests reading from structured buffer bound as root descriptor.

    Structured buffers are addressed by index so should theoretically
    support >4GB buffers, but that does not currently seem to be the case.
*/

GPU_TEST(LargeBufferReadStructuredRoot1)
{
    testReadStructured(ctx, true, 3ull << 30); // 3GB
}

// Enabled for D3D12 only since Vulkan doesn't support buffers larger than 2^32-1.
GPU_TEST(LargeBufferReadStructuredRoot2, Device::Type::D3D12)
{
    testReadStructured(ctx, true, 4ull << 30); // 4GB
}

// Enabled for D3D12 only since Vulkan doesn't support buffers larger than 2^32-1.
GPU_TEST(LargeBufferReadStructuredRoot3, Device::Type::D3D12, "Disabled due to 4GB buffer limit")
{
    testReadStructured(ctx, true, 5ull << 30); // 5GB
}

/** Tests reading from raw buffer bound as shader resource view.
    Raw buffers are addressed using a 32-bit offset so cannot exceed 4GB.
    SRVs have additional restrictions on the size.

    Last, it seems that reading from 32-bit buffers bound as SRV from
    addresses >2GB gives unexpected results for both raw and structured buffers.
*/

GPU_TEST(LargeBufferReadRawSRV1)
{
    testReadRaw(ctx, false, 2ull << 30); // 2GB
}

GPU_TEST(LargeBufferReadRawSRV2, "Disabled due to 2GB limit on raw buffer SRVs")
{
    testReadRaw(ctx, false, 3ull << 30); // 3GB
}

GPU_TEST(LargeBufferReadRawSRV3, "Disabled due to 2GB limit on raw buffer SRVs")
{
    testReadRaw(ctx, false, (4ull << 30) - 1024); // almost 4GB
}

/** Tests reading from structured buffer bound as shader resource view.
    SRVs have restrictions on the size.
*/

GPU_TEST(LargeBufferReadStructuredSRV1)
{
    testReadStructured(ctx, false, 2ull << 30); // 2GB
}

GPU_TEST(LargeBufferReadStructuredSRV2)
{
    testReadStructured(ctx, false, 3ull << 30); // 3GB
}

GPU_TEST(LargeBufferReadStructuredSRV3)
{
    testReadStructured(ctx, false, (4ull << 30) - 1024); // almost 4GB
}

/** Tests reading from 32-bit structured buffer bound as shader resource view.
    SRVs have restrictions on the size.
*/

GPU_TEST(LargeBufferReadStructuredUintSRV1)
{
    testReadStructuredUint(ctx, false, 2ull << 30); // 2GB
}

GPU_TEST(LargeBufferReadStructuredUintSRV2, "Disabled due to 2GB limit on uint buffer SRVs")
{
    testReadStructuredUint(ctx, false, 3ull << 30); // 3GB
}

GPU_TEST(LargeBufferReadStructuredUintSRV3, "Disabled due to 2GB limit on uint buffer SRVs")
{
    testReadStructuredUint(ctx, false, (4ull << 30) - 1024); // almost 4GB
}
} // namespace Falcor
