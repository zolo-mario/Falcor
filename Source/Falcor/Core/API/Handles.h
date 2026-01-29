#pragma once
#include "Core/Macros.h"
#include "Core/Platform/PlatformHandles.h"
#include <slang.h>
#include <slang-gfx.h>
#include <slang-com-ptr.h>
#include <memory>

namespace Falcor
{
using GpuAddress = uint64_t;
#if FALCOR_WINDOWS
using SharedResourceApiHandle = void*; // HANDLE
using SharedFenceApiHandle = void*;    // HANDLE
#elif FALCOR_LINUX
using SharedResourceApiHandle = void*;
using SharedFenceApiHandle = void*;
#endif
} // namespace Falcor
