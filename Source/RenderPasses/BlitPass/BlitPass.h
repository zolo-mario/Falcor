#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"

using namespace Falcor;

/**
 * Render pass that blits an input texture to an output texture.
 *
 * This pass is useful for format conversion.
 */
class BlitPass : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(BlitPass, "BlitPass", "Blit a texture into a different texture.");

    static ref<BlitPass> create(ref<Device> pDevice, const Properties& props) { return make_ref<BlitPass>(pDevice, props); }

    BlitPass(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;

    // Scripting functions
    TextureFilteringMode getFilter() const { return mFilter; }
    void setFilter(TextureFilteringMode filter) { mFilter = filter; }

private:
    void parseProperties(const Properties& props);

    TextureFilteringMode mFilter = TextureFilteringMode::Linear;
    ResourceFormat mOutputFormat = ResourceFormat::Unknown;
};
