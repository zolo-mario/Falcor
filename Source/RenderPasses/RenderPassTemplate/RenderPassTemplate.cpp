#include "RenderPassTemplate.h"

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, RenderPassTemplate>();
}

RenderPassTemplate::RenderPassTemplate(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice) {}

Properties RenderPassTemplate::getProperties() const
{
    return {};
}

RenderPassReflection RenderPassTemplate::reflect(const CompileData& compileData)
{
    // Define the required resources here
    RenderPassReflection reflector;
    // reflector.addOutput("dst");
    // reflector.addInput("src");
    return reflector;
}

void RenderPassTemplate::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // renderData holds the requested resources
    // auto& pTexture = renderData.getTexture("src");
}

void RenderPassTemplate::renderUI(Gui::Widgets& widget) {}
