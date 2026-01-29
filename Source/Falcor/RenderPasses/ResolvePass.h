#pragma once
#include "Core/Macros.h"
#include "Core/API/Formats.h"
#include "RenderGraph/RenderPass.h"
#include <memory>

namespace Falcor
{
class RenderContext;
class RenderData;

class FALCOR_API ResolvePass : public RenderPass
{
public:
    // This pass is not dynamically loaded from a plugin library,
    // but we still need to provide plugin type and info fields.
    FALCOR_PLUGIN_CLASS(ResolvePass, "ResolvePass", "Resolve a multi-sampled texture.");

    static ref<ResolvePass> create(ref<Device> pDevice, const Properties& props = {}) { return make_ref<ResolvePass>(pDevice); }

    ResolvePass(ref<Device> pDevice);

    void setFormat(ResourceFormat format) { mFormat = format; }
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;

private:
    ResourceFormat mFormat = ResourceFormat::Unknown;
};
} // namespace Falcor
