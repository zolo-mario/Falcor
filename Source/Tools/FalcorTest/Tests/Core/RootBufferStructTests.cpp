#include "Testing/UnitTest.h"
#include <random>

namespace Falcor
{
namespace
{
const uint32_t kNumElems = 256;
const std::string kRootBufferName = "rootBuf";

std::mt19937 rng;
auto dist = std::uniform_int_distribution<uint32_t>(0, 100);

void testRootBufferInStruct(GPUUnitTestContext& ctx, ShaderModel shaderModel, bool useUav)
{
    ref<Device> pDevice = ctx.getDevice();

    auto nextRandom = [&]() -> uint32_t { return dist(rng); };

    DefineList defines = {{"USE_UAV", useUav ? "1" : "0"}};
    SlangCompilerFlags compilerFlags = SlangCompilerFlags::None;

    ctx.createProgram("Tests/Core/RootBufferStructTests.cs.slang", "main", defines, compilerFlags, shaderModel);
    ctx.allocateStructuredBuffer("result", kNumElems);

    auto data = ctx.vars().getRootVar()["CB"]["data"];

    // Bind some regular buffers.
    std::vector<uint32_t> buf(kNumElems);
    {
        for (uint32_t i = 0; i < kNumElems; i++)
            buf[i] = nextRandom();
        data["buf"] =
            pDevice->createTypedBuffer<uint32_t>(kNumElems, ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, buf.data());
    }
    std::vector<uint32_t> rwBuf(kNumElems);
    {
        for (uint32_t i = 0; i < kNumElems; i++)
            rwBuf[i] = nextRandom();
        data["rwBuf"] =
            pDevice->createTypedBuffer<uint32_t>(kNumElems, ResourceBindFlags::UnorderedAccess, MemoryType::DeviceLocal, rwBuf.data());
    }

    // Test binding structured buffer to root descriptor inside struct in CB.
    std::vector<uint32_t> rootBuf(kNumElems);
    {
        for (uint32_t i = 0; i < kNumElems; i++)
            rootBuf[i] = nextRandom();

        auto pRootBuffer = pDevice->createStructuredBuffer(
            data[kRootBufferName],
            kNumElems,
            useUav ? ResourceBindFlags::UnorderedAccess : ResourceBindFlags::ShaderResource,
            MemoryType::DeviceLocal,
            rootBuf.data(),
            false /* no UAV counter */
        );

        data[kRootBufferName] = pRootBuffer;

        ref<Buffer> pBoundBuffer = data[kRootBufferName];
        EXPECT_EQ(pBoundBuffer, pRootBuffer);
    }

    // Run the program to test that we can access the buffer.
    ctx.runProgram(kNumElems, 1, 1);

    std::vector<uint32_t> result = ctx.readBuffer<uint32_t>("result");
    for (uint32_t i = 0; i < kNumElems; i++)
    {
        uint32_t r = 0;
        r += buf[i];
        r += rwBuf[i] * 2;
        r += rootBuf[i] * 3;
        EXPECT_EQ(result[i], r) << "i = " << i;
    }
}
} // namespace

GPU_TEST(RootBufferStructSRV_6_0)
{
    testRootBufferInStruct(ctx, ShaderModel::SM6_0, false);
}

GPU_TEST(RootBufferStructUAV_6_0)
{
    testRootBufferInStruct(ctx, ShaderModel::SM6_0, true);
}

GPU_TEST(RootBufferStructSRV_6_3)
{
    testRootBufferInStruct(ctx, ShaderModel::SM6_3, false);
}

GPU_TEST(RootBufferStructUAV_6_3)
{
    testRootBufferInStruct(ctx, ShaderModel::SM6_3, true);
}
} // namespace Falcor
