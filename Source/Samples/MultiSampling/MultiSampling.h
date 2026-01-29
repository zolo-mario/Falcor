#pragma once
#include "Falcor.h"
#include "Core/SampleApp.h"
#include "Core/Pass/RasterPass.h"

using namespace Falcor;

class MultiSampling : public SampleApp
{
public:
    MultiSampling(const SampleAppConfig& config);
    ~MultiSampling();

    void onLoad(RenderContext* pRenderContext) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;

private:
    ref<RasterPass> mpRasterPass;
    ref<Vao> mpVao;
    ref<Fbo> mpFbo;
    ref<Texture> mpResolvedTexture;
    uint32_t mFrame = 0;
};
