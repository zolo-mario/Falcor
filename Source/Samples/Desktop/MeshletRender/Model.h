#pragma once

#include "Falcor.h"
#include <vector>
#include <cstdint>

using namespace Falcor;

struct Subset
{
    uint32_t Offset;
    uint32_t Count;
};

struct Meshlet
{
    uint32_t VertCount;
    uint32_t VertOffset;
    uint32_t PrimCount;
    uint32_t PrimOffset;
};

struct PackedTriangle
{
    uint32_t i0 : 10;
    uint32_t i1 : 10;
    uint32_t i2 : 10;
};

struct MeshData
{
    uint32_t IndexSize;
    uint32_t VertexCount;

    std::vector<Subset> MeshletSubsets;
    std::vector<Meshlet> Meshlets;
    std::vector<uint8_t> UniqueVertexIndices;
    std::vector<PackedTriangle> PrimitiveIndices;
    std::vector<uint8_t> Vertices;
    uint32_t VertexStride;

    // Falcor GPU resources
    ref<Buffer> pVertexBuffer;
    ref<Buffer> pMeshletBuffer;
    ref<Buffer> pUniqueVertexIndexBuffer;
    ref<Buffer> pPrimitiveIndexBuffer;
};

class MeshletModel
{
public:
    bool loadFromFile(const std::filesystem::path& path);
    void uploadGpuResources(Device* pDevice, RenderContext* pRenderContext);

    size_t getMeshCount() const { return m_meshes.size(); }
    const MeshData& getMesh(size_t i) const { return m_meshes[i]; }
    MeshData& getMesh(size_t i) { return m_meshes[i]; }

    auto begin() { return m_meshes.begin(); }
    auto end() { return m_meshes.end(); }

private:
    std::vector<MeshData> m_meshes;
    std::vector<uint8_t> m_buffer;
};
