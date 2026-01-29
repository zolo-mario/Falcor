#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"
#include "Core/Pass/FullScreenPass.h"

using namespace Falcor;

class InvalidPixelDetectionPass : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(
        InvalidPixelDetectionPass,
        "InvalidPixelDetectionPass",
        "Pass that marks all NaN pixels red and Inf pixels green in an image."
    );

    static ref<InvalidPixelDetectionPass> create(ref<Device> pDevice, const Properties& props)
    {
        return make_ref<InvalidPixelDetectionPass>(pDevice, props);
    }

    InvalidPixelDetectionPass(ref<Device> pDevice, const Properties& props);

    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;

private:
    ref<FullScreenPass> mpInvalidPixelDetectPass;
    ref<Fbo> mpFbo;
    ResourceFormat mFormat = ResourceFormat::Unknown;
    bool mReady = false;
};
