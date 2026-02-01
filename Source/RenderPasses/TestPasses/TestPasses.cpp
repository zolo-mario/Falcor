#include "TestRtProgram.h"

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, TestRtProgram>();

    ScriptBindings::registerBinding(TestRtProgram::registerScriptBindings);
}
