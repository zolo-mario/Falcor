#include "PixelZoom.h"
#include "Core/API/RenderContext.h"
#include "Utils/UI/InputTypes.h"

namespace Falcor
{
static void clampToEdge(float2& pix, uint32_t width, uint32_t height, uint32_t offset)
{
    float2 posOffset = pix + float2(offset, offset);
    float2 negOffset = pix - float2(offset, offset);

    // x
    if (posOffset.x > width)
    {
        pix.x = pix.x - (posOffset.x - width);
    }
    else if (negOffset.x < 0)
    {
        pix.x = pix.x - negOffset.x;
    }

    // y
    if (posOffset.y > height)
    {
        pix.y = pix.y - (posOffset.y - height);
    }
    else if (negOffset.y < 0)
    {
        pix.y = pix.y - negOffset.y;
    }
}

PixelZoom::PixelZoom(ref<Device> pDevice, const Fbo* pBackbuffer) : mpDevice(pDevice)
{
    onResize(pBackbuffer);
}

void PixelZoom::onResize(const Fbo* pBackbuffer)
{
    FALCOR_ASSERT(pBackbuffer);
    const Fbo::Desc& desc = pBackbuffer->getDesc();
    mpSrcBlitFbo = Fbo::create2D(mpDevice, pBackbuffer->getWidth(), pBackbuffer->getHeight(), desc);
    if (mpDstBlitFbo == nullptr)
    {
        mpDstBlitFbo = Fbo::create2D(mpDevice, mDstZoomSize, mDstZoomSize, desc);
    }
}

void PixelZoom::render(RenderContext* pCtx, Fbo* backBuffer)
{
    if (mShouldZoom)
    {
        // copy backbuffer into src blit fbo
        pCtx->copyResource(mpSrcBlitFbo->getColorTexture(0).get(), backBuffer->getColorTexture(0).get());

        // blit src blit fbo into dst blit fbo
        uint32_t offset = mSrcZoomSize / 2;
        float2 srcPix = float2(mMousePos.x * backBuffer->getWidth(), mMousePos.y * backBuffer->getHeight());
        clampToEdge(srcPix, backBuffer->getWidth(), backBuffer->getHeight(), offset);
        float4 srcRect = float4(srcPix.x - offset, srcPix.y - offset, srcPix.x + offset, srcPix.y + offset);
        float4 dstRect = float4(0, 0, mDstZoomSize, mDstZoomSize);
        pCtx->blit(
            mpSrcBlitFbo->getColorTexture(0)->getSRV(),
            mpDstBlitFbo->getColorTexture(0)->getRTV(),
            srcRect,
            dstRect,
            TextureFilteringMode::Point
        );

        // blit dst blt fbo into back buffer
        offset = mDstZoomSize / 2;
        clampToEdge(srcPix, backBuffer->getWidth(), backBuffer->getHeight(), offset);
        srcRect = dstRect;
        dstRect = float4(srcPix.x - offset, srcPix.y - offset, srcPix.x + offset, srcPix.y + offset);
        pCtx->blit(
            mpDstBlitFbo->getColorTexture(0)->getSRV(),
            backBuffer->getColorTexture(0)->getRTV(),
            srcRect,
            dstRect,
            TextureFilteringMode::Point
        );
    }
}

bool PixelZoom::onMouseEvent(const MouseEvent& me)
{
    if (mShouldZoom)
    {
        mMousePos = me.pos;
        // negative to swap scroll up to zoom in and scroll down to zoom out
        int32_t zoomDelta = -1 * mZoomCoefficient * (int32_t)me.wheelDelta.y;
        mSrcZoomSize = std::max(mSrcZoomSize + zoomDelta, 3);
        return me.type != MouseEvent::Type::Move; // Do not inhibit other passes from receiving mouse movement events.
    }
    return false;
}

bool PixelZoom::onKeyboardEvent(const KeyboardEvent& ke)
{
    if (ke.type == KeyboardEvent::Type::KeyPressed || ke.type == KeyboardEvent::Type::KeyReleased)
    {
        if (ke.key == Input::Key::Z)
        {
            mShouldZoom = (ke.type == KeyboardEvent::Type::KeyPressed);
            return true;
        }
    }
    return false;
}

} // namespace Falcor
