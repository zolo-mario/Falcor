#pragma once

#include "Object.h"
#include <pybind11/pybind11.h>

PYBIND11_DECLARE_HOLDER_TYPE(T, Falcor::ref<T>, true);
