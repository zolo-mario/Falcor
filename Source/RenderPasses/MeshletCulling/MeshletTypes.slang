#pragma once
#include "Utils/HostDeviceShared.slangh"

BEGIN_NAMESPACE_FALCOR

struct GpuMeshlet
{
    uint   vertexOffset;
    uint   triangleOffset;
    uint   vertexCount;
    uint   triangleCount;

    float3 boundCenter;
    float  boundRadius;

    uint   indexStart;
    uint   indexCount;
    uint   meshID;
    uint   _pad0;
};

struct MeshletSceneData
{
    uint meshletCount;
    uint totalTriangles;
    uint totalVertices;
    uint totalIndices;
};

struct FrustumData
{
    float4 planes[6];
    float3 cameraPos;
    float  _pad0;
};

struct DrawIndexedIndirectArgs
{
    uint IndexCountPerInstance;
    uint InstanceCount;
    uint StartIndexLocation;
    int  BaseVertexLocation;
    uint StartInstanceLocation;
};

// ============ Visibility Buffer Packing Format ============
// 32-bit packing: [MeshletID: 20 bits][PrimitiveID: 12 bits]
// Supports up to ~1M Meshlets and 4096 triangles per Meshlet

static const uint kMeshletIDBits = 20;
static const uint kPrimitiveIDBits = 12;
static const uint kPrimitiveIDMask = (1u << kPrimitiveIDBits) - 1;
static const uint kMeshletIDMask = (1u << kMeshletIDBits) - 1;
static const uint kInvalidVBufferValue = 0xFFFFFFFF;

#ifndef HOST_CODE

// Pack MeshletID and PrimitiveID
uint packVisibilityBuffer(uint meshletID, uint primitiveID)
{
    return (meshletID << kPrimitiveIDBits) | (primitiveID & kPrimitiveIDMask);
}

// Unpack MeshletID
uint unpackMeshletID(uint packed)
{
    return packed >> kPrimitiveIDBits;
}

// Unpack PrimitiveID
uint unpackPrimitiveID(uint packed)
{
    return packed & kPrimitiveIDMask;
}

// Check if valid
bool isValidVBufferValue(uint packed)
{
    return packed != kInvalidVBufferValue;
}

// Generate Meshlet color (golden ratio HSV)
float3 meshletIdToColor(uint id)
{
    float hue = frac(float(id) * 0.618033988749895f);
    float3 rgb = saturate(abs(fmod(hue * 6.0f + float3(0, 4, 2), 6.0f) - 3.0f) - 1.0f);
    return lerp(float3(1, 1, 1), rgb, 0.85f) * 0.95f;
}

// Generate triangle color
float3 triangleIdToColor(uint meshletID, uint primitiveID)
{
    // Combine meshletID and primitiveID to generate richer colors
    uint combined = meshletID * 127 + primitiveID;
    float hue = frac(float(combined) * 0.618033988749895f);
    float sat = 0.6f + 0.4f * frac(float(primitiveID) * 0.31415f);

    float3 rgb = saturate(abs(fmod(hue * 6.0f + float3(0, 4, 2), 6.0f) - 3.0f) - 1.0f);
    return lerp(float3(1, 1, 1), rgb, sat) * 0.9f;
}

#endif

END_NAMESPACE_FALCOR
