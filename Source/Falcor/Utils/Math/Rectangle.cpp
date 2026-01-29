#include "Rectangle.h"
#include "Utils/Scripting/ScriptBindings.h"
#include <pybind11/operators.h>

namespace Falcor
{
FALCOR_SCRIPT_BINDING(Rectangle)
{
    using namespace pybind11::literals;

    pybind11::class_<Rectangle> rectangle(m, "Rectangle");

    rectangle.def(pybind11::init<>());
    rectangle.def(pybind11::init<const float2&>(), "p"_a);
    rectangle.def(pybind11::init<const float2&, const float2&>(), "min_point"_a, "max_point"_a);

    rectangle.def(
        "__repr__",
        [](const Rectangle& rectangle)
        {
            return "Rectangle(min_point=" + std::string(pybind11::repr(pybind11::cast(rectangle.minPoint))) +
                   ", max_point=" + std::string(pybind11::repr(pybind11::cast(rectangle.maxPoint))) + ")";
        }
    );
    rectangle.def(
        "__str__",
        [](const Rectangle& rectangle)
        {
            return "[" + std::string(pybind11::str(pybind11::cast(rectangle.minPoint))) + ", " +
                   std::string(pybind11::str(pybind11::cast(rectangle.maxPoint))) + "]";
        }
    );

    rectangle.def_readwrite("min_point", &Rectangle::minPoint);
    rectangle.def_readwrite("max_point", &Rectangle::maxPoint);

    rectangle.def_property_readonly("valid", &Rectangle::valid);
    rectangle.def_property_readonly("center", &Rectangle::center);
    rectangle.def_property_readonly("extent", &Rectangle::extent);
    rectangle.def_property_readonly("area", &Rectangle::area);
    rectangle.def_property_readonly("radius", &Rectangle::radius);

    rectangle.def("invalidate", &Rectangle::invalidate);
    rectangle.def("include", pybind11::overload_cast<const float2&>(&Rectangle::include), "p"_a);
    rectangle.def("include", pybind11::overload_cast<const Rectangle&>(&Rectangle::include), "b"_a);
    rectangle.def("intersection", &Rectangle::intersection);

    rectangle.def(pybind11::self == pybind11::self);
    rectangle.def(pybind11::self != pybind11::self);
    rectangle.def(pybind11::self | pybind11::self);
    rectangle.def(pybind11::self |= pybind11::self);
    rectangle.def(pybind11::self & pybind11::self);
    rectangle.def(pybind11::self &= pybind11::self);

    // PYTHONDEPRECATED BEGIN
    rectangle.def_readwrite("minPoint", &Rectangle::minPoint);
    rectangle.def_readwrite("maxPoint", &Rectangle::maxPoint);
    // PYTHONDEPRECATED END
}
} // namespace Falcor
