#pragma once
#include "Falcor.h"
#include "Core/SampleApp.h"
#include "Core/SampleBase.h"
#include "Utils/Properties.h"

using namespace Falcor;

namespace Karma
{
struct KarmaAppOptions
{
    std::string initialSample; ///< Sample path (e.g. "Samples/Desktop/D3D12ExecuteIndirect") or type name to load on startup.
    Properties sampleProps;    ///< Sample parameters from --arg (key=value), passed via setProperties().
};

class KarmaApp : public SampleApp
{
public:
    explicit KarmaApp(const SampleAppConfig& config, const KarmaAppOptions& options = {});
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
    std::string mInitialSample;
    Properties mSampleProps;
};
} // namespace Karma
