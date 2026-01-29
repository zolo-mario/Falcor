/**
 * Enums for WhittedTracer
 */

#pragma once
#include "Utils/HostDeviceShared.slangh"

BEGIN_NAMESPACE_FALCOR

/**
 * This enum is shared between CPU/GPU.
 * It enumerates the different the different filter modes for ray cones.
 */
enum class RayFootprintFilterMode : uint32_t
{
    Isotropic = 0,
    Anisotropic = 1,
    AnisotropicWhenRefraction = 2
};

FALCOR_ENUM_INFO(
    RayFootprintFilterMode,
    {
        { RayFootprintFilterMode::Isotropic, "Isotropic" },
        { RayFootprintFilterMode::Anisotropic, "Anisotropic" },
        { RayFootprintFilterMode::AnisotropicWhenRefraction, "AnisotropicWhenRefraction" },
    }
);
FALCOR_ENUM_REGISTER(RayFootprintFilterMode);

END_NAMESPACE_FALCOR
