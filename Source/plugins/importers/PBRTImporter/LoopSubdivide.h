// This code is based on pbrt:
// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

#pragma once
#include "Utils/Math/Vector.h"
#include <fstd/span.h> // TODO C++20: Replace with <span>
#include <vector>

namespace Falcor::pbrt
{

struct LoopSubdivideResult
{
    std::vector<float3> positions;
    std::vector<float3> normals;
    std::vector<uint32_t> indices;
};

LoopSubdivideResult loopSubdivide(uint32_t levels, fstd::span<const float3> positions, fstd::span<const uint32_t> vertices);

} // namespace Falcor::pbrt
