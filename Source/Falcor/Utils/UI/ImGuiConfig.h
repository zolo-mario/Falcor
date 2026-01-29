#pragma once

// ImGui configuration file. See imconfig.h for more details.
// We use this to provide implicit conversion between ImGui's vector types
// and Falcor's vector types.

#include "Utils/Math/VectorTypes.h"

// clang-format off
#define IM_VEC2_CLASS_EXTRA                                                             \
    constexpr ImVec2(const ::Falcor::float2& f) : x(f.x), y(f.y) {}                     \
    operator ::Falcor::float2() const { return ::Falcor::float2(x, y); }

#define IM_VEC3_CLASS_EXTRA                                                             \
    constexpr ImVec3(const ::Falcor::float3& f) : x(f.x), y(f.y), z(f.z) {}             \
    operator ::Falcor::float3() const { return ::Falcor::float3(x, y, z); }

#define IM_VEC4_CLASS_EXTRA                                                             \
    constexpr ImVec4(const ::Falcor::float4& f) : x(f.x), y(f.y), z(f.z), w(f.w) {}     \
    operator ::Falcor::float4() const { return ::Falcor::float4(x, y, z, w); }
// clang-format on
