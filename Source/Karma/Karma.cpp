#include "Karma.h"
#include "Core/Plugin.h"
#include "Utils/CrashHandler.h"
#include "Utils/Logger.h"
#include "Utils/UI/TextRenderer.h"

#include <args.hxx>
#include <imgui.h>
#include <map>
#include <sstream>

FALCOR_EXPORT_D3D12_AGILITY_SDK

namespace Karma
{
KarmaApp::KarmaApp(const SampleAppConfig& config, const KarmaAppOptions& options) : SampleApp(config), mInitialSample(options.initialSample) {}

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
        getTextRenderer().render(pRenderContext, "Select a sample from the tree", pTargetFbo, {20, 20});
    }
}

void KarmaApp::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Samples", {280, 400}, {10, 80});
    renderSampleTree(pGui);
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

std::vector<std::string> KarmaApp::splitPath(const std::string& path)
{
    std::vector<std::string> parts;
    std::istringstream ss(path);
    std::string part;
    while (std::getline(ss, part, '/'))
    {
        if (!part.empty())
            parts.push_back(part);
    }
    return parts;
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
        }
    }
}

void KarmaApp::renderSampleTree(Gui* pGui)
{
    struct TreeNode
    {
        std::map<std::string, TreeNode> children;
        std::string path;
        std::string type;
    };

    TreeNode root;
    for (const auto& [type, info] : PluginManager::instance().getInfos<SampleBase>())
    {
        const std::string& path = info.path;
        auto parts = splitPath(path);
        TreeNode* node = &root;
        for (size_t i = 0; i < parts.size(); ++i)
        {
            if (node->children.find(parts[i]) == node->children.end())
            {
                node->children[parts[i]] = TreeNode{};
                node->children[parts[i]].path = (node->path.empty() ? "" : node->path + "/") + parts[i];
            }
            node = &node->children[parts[i]];
            if (i == parts.size() - 1)
            {
                node->type = type;
            }
        }
    }

    std::function<void(const TreeNode&, const std::string&)> renderNode = [&](const TreeNode& node, const std::string& name)
    {
        if (node.children.empty())
        {
            bool selected = (mActiveSamplePath == node.path);
            if (ImGui::Selectable(name.c_str(), selected))
                selectSample(node.path, node.type);
        }
        else
        {
            bool open = ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
            if (open)
            {
                if (!node.type.empty())
                {
                    bool selected = (mActiveSamplePath == node.path);
                    if (ImGui::Selectable(("▶ " + name).c_str(), selected))
                        selectSample(node.path, node.type);
                }
                for (const auto& [childName, childNode] : node.children)
                    renderNode(childNode, childName);
                ImGui::TreePop();
            }
        }
    };

    for (const auto& [name, node] : root.children)
        renderNode(node, name);
}

} // namespace Karma

int runMain(int argc, char** argv)
{
    args::ArgumentParser parser("Karma - Sample Browser.");
    parser.helpParams.programName = "Karma";
    args::HelpFlag helpFlag(parser, "help", "Display this help menu.", {'h', "help"});
    args::ValueFlag<std::string> sampleFlag(parser, "name", "Sample to load on startup (path or type, e.g. Samples/Desktop/D3D12ExecuteIndirect).", {'s', "sample"});
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

    Karma::KarmaApp app(config, options);
    return app.run();
}

int main(int argc, char** argv)
{
    return catchAndReportAllExceptions([&]() { return runMain(argc, argv); });
}
