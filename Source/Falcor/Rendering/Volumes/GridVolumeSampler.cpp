#include "GridVolumeSampler.h"
#include "Core/Error.h"

namespace Falcor
{
    GridVolumeSampler::GridVolumeSampler(RenderContext* pRenderContext, ref<IScene> pScene, const Options& options)
        : mpScene(pScene)
        , mOptions(options)
    {
        FALCOR_ASSERT(pScene);
    }

    DefineList GridVolumeSampler::getDefines() const
    {
        DefineList defines;
        defines.add("GRID_VOLUME_SAMPLER_USE_BRICKEDGRID", std::to_string((uint32_t)mOptions.useBrickedGrid));
        defines.add("GRID_VOLUME_SAMPLER_TRANSMITTANCE_ESTIMATOR", std::to_string((uint32_t)mOptions.transmittanceEstimator));
        defines.add("GRID_VOLUME_SAMPLER_DISTANCE_SAMPLER", std::to_string((uint32_t)mOptions.distanceSampler));
        return defines;
    }

    void GridVolumeSampler::bindShaderData(const ShaderVar& var) const
    {
        FALCOR_ASSERT(var.isValid());
    }

    bool GridVolumeSampler::renderUI(Gui::Widgets& widget)
    {
        bool dirty = false;

        if (widget.checkbox("Use BrickedGrid", mOptions.useBrickedGrid))
        {
            if (!mOptions.useBrickedGrid) {
                // Switch back to modes not requiring bricked grid.
                if (requiresBrickedGrid(mOptions.transmittanceEstimator)) mOptions.transmittanceEstimator = TransmittanceEstimator::RatioTracking;
                if (requiresBrickedGrid(mOptions.distanceSampler)) mOptions.distanceSampler = DistanceSampler::DeltaTracking;
            }
            dirty = true;
        }
        if (widget.dropdown("Transmittance Estimator", mOptions.transmittanceEstimator))
        {
            // Enable bricked grid if the chosen mode requires it.
            if (requiresBrickedGrid(mOptions.transmittanceEstimator)) mOptions.useBrickedGrid = true;
            dirty = true;
        }
        if (widget.dropdown("Distance Sampler", mOptions.distanceSampler))
        {
            // Enable bricked grid if the chosen mode requires it.
            if (requiresBrickedGrid(mOptions.distanceSampler)) mOptions.useBrickedGrid = true;
            dirty = true;
        }

        return dirty;
    }
}
