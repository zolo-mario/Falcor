#pragma once
#include "Macros.h"

// Don't include GL/GLES headers
#define GLFW_INCLUDE_NONE

#if FALCOR_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#elif FALCOR_LINUX
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
// Undefine various X11 macros.
#undef None
#undef Bool
#undef Status
#undef Always
#endif
