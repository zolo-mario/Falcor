#include "GraphicsGems1.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

// Bring in the original Graphics Gems 1 C source under extern "C" so the
// C++ linker resolves the undecorated C function names correctly.
extern "C"
{
#include "src/HSLtoRGB.c"
}

GraphicsGems1::GraphicsGems1(SampleApp* pHost) : SampleBase(pHost) {}

SampleBase* GraphicsGems1::create(SampleApp* pHost)
{
    return new GraphicsGems1(pHost);
}

void GraphicsGems1::onLoad(RenderContext* /*pRenderContext*/)
{
    recomputeFromHSL();
    recomputeFromRGB();
}

void GraphicsGems1::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    // Fill the entire framebuffer with the colour produced by the C gem.
    pRenderContext->clearFbo(pTargetFbo.get(), float4(mRgbFromHSL, 1.0f), 1.0f, 0, FboAttachmentType::All);
}

void GraphicsGems1::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Graphics Gems I  -  HSL <-> RGB", {400, 420});
    renderGlobalUI(pGui);

    w.text("Gem: \"A Fast HSL-to-RGB Transform\"");
    w.text("Author: Ken Fishkin, Pixar Inc., 1989");
    w.text("Book:   Graphics Gems (Academic Press, 1990)");
    w.separator();

    // --- HSL -> RGB ---
    w.text("HSL  ->  RGB");

    bool hslChanged = false;
    hslChanged |= w.slider("Hue  (H)", mH, 0.0f, 1.0f);
    hslChanged |= w.slider("Saturation (S)", mS, 0.0f, 1.0f);
    hslChanged |= w.slider("Lightness  (L)", mL, 0.0f, 1.0f);
    if (hslChanged)
        recomputeFromHSL();

    w.rgbColor("RGB result", mRgbFromHSL);
    w.separator();

    // --- RGB -> HSL ---
    w.text("RGB  ->  HSL");

    bool rgbChanged = w.rgbColor("RGB input", mRgbInput);
    if (rgbChanged)
        recomputeFromRGB();

    w.text(fmt::format("H = {:.4f}", mHFromRGB));
    w.text(fmt::format("S = {:.4f}", mSFromRGB));
    w.text(fmt::format("L = {:.4f}", mLFromRGB));
}

void GraphicsGems1::recomputeFromHSL()
{
    double r, g, b;
    HSL_to_RGB(mH, mS, mL, &r, &g, &b);
    mRgbFromHSL = {(float)r, (float)g, (float)b};
}

void GraphicsGems1::recomputeFromRGB()
{
    double h, s, l;
    RGB_to_HSL(mRgbInput.x, mRgbInput.y, mRgbInput.z, &h, &s, &l);
    mHFromRGB = (float)h;
    mSFromRGB = (float)s;
    mLFromRGB = (float)l;
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<SampleBase, GraphicsGems1>();
}
