// Copyright (c) Microsoft. Licensed under the MIT License.
#pragma once

#include "Falcor.h"

#define MAX_VERTS 64
#define MAX_PRIMS 126
#define MAX_LOD_LEVELS 8
#define THREADS_PER_WAVE 32
#define AS_GROUP_SIZE THREADS_PER_WAVE
#define MS_GROUP_SIZE (((MAX_VERTS > MAX_PRIMS ? MAX_VERTS : MAX_PRIMS) + THREADS_PER_WAVE - 1) & ~(THREADS_PER_WAVE - 1))

namespace Falcor
{

// C++ structs matching shader layout for constant/instance buffers
struct DynamicLODConstants
{
    float4x4 View;
    float4x4 ViewProj;
    float4 Planes[6];
    float3 ViewPosition;
    float RecipTanHalfFovy;
    uint32_t RenderMode;
    uint32_t LODCount;
};

struct DynamicLODDrawParams
{
    uint32_t InstanceOffset;
    uint32_t InstanceCount;
};

struct DynamicLODInstance
{
    float4x4 World;
    float4x4 WorldInvTranspose;
    float4 BoundingSphere;
};

struct DynamicLODMeshInfo
{
    uint32_t IndexBytes;
    uint32_t MeshletCount;
    uint32_t LastMeshletVertCount;
    uint32_t LastMeshletPrimCount;
};

} // namespace Falcor
