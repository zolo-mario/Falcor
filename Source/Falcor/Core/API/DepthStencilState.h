#pragma once
#include "Types.h"
#include "Handles.h"
#include "Core/Macros.h"
#include "Core/Object.h"

namespace Falcor
{
/**
 * Depth-Stencil state
 */
class FALCOR_API DepthStencilState : public Object
{
    FALCOR_OBJECT(DepthStencilState)
public:
    /**
     * Used for stencil control.
     */
    enum class Face
    {
        Front,       ///< Front-facing primitives
        Back,        ///< Back-facing primitives
        FrontAndBack ///< Front and back-facing primitives
    };

    /**
     * Stencil operation
     */
    enum class StencilOp
    {
        Keep,             ///< Keep the stencil value
        Zero,             ///< Set the stencil value to zero
        Replace,          ///< Replace the stencil value with the reference value
        Increase,         ///< Increase the stencil value by one, wrap if necessary
        IncreaseSaturate, ///< Increase the stencil value by one, clamp if necessary
        Decrease,         ///< Decrease the stencil value by one, wrap if necessary
        DecreaseSaturate, ///< Decrease the stencil value by one, clamp if necessary
        Invert            ///< Invert the stencil data (bitwise not)
    };

    /**
     * Stencil descriptor
     */
    struct StencilDesc
    {
        ComparisonFunc func = ComparisonFunc::Disabled; ///< Stencil comparison function
        StencilOp stencilFailOp = StencilOp::Keep;      ///< Stencil operation in case stencil test fails
        StencilOp depthFailOp = StencilOp::Keep;        ///< Stencil operation in case stencil test passes but depth test fails
        StencilOp depthStencilPassOp = StencilOp::Keep; ///< Stencil operation in case stencil and depth tests pass
    };

    /**
     * Depth-stencil descriptor
     */
    class FALCOR_API Desc
    {
    public:
        friend class DepthStencilState;

        /**
         * Enable/disable depth-test
         */
        Desc& setDepthEnabled(bool enabled)
        {
            mDepthEnabled = enabled;
            return *this;
        }

        /**
         * Set the depth-function
         */
        Desc& setDepthFunc(ComparisonFunc depthFunc)
        {
            mDepthFunc = depthFunc;
            return *this;
        }

        /**
         * Enable or disable depth writes into the depth buffer
         */
        Desc& setDepthWriteMask(bool writeDepth)
        {
            mWriteDepth = writeDepth;
            return *this;
        }

        /**
         * Enable/disable stencil-test
         */
        Desc& setStencilEnabled(bool enabled)
        {
            mStencilEnabled = enabled;
            return *this;
        }

        /**
         * Set the stencil write-mask
         */
        Desc& setStencilWriteMask(uint8_t mask);

        /**
         * Set the stencil read-mask
         */
        Desc& setStencilReadMask(uint8_t mask);

        /**
         * Set the stencil comparison function
         * @param face Chooses the face to apply the function to
         * @param func Comparison function
         */
        Desc& setStencilFunc(Face face, ComparisonFunc func);

        /**
         * Set the stencil operation
         * @param face Chooses the face to apply the operation to
         * @param stencilFail Stencil operation in case stencil test fails
         * @param depthFail Stencil operation in case stencil test passes but depth test fails
         * @param depthStencilPass Stencil operation in case stencil and depth tests pass
         */
        Desc& setStencilOp(Face face, StencilOp stencilFail, StencilOp depthFail, StencilOp depthStencilPass);

        /**
         * Set the stencil ref value
         */
        Desc& setStencilRef(uint8_t value)
        {
            mStencilRef = value;
            return *this;
        };

    protected:
        bool mDepthEnabled = true;
        bool mStencilEnabled = false;
        bool mWriteDepth = true;
        ComparisonFunc mDepthFunc = ComparisonFunc::Less;
        StencilDesc mStencilFront;
        StencilDesc mStencilBack;
        uint8_t mStencilReadMask = (uint8_t)-1;
        uint8_t mStencilWriteMask = (uint8_t)-1;
        uint8_t mStencilRef = 0;
    };

    ~DepthStencilState();

    /**
     * Create a new depth-stencil state object.
     * @param desc Depth-stencil descriptor.
     * @return New object, or throws an exception if an error occurred.
     */
    static ref<DepthStencilState> create(const Desc& desc);

    /**
     * Check if depth test is enabled or disabled
     */
    bool isDepthTestEnabled() const { return mDesc.mDepthEnabled; }

    /**
     * Check if depth write is enabled or disabled
     */
    bool isDepthWriteEnabled() const { return mDesc.mWriteDepth; }

    /**
     * Get the depth comparison function
     */
    ComparisonFunc getDepthFunc() const { return mDesc.mDepthFunc; }

    /**
     * Check if stencil is enabled or disabled
     */
    bool isStencilTestEnabled() const { return mDesc.mStencilEnabled; }

    /**
     * Get the stencil descriptor for the selected face
     */
    const StencilDesc& getStencilDesc(Face face) const;

    /**
     * Get the stencil read mask
     */
    uint8_t getStencilReadMask() const { return mDesc.mStencilReadMask; }

    /**
     * Get the stencil write mask
     */
    uint8_t getStencilWriteMask() const { return mDesc.mStencilWriteMask; }

    /**
     * Get the stencil ref value
     */
    uint8_t getStencilRef() const { return mDesc.mStencilRef; }

private:
    DepthStencilState(const Desc& Desc) : mDesc(Desc) {}
    Desc mDesc;
};
} // namespace Falcor
