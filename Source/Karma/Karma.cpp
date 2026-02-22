#include "Karma.h"
#include "Core/Plugin.h"
#include "Utils/CrashHandler.h"
#include "Utils/Logger.h"
#include "Utils/UI/TextRenderer.h"

#include <args.hxx>
#include <cstdlib>
#include <imgui.h>
#include <string>

FALCOR_EXPORT_D3D12_AGILITY_SDK

namespace Karma
{
KarmaApp::KarmaApp(const SampleAppConfig& config, const KarmaAppOptions& options)
    : SampleApp(config), mInitialSample(options.initialSample), mSampleProps(options.sampleProps)
{}

KarmaApp::~KarmaApp() {}

void KarmaApp::onLoad(RenderContext* pRenderContext)
{
    PluginManager::instance().loadAllPlugins();

    if (!mInitialSample.empty())
    {
        std::string path, type;
        for (const auto& [t, info] : PluginManager::instance().getInfos<SampleBase>())
        {
            if (info.path == mInitialSample || t == mInitialSample)
            {
                path = info.path;
                type = t;
                break;
            }
        }
        if (!type.empty())
            selectSample(path, type);
        else
            logWarning("Karma: sample '{}' not found. Use path (e.g. Samples/Desktop/D3D12ExecuteIndirect) or type name.", mInitialSample);
    }
}

void KarmaApp::onShutdown()
{
    if (mpActiveSample)
        mpActiveSample->onShutdown();
    mpActiveSample.reset();
}

void KarmaApp::onResize(uint32_t width, uint32_t height)
{
    if (mpActiveSample)
        mpActiveSample->onResize(width, height);
}

void KarmaApp::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    if (mpActiveSample)
        mpActiveSample->onFrameRender(pRenderContext, pTargetFbo);
    else
    {
        const float4 clearColor(0.2f, 0.2f, 0.2f, 1.f);
        pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
        getTextRenderer().render(pRenderContext, "Select a sample from the list", pTargetFbo, {20, 20});
    }
}

void KarmaApp::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Samples", {280, 400}, {10, 80});
    renderSampleList(pGui);
    w.release();

    if (mpActiveSample)
        mpActiveSample->onGuiRender(pGui);

    renderGlobalUI(pGui);
}

bool KarmaApp::onKeyEvent(const KeyboardEvent& keyEvent)
{
    if (mpActiveSample && mpActiveSample->onKeyEvent(keyEvent))
        return true;
    return false;
}

bool KarmaApp::onMouseEvent(const MouseEvent& mouseEvent)
{
    if (mpActiveSample && mpActiveSample->onMouseEvent(mouseEvent))
        return true;
    return false;
}

void KarmaApp::onHotReload(HotReloadFlags reloaded)
{
    if (mpActiveSample)
        mpActiveSample->onHotReload(reloaded);
}

void KarmaApp::selectSample(const std::string& path, const std::string& type)
{
    if (mActiveSamplePath == path)
        return;

    if (mpActiveSample)
        mpActiveSample->onShutdown();
    mpActiveSample.reset();
    mActiveSamplePath = path;

    if (!type.empty())
    {
        SampleBase* p = PluginManager::instance().createClass<SampleBase>(type, this);
        if (p)
        {
            mpActiveSample.reset(p);
            mpActiveSample->onLoad(getRenderContext());
            if (!mSampleProps.empty())
                mpActiveSample->setProperties(mSampleProps);
        }
    }
}

void KarmaApp::renderSampleList(Gui* pGui)
{
    for (const auto& [type, info] : PluginManager::instance().getInfos<SampleBase>())
    {
        bool selected = (mActiveSamplePath == info.path);
        if (ImGui::Selectable(info.path.c_str(), selected))
            selectSample(info.path, type);
    }
}

} // namespace Karma

int runMain(int argc, char** argv)
{
    args::ArgumentParser parser("Karma - Sample Browser.");
    parser.helpParams.programName = "Karma";
    args::HelpFlag helpFlag(parser, "help", "Display this help menu.", {'h', "help"});
    args::ValueFlag<std::string> sampleFlag(parser, "name", "Sample to load on startup (path or type, e.g. Samples/Desktop/D3D12ExecuteIndirect).", {'s', "sample"});
    args::ValueFlagList<std::string> argFlag(parser, "key=value", "Sample parameter (repeatable).", {'a', "arg"});
    args::Flag headlessFlag(parser, "", "Start without opening a window.", {"headless"});

    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (const args::Help&)
    {
        std::cout << parser;
        return 0;
    }
    catch (const args::ParseError& e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    SampleAppConfig config;
    config.windowDesc.title = "Karma - Sample Browser";
    config.windowDesc.resizableWindow = true;
    if (headlessFlag)
        config.headless = true;

    Karma::KarmaAppOptions options;
    if (sampleFlag)
        options.initialSample = args::get(sampleFlag);
    if (argFlag)
    {
        for (const auto& kv : args::get(argFlag))
        {
            size_t eq = kv.find('=');
            if (eq != std::string::npos)
            {
                std::string key = kv.substr(0, eq);
                std::string value = kv.substr(eq + 1);
                // Store as number if value parses as float, else as string (no try-catch)
                const char* start = value.c_str();
                char* end = nullptr;
                double d = std::strtod(start, &end);
                if (end != start && end && *end == '\0')
                {
                    options.sampleProps[key] = d;
                    continue;
                }
                options.sampleProps[key] = value;
            }
        }
    }

    Karma::KarmaApp app(config, options);
    return app.run();
}

int main(int argc, char** argv)
{
    return catchAndReportAllExceptions([&]() { return runMain(argc, argv); });
}
