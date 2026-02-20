#pragma once
#include "Falcor.h"
#include "Core/Pass/RasterPass.h"
#include "Core/SampleBase.h"

using namespace Falcor;

class MultiSampling : public SampleBase
{
public:
    FALCOR_PLUGIN_CLASS(MultiSampling, "MultiSampling", SampleBase::PluginInfo{"Samples/MultiSampling"});

    explicit MultiSampling(SampleApp* pHost);
    ~MultiSampling();

    static SampleBase* create(SampleApp* pHost);

    void onLoad(RenderContext* pRenderContext) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;

private:
    ref<RasterPass> mpRasterPass;
    ref<Vao> mpVao;
    ref<Fbo> mpFbo;
    ref<Texture> mpResolvedTexture;
    uint32_t mFrame = 0;
};
