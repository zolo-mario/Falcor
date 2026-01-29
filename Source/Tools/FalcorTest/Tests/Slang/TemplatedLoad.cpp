#include "Testing/UnitTest.h"
#include "Utils/HostDeviceShared.slangh"
#include <random>

namespace Falcor
{
namespace
{
std::mt19937 r;
std::uniform_real_distribution u;

std::vector<uint16_t> generateData(const size_t n)
{
    std::vector<uint16_t> elems;
    for (size_t i = 0; i < n; i++)
        elems.push_back((uint16_t)f32tof16(float(u(r))));
    return elems;
}

void test(GPUUnitTestContext& ctx, const std::string& entryPoint, const size_t n)
{
    ref<Device> pDevice = ctx.getDevice();

    std::vector<uint16_t> elems = generateData(n);

    ctx.createProgram("Tests/Slang/TemplatedLoad.cs.slang", entryPoint, DefineList(), SlangCompilerFlags::None, ShaderModel::SM6_5);
    ctx.allocateStructuredBuffer("result", (uint32_t)elems.size());

    auto var = ctx.vars().getRootVar();
    var["data"] =
        pDevice->createBuffer(elems.size() * sizeof(elems[0]), ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, elems.data());

    ctx.runProgram(1, 1, 1);

    // Verify results.
    std::vector<uint16_t> result = ctx.readBuffer<uint16_t>("result");
    for (size_t i = 0; i < elems.size(); i++)
    {
        EXPECT_EQ(result[i], elems[i]) << "i = " << i;
    }
}
} // namespace

GPU_TEST(TemplatedScalarLoad16)
{
    test(ctx, "testTemplatedScalarLoad16", 20);
}

GPU_TEST(TemplatedVectorLoad16)
{
    test(ctx, "testTemplatedVectorLoad16", 20);
}

GPU_TEST(TemplatedMatrixLoad16_2x4)
{
    test(ctx, "testTemplatedMatrixLoad16_2x4", 8);
}

GPU_TEST(TemplatedMatrixLoad16_4x3)
{
    test(ctx, "testTemplatedMatrixLoad16_4x3", 12);
}
} // namespace Falcor
