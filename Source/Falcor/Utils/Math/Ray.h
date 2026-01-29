#pragma once
#include "Vector.h"

namespace Falcor
{
/**
 * Ray type.
 * This should match the layout of DXR RayDesc.
 */
struct Ray
{
    float3 origin;
    float tMin;
    float3 dir;
    float tMax;

    Ray() = default;
    explicit Ray(float3 origin, float3 dir, float tMin = 0.f, float tMax = std::numeric_limits<float>::max())
        : origin(origin), tMin(tMin), dir(dir), tMax(tMax)
    {}
};

// These are to ensure that the struct Ray match DXR RayDesc.
static_assert(offsetof(Ray, origin) == 0);
static_assert(offsetof(Ray, tMin) == sizeof(float3));
static_assert(offsetof(Ray, dir) == offsetof(Ray, tMin) + sizeof(float));
static_assert(offsetof(Ray, tMax) == offsetof(Ray, dir) + sizeof(float3));
static_assert(sizeof(Ray) == 32);
} // namespace Falcor
