#pragma once
#include "Core/Macros.h"
#include "Core/API/Texture.h"
#include "Core/API/Sampler.h"
#include "Core/Pass/ComputePass.h"
#include "Scene/Lights/EnvMap.h"

namespace Falcor
{
    class RenderContext;

    /** Environment map sampler.
        Utily class for sampling and evaluating radiance stored in an omnidirectional environment map.
    */
    class FALCOR_API EnvMapSampler
    {
    public:
        /** Create a new object.
            \param[in] pDevice GPU device.
            \param[in] pEnvMap The environment map.
        */
        EnvMapSampler(ref<Device> pDevice, ref<EnvMap> pEnvMap);
        virtual ~EnvMapSampler() = default;

        /** Bind the environment map sampler to a given shader variable.
            \param[in] var Shader variable.
        */
        void bindShaderData(const ShaderVar& var) const;

        const ref<EnvMap>& getEnvMap() const { return mpEnvMap; }

        const ref<Texture>& getImportanceMap() const { return mpImportanceMap; }

    protected:
        bool createImportanceMap(RenderContext* pRenderContext, uint32_t dimension, uint32_t samples);

        ref<Device>       mpDevice;

        ref<EnvMap>       mpEnvMap;                 ///< Environment map.

        ref<ComputePass>  mpSetupPass;              ///< Compute pass for creating the importance map.

        ref<Texture>      mpImportanceMap;          ///< Hierarchical importance map (luminance).
        ref<Sampler>      mpImportanceSampler;
    };
}
