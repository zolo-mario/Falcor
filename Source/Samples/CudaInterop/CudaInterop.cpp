#include "CudaInterop.h"
#include "CopySurface.h"
#include "Core/AssetResolver.h"
#include "Utils/CudaUtils.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

namespace
{
const std::filesystem::path kTexturePath = "test_images/smoke_puff.png";
}

CudaInterop::CudaInterop(const SampleAppConfig& config) : SampleApp(config) {}

CudaInterop::~CudaInterop() {}

void CudaInterop::onLoad(RenderContext* pRenderContext)
{
    // Initialize CUDA device
    if (!getDevice()->initCudaDevice())
        FALCOR_THROW("Failed to initialize CUDA device.");

    // Create our input and output textures
    mpInputTex = Texture::createFromFile(
        getDevice(), AssetResolver::getDefaultResolver().resolvePath(kTexturePath), false, false, ResourceBindFlags::Shared
    );
    if (!mpInputTex)
        FALCOR_THROW("Failed to load texture '{}'", kTexturePath);

    mWidth = mpInputTex->getWidth();
    mHeight = mpInputTex->getHeight();
    mpOutputTex = getDevice()->createTexture2D(
        mWidth, mHeight, mpInputTex->getFormat(), 1, 1, nullptr, ResourceBindFlags::Shared | ResourceBindFlags::ShaderResource
    );

    // Define our usage flags and then map the textures to CUDA surfaces. Surface values of 0
    // indicate an error during mapping. We need to cache mInputSurf and mOutputSurf as
    // mapTextureToSurface() can only be called once per resource.
    uint32_t usageFlags = cudaArrayColorAttachment;

    mInputSurf = cuda_utils::mapTextureToSurface(mpInputTex, usageFlags);
    if (mInputSurf == 0)
        FALCOR_THROW("Input texture to surface mapping failed");

    mOutputSurf = cuda_utils::mapTextureToSurface(mpOutputTex, usageFlags);
    if (mOutputSurf == 0)
        FALCOR_THROW("Output texture to surface mapping failed");
}

void CudaInterop::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    const Falcor::float4 clearColor(0.38f, 0.52f, 0.10f, 1);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);

    // Call the CUDA kernel
    uint32_t format =
        (getFormatType(mpInputTex->getFormat()) == FormatType::Float) ? cudaChannelFormatKindFloat : cudaChannelFormatKindUnsigned;
    launchCopySurface(mInputSurf, mOutputSurf, mWidth, mHeight, format);
    pRenderContext->blit(mpOutputTex->getSRV(), pTargetFbo->getRenderTargetView(0));
}

int runMain(int argc, char** argv)
{
    SampleAppConfig config;
    config.windowDesc.title = "Falcor-Cuda Interop";
    config.windowDesc.resizableWindow = true;

    CudaInterop cudaInterop(config);
    return cudaInterop.run();
}

int main(int argc, char** argv)
{
    return catchAndReportAllExceptions([&] { return runMain(argc, argv); });
}
