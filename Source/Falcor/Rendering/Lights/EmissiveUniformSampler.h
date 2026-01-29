#pragma once
#include "EmissiveLightSampler.h"
#include "Core/Macros.h"
#include "Utils/Properties.h"
#include "Scene/Lights/LightCollection.h"

namespace Falcor
{
    /** Emissive light sampler using uniform sampling of the lights.

        This class wraps a LightCollection object, which holds the set of lights to sample.
    */
    class FALCOR_API EmissiveUniformSampler : public EmissiveLightSampler
    {
    public:
        /** EmissiveUniformSampler configuration.
            Note if you change options, please update FALCOR_SCRIPT_BINDING in EmissiveUniformSampler.cpp
        */
        struct Options
        {
            // TODO
            //bool        usePreintegration = true;           ///< Use pre-integrated flux per triangle to guide BVH build/sampling. Only relevant if mUseBVHTree == true.

            template<typename Archive>
            void serialize(Archive& ar)
            {
            }
        };

        /** Creates a EmissiveUniformSampler for a given scene.
            \param[in] pRenderContext The render context.
            \param[in] pScene The scene.
            \param[in] options The options to override the default behavior.
        */
        EmissiveUniformSampler(RenderContext* pRenderContext, ref<ILightCollection> pLightCollection, const Options& options = Options());
        virtual ~EmissiveUniformSampler() = default;

        /** Returns the current configuration.
        */
        const Options& getOptions() const { return mOptions; }

    protected:
        // Configuration
        Options mOptions;
    };
}
