#include "Testing/UnitTest.h"
#include "Core/AssetResolver.h"
#include "Scene/Lights/EnvMap.h"
#include "Rendering/Lights/EnvMapSampler.h"

namespace Falcor
{
namespace
{
// TODO: This is not ideal, we should only access files in the runtime directory.
const std::filesystem::path kEnvMapPath = getProjectDirectory() / "media/test_scenes/envmaps/20050806-03_hd.hdr";
} // namespace

GPU_TEST(EnvMap)
{
    // Test loading a light probe.
    // This call runs setup code on the GPU to precompute the importance map.
    // If it succeeds, we at least know the code compiles and run.
    ref<EnvMap> pEnvMap = EnvMap::createFromFile(ctx.getDevice(), kEnvMapPath);
    EXPECT_NE(pEnvMap, nullptr);
    if (pEnvMap == nullptr)
        return;

    EnvMapSampler envMapSampler(ctx.getDevice(), pEnvMap);

    // Check that the importance map exists and is a square power-of-two
    // texture with a full mip map hierarchy.
    auto pImportanceMap = envMapSampler.getImportanceMap();
    EXPECT_NE(pImportanceMap, nullptr);
    if (pImportanceMap == nullptr)
        return;

    uint32_t w = pImportanceMap->getWidth();
    uint32_t h = pImportanceMap->getHeight();
    uint32_t mipCount = pImportanceMap->getMipCount();

    EXPECT(isPowerOf2(w) && w > 0);
    EXPECT_EQ(w, h);
    EXPECT_EQ(w, 1 << (mipCount - 1));
}
} // namespace Falcor
