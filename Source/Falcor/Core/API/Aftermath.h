/**
 * This file defines a wrapper of NSight Aftermath SDK and is used within
 * the Device class to add Aftermath support.
 *
 * Aftermath generates a GPU crash dump when the application crashes. This can
 * be very useful for debugging GPU crashes.
 */

#pragma once

#if FALCOR_HAS_AFTERMATH

#include "Core/Macros.h"

#include <string_view>

namespace Falcor
{

class Device;
class LowLevelContextData;

/// Aftermath feature flags.
/// See section on GFSDK_Aftermath_FeatureFlags in GFSDK_Aftermath.h for details.
/// Note: For using EnableMarkers, the Aftermath Monitor must be running on the
/// host machine.
enum AftermathFlags
{
    Minimum = 0x00000000,
    EnableMarkers = 0x00000001,
    EnableResourceTracking = 0x00000002,
    CallStackCapturing = 0x40000000,
    GenerateShaderDebugInfo = 0x00000008,
    EnableShaderErrorReporting = 0x00000010,

    Defaults = EnableMarkers | EnableResourceTracking | CallStackCapturing | GenerateShaderDebugInfo | EnableShaderErrorReporting,
};
FALCOR_ENUM_CLASS_OPERATORS(AftermathFlags);

/// Aftermath per-device context.
class AftermathContext
{
public:
    AftermathContext(Device* pDevice);
    ~AftermathContext();

    /// Initialize Aftermath on the device.
    bool initialize(AftermathFlags flags = AftermathFlags::Defaults);

    /// Add a marker to the command list.
    void addMarker(const LowLevelContextData* pLowLevelContextData, std::string_view name);

private:
    Device* mpDevice;
    bool mInitialized = false;
    void* mpLastCommandList = nullptr;
    int32_t mContextHandle = 0;
};

/// Enable GPU crash dump tracking.
void enableAftermath();

/// Disable GPU crash dump tracking.
void disableAftermath();

/// Wait for GPU crash dumps to be generated. Timeout is in seconds.
bool waitForAftermathDumps(int timeout = 5);

} // namespace Falcor

#endif // FALCOR_HAS_AFTERMATH
