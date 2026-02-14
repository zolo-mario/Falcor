#pragma once

#include "Core/Macros.h"
#include "Core/Object.h"
#include "SceneIDs.h"
#include "SceneTypes.slang"
#include "Utils/Math/Vector.h"

#include <memory>
#include <vector>

namespace Falcor
{
    class Device;
    class Scene;
    class RenderContext;
    class Buffer;

    /** Per-mesh meshlet geometry data (shared across instances).
     */
    struct MeshMeshletData
    {
        std::vector<uint32_t> meshletVertices;   ///< Vertex indices per meshlet
        std::vector<uint8_t> meshletTriangles;    ///< Triangle indices (3 per tri, uint8)
        std::vector<uint32_t> meshletVertexOffset; ///< Per-meshlet offset into meshletVertices
        std::vector<uint32_t> meshletTriangleOffset; ///< Per-meshlet offset into meshletTriangles
        std::vector<uint32_t> meshletVertexCount;
        std::vector<uint32_t> meshletTriangleCount;
        std::vector<float3> meshletBoundCenter;    ///< Object-space bound center per meshlet
        std::vector<float> meshletBoundRadius;     ///< Object-space bound radius per meshlet
    };

    /** GPU meshlet structure for rendering (matches MeshletRaster.ms.slang GpuMeshlet).
     */
    struct GpuMeshletDesc
    {
        uint32_t vertexOffset;
        uint32_t triangleOffset;
        uint32_t vertexCount;
        uint32_t triangleCount;
        float3 boundCenter;
        float boundRadius;
        uint32_t instanceID;
        uint32_t primitiveOffset;
        uint32_t meshID;
        uint32_t _pad0;
    };

    /** Scene meshlet data - builds and stores meshlet data for all mesh instances.
     *  Meshlet geometry is stored per-mesh (shared). GpuMeshlet entries are expanded
     *  per-instance with world-space bounds.
     */
    class FALCOR_API SceneMeshletData
    {
    public:
        static constexpr size_t kMaxVerticesPerMeshlet = 64;
        static constexpr size_t kMaxTrianglesPerMeshlet = 124;
        static constexpr float kConeWeight = 0.5f;

        SceneMeshletData(ref<Device> pDevice, const Scene* pScene);
        ~SceneMeshletData();

        /** Build meshlet data for all mesh instances. Call when scene changes.
         */
        void build(RenderContext* pRenderContext);

        /** Get meshlet count (total GpuMeshlets across all instances).
         */
        uint32_t getMeshletCount() const { return mMeshletCount; }

        /** Get GPU meshlet buffer (GpuMeshletDesc array).
         */
        const ref<Buffer>& getMeshletBuffer() const { return mpMeshletBuffer; }

        /** Get meshlet vertices buffer (uint32 indices).
         */
        const ref<Buffer>& getMeshletVerticesBuffer() const { return mpMeshletVertices; }

        /** Get meshlet triangles buffer (uint32, one per byte in meshopt format).
         */
        const ref<Buffer>& getMeshletTrianglesBuffer() const { return mpMeshletTriangles; }

        /** Check if meshlet data is valid (built and non-empty).
         */
        bool isValid() const { return mMeshletCount > 0 && mpMeshletBuffer; }

    private:
        void buildMeshletsForMesh(MeshID meshID, const MeshDesc& meshDesc,
            std::vector<uint32_t>& flatIndices, std::vector<float3>& positions);
        void expandMeshletsForInstance(uint32_t instanceID, const GeometryInstanceData& instance,
            const MeshMeshletData& meshData, MeshID meshID,
            uint32_t& baseVertexOffset, uint32_t& baseTriangleOffset, uint32_t& totalPrimitiveOffset);

        ref<Device> mpDevice;
        const Scene* mpScene;

        std::vector<MeshMeshletData> mMeshMeshletData;  ///< Per-mesh meshlet geometry
        std::vector<GpuMeshletDesc> mGpuMeshlets;       ///< Expanded for all instances

        ref<Buffer> mpMeshletBuffer;
        ref<Buffer> mpMeshletVertices;
        ref<Buffer> mpMeshletTriangles;

        uint32_t mMeshletCount = 0;
        bool mBuilt = false;
    };
}
