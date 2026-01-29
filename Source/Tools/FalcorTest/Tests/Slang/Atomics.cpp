#include "Testing/UnitTest.h"
#include "Core/Pass/ComputePass.h"
#include <random>

namespace Falcor
{
namespace
{
const std::string_view kShaderFile = "Tests/Slang/Atomics.cs.slang";

const uint32_t kNumElems = 256;
std::mt19937 r;
std::uniform_real_distribution<float> u;

void testInterlockedAddF16(GPUUnitTestContext& ctx, std::string_view entryPoint)
{
    ref<Device> pDevice = ctx.getDevice();

    ProgramDesc desc;
    desc.addShaderLibrary(kShaderFile).csEntry("testBufferAddF16");
    desc.setUseSPIRVBackend(); // NOTE: The SPIR-V backend is required for RWByteAddressBuffer.InterlockedAddF16() on Vulkan!
    ctx.createProgram(desc);

    std::vector<float16_t> elems(kNumElems * 2);
    for (auto& v : elems)
        v = (float16_t)u(r);
    auto dataBuf =
        pDevice->createBuffer(kNumElems * sizeof(float), ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, elems.data());

    float zeros[2] = {};
    auto resultBuf = pDevice->createBuffer(
        2 * sizeof(float), ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess, MemoryType::DeviceLocal, zeros
    );

    auto var = ctx.vars().getRootVar();
    var["data"] = dataBuf;
    var["resultBuf"] = resultBuf;

    ctx.runProgram(kNumElems, 1, 1);

    // Verify results.
    float16_t result[4] = {};
    resultBuf->getBlob(&result, 0, 2 * sizeof(float));

    float16_t a[2] = {}, b[2] = {};
    for (uint32_t i = 0; i < 2 * kNumElems; i += 2)
    {
        a[0] += elems[i];
        a[1] += elems[i + 1];
        b[0] -= elems[i];
        b[1] -= elems[i + 1];
    }
    float e = 1.f;
    EXPECT_GE(result[0] + e, a[0]);
    EXPECT_LE(result[0] - e, a[0]);
    EXPECT_GE(result[1] + e, a[1]);
    EXPECT_LE(result[1] - e, a[1]);
    EXPECT_GE(result[2] + e, b[0]);
    EXPECT_LE(result[2] - e, b[0]);
    EXPECT_GE(result[3] + e, b[1]);
    EXPECT_LE(result[3] - e, b[1]);
}
} // namespace

GPU_TEST(Atomics_Buffer_InterlockedAddF16)
{
    testInterlockedAddF16(ctx, "testBufferAddF16");
}

GPU_TEST(Atomics_Buffer_InterlockedAddF16_2)
{
    testInterlockedAddF16(ctx, "testBufferAddF16_2");
}

GPU_TEST(Atomics_Buffer_InterlockedAddF32)
{
    ref<Device> pDevice = ctx.getDevice();

    ctx.createProgram(kShaderFile, "testBufferAddF32");

    std::vector<float> elems(kNumElems);
    for (auto& v : elems)
        v = u(r);
    auto dataBuf =
        pDevice->createBuffer(kNumElems * sizeof(float), ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, elems.data());

    float zeros[2] = {};
    auto resultBuf = pDevice->createBuffer(
        2 * sizeof(float), ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess, MemoryType::DeviceLocal, zeros
    );

    auto var = ctx.vars().getRootVar();
    var["data"] = dataBuf;
    var["resultBuf"] = resultBuf;

    ctx.runProgram(kNumElems, 1, 1);

    // Verify results.
    float result[2] = {};
    resultBuf->getBlob(&result, 0, 2 * sizeof(float));

    float a = 0.f, b = 0.f;
    for (uint32_t i = 0; i < kNumElems; i++)
    {
        a += elems[i];
        b -= elems[i];
    }
    float e = 1e-3f;
    EXPECT_GE(result[0] + e, a);
    EXPECT_LE(result[0] - e, a);
    EXPECT_GE(result[1] + e, b);
    EXPECT_LE(result[1] - e, b);
}

GPU_TEST(Atomics_Texture2D_InterlockedAddF32)
{
    ref<Device> pDevice = ctx.getDevice();

    ProgramDesc desc;
    desc.addShaderLibrary(kShaderFile).csEntry("testTextureAddF32");
    desc.setUseSPIRVBackend(); // NOTE: The SPIR-V backend is required for RWTexture2D.InterlockedAddF32() on Vulkan!
    ctx.createProgram(desc);

    std::vector<float> elems(kNumElems);
    for (auto& v : elems)
        v = u(r);
    auto dataBuf =
        pDevice->createBuffer(kNumElems * sizeof(float), ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, elems.data());

    float zeros[2] = {};
    auto resultTex = pDevice->createTexture2D(
        2, 1, ResourceFormat::R32Float, 1, 1, zeros, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess
    );
    auto resultBuf = pDevice->createBuffer(
        2 * sizeof(float), ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess, MemoryType::DeviceLocal, zeros
    );

    auto var = ctx.vars().getRootVar();
    var["data"] = dataBuf;
    var["resultTex"] = resultTex;

    ctx.runProgram(kNumElems, 1, 1);

    // Copy result into readback buffer.
    {
        auto copyPass = ComputePass::create(pDevice, kShaderFile, "copyResult");
        auto copyVar = copyPass->getRootVar();
        copyVar["resultBuf"] = resultBuf;
        copyVar["resultTex"] = resultTex;
        copyPass->execute(pDevice->getRenderContext(), 256, 1);
    }

    // Verify results.
    float result[2] = {};
    resultBuf->getBlob(&result, 0, 2 * sizeof(float));

    float a = 0.f, b = 0.f;
    for (uint32_t i = 0; i < kNumElems; i++)
    {
        a += elems[i];
        b -= elems[i];
    }
    float e = 1e-3f;
    EXPECT_GE(result[0] + e, a);
    EXPECT_LE(result[0] - e, a);
    EXPECT_GE(result[1] + e, b);
    EXPECT_LE(result[1] - e, b);
}
} // namespace Falcor
