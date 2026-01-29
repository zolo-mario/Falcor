#include "ImageProcessing.h"
#include "Core/API/RenderContext.h"
#include "Utils/Math/Vector.h"

namespace Falcor
{
namespace
{
const char kCopyColorChannelShader[] = "Utils/Image/CopyColorChannel.cs.slang";
}

ImageProcessing::ImageProcessing(ref<Device> pDevice) : mpDevice(pDevice) {}

void ImageProcessing::copyColorChannel(
    RenderContext* pRenderContext,
    const ref<ShaderResourceView>& pSrc,
    const ref<UnorderedAccessView>& pDst,
    const TextureChannelFlags srcMask
)
{
    // Validate arguments.
    FALCOR_ASSERT(pSrc && pDst);
    if (pSrc->getResource()->getType() != Resource::Type::Texture2D)
        FALCOR_THROW("Source resource type must be Texture2D");
    if (pDst->getResource()->getType() != Resource::Type::Texture2D)
        FALCOR_THROW("Source resource type must be Texture2D");

    auto pSrcTex = pSrc->getResource()->asTexture();
    auto pDstTex = pDst->getResource()->asTexture();
    uint srcMip = pSrc->getViewInfo().mostDetailedMip;
    uint dstMip = pDst->getViewInfo().mostDetailedMip;
    uint2 srcDim = {pSrcTex->getWidth(srcMip), pSrcTex->getHeight(srcMip)};
    uint2 dstDim = {pDstTex->getWidth(dstMip), pDstTex->getHeight(dstMip)};
    bool srcIsInt = isIntegerFormat(pSrcTex->getFormat());
    bool dstIsInt = isIntegerFormat(pDstTex->getFormat());

    if (any(srcDim != dstDim))
        FALCOR_THROW("Source and destination views must have matching dimensions");
    if (srcIsInt != dstIsInt)
        FALCOR_THROW("Source and destination texture must have matching format type");

    uint channelIndex = 0;
    switch (srcMask)
    {
    case TextureChannelFlags::Red:
        channelIndex = 0;
        break;
    case TextureChannelFlags::Green:
        channelIndex = 1;
        break;
    case TextureChannelFlags::Blue:
        channelIndex = 2;
        break;
    case TextureChannelFlags::Alpha:
        channelIndex = 3;
        break;
    default:
        FALCOR_THROW("'channelMask' parameter must be a single color channel.");
    }

    // Prepare and execute program to copy color channel.
    ref<ComputePass> pPass;
    if (srcIsInt)
    {
        if (!mpCopyIntPass)
            mpCopyIntPass = ComputePass::create(mpDevice, kCopyColorChannelShader, "main", {{"TEXTURE_FORMAT", "uint4"}});
        pPass = mpCopyIntPass;
    }
    else
    {
        if (!mpCopyFloatPass)
            mpCopyFloatPass = ComputePass::create(mpDevice, kCopyColorChannelShader, "main", {{"TEXTURE_FORMAT", "float4"}});
        pPass = mpCopyFloatPass;
    }

    auto var = pPass->getRootVar();
    var["gSrc"].setSrv(pSrc);
    var["gDst"].setUav(pDst);
    var["CB"]["viewDim"] = srcDim;
    var["CB"]["channelIndex"] = channelIndex;

    pPass->execute(pRenderContext, uint3(srcDim, 1));
}
} // namespace Falcor
