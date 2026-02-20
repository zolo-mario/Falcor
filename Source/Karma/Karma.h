#pragma once
#include "Falcor.h"
#include "Core/SampleApp.h"
#include "Core/SampleBase.h"

using namespace Falcor;

namespace Karma
{
class KarmaApp : public SampleApp
{
public:
    explicit KarmaApp(const SampleAppConfig& config);
    ~KarmaApp();

    void onLoad(RenderContext* pRenderContext) override;
    void onShutdown() override;
    void onResize(uint32_t width, uint32_t height) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onHotReload(HotReloadFlags reloaded) override;

private:
    void renderSampleTree(Gui* pGui);
    void selectSample(const std::string& path, const std::string& type);
    std::vector<std::string> splitPath(const std::string& path);

    std::unique_ptr<SampleBase> mpActiveSample;
    std::string mActiveSamplePath;
};
} // namespace Karma
