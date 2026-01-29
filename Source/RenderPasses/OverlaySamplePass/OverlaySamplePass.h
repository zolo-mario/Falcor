#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"
#include "RenderGraph/RenderPassHelpers.h"

using namespace Falcor;

class OverlaySamplePass : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(
        OverlaySamplePass,
        "OverlaySamplePass",
        "Demonstrates how to use the renderOverlayUI callback in a renderpass to draw simple shapes to the screen."
    );

    static ref<OverlaySamplePass> create(ref<Device> pDevice, const Properties& props)
    {
        return make_ref<OverlaySamplePass>(pDevice, props);
    }

    OverlaySamplePass(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderOverlayUI(RenderContext* pRenderContext) override;
    static void registerBindings(pybind11::module& m);

private:
    void setFrameDim(const uint2 frameDim);

    uint2 mFrameDim;
    uint32_t mFrameCount = 0;
};
