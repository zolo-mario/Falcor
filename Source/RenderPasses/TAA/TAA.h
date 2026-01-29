#pragma once
#include "Falcor.h"
#include "Core/Pass/FullScreenPass.h"
#include "RenderGraph/RenderPass.h"

using namespace Falcor;

/**
 * Temporal AA class
 */
class TAA : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(TAA, "TAA", "Temporal Anti-Aliasing.");

    static ref<TAA> create(ref<Device> pDevice, const Properties& props) { return make_ref<TAA>(pDevice, props); }

    TAA(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;

    void setAlpha(float alpha) { mControls.alpha = alpha; }
    void setColorBoxSigma(float sigma) { mControls.colorBoxSigma = sigma; }
    void setAntiFlicker(bool antiFlicker) { mControls.antiFlicker = antiFlicker; }
    float getAlpha() { return mControls.alpha; }
    float getColorBoxSigma() { return mControls.colorBoxSigma; }
    bool getAntiFlicker() { return mControls.antiFlicker; }

private:
    void allocatePrevColor(const Texture* pColorOut);

    ref<FullScreenPass> mpPass;
    ref<Fbo> mpFbo;
    ref<Sampler> mpLinearSampler;

    struct
    {
        float alpha = 0.1f;
        float colorBoxSigma = 1.0f;
        bool antiFlicker = true;
    } mControls;

    ref<Texture> mpPrevColor;
};
