#include "VertexLayout.h"
#include "Core/ObjectPython.h"
#include "Utils/Scripting/ScriptBindings.h"

namespace Falcor
{
FALCOR_SCRIPT_BINDING(VertexLayout)
{
    pybind11::class_<VertexLayout, ref<VertexLayout>>(m, "VertexLayout");
}
} // namespace Falcor
