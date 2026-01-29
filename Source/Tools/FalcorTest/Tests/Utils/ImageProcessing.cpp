#include "Testing/UnitTest.h"
#include "Utils/Image/ImageProcessing.h"

namespace Falcor
{
namespace
{
template<typename T>
std::vector<T> generateTestData(size_t elems)
{
    std::vector<T> data;
    for (size_t i = 0; i < elems; i++)
    {
        float c = i * 2.5f * (i % 2 ? -1.f : 1.f);
        data.push_back(T(c));
    }
    return data;
}

template<typename T>
void testCopyColorChannel(
    GPUUnitTestContext& ctx,
    ImageProcessing& imageProcessing,
    uint32_t width,
    uint32_t height,
    ResourceFormat srcFormat,
    ResourceFormat dstFormat
)
{
    ref<Device> pDevice = ctx.getDevice();

    const auto srcChannels = getFormatChannelCount(srcFormat);
    const auto dstChannels = getFormatChannelCount(dstFormat);

    // Create test textures.
    auto data = generateTestData<T>(width * height * srcChannels);
    auto pSrc = pDevice->createTexture2D(width, height, srcFormat, 1, 1, data.data(), ResourceBindFlags::ShaderResource);
    auto pDst = pDevice->createTexture2D(
        width, height, dstFormat, 1, 1, nullptr, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess
    );

    // Test copying from color channel i=0..3.
    std::vector<TextureChannelFlags> channelMask = {
        TextureChannelFlags::Red,
        TextureChannelFlags::Green,
        TextureChannelFlags::Blue,
        TextureChannelFlags::Alpha,
    };
    for (uint32_t i = 0; i < 4; i++)
    {
        // Copy channel to destination texture.
        imageProcessing.copyColorChannel(ctx.getRenderContext(), pSrc->getSRV(), pDst->getUAV(), channelMask[i]);

        // Validate result.
        auto rawData = ctx.getRenderContext()->readTextureSubresource(pDst.get(), 0);

        EXPECT_EQ(getFormatPixelsPerBlock(dstFormat), 1);
        EXPECT_EQ(rawData.size(), width * height * getFormatBytesPerBlock(dstFormat));

        const T* result = (const T*)rawData.data();

        for (size_t j = 0; j < (size_t)width * height; j++)
        {
            T ref = data[j * srcChannels + i];
            for (uint32_t k = 0; k < dstChannels; k++)
            {
                T value = result[j * dstChannels + k];
                // TODO: Remove workaround when float16_t implements ostream operator.
                if constexpr (std::is_same<float16_t, T>::value)
                {
                    EXPECT_EQ((float)ref, (float)value) << "j=" << j << " k=" << k << " dstFormat=" + to_string(dstFormat);
                }
                else
                {
                    EXPECT_EQ(ref, value) << "j=" << j << " k=" << k << " dstFormat=" + to_string(dstFormat);
                }
            }
        }
    }
}
} // namespace

GPU_TEST(CopyColorChannel)
{
    uint32_t w = 15, h = 3;
    ImageProcessing imageProcessing(ctx.getDevice());
    testCopyColorChannel<float>(ctx, imageProcessing, w, h, ResourceFormat::RGBA32Float, ResourceFormat::RGBA32Float);
    testCopyColorChannel<float>(ctx, imageProcessing, w, h, ResourceFormat::RGBA32Float, ResourceFormat::RG32Float);
    testCopyColorChannel<float>(ctx, imageProcessing, w, h, ResourceFormat::RGBA32Float, ResourceFormat::R32Float);
    testCopyColorChannel<uint32_t>(ctx, imageProcessing, w, h, ResourceFormat::RGBA32Uint, ResourceFormat::RGBA32Uint);
    testCopyColorChannel<uint32_t>(ctx, imageProcessing, w, h, ResourceFormat::RGBA32Uint, ResourceFormat::RG32Uint);
    testCopyColorChannel<uint32_t>(ctx, imageProcessing, w, h, ResourceFormat::RGBA32Uint, ResourceFormat::R32Uint);
    testCopyColorChannel<float16_t>(ctx, imageProcessing, w, h, ResourceFormat::RGBA16Float, ResourceFormat::RGBA16Float);
    testCopyColorChannel<float16_t>(ctx, imageProcessing, w, h, ResourceFormat::RGBA16Float, ResourceFormat::RG16Float);
    testCopyColorChannel<float16_t>(ctx, imageProcessing, w, h, ResourceFormat::RGBA16Float, ResourceFormat::R16Float);
    testCopyColorChannel<uint16_t>(ctx, imageProcessing, w, h, ResourceFormat::RGBA16Unorm, ResourceFormat::RGBA16Unorm);
    testCopyColorChannel<uint16_t>(ctx, imageProcessing, w, h, ResourceFormat::RGBA16Unorm, ResourceFormat::RG16Unorm);
    testCopyColorChannel<uint16_t>(ctx, imageProcessing, w, h, ResourceFormat::RGBA16Unorm, ResourceFormat::R16Unorm);
    testCopyColorChannel<int16_t>(ctx, imageProcessing, w, h, ResourceFormat::RGBA16Int, ResourceFormat::RGBA16Int);
    testCopyColorChannel<int16_t>(ctx, imageProcessing, w, h, ResourceFormat::RGBA16Int, ResourceFormat::RG16Int);
    testCopyColorChannel<int16_t>(ctx, imageProcessing, w, h, ResourceFormat::RGBA16Int, ResourceFormat::R16Int);
    testCopyColorChannel<int8_t>(ctx, imageProcessing, w, h, ResourceFormat::RGBA8Snorm, ResourceFormat::RGBA8Snorm);
    testCopyColorChannel<int8_t>(ctx, imageProcessing, w, h, ResourceFormat::RGBA8Snorm, ResourceFormat::RG8Snorm);
    testCopyColorChannel<int8_t>(ctx, imageProcessing, w, h, ResourceFormat::RGBA8Snorm, ResourceFormat::R8Snorm);
    testCopyColorChannel<int8_t>(ctx, imageProcessing, w, h, ResourceFormat::RGBA8Int, ResourceFormat::RGBA8Int);
    testCopyColorChannel<int8_t>(ctx, imageProcessing, w, h, ResourceFormat::RGBA8Int, ResourceFormat::RG8Int);
    testCopyColorChannel<int8_t>(ctx, imageProcessing, w, h, ResourceFormat::RGBA8Int, ResourceFormat::R8Int);
}
} // namespace Falcor
