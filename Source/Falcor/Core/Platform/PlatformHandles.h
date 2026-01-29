#pragma once
#include "Core/Macros.h"

namespace Falcor
{
#if FALCOR_WINDOWS
using SharedLibraryHandle = void*; // HANDLE
using WindowHandle = void*;        // HWND
#elif FALCOR_LINUX
using SharedLibraryHandle = void*;
struct WindowHandle
{
    void* pDisplay;
    unsigned long window;
};
#else
#error "Platform not specified!"
#endif
} // namespace Falcor
