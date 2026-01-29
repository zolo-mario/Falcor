#pragma once
#include "Handles.h"
#include "Core/Macros.h"
#include "Core/Object.h"
#include "Utils/Math/Vector.h"
#include <vector>

namespace Falcor
{
/**
 * Blend state
 */
class FALCOR_API BlendState : public Object
{
    FALCOR_OBJECT(BlendState)
public:
    /**
     * Defines how to combine the blend inputs
     */
    enum class BlendOp
    {
        Add,             ///< Add src1 and src2
        Subtract,        ///< Subtract src1 from src2
        ReverseSubtract, ///< Subtract src2 from src1
        Min,             ///< Find the minimum between the sources (per-channel)
        Max              ///< Find the maximum between the sources (per-channel)
    };

    /**
     * Defines how to modulate the fragment-shader and render-target pixel values
     */
    enum class BlendFunc
    {
        Zero,                ///< (0, 0, 0, 0)
        One,                 ///< (1, 1, 1, 1)
        SrcColor,            ///< The fragment-shader output color
        OneMinusSrcColor,    ///< One minus the fragment-shader output color
        DstColor,            ///< The render-target color
        OneMinusDstColor,    ///< One minus the render-target color
        SrcAlpha,            ///< The fragment-shader output alpha value
        OneMinusSrcAlpha,    ///< One minus the fragment-shader output alpha value
        DstAlpha,            ///< The render-target alpha value
        OneMinusDstAlpha,    ///< One minus the render-target alpha value
        BlendFactor,         ///< Constant color, set using Desc#SetBlendFactor()
        OneMinusBlendFactor, ///< One minus constant color, set using Desc#SetBlendFactor()
        SrcAlphaSaturate,    ///< (f, f, f, 1), where f = min(fragment shader output alpha, 1 - render-target pixel alpha)
        Src1Color,           ///< Fragment-shader output color 1
        OneMinusSrc1Color,   ///< One minus fragment-shader output color 1
        Src1Alpha,           ///< Fragment-shader output alpha 1
        OneMinusSrc1Alpha    ///< One minus fragment-shader output alpha 1
    };

    /**
     * Descriptor used to create new blend-state
     */
    class FALCOR_API Desc
    {
    public:
        Desc();
        friend class BlendState;

        /**
         * Set the constant blend factor
         * @param[in] factor Blend factor
         */
        Desc& setBlendFactor(const float4& factor)
        {
            mBlendFactor = factor;
            return *this;
        }

        /**
         * Enable/disable independent blend modes for different render target. Only used when multiple render-targets are bound.
         * @param[in] enabled True If false, will use RenderTargetDesc[0] for all the bound render-targets. Otherwise, will use the entire
         * RenderTargetDesc[] array.
         */
        Desc& setIndependentBlend(bool enabled)
        {
            mEnableIndependentBlend = enabled;
            return *this;
        }

        /**
         * Set the blend parameters
         * @param[in] rtIndex The RT index to set the parameters into. If independent blending is disabled, only the index 0 is used.
         * @param[in] rgbOp Blend operation for the RGB channels
         * @param[in] alphaOp Blend operation for the alpha channels
         * @param[in] srcRgbFunc Blend function for the fragment-shader output RGB channels
         * @param[in] dstRgbFunc Blend function for the render-target RGB channels
         * @param[in] srcAlphaFunc Blend function for the fragment-shader output alpha channel
         * @param[in] dstAlphaFunc Blend function for the render-target alpha channel
         */
        Desc& setRtParams(
            uint32_t rtIndex,
            BlendOp rgbOp,
            BlendOp alphaOp,
            BlendFunc srcRgbFunc,
            BlendFunc dstRgbFunc,
            BlendFunc srcAlphaFunc,
            BlendFunc dstAlphaFunc
        );

        /**
         * Enable/disable blending for a specific render-target. If independent blending is disabled, only the index 0 is used.
         */
        Desc& setRtBlend(uint32_t rtIndex, bool enable)
        {
            mRtDesc[rtIndex].blendEnabled = enable;
            return *this;
        }

        /**
         * Enable/disable alpha-to-coverage
         * @param[in] enabled True to enable alpha-to-coverage, false to disable it
         */
        Desc& setAlphaToCoverage(bool enabled)
        {
            mAlphaToCoverageEnabled = enabled;
            return *this;
        }

        /**
         * Set color write-mask
         */
        Desc& setRenderTargetWriteMask(uint32_t rtIndex, bool writeRed, bool writeGreen, bool writeBlue, bool writeAlpha);

        struct RenderTargetDesc
        {
            bool blendEnabled = false;
            BlendOp rgbBlendOp = BlendOp::Add;
            BlendOp alphaBlendOp = BlendOp::Add;
            BlendFunc srcRgbFunc = BlendFunc::One;
            BlendFunc srcAlphaFunc = BlendFunc::One;
            BlendFunc dstRgbFunc = BlendFunc::Zero;
            BlendFunc dstAlphaFunc = BlendFunc::Zero;
            struct WriteMask
            {
                bool writeRed = true;
                bool writeGreen = true;
                bool writeBlue = true;
                bool writeAlpha = true;
            };
            WriteMask writeMask;
        };

    protected:
        std::vector<RenderTargetDesc> mRtDesc;
        bool mEnableIndependentBlend = false;
        bool mAlphaToCoverageEnabled = false;
        float4 mBlendFactor = float4(0, 0, 0, 0);
    };

    ~BlendState();

    /**
     * Create a new blend state object.
     * @param[in] Desc Blend state descriptor.
     * @return A new object, or throws an exception if creation failed.
     */
    static ref<BlendState> create(const Desc& desc);

    /**
     * Get the constant blend factor color
     */
    const float4& getBlendFactor() const { return mDesc.mBlendFactor; }

    /**
     * Get the RGB blend operation
     */
    BlendOp getRgbBlendOp(uint32_t rtIndex) const { return mDesc.mRtDesc[rtIndex].rgbBlendOp; }

    /**
     * Get the alpha blend operation
     */
    BlendOp getAlphaBlendOp(uint32_t rtIndex) const { return mDesc.mRtDesc[rtIndex].alphaBlendOp; }

    /**
     * Get the fragment-shader RGB blend func
     */
    BlendFunc getSrcRgbFunc(uint32_t rtIndex) const { return mDesc.mRtDesc[rtIndex].srcRgbFunc; }

    /**
     * Get the fragment-shader alpha blend func
     */
    BlendFunc getSrcAlphaFunc(uint32_t rtIndex) const { return mDesc.mRtDesc[rtIndex].srcAlphaFunc; }

    /**
     * Get the render-target RGB blend func
     */
    BlendFunc getDstRgbFunc(uint32_t rtIndex) const { return mDesc.mRtDesc[rtIndex].dstRgbFunc; }

    /**
     * Get the render-target alpha blend func
     */
    BlendFunc getDstAlphaFunc(uint32_t rtIndex) const { return mDesc.mRtDesc[rtIndex].dstAlphaFunc; }

    /**
     * Check if blend is enabled
     */
    bool isBlendEnabled(uint32_t rtIndex) const { return mDesc.mRtDesc[rtIndex].blendEnabled; }

    /**
     * Check if alpha-to-coverage is enabled
     */
    bool isAlphaToCoverageEnabled() const { return mDesc.mAlphaToCoverageEnabled; }

    /**
     * Check if independent blending is enabled
     */
    bool isIndependentBlendEnabled() const { return mDesc.mEnableIndependentBlend; }

    /**
     * Get a render-target descriptor
     */
    const Desc::RenderTargetDesc& getRtDesc(size_t rtIndex) const { return mDesc.mRtDesc[rtIndex]; }

    /**
     * Get the render-target array size
     */
    uint32_t getRtCount() const { return (uint32_t)mDesc.mRtDesc.size(); }

private:
    BlendState(const Desc& Desc) : mDesc(Desc) {}
    const Desc mDesc;
};
} // namespace Falcor
