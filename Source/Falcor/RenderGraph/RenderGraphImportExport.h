#pragma once
#include "RenderGraph.h"
#include "Core/Macros.h"
#include <filesystem>
#include <string>
#include <vector>

namespace Falcor
{
class FALCOR_API RenderGraphImporter
{
public:
    /**
     * Import a graph from a file.
     * @param[in] graphName The name of the graph to import
     * @param[in] path The graphs file path. If the path is empty, the function will search for a file called `<graphName>.py`
     * @param[in] funcName The function name inside the graph script. If the string is empty, will try invoking a function called
     * `render_graph_<graphName>()`
     * @return A new render-graph object or nullptr if something went horribly wrong
     */
    static ref<RenderGraph> import(std::string graphName, std::filesystem::path path = {}, std::string funcName = {});

    /**
     * Import all the graphs found in the script's global namespace
     */
    static std::vector<ref<RenderGraph>> importAllGraphs(const std::filesystem::path& path);
};

class FALCOR_API RenderGraphExporter
{
public:
    static std::string getIR(const ref<RenderGraph>& pGraph);
    static std::string getFuncName(const std::string& graphName);
    static bool save(const ref<RenderGraph>& pGraph, std::filesystem::path path = {});
};
} // namespace Falcor
