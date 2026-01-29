#include "Testing/UnitTest.h"
#include "Utils/Image/TextureManager.h"

namespace Falcor
{
GPU_TEST(TextureManager_LoadMips)
{
    ref<Device> pDevice = ctx.getDevice();

    TextureManager textureManager(pDevice, 10);

    std::filesystem::path path = getRuntimeDirectory() / "data/tests/tiny_<MIP>.png";

    auto handle = textureManager.loadTexture(path, false, false, ResourceBindFlags::ShaderResource, false);
    EXPECT(handle.isValid());
    EXPECT(!handle.isUdim());

    auto tex = textureManager.getTexture(handle);
    ASSERT(tex != nullptr);

    EXPECT_EQ(tex->getWidth(), 4);
    EXPECT_EQ(tex->getHeight(), 4);
    EXPECT_EQ(tex->getDepth(), 1);
    EXPECT_EQ(tex->getMipCount(), 3);
    EXPECT_EQ(tex->getArraySize(), 1);
}
} // namespace Falcor
