#include "Testing/UnitTest.h"

namespace Falcor
{
namespace
{
const uint32_t kElementCount = 256;

std::vector<uint32_t> getTestData()
{
    std::vector<uint32_t> data(kElementCount);
    for (uint32_t i = 0; i < kElementCount; i++)
        data[i] = i;
    return data;
}

const std::vector<uint32_t> kTestData = getTestData();

/** Create buffer with the given CPU access and elements initialized to 0,1,2,...
 */
ref<Buffer> createTestBuffer(GPUUnitTestContext& ctx, MemoryType memoryType, bool initialize)
{
    return ctx.getDevice()->createBuffer(
        kElementCount * sizeof(uint32_t),
        ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
        memoryType,
        initialize ? kTestData.data() : nullptr
    );
}

void checkData(GPUUnitTestContext& ctx, const uint32_t* pData)
{
    for (uint32_t i = 0; i < kElementCount; i++)
        EXPECT_EQ(pData[i], i) << "i = " << i;
}

void initBufferIndirect(GPUUnitTestContext& ctx, ref<Buffer> pBuffer)
{
    auto pInitData = createTestBuffer(ctx, MemoryType::DeviceLocal, true);
    ctx.getRenderContext()->copyResource(pBuffer.get(), pInitData.get());
    ctx.getRenderContext()->submit(true);
}

std::vector<uint32_t> readBufferIndirect(GPUUnitTestContext& ctx, ref<Buffer> pBuffer)
{
    ref<Buffer> pResult = createTestBuffer(ctx, MemoryType::DeviceLocal, false);
    ctx.getRenderContext()->copyResource(pResult.get(), pBuffer.get());
    ctx.getRenderContext()->submit(true);
    return pResult->getElements<uint32_t>();
}

void checkBufferIndirect(GPUUnitTestContext& ctx, ref<Buffer> pBuffer)
{
    return checkData(ctx, readBufferIndirect(ctx, pBuffer).data());
}

} // namespace

GPU_TEST(BufferDeviceLocalWrite)
{
    // Create without init data, then set data using setBlob().
    {
        ref<Buffer> pBuffer = createTestBuffer(ctx, MemoryType::DeviceLocal, false);
        pBuffer->setBlob(kTestData.data(), 0, kTestData.size() * sizeof(uint32_t));
        checkBufferIndirect(ctx, pBuffer);
    }

    // Create with init data.
    {
        ref<Buffer> pBuffer = createTestBuffer(ctx, MemoryType::DeviceLocal, true);
        checkBufferIndirect(ctx, pBuffer);
    }
}

GPU_TEST(BufferDeviceLocalRead)
{
    ref<Buffer> pBuffer = createTestBuffer(ctx, MemoryType::DeviceLocal, false);
    initBufferIndirect(ctx, pBuffer);

    std::vector<uint32_t> data(kElementCount);
    pBuffer->getBlob(data.data(), 0, kElementCount * sizeof(uint32_t));
    checkData(ctx, data.data());
}

GPU_TEST(BufferUploadWrite)
{
    // Create without init data, then set data using setBlob().
    {
        ref<Buffer> pBuffer = createTestBuffer(ctx, MemoryType::Upload, false);
        pBuffer->setBlob(kTestData.data(), 0, kTestData.size() * sizeof(uint32_t));
        checkBufferIndirect(ctx, pBuffer);
    }

    // Create with init data.
    {
        ref<Buffer> pBuffer = createTestBuffer(ctx, MemoryType::Upload, true);
        checkBufferIndirect(ctx, pBuffer);
    }
}

GPU_TEST(BufferUploadMap)
{
    ref<Buffer> pBuffer = createTestBuffer(ctx, MemoryType::Upload, false);
    uint32_t* pData = reinterpret_cast<uint32_t*>(pBuffer->map());
    for (uint32_t i = 0; i < kElementCount; ++i)
        pData[i] = kTestData[i];
    pBuffer->unmap();
    checkBufferIndirect(ctx, pBuffer);
}

GPU_TEST(BufferReadbackRead)
{
    ref<Buffer> pBuffer = createTestBuffer(ctx, MemoryType::ReadBack, false);
    initBufferIndirect(ctx, pBuffer);

    std::vector<uint32_t> data(kElementCount);
    pBuffer->getBlob(data.data(), 0, kElementCount * sizeof(uint32_t));
    checkData(ctx, data.data());
}

GPU_TEST(BufferReadbackMap)
{
    ref<Buffer> pBuffer = createTestBuffer(ctx, MemoryType::ReadBack, false);
    initBufferIndirect(ctx, pBuffer);

    const uint32_t* pData = reinterpret_cast<const uint32_t*>(pBuffer->map());
    checkData(ctx, pData);
    pBuffer->unmap();
}

} // namespace Falcor
