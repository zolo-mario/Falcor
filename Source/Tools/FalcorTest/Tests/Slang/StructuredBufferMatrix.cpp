#include "Falcor.h"
#include "Testing/UnitTest.h"

namespace Falcor
{
namespace
{
void runTest2(GPUUnitTestContext& ctx, DefineList defines)
{
    ref<Device> pDevice = ctx.getDevice();

    ctx.createProgram("Tests/Slang/StructuredBufferMatrix.cs.slang", "testStructuredBufferMatrixLoad2", defines);
    ctx.allocateStructuredBuffer("result", 16);

    auto var = ctx.vars().getRootVar();
    auto pData =
        pDevice->createStructuredBuffer(var["data2"], 1, ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, nullptr, false);

    EXPECT_EQ(pData->getElementCount(), 1);
    EXPECT_EQ(pData->getStructSize(), 32);

    std::vector<float16_t> initData(16);
    for (size_t i = 0; i < 16; i++)
        initData[i] = float16_t((float)i + 0.75f);
    pData->setBlob(initData.data(), 0, 32);

    var["data2"] = pData;

    ctx.runProgram(1, 1, 1);

    // Verify results.
    std::vector<float> result = ctx.readBuffer<float>("result");
    for (size_t i = 0; i < 16; i++)
    {
        EXPECT_EQ(result[i], (float)i + 0.75f) << "i = " << i;
    }
}
} // namespace

GPU_TEST(StructuredBufferMatrixLoad1)
{
    ref<Device> pDevice = ctx.getDevice();

    ctx.createProgram("Tests/Slang/StructuredBufferMatrix.cs.slang", "testStructuredBufferMatrixLoad1");
    ctx.allocateStructuredBuffer("result", 32);

    auto var = ctx.vars().getRootVar();
    auto pData =
        pDevice->createStructuredBuffer(var["data1"], 1, ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, nullptr, false);

    EXPECT_EQ(pData->getElementCount(), 1);
    EXPECT_EQ(pData->getStructSize(), 100);

    std::vector<uint8_t> initData(100);
    for (size_t i = 0; i < 18; i++)
        ((float*)initData.data())[i] = (float)i + 0.5f;
    for (size_t i = 0; i < 14; i++)
        ((float16_t*)(initData.data() + 72))[i] = float16_t((float)i + 18.5f);
    pData->setBlob(initData.data(), 0, 100);

    var["data1"] = pData;

    ctx.runProgram(1, 1, 1);

    // Verify results.
    std::vector<float> result = ctx.readBuffer<float>("result");
    for (size_t i = 0; i < 32; i++)
    {
        EXPECT_EQ(result[i], (float)i + 0.5f) << "i = " << i;
    }
}

GPU_TEST(StructuredBufferMatrixLoad2_1)
{
    DefineList defines = {{"LAYOUT", "1"}};
    runTest2(ctx, defines);
}

// TODO: Enable when https://github.com/microsoft/DirectXShaderCompiler/issues/4492 has been resolved.
GPU_TEST(StructuredBufferMatrixLoad2_2, "Disabled due to compiler bug")
{
    DefineList defines = {{"LAYOUT", "2"}};
    runTest2(ctx, defines);
}

// TODO: Enable when https://github.com/microsoft/DirectXShaderCompiler/issues/4492 has been resolved.
GPU_TEST(StructuredBufferMatrixLoad2_3, "Disabled due to compiler bug")
{
    DefineList defines = {{"LAYOUT", "3"}};
    runTest2(ctx, defines);
}
} // namespace Falcor
