#pragma once
#include "Falcor.h"
#include "Core/SampleApp.h"
#include "Core/Pass/FullScreenPass.h"

using namespace Falcor;

class ShaderToy : public SampleApp
{
public:
    ShaderToy(const SampleAppConfig& config);
    ~ShaderToy();

    void onLoad(RenderContext* pRenderContext) override;
    void onResize(uint32_t width, uint32_t height) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;

private:
    ref<Sampler> mpLinearSampler;
    float mAspectRatio = 0;
    ref<RasterizerState> mpNoCullRastState;
    ref<DepthStencilState> mpNoDepthDS;
    ref<BlendState> mpOpaqueBS;
    ref<FullScreenPass> mpMainPass;
};
