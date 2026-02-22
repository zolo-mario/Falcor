#pragma once
#include "Core/Macros.h"
#include "Core/Plugin.h"
#include "Core/SampleApp.h"
#include "Utils/Properties.h"

namespace Falcor
{
class RenderContext;
class Gui;

/**
 * Plugin base class for Karma sample browser.
 * Samples are loaded as plugins (DLLs) and registered via registerPlugin.
 *
 * Core lifecycle (minimum to implement): onLoad, onFrameRender, onResize.
 * Optional hooks: onShutdown, onGuiRender, onKeyEvent, onMouseEvent, onHotReload, setProperties, getProperties.
 */
class FALCOR_API SampleBase
{
public:
    struct PluginInfo
    {
        std::string path; ///< Display path (e.g. "Samples/HelloDXR", "Samples/Desktop/D3D12HelloWorld")
    };

    using PluginCreate = SampleBase* (*)(SampleApp* pHost);

    FALCOR_PLUGIN_BASE_CLASS(SampleBase);

    explicit SampleBase(SampleApp* pHost);
    virtual ~SampleBase();

    /// Core lifecycle
    virtual void onLoad(RenderContext* pRenderContext) {}
    virtual void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) {}
    virtual void onResize(uint32_t width, uint32_t height) {}

    /// Optional hooks (default no-op)
    virtual void onShutdown() {}
    virtual void onGuiRender(Gui* pGui) {}
    virtual bool onKeyEvent(const KeyboardEvent& keyEvent) { return false; }
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) { return false; }
    virtual void onHotReload(HotReloadFlags reloaded) {}

    virtual void setProperties(const Properties& props) {}
    virtual Properties getProperties() const { return {}; }

    const ref<Device>& getDevice() const { return mpHost->getDevice(); }
    const ref<Fbo>& getTargetFbo() const { return mpHost->getTargetFbo(); }
    Clock& getGlobalClock() { return mpHost->getGlobalClock(); }
    const Clock& getGlobalClock() const { return mpHost->getGlobalClock(); }
    TextRenderer& getTextRenderer() { return mpHost->getTextRenderer(); }
    Window* getWindow() { return mpHost->getWindow(); }
    ProgressBar& getProgressBar() { return mpHost->getProgressBar(); }
    const InputState& getInputState() { return mpHost->getInputState(); }
    FrameRate& getFrameRate() { return mpHost->getFrameRate(); }
    void resizeFrameBuffer(uint32_t width, uint32_t height) { mpHost->resizeFrameBuffer(width, height); }
    void renderGlobalUI(Gui* pGui) { mpHost->renderGlobalUI(pGui); }

protected:
    SampleApp* mpHost;
};
} // namespace Falcor
