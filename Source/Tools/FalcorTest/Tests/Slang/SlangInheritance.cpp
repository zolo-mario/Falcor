#include "Testing/UnitTest.h"
#include "SlangInheritance.cs.slang"

namespace Falcor
{
GPU_TEST(SlangStructInheritanceReflection, "Not working yet")
{
    ctx.createProgram("Tests/Slang/SlangInheritance.cs.slang", "main", DefineList(), SlangCompilerFlags::None, ShaderModel::SM6_5);

    // Reflection of struct A.
    auto typeA = ctx.getProgram()->getReflector()->findType("A");
    EXPECT(typeA != nullptr);
    if (typeA)
    {
        EXPECT_EQ(typeA->getByteSize(), 4);

        auto scalarA = typeA->findMember("scalar");
        EXPECT(scalarA != nullptr);
        if (scalarA)
        {
            EXPECT_EQ(scalarA->getByteOffset(), 0);
            EXPECT_EQ(scalarA->getType()->getByteSize(), 4);
        }
    }

    // Reflection of struct B inheriting from A
    // Expect A's members to be placed first before B's.
    auto typeB = ctx.getProgram()->getReflector()->findType("B");
    EXPECT(typeB != nullptr);
    if (typeB)
    {
        EXPECT_EQ(typeB->getByteSize(), 16);

        auto scalarB = typeB->findMember("scalar");
        EXPECT(scalarB != nullptr);
        if (scalarB)
        {
            EXPECT_EQ(scalarB->getByteOffset(), 0);
            EXPECT_EQ(scalarB->getType()->getByteSize(), 4);
        }

        auto vectorB = typeB->findMember("vector");
        EXPECT(vectorB != nullptr);
        if (vectorB)
        {
            EXPECT_EQ(vectorB->getByteOffset(), 4);
            EXPECT_EQ(vectorB->getType()->getByteSize(), 12);
        }
    }
}

GPU_TEST(SlangStructInheritanceLayout)
{
    ref<Device> pDevice = ctx.getDevice();

    ctx.createProgram("Tests/Slang/SlangInheritance.cs.slang", "main", DefineList(), SlangCompilerFlags::None, ShaderModel::SM6_5);
    ShaderVar var = ctx.vars().getRootVar();

    // TODO: Use built-in buffer when reflection of struct inheritance works (see #1306).
    // ctx.allocateStructuredBuffer("result", 1);
    auto pResult = pDevice->createStructuredBuffer(
        16, 1, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess, MemoryType::DeviceLocal, nullptr, false
    );
    var["result"] = pResult;

    std::vector<uint32_t> initData(4);
    initData[0] = 59941431;
    initData[1] = asuint(3.13f);
    initData[2] = asuint(5.11f);
    initData[3] = asuint(7.99f);

    var["data"] = pDevice->createTypedBuffer<uint>(
        (uint32_t)initData.size(), ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, initData.data()
    );

    ctx.runProgram();

    // Check struct layout on the host.
    // Expect A's members to be placed first before B's.
    EXPECT_EQ(sizeof(B), 16);
    EXPECT_EQ(sizeof(B::scalar), 4);
    EXPECT_EQ(sizeof(B::vector), 12);
    EXPECT_EQ(offsetof(B, scalar), 0);
    EXPECT_EQ(offsetof(B, vector), 4);

    std::vector<uint32_t> result = pResult->getElements<uint32_t>();

    // Check struct fields read back from the GPU.
    // Slang uses the same struct layout as the host.
    EXPECT_EQ(result[0], initData[0]);
    EXPECT_EQ(result[1], initData[1]);
    EXPECT_EQ(result[2], initData[2]);
    EXPECT_EQ(result[3], initData[3]);
}
} // namespace Falcor
