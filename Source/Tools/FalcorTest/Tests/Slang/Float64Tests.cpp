#include "Testing/UnitTest.h"
#include "Utils/HostDeviceShared.slangh"
#include <fstd/bit.h> // TODO C++20: Replace with <bit>
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

void test(GPUUnitTestContext& ctx, ShaderModel shaderModel, bool useUav)
{
    ref<Device> pDevice = ctx.getDevice();

    DefineList defines = {{"USE_UAV", useUav ? "1" : "0"}};

    ctx.createProgram("Tests/Slang/Float64Tests.cs.slang", "testFloat64", defines, SlangCompilerFlags::None, shaderModel);
    ctx.allocateStructuredBuffer("result", kNumElems);

    std::vector<uint64_t> elems(kNumElems);
    for (auto& v : elems)
        v = fstd::bit_cast<uint64_t>(u(r));
    auto var = ctx.vars().getRootVar();
    auto pBuf = pDevice->createStructuredBuffer(
        var["data"],
        kNumElems,
        ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
        MemoryType::DeviceLocal,
        elems.data()
    );
    var["data"] = pBuf;

    ctx.runProgram(kNumElems, 1, 1);

    // Verify results.
    std::vector<uint64_t> result = ctx.readBuffer<uint64_t>("result");
    for (uint32_t i = 0; i < kNumElems; i++)
    {
        EXPECT_EQ(result[i], elems[i]) << "i = " << i << " shaderModel=" << enumToString(shaderModel);
    }
}
} // namespace

GPU_TEST(StructuredBufferLoadFloat64)
{
    for (auto sm : kShaderModels)
        test(ctx, sm, false);
}

GPU_TEST(RWStructuredBufferLoadFloat64)
{
    for (auto sm : kShaderModels)
        test(ctx, sm, true);
}
} // namespace Falcor
