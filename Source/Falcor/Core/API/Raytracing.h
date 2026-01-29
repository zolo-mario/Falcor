#pragma once
#include "Core/Macros.h"
#include "Utils/Math/Vector.h"
#include <cstdint>

namespace Falcor
{
/**
 * Raytracing pipeline flags.
 */
enum class RtPipelineFlags : uint32_t
{
    None = 0,
    SkipTriangles = 0x1,
    SkipProceduralPrimitives = 0x2,
};
FALCOR_ENUM_CLASS_OPERATORS(RtPipelineFlags);

/**
 * Raytracing axis-aligned bounding box.
 */
struct RtAABB
{
    float3 min;
    float3 max;
};

/**
 * Flags passed to TraceRay(). These must match the device side.
 */
enum class RayFlags : uint32_t
{
    None,
    ForceOpaque = 0x1,
    ForceNonOpaque = 0x2,
    AcceptFirstHitAndEndSearch = 0x4,
    SkipClosestHitShader = 0x8,
    CullBackFacingTriangles = 0x10,
    CullFrontFacingTriangles = 0x20,
    CullOpaque = 0x40,
    CullNonOpaque = 0x80,
    SkipTriangles = 0x100,
    SkipProceduralPrimitives = 0x200,
};
FALCOR_ENUM_CLASS_OPERATORS(RayFlags);

// Maximum raytracing attribute size.
inline constexpr uint32_t getRaytracingMaxAttributeSize()
{
    return 32;
}
} // namespace Falcor
