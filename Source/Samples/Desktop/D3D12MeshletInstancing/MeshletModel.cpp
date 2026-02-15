//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// Ported to Falcor from DirectX-Graphics-Samples D3D12MeshletInstancing.
//
//*********************************************************
#include "MeshletModel.h"
#include "Core/API/Device.h"
#include "Core/API/RenderContext.h"
#include <fstream>
#include <algorithm>
#include <cstring>

namespace Falcor
{

namespace
{
    const uint32_t c_prolog = 'MSHL';

    enum FileVersion
    {
        FILE_VERSION_INITIAL = 0,
        CURRENT_FILE_VERSION = FILE_VERSION_INITIAL
    };

    struct FileHeader
    {
        uint32_t Prolog;
        uint32_t Version;
        uint32_t MeshCount;
        uint32_t AccessorCount;
        uint32_t BufferViewCount;
        uint32_t BufferSize;
    };

    struct MeshHeader
    {
        uint32_t Indices;
        uint32_t IndexSubsets;
        int32_t Attributes[5]; // Position, Normal, TexCoord, Tangent, Bitangent
        uint32_t Meshlets;
        uint32_t MeshletSubsets;
        uint32_t UniqueVertexIndices;
        uint32_t PrimitiveIndices;
        uint32_t CullData;
    };

    struct BufferView
    {
        uint32_t Offset;
        uint32_t Size;
    };

    struct Accessor
    {
        uint32_t BufferView;
        uint32_t Offset;
        uint32_t Size;
        uint32_t Stride;
        uint32_t Count;
    };

    uint32_t getFormatSize(uint32_t format)
    {
        switch (format)
        {
        case 16: return 16; // R32G32B32A32
        case 12: return 12; // R32G32B32
        case 8: return 8;   // R32G32
        case 4: return 4;   // R32
        default: return 12;
        }
    }

    template <typename T, typename U>
    constexpr T divRoundUp(T num, U denom)
    {
        return (num + denom - 1) / denom;
    }
}

uint32_t MeshletMesh::getLastMeshletPackCount(uint32_t subsetIndex, uint32_t maxGroupVerts, uint32_t maxGroupPrims) const
{
    if (Meshlets.empty())
        return 0;
    const auto& subset = MeshletSubsets[subsetIndex];
    const auto& meshlet = Meshlets[subset.Offset + subset.Count - 1];
    return std::min(maxGroupVerts / meshlet.VertCount, maxGroupPrims / meshlet.PrimCount);
}

bool MeshletModel::loadFromFile(Device* pDevice, const std::filesystem::path& path)
{
    mpDevice = pDevice;

    std::ifstream stream(path, std::ios::binary);
    if (!stream.is_open())
        return false;

    std::vector<MeshHeader> meshes;
    std::vector<BufferView> bufferViews;
    std::vector<Accessor> accessors;

    FileHeader header;
    stream.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (header.Prolog != c_prolog)
        return false;
    if (header.Version != CURRENT_FILE_VERSION)
        return false;

    meshes.resize(header.MeshCount);
    stream.read(reinterpret_cast<char*>(meshes.data()), meshes.size() * sizeof(meshes[0]));

    accessors.resize(header.AccessorCount);
    stream.read(reinterpret_cast<char*>(accessors.data()), accessors.size() * sizeof(accessors[0]));

    bufferViews.resize(header.BufferViewCount);
    stream.read(reinterpret_cast<char*>(bufferViews.data()), bufferViews.size() * sizeof(bufferViews[0]));

    mBuffer.resize(header.BufferSize);
    stream.read(reinterpret_cast<char*>(mBuffer.data()), header.BufferSize);

    mMeshes.resize(meshes.size());
    for (uint32_t i = 0; i < (uint32_t)meshes.size(); ++i)
    {
        auto& meshView = meshes[i];
        auto& mesh = mMeshes[i];

        // Index data
        {
            Accessor& accessor = accessors[meshView.Indices];
            BufferView& bufferView = bufferViews[accessor.BufferView];
            mesh.IndexSize = accessor.Size;
            mesh.IndexCount = accessor.Count;
        }

        // Meshlet data
        {
            Accessor& accessor = accessors[meshView.Meshlets];
            BufferView& bufferView = bufferViews[accessor.BufferView];
            const MeshletData* src = reinterpret_cast<const MeshletData*>(mBuffer.data() + bufferView.Offset);
            mesh.Meshlets.assign(src, src + accessor.Count);
        }

        // Meshlet Subsets
        {
            Accessor& accessor = accessors[meshView.MeshletSubsets];
            BufferView& bufferView = bufferViews[accessor.BufferView];
            const MeshletSubset* src = reinterpret_cast<const MeshletSubset*>(mBuffer.data() + bufferView.Offset);
            mesh.MeshletSubsets.assign(src, src + accessor.Count);
        }

        // Unique Vertex Indices
        {
            Accessor& accessor = accessors[meshView.UniqueVertexIndices];
            BufferView& bufferView = bufferViews[accessor.BufferView];
            mesh.UniqueVertexIndices.assign(mBuffer.data() + bufferView.Offset, mBuffer.data() + bufferView.Offset + bufferView.Size);
        }

        // Primitive Indices (PackedTriangle - 4 bytes each)
        {
            Accessor& accessor = accessors[meshView.PrimitiveIndices];
            BufferView& bufferView = bufferViews[accessor.BufferView];
            mesh.PrimitiveIndices.assign(mBuffer.data() + bufferView.Offset, mBuffer.data() + bufferView.Offset + bufferView.Size);
        }

        // Vertex data - MeshletInstancing expects POSITION + NORMAL (LayoutDesc.NumElements == 2)
        // Find Position and Normal buffer views
        uint32_t positionBufView = (meshView.Attributes[0] >= 0) ? accessors[meshView.Attributes[0]].BufferView : (uint32_t)-1;
        uint32_t normalBufView = (meshView.Attributes[1] >= 0) ? accessors[meshView.Attributes[1]].BufferView : (uint32_t)-1;

        std::vector<uint32_t> vbMap;
        uint32_t vertexStride = 0;
        for (int attr = 0; attr < 2; ++attr) // Position, Normal only
        {
            if (meshView.Attributes[attr] < 0)
                continue;
            Accessor& accessor = accessors[meshView.Attributes[attr]];
            auto it = std::find(vbMap.begin(), vbMap.end(), accessor.BufferView);
            if (it == vbMap.end())
            {
                vbMap.push_back(accessor.BufferView);
                vertexStride += (attr == 0) ? 12 : 12; // float3
            }
        }

        BufferView& posView = bufferViews[accessors[meshView.Attributes[0]].BufferView];
        BufferView& normView = (meshView.Attributes[1] >= 0) ? bufferViews[accessors[meshView.Attributes[1]].BufferView] : posView;

        uint32_t vertexCount = accessors[meshView.Attributes[0]].Count;
        mesh.VertexCount = vertexCount;

        // Build interleaved Position+Normal vertex buffer (24 bytes per vertex)
        mesh.VertexData.resize(vertexCount * 6);
        const float* posData = reinterpret_cast<const float*>(mBuffer.data() + posView.Offset);
        const float* normData = nullptr;
        if (meshView.Attributes[1] >= 0)
        {
            BufferView& nv = bufferViews[accessors[meshView.Attributes[1]].BufferView];
            normData = reinterpret_cast<const float*>(mBuffer.data() + nv.Offset);
        }
        for (uint32_t v = 0; v < vertexCount; ++v)
        {
            mesh.VertexData[v * 6 + 0] = posData[v * 3 + 0];
            mesh.VertexData[v * 6 + 1] = posData[v * 3 + 1];
            mesh.VertexData[v * 6 + 2] = posData[v * 3 + 2];
            if (normData)
            {
                mesh.VertexData[v * 6 + 3] = normData[v * 3 + 0];
                mesh.VertexData[v * 6 + 4] = normData[v * 3 + 1];
                mesh.VertexData[v * 6 + 5] = normData[v * 3 + 2];
            }
            else
            {
                mesh.VertexData[v * 6 + 3] = 0.f;
                mesh.VertexData[v * 6 + 4] = 1.f;
                mesh.VertexData[v * 6 + 5] = 0.f;
            }
        }

        // Bounding sphere from positions (per-mesh, then merge)
        float minX = 1e9f, minY = 1e9f, minZ = 1e9f;
        float maxX = -1e9f, maxY = -1e9f, maxZ = -1e9f;
        for (uint32_t v = 0; v < vertexCount; ++v)
        {
            float x = mesh.VertexData[v * 6 + 0], y = mesh.VertexData[v * 6 + 1], z = mesh.VertexData[v * 6 + 2];
            minX = std::min(minX, x);
            minY = std::min(minY, y);
            minZ = std::min(minZ, z);
            maxX = std::max(maxX, x);
            maxY = std::max(maxY, y);
            maxZ = std::max(maxZ, z);
        }
        float cx = (minX + maxX) * 0.5f, cy = (minY + maxY) * 0.5f, cz = (minZ + maxZ) * 0.5f;
        float dx = maxX - minX, dy = maxY - minY, dz = maxZ - minZ;
        float r = std::sqrt(dx * dx + dy * dy + dz * dz) * 0.5f;

        if (i == 0)
        {
            mBoundingSphere.center[0] = cx;
            mBoundingSphere.center[1] = cy;
            mBoundingSphere.center[2] = cz;
            mBoundingSphere.radius = r;
        }
        else
        {
            float d = std::sqrt(
                (cx - mBoundingSphere.center[0]) * (cx - mBoundingSphere.center[0]) +
                (cy - mBoundingSphere.center[1]) * (cy - mBoundingSphere.center[1]) +
                (cz - mBoundingSphere.center[2]) * (cz - mBoundingSphere.center[2]));
            float newR = (d + r + mBoundingSphere.radius) * 0.5f;
            float t = (newR - mBoundingSphere.radius) / (d + 1e-9f);
            mBoundingSphere.center[0] += t * (cx - mBoundingSphere.center[0]);
            mBoundingSphere.center[1] += t * (cy - mBoundingSphere.center[1]);
            mBoundingSphere.center[2] += t * (cz - mBoundingSphere.center[2]);
            mBoundingSphere.radius = newR;
        }
    }

    return true;
}

void MeshletModel::uploadGpuResources(RenderContext* pRenderContext)
{
    for (uint32_t i = 0; i < (uint32_t)mMeshes.size(); ++i)
    {
        auto& m = mMeshes[i];

        m.pVertexBuffer = mpDevice->createStructuredBuffer(
            sizeof(float) * 6,
            (uint32_t)m.VertexData.size() / 6,
            ResourceBindFlags::ShaderResource,
            MemoryType::DeviceLocal,
            m.VertexData.data());

        m.pMeshletBuffer = mpDevice->createStructuredBuffer(
            sizeof(MeshletData),
            (uint32_t)m.Meshlets.size(),
            ResourceBindFlags::ShaderResource,
            MemoryType::DeviceLocal,
            m.Meshlets.data());

        uint32_t uviSize = divRoundUp((uint32_t)m.UniqueVertexIndices.size(), 4) * 4;
        m.pUniqueVertexIndexBuffer = mpDevice->createBuffer(
            uviSize,
            ResourceBindFlags::ShaderResource,
            MemoryType::DeviceLocal,
            m.UniqueVertexIndices.data());

        uint32_t primCount = (uint32_t)m.PrimitiveIndices.size() / 4; // 4 bytes per packed triangle
        m.pPrimitiveIndexBuffer = mpDevice->createStructuredBuffer(
            sizeof(uint32_t),
            primCount,
            ResourceBindFlags::ShaderResource,
            MemoryType::DeviceLocal,
            m.PrimitiveIndices.data());
    }
}

} // namespace Falcor
