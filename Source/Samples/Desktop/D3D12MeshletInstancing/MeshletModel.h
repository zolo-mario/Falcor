//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// Ported to Falcor from DirectX-Graphics-Samples D3D12MeshletInstancing.
//
//*********************************************************
#pragma once

#include "Falcor.h"
#include <vector>
#include <cstdint>

namespace Falcor
{

struct MeshletSubset
{
    uint32_t Offset;
    uint32_t Count;
};

struct MeshletData
{
    uint32_t VertCount;
    uint32_t VertOffset;
    uint32_t PrimCount;
    uint32_t PrimOffset;
};

struct MeshletMesh
{
    uint32_t IndexSize;
    uint32_t IndexCount;
    uint32_t VertexCount;

    std::vector<MeshletSubset> MeshletSubsets;
    std::vector<MeshletData> Meshlets;
    std::vector<uint8_t> UniqueVertexIndices;
    std::vector<uint8_t> PrimitiveIndices; // PackedTriangle array
    std::vector<float> VertexData;         // Interleaved Position(3) + Normal(3) per vertex

    ref<Buffer> pVertexBuffer;
    ref<Buffer> pMeshletBuffer;
    ref<Buffer> pUniqueVertexIndexBuffer;
    ref<Buffer> pPrimitiveIndexBuffer;

    uint32_t getLastMeshletPackCount(uint32_t subsetIndex, uint32_t maxGroupVerts, uint32_t maxGroupPrims) const;
};

struct BoundingSphereF
{
    float center[3];
    float radius;
};

class MeshletModel
{
public:
    bool loadFromFile(Device* pDevice, const std::filesystem::path& path);
    void uploadGpuResources(RenderContext* pRenderContext);

    uint32_t getMeshCount() const { return (uint32_t)mMeshes.size(); }
    const MeshletMesh& getMesh(uint32_t i) const { return mMeshes[i]; }
    MeshletMesh& getMesh(uint32_t i) { return mMeshes[i]; }

    const BoundingSphereF& getBoundingSphere() const { return mBoundingSphere; }

    auto begin() { return mMeshes.begin(); }
    auto end() { return mMeshes.end(); }

private:
    std::vector<MeshletMesh> mMeshes;
    BoundingSphereF mBoundingSphere;
    std::vector<uint8_t> mBuffer;
    ref<Device> mpDevice;
};

} // namespace Falcor
