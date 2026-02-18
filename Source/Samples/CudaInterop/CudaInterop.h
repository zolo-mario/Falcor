#pragma once
#include <cuda.h>
#include "Falcor.h"
#include "Core/SampleApp.h"
#include <cuda_runtime.h>

using namespace Falcor;

class CudaInterop : public SampleApp
{
public:
    CudaInterop(const SampleAppConfig& config);
    ~CudaInterop();

    void onLoad(RenderContext* pRenderContext) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;

private:
    uint32_t mWidth;
    uint32_t mHeight;
    ref<Texture> mpInputTex;
    ref<Texture> mpOutputTex;
    cudaSurfaceObject_t mInputSurf;
    cudaSurfaceObject_t mOutputSurf;
};
