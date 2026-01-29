#pragma once
#include "Core/Macros.h"
#include "Core/API/Buffer.h"
#include "Core/API/Fence.h"
#include "Core/Pass/ComputePass.h"
#include "Utils/Math/Vector.h"
#include "Scene/Scene.h"
#include "Scene/SceneIDs.h"
#include <memory>

namespace Falcor
{
    /** Utility class for BSDF integration.
    */
    class FALCOR_API BSDFIntegrator
    {
    public:
        /// Constructor.
        BSDFIntegrator(ref<Device> pDevice, const ref<Scene>& pScene);

        /** Integrate the BSDF for a material given a single incident direction.
            The BSDF is assumed to be isotropic and is integrated over outgoing directions in the upper hemisphere.
            \param[in] pRenderContext The context.
            \param[in] materialID The material to integrate.
            \param[in] cosTheta Cosine theta angle of incident direction.
            \return The integral value.
        */
        float3 integrateIsotropic(RenderContext* pRenderContext, const MaterialID materialID, const float cosTheta);

        /** Integrate the BSDF for a material given an array of incident directions.
            The BSDF is assumed to be isotropic and is integrated over outgoing directions in the upper hemisphere.
            \param[in] pRenderContext The context.
            \param[in] materialID The material to integrate.
            \param[in] cosThetas Cosine theta angles of incident directions.
            \return Array of integral values.
        */
        std::vector<float3> integrateIsotropic(RenderContext* pRenderContext, const MaterialID materialID, const std::vector<float>& cosThetas);

    private:
        void integrationPass(RenderContext* pRenderContext, const MaterialID materialID, const uint32_t gridCount) const;
        void finalPass(RenderContext* pRenderContext, const uint32_t gridCount) const;

        ref<Device> mpDevice;
        ref<Scene> mpScene;
        ref<ComputePass> mpIntegrationPass;         ///< Integration pass.
        ref<ComputePass> mpFinalPass;               ///< Final reduction pass.
        ref<Buffer> mpCosThetaBuffer;               ///< Buffer for uploading incident cos theta angles.
        ref<Buffer> mpResultBuffer;                 ///< Buffer for intermediate results.
        ref<Buffer> mpFinalResultBuffer;            ///< Buffer for final results after reduction.
        ref<Buffer> mpStagingBuffer;                ///< Staging buffer for readback of final results.
        uint32_t mResultCount;                      ///< Number of intermediate results per integration grid.
    };
}
