#include "Testing/UnitTest.h"
#include "SlangReinterpretCast.cs.slang"
#include <random>

namespace Falcor
{
namespace
{
static_assert(sizeof(Blob) == 48, "Unpexected size of struct Blob");
static_assert(sizeof(A) == 48, "Unpexected size of struct A");
static_assert(sizeof(B) == 48, "Unpexected size of struct B");
static_assert(sizeof(C) == 48, "Unpexected size of struct C");
static_assert(sizeof(D) == 48, "Unpexected size of struct D");
static_assert(sizeof(E) == 48, "Unpexected size of struct E");
static_assert(sizeof(F) == 48, "Unpexected size of struct F");

const uint32_t kElems = 128;
} // namespace

GPU_TEST(SlangReinterpretCast)
{
    ref<Device> pDevice = ctx.getDevice();

    ctx.createProgram("Tests/Slang/SlangReinterpretCast.cs.slang", "main", DefineList(), SlangCompilerFlags::None, ShaderModel::SM6_5);
    ctx.allocateStructuredBuffer("resultA", kElems);
    ctx.allocateStructuredBuffer("resultB", kElems);
    ctx.allocateStructuredBuffer("resultC", kElems);
    ctx.allocateStructuredBuffer("resultD", kElems);
    ctx.allocateStructuredBuffer("resultE", kElems);
    ctx.allocateStructuredBuffer("resultF", kElems);

    std::mt19937 r;
    std::uniform_real_distribution<float> u;

    std::vector<A> data(kElems);
    for (auto& v : data)
    {
        v.a = r();
        v.b = u(r);
        v.c = float16_t(u(r));
        v.d = int16_t(r());
        v.e = {r(), r()};
        v.f = {u(r), u(r), u(r)};
        v.g = r();
        v.h = {float16_t(u(r)), float16_t(u(r)), float16_t(u(r))};
        v.i = uint16_t(r());
        v.j = {float16_t(u(r)), float16_t(u(r))};
    }

    ShaderVar var = ctx.vars().getRootVar();
    var["data"] =
        pDevice->createStructuredBuffer(sizeof(A), kElems, ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, data.data());

    ctx.runProgram(kElems);

    // Verify final result matches our input.
    std::vector<A> result = ctx.readBuffer<A>("resultA");
    for (size_t i = 0; i < data.size(); i++)
    {
        EXPECT_EQ(result[i].a, data[i].a);
        EXPECT_EQ(result[i].b, data[i].b);
        EXPECT_EQ((float)result[i].c, (float)data[i].c);
        EXPECT_EQ(result[i].d, data[i].d);
        EXPECT_EQ(result[i].e.x, data[i].e.x);
        EXPECT_EQ(result[i].e.y, data[i].e.y);
        EXPECT_EQ((float)result[i].f.x, (float)data[i].f.x);
        EXPECT_EQ((float)result[i].f.y, (float)data[i].f.y);
        EXPECT_EQ((float)result[i].f.z, (float)data[i].f.z);
        EXPECT_EQ(result[i].g, data[i].g);
        EXPECT_EQ((float)result[i].h.x, (float)data[i].h.x);
        EXPECT_EQ((float)result[i].h.y, (float)data[i].h.y);
        EXPECT_EQ((float)result[i].h.z, (float)data[i].h.z);
        EXPECT_EQ(result[i].i, data[i].i);
        EXPECT_EQ((float)result[i].j.x, (float)data[i].j.x);
        EXPECT_EQ((float)result[i].j.y, (float)data[i].j.y);
    }

    // Verify the intermediate results. We'll just do a binary comparison for simplicity.
    auto verify = [&](const char* bufferName)
    {
        std::vector<uint32_t> result = ctx.readBuffer<uint32_t>(bufferName);
        const uint32_t* rawData = reinterpret_cast<const uint32_t*>(data.data());
        for (size_t i = 0; i < data.size() * sizeof(data[0]) / 4; i++)
        {
            EXPECT_EQ(result[i], rawData[i]) << "i = " << i << " buffer " << bufferName;
        }
    };

    verify("resultA");
    verify("resultB");
    verify("resultC");
    verify("resultD");
    verify("resultE");
    verify("resultF");
}
} // namespace Falcor
