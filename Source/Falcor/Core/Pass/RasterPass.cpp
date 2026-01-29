#include "RasterPass.h"
#include "Core/API/RenderContext.h"

namespace Falcor
{
ref<RasterPass> RasterPass::create(ref<Device> pDevice, const ProgramDesc& desc, const DefineList& defines)
{
    return ref<RasterPass>(new RasterPass(pDevice, desc, defines));
}

ref<RasterPass> RasterPass::create(
    ref<Device> pDevice,
    const std::filesystem::path& path,
    const std::string& vsEntry,
    const std::string& psEntry,
    const DefineList& defines
)
{
    ProgramDesc desc;
    desc.addShaderLibrary(path).vsEntry(vsEntry).psEntry(psEntry);
    return create(pDevice, desc, defines);
}

RasterPass::RasterPass(ref<Device> pDevice, const ProgramDesc& progDesc, const DefineList& programDefines)
    : BaseGraphicsPass(pDevice, progDesc, programDefines)
{}

void RasterPass::drawIndexed(RenderContext* pRenderContext, uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
{
    pRenderContext->drawIndexed(mpState.get(), mpVars.get(), indexCount, startIndexLocation, baseVertexLocation);
}

void RasterPass::draw(RenderContext* pRenderContext, uint32_t vertexCount, uint32_t startVertexLocation)
{
    pRenderContext->draw(mpState.get(), mpVars.get(), vertexCount, startVertexLocation);
}
} // namespace Falcor
