#pragma once
#include "Core/Macros.h"
#include "Core/API/Formats.h"
#include "Core/API/Buffer.h"
#include "Core/API/Texture.h"
#include "Core/Pass/ComputePass.h"
#include "Utils/Math/Vector.h"
#include <memory>
#include <vector>

namespace Falcor
{
class RenderContext;

/**
 * A class for analyzing texture contents.
 */
class FALCOR_API TextureAnalyzer
{
public:
    /// Texture analysis result.
    struct Result
    {
        uint32_t mask;        ///< Bits 0-3 indicate which color channels (RGBA) are varying (0 = constant, 1 = varying in i:th bit).
                              ///< Bits 4-19 indicate numerical range of texture (4 bits per channel). Bits 20-31 are reserved.
        uint32_t reserved[3]; ///< Reserved bits.

        float4 value;    ///< The constant color value in RGBA fp32 format. Only valid for channels that are identified as constant.
        float4 minValue; ///< The minimum color value in RGBA fp32 format. NOTE: Clamped to zero.
        float4 maxValue; ///< The maximum color value in RGBA fp32 format. NOTE: Clamped to zero.

        enum class RangeFlags : uint32_t
        {
            Pos = 0x1, ///< Texture channel has positive values > 0;
            Neg = 0x2, ///< Texture channel has negative values < 0;
            Inf = 0x4, ///< Texture channel has +/-inf values.
            NaN = 0x8, ///< Texture channel has NaN values.
        };

        bool isConstant(uint32_t channelMask) const { return (mask & channelMask) == 0; }
        bool isConstant(TextureChannelFlags channelMask) const { return isConstant((uint32_t)channelMask); }

        bool isPos(TextureChannelFlags channelMask) const { return getRange(channelMask) & (uint32_t)RangeFlags::Pos; }
        bool isNeg(TextureChannelFlags channelMask) const { return getRange(channelMask) & (uint32_t)RangeFlags::Neg; }
        bool isInf(TextureChannelFlags channelMask) const { return getRange(channelMask) & (uint32_t)RangeFlags::Inf; }
        bool isNaN(TextureChannelFlags channelMask) const { return getRange(channelMask) & (uint32_t)RangeFlags::NaN; }

        /**
         * Returns the numerical range of texels in the given color channels.
         * @param[in] channelMask Which color channels to look at.
         * @return Union of 'RangeFlags' flags (0 = no texels, 1 = at least one texel).
         */
        uint32_t getRange(TextureChannelFlags channelMask) const
        {
            uint32_t range = 0;
            for (int i = 0; i < 4; i++)
            {
                if ((uint32_t)channelMask & (1 << i))
                {
                    range |= mask >> (4 + 4 * i);
                }
            }
            return range & 0xf;
        }
    };

    /**
     * Constructor. Throws an exception if creation failed.
     */
    TextureAnalyzer(ref<Device> pDevice);

    /**
     * Analyze 2D texture to check if it has a constant color.
     * Throws an exception if the input texture is of unsupported format or dimension.
     * The result is written in the format of the 'Result' struct (64B total).
     * @param[in] pRenderContext The context.
     * @param[in] pInput Input texture. This has to be a 2D non-MSAA texture of floating-point format.
     * @param[in] mipLevel The mip level.
     * @param[in] arraySlice The array slice.
     * @param[in] pResult GPU buffer to where the result is written. This is expected to have UAV bind flag and be cleared to zero before
     * the call.
     * @param[in] resultOffset Offset into result buffer to where the result is written.
     * @param[in] clearResult Flag indicating whether the function should clear the result buffer first.
     */
    void analyze(
        RenderContext* pRenderContext,
        const ref<Texture> pInput,
        uint32_t mipLevel,
        uint32_t arraySlice,
        ref<Buffer> pResult,
        uint64_t resultOffset = 0,
        bool clearResult = true
    );

    /**
     * Batch analysis of a set of 2D textures. This is more efficient than calling analyze() repeatedly.
     * The first mip level and array slice of each texture is used.
     * Throws an exception if any input texture is of unsupported format or dimension.
     * The result is written as an arary of 'Result' structs (64B each).
     * @param[in] pRenderContext The context.
     * @param[in] inputs Array of input textures. These have to be 2D non-MSAA textures of floating-point formats.
     * @param[in] pResult GPU buffer to where the result is written. This is expected to have UAV bind flag.
     * @param[in] clearResult Flag indicating whether the function should clear the result buffer first.
     */
    void analyze(RenderContext* pRenderContext, const std::vector<ref<Texture>>& inputs, ref<Buffer> pResult, bool clearResult = true);

    /**
     * Helper function to clear the results buffer.
     * @param[in] pRenderContext The context.
     * @param[in] pResult GPU buffer to clear. This is expected to have UAV bind flag.
     * @param[in] resultOffset Offset into result buffer to where the first result is stored.
     * @param[in] resultsCount Number of result structs.
     */
    void clear(RenderContext* pRenderContext, ref<Buffer> pResult, uint64_t resultOffset, size_t resultCount) const;

    /**
     * Returns the size of the generated result for one texture in bytes.
     */
    static size_t getResultSize();

private:
    void checkFormatSupport(const ref<Texture> pInput, uint32_t mipLevel, uint32_t arraySlice) const;

    ref<Device> mpDevice;
    ref<ComputePass> mpClearPass;
    ref<ComputePass> mpAnalyzePass;
};

FALCOR_ENUM_CLASS_OPERATORS(TextureAnalyzer::Result::RangeFlags);
} // namespace Falcor
