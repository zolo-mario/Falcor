#pragma once
#include "Core/Macros.h"
#include "Core/API/Formats.h"
#include "Core/API/ResourceViews.h"
#include "Core/Pass/ComputePass.h"
#include <memory>

namespace Falcor
{
class RenderContext;

/**
 * Image processing utilities.
 */
class FALCOR_API ImageProcessing
{
public:
    /// Constructor.
    ImageProcessing(ref<Device> pDevice);

    /**
     * Copy single mip level and color channel from source to destination.
     * The views must have matching dimension and format type (float vs integer).
     * The source value is written to all color channels of the destination.
     * The function throws if the requirements are not fulfilled.
     * @param[in] pRenderContxt The render context.
     * @param[in] pSrc Resource view for source texture.
     * @param[in] pDst Unordered access view for destination texture.
     * @param[in] srcMask Mask specifying which source color channel to copy. Must be a single channel.
     */
    void copyColorChannel(
        RenderContext* pRenderContxt,
        const ref<ShaderResourceView>& pSrc,
        const ref<UnorderedAccessView>& pDst,
        const TextureChannelFlags srcMask
    );

private:
    ref<Device> mpDevice;
    ref<ComputePass> mpCopyFloatPass;
    ref<ComputePass> mpCopyIntPass;
};
} // namespace Falcor
