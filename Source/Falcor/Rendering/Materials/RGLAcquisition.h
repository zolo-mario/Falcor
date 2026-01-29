#pragma once
#include "Core/Macros.h"
#include "Core/API/Buffer.h"
#include "Core/Pass/ComputePass.h"
#include "Scene/Scene.h"
#include "Scene/Material/RGLFile.h"
#include <memory>

namespace Falcor
{
    /** This class allows taking a virtual measurement of a BRDF
        and converting it into the parametrization proposed by
        Dupuy & Jakob,  "An Adaptive Parameterization for Efficient
        Material Acquisition and Rendering".
    */
    class FALCOR_API RGLAcquisition
    {
    public:
        /// Constructor.
        RGLAcquisition(ref<Device> pDevice, const ref<Scene>& pScene);

        void acquireIsotropic(RenderContext* pRenderContext, const MaterialID materialID);
        RGLFile toRGLFile();

    private:
        ref<Device> mpDevice;
        ref<Scene> mpScene;
        ref<ComputePass> mpRetroReflectionPass;
        ref<ComputePass> mpBuildKernelPass;
        ref<ComputePass> mpPowerIterationPass;
        ref<ComputePass> mpIntegrateSigmaPass;
        ref<ComputePass> mpSumSigmaPass;
        ref<ComputePass> mpComputeThetaPass;
        ref<ComputePass> mpComputeVNDFPass;
        ref<ComputePass> mpAcquireBRDFPass;

        ref<Buffer> mpNDFDirectionsBuffer;      ///< Stores hemispherical directions of entries in the NDF table.
        ref<Buffer> mpRetroBuffer;              ///< 2D table storing measured retroreflecton of BRDF.
        ref<Buffer> mpNDFKernelBuffer;          ///< Stores kernel matrix of Fredholm problem for retrieving NDF.
        ref<Buffer> mpNDFBuffer;                ///< 2D table storing the retrieved NDF.
        ref<Buffer> mpNDFBufferTmp;
        ref<Buffer> mpSigmaBuffer;              ///< 2D table of projected microfacet area, integrated numerically.
        ref<Buffer> mpThetaBuffer;              ///< 1D tables storing angles at which measurements are taken.
        ref<Buffer> mpPhiBuffer;
        ref<Buffer> mpVNDFBuffer;               ///< 4D table (over wi x wo domains) containing the visible distribution of normals.
        ref<Buffer> mpVNDFMargBuffer;           ///< Auxiliary buffers for sampling the VNDF.
        ref<Buffer> mpVNDFCondBuffer;
        ref<Buffer> mpLumiBuffer;               ///< 4D table of measured luminance and RGB reflectance of RGB.
        ref<Buffer> mpRGBBuffer;
    };
}
