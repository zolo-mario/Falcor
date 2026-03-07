#pragma once
#include "Falcor.h"
#include "Core/SampleBase.h"

using namespace Falcor;

class GraphicsGems1 : public SampleBase
{
public:
    FALCOR_PLUGIN_CLASS(GraphicsGems1, "GraphicsGems1", SampleBase::PluginInfo{"Samples/GraphicsGems/GG1"});

    explicit GraphicsGems1(SampleApp* pHost);

    static SampleBase* create(SampleApp* pHost);

    void onLoad(RenderContext* pRenderContext) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;

private:
    void recomputeFromHSL();
    void recomputeFromRGB();

    float mH = 0.0f;
    float mS = 1.0f;
    float mL = 0.5f;
    float3 mRgbFromHSL = {1.0f, 0.0f, 0.0f};

    float3 mRgbInput = {1.0f, 0.0f, 0.0f};
    float mHFromRGB = 0.0f;
    float mSFromRGB = 1.0f;
    float mLFromRGB = 0.5f;
};
