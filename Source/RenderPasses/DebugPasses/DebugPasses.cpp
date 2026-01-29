#include "SplitScreenPass/SplitScreenPass.h"
#include "InvalidPixelDetectionPass/InvalidPixelDetectionPass.h"
#include "SideBySidePass/SideBySidePass.h"
#include "ColorMapPass/ColorMapPass.h"

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, SplitScreenPass>();
    registry.registerClass<RenderPass, InvalidPixelDetectionPass>();
    registry.registerClass<RenderPass, SideBySidePass>();
    registry.registerClass<RenderPass, ColorMapPass>();
}
