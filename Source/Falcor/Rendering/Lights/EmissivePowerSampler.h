#pragma once
#include "EmissiveLightSampler.h"
#include "Core/Macros.h"
#include "Scene/Lights/LightCollection.h"
#include <random>
#include <vector>

namespace Falcor
{
    class RenderContext;
    struct ShaderVar;

    /** Sample geometry proportionally to its emissive power.
    */
    class FALCOR_API EmissivePowerSampler : public EmissiveLightSampler
    {
    public:
        struct AliasTable
        {
            float weightSum;                ///< Total weight of all elements used to create the alias table
            uint32_t N;                     ///< Number of entries in the alias table (and # elements in the buffers)
            ref<Buffer> fullTable;          ///< A compressed/packed merged table.  Max 2^24 (16 million) entries per table.
        };

        /** Creates a EmissivePowerSampler for a given scene.
            \param[in] pRenderContext The render context.
            \param[in] pScene The scene.
            \param[in] options The options to override the default behavior.
        */
        EmissivePowerSampler(RenderContext* pRenderContext, ref<ILightCollection> pLightCollection);
        virtual ~EmissivePowerSampler() = default;

        /** Updates the sampler to the current frame.
            \param[in] pRenderContext The render context.
            \param[in] pLightCollection Updated LightCollection
            \return True if the sampler was updated.
        */
        virtual bool update(RenderContext* pRenderContext, ref<ILightCollection> pLightCollection) override;

        /** Bind the light sampler data to a given shader variable.
            \param[in] var Shader variable.
        */
        virtual void bindShaderData(const ShaderVar& var) const override;

    protected:
        /** Generate an alias table
            \param[in] weights  The weights we'd like to sample each entry proportional to
            \returns The alias table
        */
        AliasTable generateAliasTable(std::vector<float> weights);

        // Internal state
        bool                            mNeedsRebuild = true;   ///< Trigger rebuild on the next call to update(). We should always build on the first call, so the initial value is true.

        std::mt19937                    mAliasTableRng;
        AliasTable                      mTriangleTable;
    };
}
