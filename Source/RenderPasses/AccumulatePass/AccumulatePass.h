#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"
#include "RenderGraph/RenderPassHelpers.h"

using namespace Falcor;

/**
 * Temporal accumulation render pass.
 *
 * This pass takes a texture as input and writes the temporally accumulated
 * result to an output texture. The pass keeps intermediate data internally.
 *
 * For accumulating many samples for ground truth rendering etc., fp32 precision
 * is not always sufficient. The pass supports higher precision modes using
 * either error compensation (Kahan summation) or double precision math.
 */
class AccumulatePass : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(AccumulatePass, "AccumulatePass", "Temporal accumulation.");

    static ref<AccumulatePass> create(ref<Device> pDevice, const Properties& props) { return make_ref<AccumulatePass>(pDevice, props); }

    AccumulatePass(ref<Device> pDevice, const Properties& props);
    virtual ~AccumulatePass() = default;

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) override { return false; }
    virtual bool onKeyEvent(const KeyboardEvent& keyEvent) override { return false; }
    virtual void onHotReload(HotReloadFlags reloaded) override;

    bool isEnabled() const { return mEnabled; }
    void setEnabled(bool enabled);

    // Scripting functions
    void reset();

    enum class Precision : uint32_t
    {
        Double,            ///< Standard summation in double precision.
        Single,            ///< Standard summation in single precision.
        SingleCompensated, ///< Compensated summation (Kahan summation) in single precision.
    };

    FALCOR_ENUM_INFO(
        Precision,
        {
            {Precision::Double, "Double"},
            {Precision::Single, "Single"},
            {Precision::SingleCompensated, "SingleCompensated"},
        }
    );

    enum class OverflowMode : uint32_t
    {
        Stop,  ///< Stop accumulation and retain accumulated image.
        Reset, ///< Reset accumulation.
        EMA,   ///< Switch to exponential moving average accumulation.
    };

    FALCOR_ENUM_INFO(
        OverflowMode,
        {
            {OverflowMode::Stop, "Stop"},
            {OverflowMode::Reset, "Reset"},
            {OverflowMode::EMA, "EMA"},
        }
    );

protected:
    void prepareAccumulation(RenderContext* pRenderContext, uint32_t width, uint32_t height);
    void accumulate(RenderContext* pRenderContext, const ref<Texture>& pSrc, const ref<Texture>& pDst);

    // Internal state

    /// The current scene (or nullptr if no scene).
    ref<Scene> mpScene;

    /// Accumulation programs, one per mode.
    std::map<Precision, ref<Program>> mpProgram;
    ref<ProgramVars> mpVars;
    ref<ComputeState> mpState;

    /// Format type of the source that gets accumulated.
    FormatType mSrcType;

    /// Number of accumulated frames. This is reset upon changes.
    uint32_t mFrameCount = 0;
    /// Current frame dimension in pixels.
    uint2 mFrameDim = {0, 0};
    /// Last frame running sum. Used in Single and SingleKahan mode.
    ref<Texture> mpLastFrameSum;
    /// Last frame running compensation term. Used in SingleKahan mode.
    ref<Texture> mpLastFrameCorr;
    /// Last frame running sum (lo bits). Used in Double mode.
    ref<Texture> mpLastFrameSumLo;
    /// Last frame running sum (hi bits). Used in Double mode.
    ref<Texture> mpLastFrameSumHi;

    // UI variables

    /// True if accumulation is enabled.
    bool mEnabled = true;
    /// Reset accumulation automatically upon scene changes and refresh flags.
    bool mAutoReset = true;

    Precision mPrecisionMode = Precision::Single;
    /// Maximum number of frames to accumulate before triggering overflow. 0 means infinite accumulation.
    uint32_t mMaxFrameCount = 0;
    /// What to do after maximum number of frames are accumulated.
    OverflowMode mOverflowMode = OverflowMode::Stop;

    /// Output format (uses default when set to ResourceFormat::Unknown).
    ResourceFormat mOutputFormat = ResourceFormat::Unknown;
    /// Selected output size.
    RenderPassHelpers::IOSize mOutputSizeSelection = RenderPassHelpers::IOSize::Default;
    /// Output size in pixels when 'Fixed' size is selected.
    uint2 mFixedOutputSize = {512, 512};
};

FALCOR_ENUM_REGISTER(AccumulatePass::Precision);
FALCOR_ENUM_REGISTER(AccumulatePass::OverflowMode);
