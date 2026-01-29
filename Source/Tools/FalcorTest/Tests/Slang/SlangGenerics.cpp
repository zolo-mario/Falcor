#include "Testing/UnitTest.h"

namespace Falcor
{
namespace
{
void runTest(GPUUnitTestContext& ctx, const std::string& entryPoint, DefineList defines)
{
    ref<Device> pDevice = ctx.getDevice();

    ProgramDesc desc;
    desc.addShaderLibrary("Tests/Slang/SlangGenerics.cs.slang").csEntry(entryPoint);
    ctx.createProgram(desc, defines);
    ctx.allocateStructuredBuffer("result", 128);

    // Run program.
    ctx.runProgram(32, 1, 1);

    std::vector<uint32_t> result = ctx.readBuffer<uint32_t>("result");
    for (uint32_t i = 0; i < 32; i++)
    {
        EXPECT_EQ(result[4 * i + 0], (i + 0) * 12);
        EXPECT_EQ(result[4 * i + 1], (i + 1) * 12);
        EXPECT_EQ(result[4 * i + 2], (i + 2) * 12);
        EXPECT_EQ(result[4 * i + 3], (i + 3) * 12);
    }
}
} // namespace

GPU_TEST(Slang_GenericsInterface_Int)
{
    runTest(ctx, "testGenericsInterface", DefineList{{"TEST_A", "1"}, {"USE_INT", "1"}});
}

GPU_TEST(Slang_GenericsInterface_UInt)
{
    runTest(ctx, "testGenericsInterface", DefineList{{"TEST_A", "1"}});
}

GPU_TEST(Slang_GenericsFunction_Int)
{
    runTest(ctx, "testGenericsFunction", DefineList{{"TEST_B", "1"}, {"USE_INT", "1"}});
}

GPU_TEST(Slang_GenericsFunction_UInt)
{
    runTest(ctx, "testGenericsFunction", DefineList{{"TEST_B", "1"}});
}
} // namespace Falcor
