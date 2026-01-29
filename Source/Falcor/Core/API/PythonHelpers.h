#pragma once

#include "Core/API/Formats.h"
#include "Core/Program/Program.h"
#include "Utils/Scripting/ScriptBindings.h"
#include "Utils/Scripting/ndarray.h"

#include <optional>

namespace Falcor
{

inline size_t getDtypeByteSize(pybind11::dlpack::dtype dtype)
{
    return (dtype.bits * dtype.lanes) / 8;
}

template<typename... Args>
size_t getNdarraySize(const pybind11::ndarray<Args...>& array)
{
    size_t size = 1;
    for (size_t i = 0; i < array.ndim(); i++)
        size *= array.shape(i);
    return size;
}

template<typename... Args>
size_t getNdarrayByteSize(const pybind11::ndarray<Args...>& array)
{
    return getNdarraySize(array) * getDtypeByteSize(array.dtype());
}

template<typename... Args>
size_t isNdarrayContiguous(const pybind11::ndarray<Args...>& array)
{
    if (array.ndim() == 0)
        return false;
    size_t prod = 1;
    for (size_t i = array.ndim() - 1;;)
    {
        if (array.stride(i) != prod)
            return false;
        prod *= array.shape(i);
        if (i == 0)
            break;
        --i;
    }
    return true;
}

pybind11::dlpack::dtype dataTypeToDtype(DataType type);
std::optional<pybind11::dlpack::dtype> resourceFormatToDtype(ResourceFormat format);

pybind11::dict defineListToPython(const DefineList& defines);
DefineList defineListFromPython(const pybind11::dict& dict);

pybind11::dict typeConformanceListToPython(const TypeConformanceList& conformances);
TypeConformanceList typeConformanceListFromPython(const pybind11::dict& dict);

ProgramDesc programDescFromPython(const pybind11::kwargs& kwargs);

} // namespace Falcor
