#include "GFXAPI.h"
#include "Aftermath.h"
#include "Core/Error.h"
#include "Utils/Logger.h"

#if FALCOR_HAS_D3D12
#include "dxgi.h"
#endif

namespace Falcor
{
void gfxReportError(const char* api, const char* call, gfx::Result result)
{
    const char* resultStr = nullptr;
#if FALCOR_HAS_D3D12
    switch (result)
    {
    case DXGI_ERROR_DEVICE_REMOVED:
        resultStr = "DXGI_ERROR_DEVICE_REMOVED";
        break;
    case DXGI_ERROR_DEVICE_HUNG:
        resultStr = "DXGI_ERROR_DEVICE_HUNG";
        break;
    case DXGI_ERROR_DEVICE_RESET:
        resultStr = "DXGI_ERROR_DEVICE_RESET";
        break;
    }
#endif

#if FALCOR_HAS_AFTERMATH
    if (!waitForAftermathDumps())
        logError("Aftermath GPU crash dump generation failed.");
#endif

    std::string fullMsg = resultStr ? fmt::format("{} call '{}' failed with error {} ({}).", api, call, result, resultStr)
                                    : fmt::format("{} call '{}' failed with error {}", api, call, result);

    reportFatalErrorAndTerminate(fullMsg);
}
} // namespace Falcor
