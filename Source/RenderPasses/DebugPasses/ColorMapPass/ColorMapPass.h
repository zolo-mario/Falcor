#pragma once
#include "Falcor.h"
#include "Core/Pass/FullScreenPass.h"
#include "RenderGraph/RenderPass.h"
#include "Utils/Algorithm/ParallelReduction.h"
#include "ColorMapParams.slang"

using namespace Falcor;

class ColorMapPass : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(ColorMapPass, "ColorMapPass", "Pass that applies a color map to the input.");

    static ref<ColorMapPass> create(ref<Device> pDevice, const Properties& props) { return make_ref<ColorMapPass>(pDevice, props); }

    ColorMapPass(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;

private:
    ColorMap mColorMap = ColorMap::Jet;
    uint32_t mChannel = 0;
    bool mAutoRange = true;
    float mMinValue = 0.f;
    float mMaxValue = 1.f;

    ref<FullScreenPass> mpColorMapPass;
    ref<Fbo> mpFbo;
    bool mRecompile = true;

    class AutoRanging
    {
    public:
        AutoRanging(ref<Device> pDevice);

        std::optional<std::pair<double, double>> getMinMax(RenderContext* pRenderContext, const ref<Texture>& texture, uint32_t channel);

    private:
        std::unique_ptr<ParallelReduction> mpParallelReduction;
        ref<Buffer> mpReductionResult;
        ref<Fence> mpFence;
        bool mReductionAvailable = false;
    };

    std::unique_ptr<AutoRanging> mpAutoRanging;
    double mAutoMinValue;
    double mAutoMaxValue;
};
