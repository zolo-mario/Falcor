#include "Testing/UnitTest.h"
#include "Utils/HostDeviceShared.slangh"
#include <random>

namespace Falcor
{
namespace
{
std::vector<ShaderModel> kShaderModels = {
    {ShaderModel::SM6_2},
    {ShaderModel::SM6_3},
};

const uint32_t kNumElems = 256;
std::mt19937 r;
std::uniform_real_distribution u;

void test(GPUUnitTestContext& ctx, ShaderModel shaderModel, bool useUav, bool useStructured)
{
    ref<Device> pDevice = ctx.getDevice();

    DefineList defines = {{"USE_UAV", useUav ? "1" : "0"}};

    ctx.createProgram("Tests/Slang/Float16Tests.cs.slang", "testFloat16", defines, SlangCompilerFlags::None, shaderModel);
    ctx.allocateStructuredBuffer("result", kNumElems);

    std::vector<uint16_t> elems(kNumElems);
    for (auto& v : elems)
        v = f32tof16(float(u(r)));
    auto var = ctx.vars().getRootVar();

    if (useStructured)
    {
        auto buf = pDevice->createStructuredBuffer(
            var["data"],
            kNumElems,
            ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
            MemoryType::DeviceLocal,
            elems.data()
        );
        ASSERT_EQ(buf->getStructSize(), sizeof(float16_t));
        ASSERT_EQ(buf->getElementCount(), kNumElems);
        var["data"] = buf;
    }
    else
    {
        auto buf = pDevice->createBuffer(
            kNumElems * sizeof(float16_t),
            ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
            MemoryType::DeviceLocal,
            elems.data()
        );
        ASSERT_EQ(buf->getSize(), kNumElems * sizeof(float16_t));
        var["data"] = buf;
    }

    ctx.runProgram(kNumElems, 1, 1);

    // Verify results.
    std::vector<uint16_t> result = ctx.readBuffer<uint16_t>("result");
    for (uint32_t i = 0; i < kNumElems; i++)
    {
        EXPECT_EQ(result[i], elems[i]) << "i = " << i << " shaderModel=" << enumToString(shaderModel);
    }
}
} // namespace

GPU_TEST(StructuredBuffer_LoadFloat16_Structured)
{
    for (auto sm : kShaderModels)
        test(ctx, sm, false, true);
}

GPU_TEST(StructuredBuffer_LoadFloat16_Raw)
{
    for (auto sm : kShaderModels)
        test(ctx, sm, false, false);
}

GPU_TEST(RWStructuredBuffer_LoadFloat16_Structured)
{
    for (auto sm : kShaderModels)
        test(ctx, sm, true, true);
}

GPU_TEST(RWStructuredBuffer_LoadFloat16_Raw)
{
    for (auto sm : kShaderModels)
        test(ctx, sm, true, false);
}

} // namespace Falcor
