#pragma once
#include "Falcor.h"
#include "Core/Pass/FullScreenPass.h"
#include "Core/SampleBase.h"

using namespace Falcor;

class ShaderToy : public SampleBase
{
public:
    FALCOR_PLUGIN_CLASS(ShaderToy, "ShaderToy", SampleBase::PluginInfo{"Samples/ShaderToy"});

    explicit ShaderToy(SampleApp* pHost);
    ~ShaderToy();

    static SampleBase* create(SampleApp* pHost);

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
