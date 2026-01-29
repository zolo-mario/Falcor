#pragma once
#include "Falcor.h"
#include "Core/Pass/FullScreenPass.h"
#include "RenderGraph/RenderPass.h"

using namespace Falcor;

class GaussianBlur : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(GaussianBlur, "GaussianBlur", "Gaussian blur.");

    static ref<GaussianBlur> create(ref<Device> pDevice, const Properties& props) { return make_ref<GaussianBlur>(pDevice, props); }

    GaussianBlur(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;

    void setKernelWidth(uint32_t kernelWidth);
    void setSigma(float sigma);
    uint32_t getKernelWidth() { return mKernelWidth; }
    float getSigma() { return mSigma; }

    static void registerBindings(pybind11::module& m);

private:
    uint32_t mKernelWidth = 5;
    float mSigma = 2.0f;
    bool mReady = false;
    void createTmpFbo(const Texture* pSrc);
    void updateKernel();

    ref<FullScreenPass> mpHorizontalBlur;
    ref<FullScreenPass> mpVerticalBlur;
    ref<Fbo> mpFbo;
    ref<Fbo> mpTmpFbo;
    ref<Sampler> mpSampler;
};
