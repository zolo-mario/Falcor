#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"

using namespace Falcor;

/**
 * Simple pass for time-dependent fading between two buffers.
 */
class CrossFade : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(CrossFade, "CrossFade", "CrossFade pass.");

    static ref<CrossFade> create(ref<Device> pDevice, const Properties& props) { return make_ref<CrossFade>(pDevice, props); }

    CrossFade(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;

private:
    uint2 mFrameDim = {0, 0};
    float mScaleA = 1.f;
    float mScaleB = 1.f;
    ResourceFormat mOutputFormat = ResourceFormat::RGBA32Float;

    ref<ComputePass> mpFadePass;

    ref<Scene> mpScene;
    uint32_t mMixFrame = 0;
    bool mEnableAutoFade = true;
    uint32_t mWaitFrameCount = 10;
    uint32_t mFadeFrameCount = 100;
    /// Fixed fade factor (t) used when auto-fade is disabled. The output is: (1-t)*A + t*B.
    float mFadeFactor = 0.5f;
};
