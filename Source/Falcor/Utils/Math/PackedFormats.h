#pragma once
#include "Vector.h"
#include "FormatConversion.h"
#include <cmath>

/**
 * Host-side utility functions for format conversion.
 *
 * The functions defined here should match the corresponding GPU-side
 * functions, but numerical differences are possible.
 */

namespace Falcor
{
/**
 * Helper function to reflect the folds of the lower hemisphere
 * over the diagonals in the octahedral map.
 */
inline float2 oct_wrap(float2 v)
{
    return {(1.f - std::abs(v.y)) * (v.x >= 0.f ? 1.f : -1.f), (1.f - std::abs(v.x)) * (v.y >= 0.f ? 1.f : -1.f)};
}

/**
 * Converts normalized direction to the octahedral map (non-equal area, signed normalized).
 * @param[in] n Normalized direction.
 * @return Position in octahedral map in [-1,1] for each component.
 */
inline float2 ndir_to_oct_snorm(float3 n)
{
    // Project the sphere onto the octahedron (|x|+|y|+|z| = 1) and then onto the xy-plane.
    float2 p = float2(n.x, n.y) * (1.f / (std::abs(n.x) + std::abs(n.y) + std::abs(n.z)));
    p = (n.z < 0.f) ? oct_wrap(p) : p;
    return p;
}

/**
 * Converts point in the octahedral map to normalized direction (non-equal area, signed normalized).
 * @param[in] p Position in octahedral map in [-1,1] for each component.
 * @return Normalized direction.
 */
inline float3 oct_to_ndir_snorm(float2 p)
{
    float3 n = float3(p.x, p.y, 1.f - std::abs(p.x) - std::abs(p.y));
    float2 tmp = (n.z < 0.0) ? oct_wrap(float2(n.x, n.y)) : float2(n.x, n.y);
    n.x = tmp.x;
    n.y = tmp.y;
    return normalize(n);
}

/**
 * Encode a normal packed as 2x 16-bit snorms in the octahedral mapping.
 */
inline uint32_t encodeNormal2x16(float3 normal)
{
    float2 octNormal = ndir_to_oct_snorm(normal);
    return packSnorm2x16(octNormal);
}

/**
 * Decode a normal packed as 2x 16-bit snorms in the octahedral mapping.
 */
inline float3 decodeNormal2x16(uint32_t packedNormal)
{
    float2 octNormal = unpackSnorm2x16(packedNormal);
    return oct_to_ndir_snorm(octNormal);
}
} // namespace Falcor
