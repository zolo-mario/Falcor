#include "Model.h"
#include <fstream>
#include <algorithm>
#include <cstring>

namespace
{
    constexpr uint32_t kAttributeCount = 5;
    constexpr uint32_t kProlog = 'MSHL';
    constexpr uint32_t kCurrentFileVersion = 0;

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
        uint32_t Attributes[kAttributeCount];
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
        case 16: return 16; // R32G32B32A32_FLOAT
        case 12: return 12; // R32G32B32_FLOAT
        case 8: return 8;   // R32G32_FLOAT
        case 4: return 4;   // R32_FLOAT
        default: return 12;
        }
    }

    template<typename T, typename U>
    constexpr T divRoundUp(T num, U denom)
    {
        return (num + denom - 1) / denom;
    }
}

bool MeshletModel::loadFromFile(const std::filesystem::path& path)
{
    std::ifstream stream(path, std::ios::binary);
    if (!stream.is_open())
        return false;

    FileHeader header;
    stream.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (header.Prolog != kProlog || header.Version != kCurrentFileVersion)
        return false;

    std::vector<MeshHeader> meshes(header.MeshCount);
    stream.read(reinterpret_cast<char*>(meshes.data()), meshes.size() * sizeof(MeshHeader));

    std::vector<Accessor> accessors(header.AccessorCount);
    stream.read(reinterpret_cast<char*>(accessors.data()), accessors.size() * sizeof(Accessor));

    std::vector<BufferView> bufferViews(header.BufferViewCount);
    stream.read(reinterpret_cast<char*>(bufferViews.data()), bufferViews.size() * sizeof(BufferView));

    m_buffer.resize(header.BufferSize);
    stream.read(reinterpret_cast<char*>(m_buffer.data()), header.BufferSize);

    m_meshes.resize(meshes.size());
    for (uint32_t i = 0; i < meshes.size(); ++i)
    {
        auto& meshView = meshes[i];
        auto& mesh = m_meshes[i];

        // Index size from indices accessor
        {
            Accessor& accessor = accessors[meshView.Indices];
            BufferView& bufferView = bufferViews[accessor.BufferView];
            mesh.IndexSize = accessor.Size;
        }

        // Vertex data - find Position and Normal attributes
        uint32_t positionOffset = 0;
        uint32_t normalOffset = 12;
        uint32_t vbIndexPos = 0;
        std::vector<uint32_t> vbMap;

        for (uint32_t j = 0; j < kAttributeCount; ++j)
        {
            if (meshView.Attributes[j] == 0xFFFFFFFF)
                continue;
            Accessor& accessor = accessors[meshView.Attributes[j]];
            auto it = std::find(vbMap.begin(), vbMap.end(), accessor.BufferView);
            if (it == vbMap.end())
            {
                vbMap.push_back(accessor.BufferView);
                BufferView& bv = bufferViews[accessor.BufferView];
                mesh.VertexStride = accessor.Stride;
                mesh.VertexCount = static_cast<uint32_t>(bv.Size) / accessor.Stride;
                mesh.Vertices.resize(bv.Size);
                std::memcpy(mesh.Vertices.data(), m_buffer.data() + bv.Offset, bv.Size);
            }
        }

        // Position offset 0, Normal offset 12 (after float3)
        positionOffset = 0;
        normalOffset = 12;

        // Meshlets
        {
            Accessor& accessor = accessors[meshView.Meshlets];
            BufferView& bufferView = bufferViews[accessor.BufferView];
            const Meshlet* src = reinterpret_cast<const Meshlet*>(m_buffer.data() + bufferView.Offset);
            mesh.Meshlets.assign(src, src + accessor.Count);
        }

        // Meshlet Subsets
        {
            Accessor& accessor = accessors[meshView.MeshletSubsets];
            BufferView& bufferView = bufferViews[accessor.BufferView];
            const Subset* src = reinterpret_cast<const Subset*>(m_buffer.data() + bufferView.Offset);
            mesh.MeshletSubsets.assign(src, src + accessor.Count);
        }

        // Unique Vertex Indices
        {
            Accessor& accessor = accessors[meshView.UniqueVertexIndices];
            BufferView& bufferView = bufferViews[accessor.BufferView];
            mesh.UniqueVertexIndices.resize(bufferView.Size);
            std::memcpy(mesh.UniqueVertexIndices.data(), m_buffer.data() + bufferView.Offset, bufferView.Size);
        }

        // Primitive Indices
        {
            Accessor& accessor = accessors[meshView.PrimitiveIndices];
            BufferView& bufferView = bufferViews[accessor.BufferView];
            const PackedTriangle* src = reinterpret_cast<const PackedTriangle*>(m_buffer.data() + bufferView.Offset);
            mesh.PrimitiveIndices.assign(src, src + accessor.Count);
        }
    }

    return true;
}

void MeshletModel::uploadGpuResources(Device* pDevice, RenderContext* pRenderContext)
{
    for (auto& m : m_meshes)
    {
        // Create packed vertex buffer: float3 Position + float3 Normal per vertex
        struct Vertex
        {
            float position[3];
            float normal[3];
        };
        std::vector<Vertex> packedVerts(m.VertexCount);
        for (uint32_t i = 0; i < m.VertexCount; ++i)
        {
            const uint8_t* v = m.Vertices.data() + i * m.VertexStride;
            std::memcpy(packedVerts[i].position, v, 12);
            std::memcpy(packedVerts[i].normal, v + 12, 12);
        }

        m.pVertexBuffer = pDevice->createStructuredBuffer(
            sizeof(Vertex),
            m.VertexCount,
            ResourceBindFlags::ShaderResource,
            MemoryType::DeviceLocal,
            packedVerts.data());

        m.pMeshletBuffer = pDevice->createStructuredBuffer(
            sizeof(Meshlet),
            (uint32_t)m.Meshlets.size(),
            ResourceBindFlags::ShaderResource,
            MemoryType::DeviceLocal,
            m.Meshlets.data());

        size_t uviAligned = divRoundUp(m.UniqueVertexIndices.size(), 4u) * 4;
        m.pUniqueVertexIndexBuffer = pDevice->createBuffer(
            uviAligned,
            ResourceBindFlags::ShaderResource,
            MemoryType::DeviceLocal,
            m.UniqueVertexIndices.data());

        m.pPrimitiveIndexBuffer = pDevice->createStructuredBuffer(
            sizeof(PackedTriangle),
            (uint32_t)m.PrimitiveIndices.size(),
            ResourceBindFlags::ShaderResource,
            MemoryType::DeviceLocal,
            m.PrimitiveIndices.data());
    }
}
