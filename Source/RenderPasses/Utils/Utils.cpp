#include "CrossFade/CrossFade.h"
#include "Composite/Composite.h"
#include "GaussianBlur/GaussianBlur.h"

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, CrossFade>();

    registry.registerClass<RenderPass, Composite>();

    registry.registerClass<RenderPass, GaussianBlur>();
    ScriptBindings::registerBinding(GaussianBlur::registerBindings);
}
