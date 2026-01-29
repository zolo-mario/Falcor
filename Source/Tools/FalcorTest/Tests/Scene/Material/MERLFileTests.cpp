#include "Testing/UnitTest.h"
#include "Core/AssetResolver.h"
#include "Scene/Material/MERLFile.h"
#include "Scene/Material/MERLMaterialData.slang"

namespace Falcor
{
GPU_TEST(MERLFile)
{
    // TODO: This is not ideal, we should only access files in the runtime directory.
    const std::filesystem::path path = getProjectDirectory() / "media/test_scenes/materials/data/gray-lambert.binary";

    MERLFile merlFile;
    bool result = merlFile.loadBRDF(path);
    ASSERT(result);

    const auto desc = merlFile.getDesc();
    EXPECT_EQ(desc.name, "gray-lambert");

    const auto data = merlFile.getData();
    EXPECT_EQ(data.size(), 90 * 90 * 360 / 2);

    const float3 expected = float3(0.5f);
    auto lut = merlFile.prepareAlbedoLUT(ctx.getDevice());
    EXPECT_EQ(lut.size(), MERLMaterialData::kAlbedoLUTSize);
    for (auto v : lut)
    {
        EXPECT_EQ(v.x, expected.x);
        EXPECT_EQ(v.y, expected.y);
        EXPECT_EQ(v.z, expected.z);
    }
}
} // namespace Falcor
