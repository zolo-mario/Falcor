#include "Testing/UnitTest.h"
#include "Utils/HostDeviceShared.slangh"

#include <limits>
#include <random>

namespace Falcor
{

namespace
{
std::pair<int, float2> getCpuResult(int type, int2 value, float3 data)
{
    switch (type)
    {
    case 0:
        return std::make_pair(value[0] - value[1], float2(data[0] - data[1], -data[2]));
    case 1:
        return std::make_pair(value[0] - value[1] + 1, float2(data[0], data[2]));
    case 2:
        return std::make_pair(value[0] - value[1] + 2, float2(data[0], -data[2]));
    case 3:
        return std::make_pair(value[0] - value[1] + 3, float2(data[0] + data[1], data[2]));
    }
    return std::make_pair(-65537, float2(std::numeric_limits<float>::quiet_NaN()));
}

const uint32_t kNumTests = 16;
std::mt19937 r;
std::uniform_real_distribution uf;
std::uniform_int_distribution ui;

} // namespace

GPU_TEST(Inheritance_ManualCreate)
{
    ref<Device> pDevice = ctx.getDevice();

    DefineList defines;
    defines.add("NUM_TESTS", std::to_string(kNumTests));
    ctx.createProgram(
        "Tests/Slang/InheritanceTests.cs.slang", "testInheritanceManual", defines, SlangCompilerFlags::None, ShaderModel::SM6_5
    );
    ctx.allocateStructuredBuffer("resultsInt", kNumTests);
    ctx.allocateStructuredBuffer("resultsFloat", kNumTests);

    std::vector<int> testType(kNumTests);
    // The first value is value0 in TestInterfaceBase, second is value1 in the inherited classes.
    // This tests that the memory order of base class and inherited class members has not changed with Slang updates.
    std::vector<int2> testValue(kNumTests);
    std::vector<float3> data(kNumTests);

    for (size_t i = 0; i < kNumTests; ++i)
    {
        testType[i] = i % 4;
        testValue[i][0] = ui(r);
        testValue[i][1] = ui(r);
        data[i][0] = float(uf(r));
        data[i][1] = float(uf(r));
        data[i][2] = float(uf(r));
    }

    auto var = ctx.vars().getRootVar();
    var["testType"] = pDevice->createStructuredBuffer(
        var["testType"], (uint32_t)testType.size(), ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, testType.data()
    );
    var["testValue"] = pDevice->createStructuredBuffer(
        var["testValue"], (uint32_t)testValue.size(), ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, testValue.data()
    );
    var["data"] = pDevice->createStructuredBuffer(
        var["data"], (uint32_t)testType.size(), ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, data.data()
    );

    ctx.runProgram(kNumTests, 1, 1);

    // Verify results.
    std::vector<int> resultsInt = ctx.readBuffer<int>("resultsInt");
    std::vector<float2> resultsFloat = ctx.readBuffer<float2>("resultsFloat");
    for (uint32_t i = 0; i < kNumTests; i++)
    {
        const auto expected = getCpuResult(testType[i], testValue[i], data[i]);
        EXPECT_EQ(resultsInt[i], expected.first) << "i = " << i;
        EXPECT_EQ(resultsFloat[i], expected.second) << "i = " << i;
    }
}

GPU_TEST(Inheritance_ConformanceCreate)
{
    ref<Device> pDevice = ctx.getDevice();

    DefineList defines;
    defines.add("NUM_TESTS", std::to_string(kNumTests));
    ProgramDesc desc;
    desc.addShaderLibrary("Tests/Slang/InheritanceTests.cs.slang");
    desc.csEntry("testInheritanceConformance");
    desc.setShaderModel(ShaderModel::SM6_5);

    TypeConformanceList typeConformancess{
        {{"TestV0SubNeg", "ITestInterface"}, 0},
        {{"TestV1DefDef", "ITestInterface"}, 1},
        {{"TestV2DefNeg", "ITestInterface"}, 2},
        {{"TestV3SumDef", "ITestInterface"}, 3},
    };
    desc.addTypeConformances(typeConformancess);

    ctx.createProgram(desc, defines);
    ctx.allocateStructuredBuffer("resultsInt", kNumTests);
    ctx.allocateStructuredBuffer("resultsFloat", kNumTests);

    std::vector<int> testType(kNumTests);
    // The first value is value0 in TestInterfaceBase, second is value1 in the inherited classes.
    // This tests that the memory order of base class and inherited class members has not changed with Slang updates.
    std::vector<int2> testValue(kNumTests);
    std::vector<float3> data(kNumTests);

    for (size_t i = 0; i < kNumTests; ++i)
    {
        testType[i] = i % 4;
        testValue[i][0] = ui(r);
        testValue[i][1] = ui(r);
        data[i][0] = float(uf(r));
        data[i][1] = float(uf(r));
        data[i][2] = float(uf(r));
    }

    auto var = ctx.vars().getRootVar();
    var["testType"] = pDevice->createStructuredBuffer(
        var["testType"], (uint32_t)testType.size(), ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, testType.data()
    );
    var["testValue"] = pDevice->createStructuredBuffer(
        var["testValue"], (uint32_t)testValue.size(), ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, testValue.data()
    );
    var["data"] = pDevice->createStructuredBuffer(
        var["data"], (uint32_t)testType.size(), ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, data.data()
    );

    ctx.runProgram(kNumTests, 1, 1);

    // Verify results.
    std::vector<int> resultsInt = ctx.readBuffer<int>("resultsInt");
    std::vector<float2> resultsFloat = ctx.readBuffer<float2>("resultsFloat");
    for (uint32_t i = 0; i < kNumTests; i++)
    {
        const auto expected = getCpuResult(testType[i], testValue[i], data[i]);
        EXPECT_EQ(resultsInt[i], expected.first) << "i = " << i;
        EXPECT_EQ(resultsFloat[i], expected.second) << "i = " << i;
    }
}
/// This correctly and reliably fails, but there is no way to automatically test it.
// GPU_TEST(Inheritance_CheckInvalid)
// {
//     DefineList defines;
//     defines.add("NUM_TESTS", std::to_string(kNumTests));
//     defines.add("COMPILE_WITH_ERROR", "1");

//     ctx.createProgram("Tests/Slang/InheritanceTests.cs.slang", "testInheritance", defines, SlangCompilerFlags::None, ShaderModel::SM6_5);
// }

} // namespace Falcor
