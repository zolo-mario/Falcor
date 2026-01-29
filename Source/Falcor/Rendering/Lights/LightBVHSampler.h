#pragma once
#include "EmissiveLightSampler.h"
#include "LightBVH.h"
#include "LightBVHBuilder.h"
#include "LightBVHSamplerSharedDefinitions.slang"
#include "Core/Macros.h"
#include "Utils/Properties.h"
#include "Utils/Math/AABB.h"
#include "Scene/Lights/LightCollection.h"
#include <memory>

namespace Falcor
{
    /** The CPU pointer to the lighting system's acceleration structure is
        passed to RenderPass::execute() via a field with this name in the
        dictionary.
    */
    static const char kLightingAccelerationStructure[] = "_lightingAccelerationStructure";

    /** Emissive light sampler using a light BVH.

        This class wraps a LightCollection object, which holds the set of lights to sample.
        Internally, the class build a BVH over the light sources.
    */
    class FALCOR_API LightBVHSampler : public EmissiveLightSampler
    {
    public:
        /** LightBVHSampler configuration.
            Note if you change options, please update FALCOR_SCRIPT_BINDING in LightBVHSampler.cpp
        */
        struct Options
        {
            // Build options
            LightBVHBuilder::Options buildOptions;

            // Traversal options
            bool        useBoundingCone = true;             ///< Use bounding cone to BVH nodes to bound NdotL when computing probabilities.
            bool        useLightingCone = true;             ///< Use lighting cone in BVH nodes to cull backfacing lights when computing probabilities.
            bool        disableNodeFlux = false;            ///< Do not take per-node flux into account in sampling.
            bool        useUniformTriangleSampling = true;  ///< Use uniform sampling to select a triangle within the sampled leaf node.

            SolidAngleBoundMethod solidAngleBoundMethod = SolidAngleBoundMethod::Sphere; ///< Method to use to bound the solid angle subtended by a cluster.

            // Note: Empty constructor needed for clang due to the use of the nested struct constructor in the parent constructor.
            Options() {}

            template<typename Archive>
            void serialize(Archive& ar)
            {
                ar("buildOptions", buildOptions);
                ar("useBoundingCone", useBoundingCone);
                ar("useLightingCone", useLightingCone);
                ar("disableNodeFlux", disableNodeFlux);
                ar("useUniformTriangleSampling", useUniformTriangleSampling);
                ar("solidAngleBoundMethod", solidAngleBoundMethod);
            }
        };

        /** Creates a LightBVHSampler for a given scene.
            \param[in] pRenderContext The render context.
            \param[in] pScene The scene.
            \param[in] options The options to override the default behavior.
        */
        LightBVHSampler(RenderContext* pRenderContext, ref<ILightCollection> pLightCollection, const Options& options = Options());
        virtual ~LightBVHSampler() = default;

        /** Updates the sampler to the current frame.
            \param[in] pRenderContext The render context.
            \param[in] pLightCollection Updated LightCollection
            \return True if the sampler was updated.
        */
        virtual bool update(RenderContext* pRenderContext, ref<ILightCollection> pLightCollection) override;

        /** Return a list of shader defines to use this light sampler.
        *   \return Returns a list of shader defines.
        */
        virtual DefineList getDefines() const override;

        /** Bind the light sampler data to a given shader variable.
            \param[in] var Shader variable.
        */
        virtual void bindShaderData(const ShaderVar& var) const override;

        /** Render the GUI.
            \return True if setting the refresh flag is needed, false otherwise.
        */
        virtual bool renderUI(Gui::Widgets& widget) override;

        /** Returns the current configuration.
        */
        const Options& getOptions() const { return mOptions; }

        void setOptions(const Options& options);

    protected:
        /// Configuration options.
        Options mOptions;

        // Internal state
        std::unique_ptr<LightBVHBuilder> mpBVHBuilder;
        std::unique_ptr<LightBVH> mpBVH;

        /// Trigger rebuild on the next call to update(). We should always build on the first call, so the initial value is true.
        bool mNeedsRebuild = true;
    };
}
