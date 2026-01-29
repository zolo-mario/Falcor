#include "Types.h"
#include "Utils/Scripting/ScriptBindings.h"

namespace Falcor
{

FALCOR_SCRIPT_BINDING(Types)
{
    pybind11::falcor_enum<ShaderModel> shaderModel(m, "ShaderModel");
    ScriptBindings::addEnumBinaryOperators(shaderModel);

    pybind11::falcor_enum<DataType>(m, "DataType");
    // Register attributes on the main module. This is similar to numpy/pytorch.
    for (const auto& item : EnumInfo<DataType>::items())
        m.attr(item.second.c_str()) = item.first;

    pybind11::falcor_enum<ComparisonFunc>(m, "ComparisonFunc");
}

} // namespace Falcor
