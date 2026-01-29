#include "Testing/UnitTest.h"

namespace Falcor
{
GPU_TEST(BufferAliasing_Read)
{
    ref<Device> pDevice = ctx.getDevice();

    const size_t N = 32;

    std::vector<float> initData(N);
    for (size_t i = 0; i < initData.size(); i++)
        initData[i] = (float)i;
    auto pBuffer =
        pDevice->createBuffer(initData.size() * sizeof(float), ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, initData.data());

    ctx.createProgram("Tests/Core/ResourceAliasing.cs.slang", "testRead");
    ctx.allocateStructuredBuffer("result", N * 3);

    // Bind buffer to two separate vars to test resource aliasing.
    ctx["bufA1"] = pBuffer;
    ctx["bufA2"] = pBuffer;
    ctx["bufA3"] = pBuffer;

    ctx.runProgram(N, 1, 1);

    std::vector<float> result = ctx.readBuffer<float>("result");
    for (size_t i = 0; i < N; i++)
    {
        EXPECT_EQ(result[i], (float)i) << "i = " << i;
        EXPECT_EQ(result[i + N], (float)i) << "i = " << i;
        EXPECT_EQ(result[i + 2 * N], (float)i) << "i = " << i;
    }
}

GPU_TEST(BufferAliasing_ReadWrite)
{
    ref<Device> pDevice = ctx.getDevice();

    const size_t N = 32;

    std::vector<float> initData(N * 3);
    for (size_t i = 0; i < initData.size(); i++)
        initData[i] = (float)i;
    auto pBuffer = pDevice->createBuffer(
        initData.size() * sizeof(float),
        ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
        MemoryType::DeviceLocal,
        initData.data()
    );

    ctx.createProgram("Tests/Core/ResourceAliasing.cs.slang", "testReadWrite");

    // Bind buffer to two separate vars to test resource aliasing.
    ctx["bufB1"] = pBuffer;
    ctx["bufB2"] = pBuffer;
    ctx["bufB3"] = pBuffer;

    ctx.runProgram(N, 1, 1);

    std::vector<float> result = pBuffer->getElements<float>();
    for (size_t i = 0; i < N; i++)
    {
        EXPECT_EQ(result[i], (float)(N - i)) << "i = " << i;
        EXPECT_EQ(result[i + N], (float)(N - i)) << "i = " << i;
        EXPECT_EQ(result[i + 2 * N], (float)(N - i)) << "i = " << i;
    }
}

GPU_TEST(BufferAliasing_StructRead, "Disabled because <uint> version fails")
{
    ref<Device> pDevice = ctx.getDevice();

    const size_t N = 32;

    std::vector<float> initData(N);
    for (size_t i = 0; i < initData.size(); i++)
        initData[i] = (float)i;
    auto pBuffer = pDevice->createStructuredBuffer(
        initData.size() * sizeof(float), 1, ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, initData.data(), false
    );

    ctx.createProgram("Tests/Core/ResourceAliasing.cs.slang", "testStructRead");
    ctx.allocateStructuredBuffer("result", N * 3);

    // Bind buffer to three separate vars to test resource aliasing.
    ctx["bufStruct1"] = pBuffer;
    ctx["bufStruct2"] = pBuffer;
    ctx["bufStruct3"] = pBuffer;

    ctx.runProgram(N, 1, 1);

    std::vector<float> result = ctx.readBuffer<float>("result");
    for (size_t i = 0; i < N; i++)
    {
        EXPECT_EQ(result[i], (float)i) << "i = " << i;
        EXPECT_EQ(result[i + N], (float)i) << "i = " << i;
        EXPECT_EQ(result[i + 2 * N], (float)i) << "i = " << i;
    }
}
} // namespace Falcor
