#pragma once
#include "Core/API/FBO.h"
#include "Utils/Math/Vector.h"
#include <memory>

namespace Falcor
{
class RenderContext;
struct MouseEvent;
struct KeyboardEvent;

/**
 * Magnifies a region of the screen to assist with inspecting details
 */
class PixelZoom
{
public:
    /// Constructor. Throws an exception if creation failed.
    PixelZoom(ref<Device> pDevice, const Fbo* pBackbuffer);

    /**
     * Does zoom operation if mShouldZoom is true (if ctrl+alt pressed this frame)
     * @param pCtx Pointer to the render context
     * @param backbuffer Pointer to the swap chain FBO
     */
    void render(RenderContext* pCtx, Fbo* backBuffer);

    /**
     * Stores data about mouse needed for zooming
     * @param me the mouse event
     */
    bool onMouseEvent(const MouseEvent& me);

    /**
     * Checks if it should zoom
     * @param ke Keyboard event
     */
    bool onKeyboardEvent(const KeyboardEvent& ke);

    /**
     * Handle resize events
     */
    void onResize(const Fbo* pBackbuffer);

private:
    ref<Device> mpDevice;

    int32_t mSrcZoomSize = 5;
    const uint32_t mDstZoomSize = 200;
    const uint32_t mZoomCoefficient = 4;

    ref<Fbo> mpSrcBlitFbo;
    ref<Fbo> mpDstBlitFbo;
    float2 mMousePos = {};
    bool mShouldZoom = false;
};
} // namespace Falcor
