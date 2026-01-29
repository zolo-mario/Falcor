#include "RasterizerState.h"
#include "GFXAPI.h"
#include "Core/ObjectPython.h"
#include "Utils/Scripting/ScriptBindings.h"

namespace Falcor
{
ref<RasterizerState> RasterizerState::create(const Desc& desc)
{
    return ref<RasterizerState>(new RasterizerState(desc));
}

RasterizerState::~RasterizerState() = default;

FALCOR_SCRIPT_BINDING(RasterizerState)
{
    pybind11::class_<RasterizerState, ref<RasterizerState>>(m, "RasterizerState");
}
} // namespace Falcor
