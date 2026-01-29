#pragma once
#include "GridVolumeSamplerParams.slang"
#include "Core/Macros.h"
#include "Core/Program/DefineList.h"
#include "Utils/Properties.h"
#include "Utils/UI/Gui.h"
#include "Scene/Scene.h"

namespace Falcor
{
    class RenderContext;
    struct ShaderVar;

    /** Grid volume sampler.
        Utily class for evaluating transmittance and sampling scattering in grid volumes.
    */
    class FALCOR_API GridVolumeSampler
    {
    public:
        /** Grid volume sampler configuration options.
        */
        struct Options
        {
            TransmittanceEstimator transmittanceEstimator = TransmittanceEstimator::RatioTrackingLocalMajorant;
            DistanceSampler distanceSampler = DistanceSampler::DeltaTrackingLocalMajorant;
            bool useBrickedGrid = true;

            // Note: Empty constructor needed for clang due to the use of the nested struct constructor in the parent constructor.
            Options() {}

            template<typename Archive>
            void serialize(Archive& ar)
            {
                ar("transmittanceEstimator", transmittanceEstimator);
                ar("distanceSampler", distanceSampler);
                ar("useBrickedGrid", useBrickedGrid);
            }
        };

        /** Create a new object.
            \param[in] pRenderContext A render-context that will be used for processing.
            \param[in] pScene The scene.
            \param[in] options Configuration options.
        */
        GridVolumeSampler(RenderContext* pRenderContext, ref<IScene> pScene, const Options& options = Options());
        virtual ~GridVolumeSampler() = default;

        /** Get a list of shader defines for using the grid volume sampler.
            \return Returns a list of defines.
        */
        DefineList getDefines() const;

        /** Bind the grid volume sampler to a given shader variable.
            \param[in] var Shader variable.
        */
        void bindShaderData(const ShaderVar& var) const;

        /** Render the GUI.
            \return True if options were changed, false otherwise.
        */
        bool renderUI(Gui::Widgets& widget);

        /** Returns the current configuration.
        */
        const Options& getOptions() const { return mOptions; }

        void setOptions(const Options& options) { mOptions = options; }

    protected:
        ref<IScene>             mpScene;            ///< Scene.

        Options                 mOptions;
    };
}
