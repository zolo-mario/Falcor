#include "Testing/UnitTest.h"

namespace Falcor
{
GPU_TEST(Slang_Extension)
{
    ref<Device> pDevice = ctx.getDevice();

    ProgramDesc desc;
    desc.addShaderLibrary("Tests/Slang/SlangExtension.cs.slang").csEntry("main");
    ctx.createProgram(desc, DefineList());
    ctx.allocateStructuredBuffer("result", 6);

    // Run program.
    ctx.runProgram(1, 1, 1);

    std::vector<uint32_t> result = ctx.readBuffer<uint32_t>("result");
    for (uint32_t i = 0; i < 6; i++)
    {
        EXPECT_EQ(result[i], 2u);
    }
}
} // namespace Falcor
