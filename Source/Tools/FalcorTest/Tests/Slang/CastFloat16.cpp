#include "Testing/UnitTest.h"
#include "Utils/HostDeviceShared.slangh"
#include <random>

namespace Falcor
{
namespace
{
const uint32_t kNumElems = 256;
std::mt19937 r;
std::uniform_real_distribution u;
} // namespace

GPU_TEST(CastFloat16)
{
    ref<Device> pDevice = ctx.getDevice();

    ctx.createProgram("Tests/Slang/CastFloat16.cs.slang", "testCastFloat16", DefineList(), SlangCompilerFlags::None, ShaderModel::SM6_5);
    ctx.allocateStructuredBuffer("result", kNumElems);

    std::vector<uint16_t> elems(kNumElems * 2);
    for (auto& v : elems)
        v = f32tof16(float(u(r)));
    auto var = ctx.vars().getRootVar();
    auto buf = pDevice->createStructuredBuffer(
        var["data"], (uint32_t)elems.size(), ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, elems.data()
    );
    ASSERT_EQ(buf->getStructSize(), sizeof(float16_t));
    ASSERT_EQ(buf->getElementCount(), elems.size());

    var["data"] = buf;

    ctx.runProgram(kNumElems, 1, 1);

    // Verify results.
    std::vector<uint32_t> result = ctx.readBuffer<uint32_t>("result");
    for (uint32_t i = 0; i < kNumElems; i++)
    {
        uint16_t ix = elems[2 * i];
        uint16_t iy = elems[2 * i + 1];
        uint32_t expected = (uint32_t(iy) << 16) | ix;
        EXPECT_EQ(result[i], expected) << "i = " << i;
    }
}
} // namespace Falcor
