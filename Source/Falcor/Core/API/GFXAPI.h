#pragma once
#include "Handles.h"
#include "Core/Macros.h"
#include "Utils/Logger.h"

#include <slang.h>
#include <slang-gfx.h>
#include <slang-com-ptr.h>

#define FALCOR_GFX_CALL(call)                      \
    {                                              \
        gfx::Result result_ = call;                \
        if (SLANG_FAILED(result_))                 \
        {                                          \
            gfxReportError("GFX", #call, result_); \
        }                                          \
    }

#if FALCOR_HAS_D3D12
#define FALCOR_D3D_CALL(call)                      \
    {                                              \
        HRESULT result_ = call;                    \
        if (FAILED(result_))                       \
        {                                          \
            gfxReportError("D3D", #call, result_); \
        }                                          \
    }
#define FALCOR_GET_COM_INTERFACE(_base, _type, _var) \
    FALCOR_MAKE_SMART_COM_PTR(_type);                \
    FALCOR_CONCAT_STRINGS(_type, Ptr) _var;          \
    FALCOR_D3D_CALL(_base->QueryInterface(IID_PPV_ARGS(&_var)));
#define FALCOR_MAKE_SMART_COM_PTR(_a) _COM_SMARTPTR_TYPEDEF(_a, __uuidof(_a))
#endif

namespace Falcor
{
/**
 * Report a GFX or D3D error.
 * This will throw a RuntimeError exception.
 */
FALCOR_API void gfxReportError(const char* api, const char* call, gfx::Result result);

} // namespace Falcor
