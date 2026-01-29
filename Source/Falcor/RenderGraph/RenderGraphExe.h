#pragma once
#include "RenderPass.h"
#include "ResourceCache.h"
#include "Core/Macros.h"
#include "Core/HotReloadFlags.h"
#include "Core/API/Formats.h"
#include "Utils/Math/Vector.h"
#include "Utils/UI/Gui.h"
#include "Utils/Dictionary.h"
#include <memory>
#include <string>
#include <vector>

namespace Falcor
{
class RenderGraphCompiler;
class RenderContext;

class FALCOR_API RenderGraphExe
{
public:
    struct Context
    {
        RenderContext* pRenderContext;
        Dictionary& passesDictionary;
        uint2 defaultTexDims;
        ResourceFormat defaultTexFormat;
    };

    /**
     * Execute the graph
     */
    void execute(const Context& ctx);

    /**
     * Render the UI
     */
    void renderUI(RenderContext* pRenderContext, Gui::Widgets& widget);

    /**
     * Render the overlay UI
     */
    void renderOverlayUI(RenderContext* pRenderContext);

    /**
     * Mouse event handler.
     * Returns true if the event was handled by the object, false otherwise
     */
    bool onMouseEvent(const MouseEvent& mouseEvent);

    /**
     * Keyboard event handler
     * Returns true if the event was handled by the object, false otherwise
     */
    bool onKeyEvent(const KeyboardEvent& keyEvent);

    /**
     * Called upon hot reload (by pressing F5).
     * @param[in] reloaded Resources that have been reloaded.
     */
    void onHotReload(HotReloadFlags reloaded);

    /**
     * Get a resource from the cache
     */
    ref<Resource> getResource(const std::string& name) const;

    /**
     * Set an external input resource
     * @param[in] name Input name. Has the format `renderPassName.resourceName`
     * @param[in] pResource The resource to bind. If this is nullptr, will unregister the resource
     */
    void setInput(const std::string& name, const ref<Resource>& pResource);

private:
    friend class RenderGraphCompiler;

    void insertPass(const std::string& name, const ref<RenderPass>& pPass);

    struct Pass
    {
        std::string name;
        ref<RenderPass> pPass;

    private:
        friend class RenderGraphExe; // Force RenderGraphCompiler to use insertPass() by hiding this Ctor from it
        Pass(const std::string& name_, const ref<RenderPass>& pPass_) : name(name_), pPass(pPass_) {}
    };

    std::vector<Pass> mExecutionList;
    std::unique_ptr<ResourceCache> mpResourceCache;
};
} // namespace Falcor
