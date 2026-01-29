#include "Swapchain.h"
#include "Device.h"
#include "GFXAPI.h"
#include "GFXHelpers.h"

namespace Falcor
{

Swapchain::Swapchain(ref<Device> pDevice, const Desc& desc, WindowHandle windowHandle) : mpDevice(pDevice), mDesc(desc)
{
    FALCOR_ASSERT(mpDevice);

    FALCOR_CHECK(desc.format != ResourceFormat::Unknown, "Invalid format");
    FALCOR_CHECK(desc.width > 0, "Invalid width");
    FALCOR_CHECK(desc.height > 0, "Invalid height");
    FALCOR_CHECK(desc.imageCount > 0, "Invalid image count");

    gfx::ISwapchain::Desc gfxDesc = {};
    gfxDesc.format = getGFXFormat(desc.format);
    gfxDesc.width = desc.width;
    gfxDesc.height = desc.height;
    gfxDesc.imageCount = desc.imageCount;
    gfxDesc.enableVSync = desc.enableVSync;
    gfxDesc.queue = mpDevice->getGfxCommandQueue();
#if FALCOR_WINDOWS
    gfx::WindowHandle gfxWindowHandle = gfx::WindowHandle::FromHwnd(windowHandle);
#elif FALCOR_LINUX
    gfx::WindowHandle gfxWindowHandle = gfx::WindowHandle::FromXWindow(windowHandle.pDisplay, windowHandle.window);
#endif
    FALCOR_GFX_CALL(mpDevice->getGfxDevice()->createSwapchain(gfxDesc, gfxWindowHandle, mGfxSwapchain.writeRef()));

    prepareImages();
}

const ref<Texture>& Swapchain::getImage(uint32_t index) const
{
    FALCOR_ASSERT(index <= mImages.size());
    return mImages[index];
}

void Swapchain::present()
{
    FALCOR_GFX_CALL(mGfxSwapchain->present());
}

int Swapchain::acquireNextImage()
{
    return mGfxSwapchain->acquireNextImage();
}

void Swapchain::resize(uint32_t width, uint32_t height)
{
    FALCOR_CHECK(width > 0, "Invalid width");
    FALCOR_CHECK(height > 0, "Invalid height");

    mImages.clear();
    mpDevice->wait();
    FALCOR_GFX_CALL(mGfxSwapchain->resize(width, height));
    prepareImages();
}

bool Swapchain::isOccluded()
{
    return mGfxSwapchain->isOccluded();
}

void Swapchain::setFullScreenMode(bool mode)
{
    FALCOR_GFX_CALL(mGfxSwapchain->setFullScreenMode(mode));
}

void Swapchain::prepareImages()
{
    for (uint32_t i = 0; i < mDesc.imageCount; ++i)
    {
        Slang::ComPtr<gfx::ITextureResource> resource;
        FALCOR_GFX_CALL(mGfxSwapchain->getImage(i, resource.writeRef()));
        mImages.push_back(mpDevice->createTextureFromResource(
            resource,
            Texture::Type::Texture2D,
            mDesc.format,
            mDesc.width,
            mDesc.height,
            1,
            1,
            1,
            1,
            ResourceBindFlags::RenderTarget,
            Resource::State::Undefined
        ));
    }
}

} // namespace Falcor
