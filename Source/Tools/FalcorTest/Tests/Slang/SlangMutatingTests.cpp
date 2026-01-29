#include "Testing/UnitTest.h"

namespace Falcor
{
GPU_TEST(SlangMutating)
{
    ref<Device> pDevice = ctx.getDevice();

    ctx.createProgram("Tests/Slang/SlangMutatingTests.cs.slang", "main", DefineList(), SlangCompilerFlags::None, ShaderModel::SM6_3);
    ctx.allocateStructuredBuffer("result", 1);

    ShaderVar var = ctx.vars().getRootVar();
    uint4 v = {11, 22, 33, 44};
    var["buffer"] = pDevice->createTypedBuffer<uint4>(1, ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, &v);

    ctx.runProgram();

    std::vector<uint32_t> result = ctx.readBuffer<uint32_t>("result");
    EXPECT_EQ(result[0], 33);
}
} // namespace Falcor
