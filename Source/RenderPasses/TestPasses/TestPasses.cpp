#include "TestRtProgram.h"
#include "TestPyTorchPass.h"

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, TestRtProgram>();
    registry.registerClass<RenderPass, TestPyTorchPass>();

    ScriptBindings::registerBinding(TestRtProgram::registerScriptBindings);
    ScriptBindings::registerBinding(TestPyTorchPass::registerScriptBindings);
}
