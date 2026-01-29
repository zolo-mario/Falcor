#pragma once
#include "Utils/HostDeviceShared.slangh"

BEGIN_NAMESPACE_FALCOR

enum class PixelStatsRayType
{
    Visibility = 0,
    ClosestHit = 1,

    Count
};

END_NAMESPACE_FALCOR
