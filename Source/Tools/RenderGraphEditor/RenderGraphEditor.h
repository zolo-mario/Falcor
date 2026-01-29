#pragma once
#include "Falcor.h"
#include "Core/SampleApp.h"
#include "RenderGraph/RenderGraph.h"
#include "RenderGraph/RenderGraphUI.h"

using namespace Falcor;

class RenderGraphEditor : public SampleApp
{
public:
    struct Options
    {
        std::string graphFile;
        std::string graphName;
        bool runFromMogwai = false;
    };

    RenderGraphEditor(const SampleAppConfig& config, const Options& options);
    ~RenderGraphEditor();

    void onLoad(RenderContext* pRenderContext) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onResize(uint32_t width, uint32_t height) override;
    void onGuiRender(Gui* pGui) override;
    void onDroppedFile(const std::filesystem::path& path) override;

private:
    void createNewGraph(const std::string& renderGraphName);
    void loadGraphsFromFile(const std::filesystem::path& path, const std::string& graphName = "");
    void serializeRenderGraph(const std::filesystem::path& path);
    void deserializeRenderGraph(const std::filesystem::path& path);
    void renderLogWindow(Gui::Widgets& widget);

    Options mOptions;

    std::vector<ref<RenderGraph>> mpGraphs;
    std::vector<RenderGraphUI> mRenderGraphUIs;
    std::unordered_map<std::string, uint32_t> mGraphNamesToIndex;
    size_t mCurrentGraphIndex;
    uint2 mWindowSize;
    std::string mCurrentLog;
    std::string mNextGraphString;
    std::string mCurrentGraphOutput;
    std::string mGraphOutputEditString;
    std::filesystem::path mUpdateFilePath;
    ref<Texture> mpDefaultIconTex;

    Gui::DropdownList mOpenGraphNames;
    bool mShowCreateGraphWindow = false;
    bool mShowDebugWindow = false;
    bool mViewerRunning = false;
    size_t mViewerProcess = 0;
    bool mResetGuiWindows = false;
};
