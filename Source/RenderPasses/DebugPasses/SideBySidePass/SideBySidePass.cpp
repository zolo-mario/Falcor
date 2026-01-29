#include "SideBySidePass.h"

namespace
{
const std::string kImageLeftBound = "imageLeftBound";

// Where is our shader located?
const std::string kSplitShader = "RenderPasses/DebugPasses/SideBySidePass/SideBySide.ps.slang";
} // namespace

SideBySidePass::SideBySidePass(ref<Device> pDevice, const Properties& props) : ComparisonPass(pDevice)
{
    createProgram();

    for (const auto& [key, value] : props)
    {
        if (key == kImageLeftBound)
            mImageLeftBound = value;
        else if (!parseKeyValuePair(key, value))
        {
            logWarning("Unknown property '{}' in a SideBySidePass properties.", key);
        }
    }
}

void SideBySidePass::createProgram()
{
    // Create our shader that splits the screen.
    mpSplitShader = FullScreenPass::create(mpDevice, kSplitShader);
}

void SideBySidePass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    mpSplitShader->getRootVar()["GlobalCB"]["gLeftBound"] = mImageLeftBound;
    ComparisonPass::execute(pRenderContext, renderData);
}

void SideBySidePass::renderUI(Gui::Widgets& widget)
{
    uint32_t width = pDstFbo ? pDstFbo->getWidth() : 0;
    widget.slider("View Slider", mImageLeftBound, 0u, width / 2);
    ComparisonPass::renderUI(widget);
}
