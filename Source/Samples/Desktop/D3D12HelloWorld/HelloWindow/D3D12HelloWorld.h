#pragma once
#include "Falcor.h"
#include "Core/SampleBase.h"

using namespace Falcor;

class D3D12HelloWorld : public SampleBase
{
public:
    FALCOR_PLUGIN_CLASS(D3D12HelloWorld, "D3D12HelloWorld", SampleBase::PluginInfo{"Samples/Desktop/D3D12HelloWorld/HelloWindow"});

    explicit D3D12HelloWorld(SampleApp* pHost);

    static SampleBase* create(SampleApp* pHost);

    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;

private:
};
