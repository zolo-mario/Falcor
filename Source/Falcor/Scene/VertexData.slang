#pragma once
#include "Utils/HostDeviceShared.slangh"

BEGIN_NAMESPACE_FALCOR

/** Struct representing interpolated vertex attributes in world space.
    Note the tangent is not guaranteed to be orthogonal to the normal.
    The bitangent should be computed: cross(normal, tangent.xyz) * tangent.w.
    The tangent space is orthogonalized in prepareShadingData().
*/
struct VertexData
{
    float3 posW;            ///< Position in world space.
    float3 normalW;         ///< Shading normal in world space (normalized).
    float4 tangentW;        ///< Shading tangent in world space (normalized). The last component is guaranteed to be +-1.0 or zero if tangents are missing.
    float2 texC;            ///< Texture coordinate.
    float3 faceNormalW;     ///< Face normal in world space (normalized).
    float  curveRadius;     ///< Curve cross-sectional radius. Valid only for geometry generated from curves.
    float  coneTexLODValue; ///< Texture LOD data for cone tracing. This is zero, unless getVertexDataRayCones() is used.
};

END_NAMESPACE_FALCOR
