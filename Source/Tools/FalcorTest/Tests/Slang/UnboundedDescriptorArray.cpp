#include "Testing/UnitTest.h"

namespace Falcor
{
GPU_TEST(UnboundedDescriptorArray, "Unbounded arrays are not yet supported")
{
    ref<Device> pDevice = ctx.getDevice();

    const uint32_t kTexCount = 4;

    ctx.createProgram("Tests/Slang/UnboundedDescriptorArray.cs.slang", "main", DefineList(), SlangCompilerFlags::None, ShaderModel::SM6_5);
    ctx.allocateStructuredBuffer("result", kTexCount);

    auto var = ctx.vars().getRootVar()["resources"];
    for (size_t i = 0; i < kTexCount; i++)
    {
        float initData = (float)(i + 1);
        var["textures"][i] = pDevice->createTexture2D(1, 1, ResourceFormat::R32Float, 1, 1, &initData);
    }

    ctx.runProgram(kTexCount, 1, 1);

    std::vector<float> result = ctx.readBuffer<float>("result");
    for (size_t i = 0; i < kTexCount; i++)
    {
        float expected = (float)(i + 1);
        EXPECT_EQ(result[i], expected) << "i = " << i;
    }
}
} // namespace Falcor
