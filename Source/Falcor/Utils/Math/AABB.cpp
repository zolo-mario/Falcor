#include "AABB.h"
#include "Utils/Scripting/ScriptBindings.h"
#include <pybind11/operators.h>

namespace Falcor
{
FALCOR_SCRIPT_BINDING(AABB)
{
    using namespace pybind11::literals;

    pybind11::class_<AABB> aabb(m, "AABB");

    aabb.def(pybind11::init<>());
    aabb.def(pybind11::init<const float3&>(), "p"_a);
    aabb.def(pybind11::init<const float3&, const float3&>(), "min_point"_a, "max_point"_a);

    aabb.def(
        "__repr__",
        [](const AABB& aabb)
        {
            return "AABB(min_point=" + std::string(pybind11::repr(pybind11::cast(aabb.minPoint))) +
                   ", max_point=" + std::string(pybind11::repr(pybind11::cast(aabb.maxPoint))) + ")";
        }
    );
    aabb.def(
        "__str__",
        [](const AABB& aabb)
        {
            return "[" + std::string(pybind11::str(pybind11::cast(aabb.minPoint))) + ", " +
                   std::string(pybind11::str(pybind11::cast(aabb.maxPoint))) + "]";
        }
    );

    aabb.def_readwrite("min_point", &AABB::minPoint);
    aabb.def_readwrite("max_point", &AABB::maxPoint);

    aabb.def_property_readonly("valid", &AABB::valid);
    aabb.def_property_readonly("center", &AABB::center);
    aabb.def_property_readonly("extent", &AABB::extent);
    aabb.def_property_readonly("area", &AABB::area);
    aabb.def_property_readonly("volume", &AABB::volume);
    aabb.def_property_readonly("radius", &AABB::radius);

    aabb.def("invalidate", &AABB::invalidate);
    aabb.def("include", pybind11::overload_cast<const float3&>(&AABB::include), "p"_a);
    aabb.def("include", pybind11::overload_cast<const AABB&>(&AABB::include), "b"_a);
    aabb.def("intersection", &AABB::intersection);

    aabb.def(pybind11::self == pybind11::self);
    aabb.def(pybind11::self != pybind11::self);
    aabb.def(pybind11::self | pybind11::self);
    aabb.def(pybind11::self |= pybind11::self);
    aabb.def(pybind11::self & pybind11::self);
    aabb.def(pybind11::self &= pybind11::self);

    // PYTHONDEPRECATED BEGIN
    aabb.def_readwrite("minPoint", &AABB::minPoint);
    aabb.def_readwrite("maxPoint", &AABB::maxPoint);
    // PYTHONDEPRECATED END
}
} // namespace Falcor
