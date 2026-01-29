#include "RenderPassHelpers.h"
#include "RenderPass.h"
#include "Core/Error.h"
#include "Utils/Scripting/ScriptBindings.h"

namespace Falcor
{
uint2 RenderPassHelpers::calculateIOSize(const IOSize selection, const uint2 fixedSize, const uint2 windowSize)
{
    uint2 sz = {};
    if (selection == RenderPassHelpers::IOSize::Fixed)
        sz = fixedSize;
    else if (selection == RenderPassHelpers::IOSize::Full)
        sz = windowSize;
    else if (selection == RenderPassHelpers::IOSize::Half)
        sz = windowSize / uint2(2);
    else if (selection == RenderPassHelpers::IOSize::Quarter)
        sz = windowSize / uint2(4);
    else if (selection == RenderPassHelpers::IOSize::Double)
        sz = windowSize * uint2(2);
    else
        FALCOR_ASSERT(selection == RenderPassHelpers::IOSize::Default);
    return sz;
}

} // namespace Falcor
