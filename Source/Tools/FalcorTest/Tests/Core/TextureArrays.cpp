#include "Testing/UnitTest.h"
#include "Core/Pass/ComputePass.h"

namespace Falcor
{
namespace
{
void runTest(GPUUnitTestContext& ctx, std::array<int, 3> bits)
{
    ref<Device> pDevice = ctx.getDevice();

    FALCOR_ASSERT(bits[0] + bits[1] + bits[2] == 3);
    DefineList defines = {
        {"BITS_I", std::to_string(bits[0])},
        {"BITS_J", std::to_string(bits[1])},
        {"BITS_K", std::to_string(bits[2])},
    };

    // Create textures.
    std::vector<float> init(16 * 16, 0.f);
    ref<Texture> tex[8];
    for (size_t i = 0; i < 8; i++)
        tex[i] = pDevice->createTexture2D(
            16, 16, ResourceFormat::R32Float, 1, 1, init.data(), ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess
        );

    auto bindTextures = [&](ShaderVar var)
    {
        for (size_t idx = 0; idx < 8; idx++)
        {
            uint i = idx & ((1 << bits[0]) - 1);
            uint j = (idx >> bits[0]) & ((1 << bits[1]) - 1);
            uint k = (idx >> (bits[0] + bits[1])) & ((1 << bits[2]) - 1);
            var["tex"][i][j][k] = tex[idx];
        }
    };

    // Create programs.
    {
        ProgramDesc desc;
        desc.addShaderLibrary("Tests/Core/TextureArrays.cs.slang").csEntry("testWrite");
        ctx.createProgram(desc, defines);
    }

    ref<ComputePass> readPass;
    {
        ProgramDesc desc;
        desc.addShaderLibrary("Tests/Core/TextureArrays.cs.slang").csEntry("testRead");
        readPass = ComputePass::create(pDevice, desc, defines);
    }

    std::vector<float> zeros(16 * 16 * 8, 0.f);
    auto resultBuf = pDevice->createBuffer(
        16 * 16 * 8 * sizeof(float),
        ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
        MemoryType::DeviceLocal,
        zeros.data()
    );

    // Write textures.
    bindTextures(ctx.getVars()->getRootVar());
    ctx.getVars()->getRootVar()["result"] = resultBuf;
    ctx.runProgram(16, 16, 8);

    // Read textures.
    bindTextures(readPass->getRootVar());
    readPass->getRootVar()["result"] = resultBuf;
    readPass->execute(pDevice->getRenderContext(), 16, 16, 8);

    // Verify result.
    std::vector<float> result(16 * 16 * 8);
    resultBuf->getBlob(result.data(), 0, 16 * 16 * 8 * sizeof(float));

    size_t i = 0;
    for (uint32_t z = 0; z < 8; z++)
    {
        for (uint32_t y = 0; y < 16; y++)
        {
            for (uint32_t x = 0; x < 16; x++)
            {
                EXPECT_EQ(result[i], (float)x * y + z) << "i=" << i << " x=" << x << " y=" << y << " z=" << z;
                ++i;
            }
        }
    }
}
} // namespace

GPU_TEST(Texture_NestedArrays)
{
    // Nested texture arrays are not supported on Vulkan, so expect error when using Vulkan device
    if (ctx.getDevice()->getType() != Device::Type::Vulkan)
    {
        runTest(ctx, {1, 1, 1});
        runTest(ctx, {2, 0, 1});
        runTest(ctx, {0, 0, 3});
    }
    else
    {
        EXPECT_THROW(runTest(ctx, {1, 1, 1}));
        EXPECT_THROW(runTest(ctx, {2, 0, 1}));
        EXPECT_THROW(runTest(ctx, {0, 0, 3}));
    }
}
} // namespace Falcor
