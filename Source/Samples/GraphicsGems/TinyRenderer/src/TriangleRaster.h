#pragma once

#include "Utils/Math/VectorTypes.h"

namespace tiny
{

/// 2D barycentric coordinates for point \a p with respect to triangle \a a,\a b,\a c (screen space).
/// Returns (u,v,w) with p = u*a + v*b + w*c; if degenerate, u may be negative (caller should discard).
Falcor::float3 barycentric2D(Falcor::float2 a, Falcor::float2 b, Falcor::float2 c, Falcor::float2 p);

} // namespace tiny
