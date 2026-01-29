#include "Animatable.h"
#include "Utils/Scripting/ScriptBindings.h"

namespace Falcor
{
    FALCOR_SCRIPT_BINDING(Animatable)
    {
        pybind11::class_<Animatable, ref<Animatable>> animatable(m, "Animatable");
        animatable.def_property_readonly("hasAnimation", &Animatable::hasAnimation);
        animatable.def_property("animated", &Animatable::isAnimated, &Animatable::setIsAnimated);
    }
}
