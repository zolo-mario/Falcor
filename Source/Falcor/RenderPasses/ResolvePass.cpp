#include "ResolvePass.h"
#include "Core/API/RenderContext.h"
#include "Utils/Logger.h"

namespace Falcor
{
static const std::string kDst = "dst";
static const std::string kSrc = "src";

ResolvePass::ResolvePass(ref<Device> pDevice) : RenderPass(pDevice) {}

RenderPassReflection ResolvePass::reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    reflector.addInput(kSrc, "Multi-sampled texture").format(mFormat).texture2D(0, 0, 0);
    reflector.addOutput(kDst, "Destination texture. Must have a single sample").format(mFormat).texture2D(0, 0, 1);
    return reflector;
}

void ResolvePass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    auto pSrcTex = renderData.getTexture(kSrc);
    auto pDstTex = renderData.getTexture(kDst);

    if (pSrcTex && pDstTex)
    {
        if (pSrcTex->getSampleCount() == 1)
        {
            logWarning("ResolvePass::execute() - Cannot resolve from a non-multisampled texture.");
            return;
        }

        pRenderContext->resolveResource(pSrcTex, pDstTex);
    }
    else
    {
        logWarning("ResolvePass::execute() - missing an input or output resource.");
    }
}
} // namespace Falcor
