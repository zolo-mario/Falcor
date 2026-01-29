#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"
#include "Utils/Algorithm/ParallelReduction.h"
#include <fstream>

using namespace Falcor;

class ErrorMeasurePass : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(ErrorMeasurePass, "ErrorMeasurePass", "Measures error with respect to a reference image.");

    enum class OutputId
    {
        Source,
        Reference,
        Difference,
        Count
    };

    FALCOR_ENUM_INFO(
        OutputId,
        {
            {OutputId::Source, "Source"},
            {OutputId::Reference, "Reference"},
            {OutputId::Difference, "Difference"},
        }
    );

    static ref<ErrorMeasurePass> create(ref<Device> pDevice, const Properties& props) { return make_ref<ErrorMeasurePass>(pDevice, props); }

    ErrorMeasurePass(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual bool onKeyEvent(const KeyboardEvent& keyEvent) override;

private:
    bool loadReference();
    ref<Texture> getReference(const RenderData& renderData) const;
    bool loadMeasurementsFile();
    void saveMeasurementsToFile();

    void runDifferencePass(RenderContext* pRenderContext, const RenderData& renderData);
    void runReductionPasses(RenderContext* pRenderContext, const RenderData& renderData);

    ref<ComputePass> mpErrorMeasurerPass;
    std::unique_ptr<ParallelReduction> mpParallelReduction;

    struct
    {
        float3 error;   ///< Error (either L1 or MSE) in RGB.
        float avgError; ///< Error averaged over color components.
        bool valid = false;
    } mMeasurements;

    // Internal state

    float3 mRunningError = float3(0.f, 0.f, 0.f);
    /// A negative value indicates that both running error values are invalid.
    float mRunningAvgError = -1.f;

    ref<Texture> mpReferenceTexture;
    ref<Texture> mpDifferenceTexture;

    std::ofstream mMeasurementsFile;

    // UI variables

    /// Path to the reference used in the comparison.
    std::filesystem::path mReferenceImagePath;
    /// Path to the output file where measurements are stored (.csv).
    std::filesystem::path mMeasurementsFilePath;

    /// If true, do not measure error on pixels that belong to the background.
    bool mIgnoreBackground = true;
    /// Compute the square difference when creating the difference image.
    bool mComputeSquaredDifference = true;
    /// Compute the average of the RGB components when creating the difference image.
    bool mComputeAverage = false;
    /// If true, use loaded reference image instead of input.
    bool mUseLoadedReference = false;
    /// Use exponetial moving average (EMA) for the computed error.
    bool mReportRunningError = true;
    /// Coefficient used for the exponential moving average. Larger values mean slower response.
    float mRunningErrorSigma = 0.995f;

    OutputId mSelectedOutputId = OutputId::Source;

    static const Gui::RadioButtonGroup sOutputSelectionButtons;
    static const Gui::RadioButtonGroup sOutputSelectionButtonsSourceOnly;
};

FALCOR_ENUM_REGISTER(ErrorMeasurePass::OutputId);
