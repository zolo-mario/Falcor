#pragma once
#include "Handles.h"
#include "Core/Macros.h"
#include "Core/Object.h"
#include "Core/Enum.h"
#include <memory>

namespace Falcor
{
/**
 * Rasterizer state
 */
class FALCOR_API RasterizerState : public Object
{
    FALCOR_OBJECT(RasterizerState)
public:
    /**
     * Cull mode
     */
    enum class CullMode : uint32_t
    {
        None,  ///< No culling
        Front, ///< Cull front-facing primitives
        Back,  ///< Cull back-facing primitives
    };

    FALCOR_ENUM_INFO(
        CullMode,
        {
            {CullMode::None, "None"},
            {CullMode::Front, "Front"},
            {CullMode::Back, "Back"},
        }
    );

    /**
     * Polygon fill mode
     */
    enum class FillMode
    {
        Wireframe, ///< Wireframe
        Solid      ///< Solid
    };

    /**
     * Rasterizer state descriptor
     */
    class FALCOR_API Desc
    {
    public:
        friend class RasterizerState;

        /**
         * Set the cull mode
         */
        Desc& setCullMode(CullMode mode)
        {
            mCullMode = mode;
            return *this;
        }

        /**
         * Set the fill mode
         */
        Desc& setFillMode(FillMode mode)
        {
            mFillMode = mode;
            return *this;
        }

        /**
         * Determines how to interpret triangle direction.
         * @param isFrontCCW If true, a triangle is front-facing if is vertices are counter-clockwise. If false, the opposite.
         */
        Desc& setFrontCounterCW(bool isFrontCCW)
        {
            mIsFrontCcw = isFrontCCW;
            return *this;
        }

        /**
         * Set the depth-bias. The depth bias is calculated as
         * \code
         * bias = (float)depthBias * r + slopeScaledBias * maxDepthSlope
         * \endcode
         * where r is the minimum representable value in the depth buffer and maxDepthSlope is the maximum of the horizontal and vertical
         * slopes of the depth value in the pixel.\n See <a
         * href="https://msdn.microsoft.com/en-us/library/windows/desktop/cc308048%28v=vs.85%29.aspx">the DX documentation</a> for depth
         * bias explanation.
         */
        Desc& setDepthBias(int32_t depthBias, float slopeScaledBias)
        {
            mSlopeScaledDepthBias = slopeScaledBias;
            mDepthBias = depthBias;
            return *this;
        }

        /**
         * Determines weather to clip or cull the vertices based on depth
         * @param clampDepth If true, clamp depth value to the viewport extent. If false, clip primitives to near/far Z-planes
         */
        Desc& setDepthClamp(bool clampDepth)
        {
            mClampDepth = clampDepth;
            return *this;
        }

        /**
         * Enable/disable anti-aliased lines. Actual anti-aliasing algorithm is implementation dependent, but usually uses quadrilateral
         * lines.
         */
        Desc& setLineAntiAliasing(bool enableLineAA)
        {
            mEnableLinesAA = enableLineAA;
            return *this;
        };

        /**
         * Enable/disable scissor test
         */
        Desc& setScissorTest(bool enabled)
        {
            mScissorEnabled = enabled;
            return *this;
        }

        /**
         * Enable/disable conservative rasterization
         */
        Desc& setConservativeRasterization(bool enabled)
        {
            mConservativeRaster = enabled;
            return *this;
        }

        /**
         * Set the forced sample count. Useful when using UAV
         */
        Desc& setForcedSampleCount(uint32_t samples)
        {
            mForcedSampleCount = samples;
            return *this;
        }

    protected:
        CullMode mCullMode = CullMode::Back;
        FillMode mFillMode = FillMode::Solid;
        bool mIsFrontCcw = true;
        float mSlopeScaledDepthBias = 0;
        int32_t mDepthBias = 0;
        bool mClampDepth = false;
        bool mScissorEnabled = false;
        bool mEnableLinesAA = true;
        uint32_t mForcedSampleCount = 0;
        bool mConservativeRaster = false;
    };

    ~RasterizerState();

    /**
     * Create a new rasterizer state.
     * @param[in] desc Rasterizer state descriptor.
     * @return A new object, or throws an exception if creation failed.
     */
    static ref<RasterizerState> create(const Desc& desc);

    /**
     * Get the cull mode
     */
    CullMode getCullMode() const { return mDesc.mCullMode; }
    /**
     * Get the fill mode
     */
    FillMode getFillMode() const { return mDesc.mFillMode; }
    /**
     * Check what is the winding order for triangles to be considered front-facing
     */
    bool isFrontCounterCW() const { return mDesc.mIsFrontCcw; }
    /**
     * Get the slope-scaled depth bias
     */
    float getSlopeScaledDepthBias() const { return mDesc.mSlopeScaledDepthBias; }
    /**
     * Get the depth bias
     */
    int32_t getDepthBias() const { return mDesc.mDepthBias; }
    /**
     * Check if depth clamp is enabled
     */
    bool isDepthClampEnabled() const { return mDesc.mClampDepth; }
    /**
     * Check if scissor test is enabled
     */
    bool isScissorTestEnabled() const { return mDesc.mScissorEnabled; }
    /**
     * Check if anti-aliased lines are enabled
     */
    bool isLineAntiAliasingEnabled() const { return mDesc.mEnableLinesAA; }

    /**
     * Check if conservative rasterization is enabled
     */
    bool isConservativeRasterizationEnabled() const { return mDesc.mConservativeRaster; }

    /**
     * Get the forced sample count
     */
    uint32_t getForcedSampleCount() const { return mDesc.mForcedSampleCount; }

private:
    RasterizerState(const Desc& Desc) : mDesc(Desc) {}
    Desc mDesc;
};

FALCOR_ENUM_REGISTER(RasterizerState::CullMode);

} // namespace Falcor
