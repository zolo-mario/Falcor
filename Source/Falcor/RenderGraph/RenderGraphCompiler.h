#pragma once
#include "RenderPass.h"
#include "RenderPassReflection.h"
#include "ResourceCache.h"
#include "RenderGraphExe.h"
#include "Core/Macros.h"
#include <string>
#include <utility>
#include <vector>

namespace Falcor
{
class RenderGraph;

class FALCOR_API RenderGraphCompiler
{
public:
    struct Dependencies
    {
        ResourceCache::DefaultProperties defaultResourceProps;
        ResourceCache::ResourcesMap externalResources;
    };
    static std::unique_ptr<RenderGraphExe> compile(RenderGraph& graph, RenderContext* pRenderContext, const Dependencies& dependencies);

private:
    RenderGraphCompiler(RenderGraph& graph, const Dependencies& dependencies);

    RenderGraph& mGraph;
    ref<Device> mpDevice;
    const Dependencies& mDependencies;

    struct PassData
    {
        uint32_t index;
        ref<RenderPass> pPass;
        std::string name;
        RenderPassReflection reflector;
    };
    std::vector<PassData> mExecutionList;

    // TODO Better way to track history, or avoid changing the original graph altogether?
    struct
    {
        std::vector<std::string> generatedPasses;
        std::vector<std::pair<std::string, std::string>> removedEdges;
    } mCompilationChanges;

    void resolveExecutionOrder();
    void compilePasses(RenderContext* pRenderContext);
    bool insertAutoPasses();
    void allocateResources(ref<Device> pDevice, ResourceCache* pResourceCache);
    void validateGraph() const;
    void restoreCompilationChanges();
    RenderPass::CompileData prepPassCompilationData(const PassData& passData);
};
} // namespace Falcor
