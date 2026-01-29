#include "RenderGraphExe.h"
#include "Utils/Timing/Profiler.h"

namespace Falcor
{
void RenderGraphExe::execute(const Context& ctx)
{
    FALCOR_PROFILE(ctx.pRenderContext, "RenderGraphExe::execute()");

    for (const auto& pass : mExecutionList)
    {
        FALCOR_PROFILE(ctx.pRenderContext, pass.name);

        RenderData renderData(pass.name, *mpResourceCache, ctx.passesDictionary, ctx.defaultTexDims, ctx.defaultTexFormat);
        pass.pPass->execute(ctx.pRenderContext, renderData);
    }
}

void RenderGraphExe::renderUI(RenderContext* pRenderContext, Gui::Widgets& widget)
{
    for (const auto& p : mExecutionList)
    {
        const auto& pPass = p.pPass;

        if (auto passGroup = widget.group(p.name))
        {
            // Create a unique ID scope per render pass so we can have multiple instances of a render render using the same widget IDs.
            IDScope idScope(p.pPass.get());

            const auto& desc = pPass->getDesc();
            if (desc.size())
                passGroup.tooltip(desc);
            pPass->renderUI(pRenderContext, passGroup);
        }
    }
}

void RenderGraphExe::renderOverlayUI(RenderContext* pRenderContext)
{
    for (const auto& p : mExecutionList)
    {
        const auto& pPass = p.pPass;
        pPass->renderOverlayUI(pRenderContext);
    }
}

bool RenderGraphExe::onMouseEvent(const MouseEvent& mouseEvent)
{
    bool b = false;
    for (const auto& p : mExecutionList)
    {
        const auto& pPass = p.pPass;
        b = b || pPass->onMouseEvent(mouseEvent);
    }
    return b;
}

bool RenderGraphExe::onKeyEvent(const KeyboardEvent& keyEvent)
{
    bool b = false;
    for (const auto& p : mExecutionList)
    {
        const auto& pPass = p.pPass;
        b = b || pPass->onKeyEvent(keyEvent);
    }
    return b;
}

void RenderGraphExe::onHotReload(HotReloadFlags reloaded)
{
    for (const auto& p : mExecutionList)
    {
        const auto& pPass = p.pPass;
        pPass->onHotReload(reloaded);
    }
}

void RenderGraphExe::insertPass(const std::string& name, const ref<RenderPass>& pPass)
{
    mExecutionList.push_back(Pass(name, pPass));
}

ref<Resource> RenderGraphExe::getResource(const std::string& name) const
{
    FALCOR_ASSERT(mpResourceCache);
    return mpResourceCache->getResource(name);
}

void RenderGraphExe::setInput(const std::string& name, const ref<Resource>& pResource)
{
    mpResourceCache->registerExternalResource(name, pResource);
}
} // namespace Falcor
