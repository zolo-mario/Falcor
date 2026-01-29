#pragma once

#include "Core/API/Formats.h"
#include "Core/API/Sampler.h"
#include "Core/API/RenderContext.h"
#include "Core/Pass/ComputePass.h"

namespace Falcor::pbrt
{

/**
 * Helper class to convert env map from equal-area octahedral mapping to lat-long mapping.
 */
class EnvMapConverter
{
public:
    EnvMapConverter(ref<Device> pDevice) : mpDevice(pDevice)
    {
        mpComputePass = ComputePass::create(mpDevice, "plugins/importers/PBRTImporter/EnvMapConverter.cs.slang");

        Sampler::Desc desc;
        desc.setFilterMode(TextureFilteringMode::Linear, TextureFilteringMode::Linear, TextureFilteringMode::Linear);
        desc.setAddressingMode(TextureAddressingMode::Clamp, TextureAddressingMode::Clamp, TextureAddressingMode::Clamp);
        mpSampler = mpDevice->createSampler(desc);
    }

    /**
     * Convert texture from equal-area octahedral mapping to lat-long mapping.
     * The output texture will have resolution [2 * width, height] of the input texture and RGBAF32 format.
     * @param[in] pRenderContext Render context.
     * @param[in] pSrcTexture Source texture with envmap in equal-area octahedral mapping.
     * @return Texture with envmap in lat-long mapping.
     */
    ref<Texture> convertEqualAreaOctToLatLong(RenderContext* pRenderContext, const ref<Texture>& pSrcTexture) const
    {
        FALCOR_ASSERT(pSrcTexture);
        FALCOR_ASSERT(pSrcTexture->getWidth() == pSrcTexture->getHeight());

        uint2 dstDim{pSrcTexture->getWidth() * 2, pSrcTexture->getHeight()};

        ref<Texture> pDstTexture = mpDevice->createTexture2D(
            dstDim.x,
            dstDim.y,
            ResourceFormat::RGBA32Float,
            1,
            1,
            nullptr,
            ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess
        );

        auto vars = mpComputePass->getRootVar()["gEnvMapConverter"];
        vars["src"] = pSrcTexture;
        vars["srcSampler"] = mpSampler;
        vars["dst"] = pDstTexture;
        vars["dstDim"] = dstDim;
        mpComputePass->execute(pRenderContext, uint3(dstDim, 1));

        return pDstTexture;
    }

private:
    ref<Device> mpDevice;
    ref<ComputePass> mpComputePass;
    ref<Sampler> mpSampler;
};

} // namespace Falcor::pbrt
