#include "Utils/HostDeviceShared.slangh" // For bit packing macros

/** Hints passed in when creating a material instance.
*/
enum MaterialInstanceHints : uint
{
    None = 0,
    DisableNormalMapping = 0x1,     ///< Turn off normal mapping if supported.
    AdjustShadingNormal = 0x2,      ///< Adjust shading normal to reduce risk of back-facing hits.
};

bool isNormalMappingEnabled(uint hints)
{
    return !bool(hints & (uint)MaterialInstanceHints::DisableNormalMapping);
}

bool isAdjustShadingNormalEnabled(uint hints)
{
    return bool(hints & (uint)MaterialInstanceHints::AdjustShadingNormal);
}
