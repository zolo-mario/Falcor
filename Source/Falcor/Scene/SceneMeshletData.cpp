#include "SceneMeshletData.h"
#include "Scene.h"
#include "Core/API/Buffer.h"
#include "Core/API/Device.h"
#include "Core/API/RenderContext.h"
#include "Utils/Logger.h"

#include <meshoptimizer.h>

namespace Falcor
{
    SceneMeshletData::SceneMeshletData(ref<Device> pDevice, const Scene* pScene)
        : mpDevice(pDevice)
        , mpScene(pScene)
    {
    }

    SceneMeshletData::~SceneMeshletData() = default;

    void SceneMeshletData::build(RenderContext* pRenderContext)
    {
        if (!mpScene || mpScene->getMeshCount() == 0 || mpScene->getGeometryInstanceCount() == 0)
        {
            mMeshMeshletData.clear();
            mGpuMeshlets.clear();
            mpMeshletBuffer = nullptr;
            mpMeshletVertices = nullptr;
            mpMeshletTriangles = nullptr;
            mMeshletCount = 0;
            mBuilt = true;
            return;
        }

        mMeshMeshletData.resize(mpScene->getMeshCount());
        std::vector<uint32_t> flatIndices;
        std::vector<float3> positions;

        for (MeshID meshID{0}; meshID.get() < mpScene->getMeshCount(); ++meshID)
        {
            const auto& meshDesc = mpScene->getMesh(meshID);
            if (meshDesc.getTriangleCount() == 0)
                continue;
            buildMeshletsForMesh(meshID, meshDesc, flatIndices, positions);
        }

        std::vector<uint32_t> meshBaseVertexOffset(mpScene->getMeshCount(), 0);
        std::vector<uint32_t> meshBaseTriangleOffset(mpScene->getMeshCount(), 0);
        uint32_t runningVertexOffset = 0;
        uint32_t runningTriangleOffset = 0;
        for (MeshID meshID{0}; meshID.get() < mpScene->getMeshCount(); ++meshID)
        {
            meshBaseVertexOffset[meshID.get()] = runningVertexOffset;
            meshBaseTriangleOffset[meshID.get()] = runningTriangleOffset;
            if (meshID.get() < mMeshMeshletData.size() && !mMeshMeshletData[meshID.get()].meshletVertices.empty())
            {
                runningVertexOffset += (uint32_t)mMeshMeshletData[meshID.get()].meshletVertices.size();
                runningTriangleOffset += (uint32_t)mMeshMeshletData[meshID.get()].meshletTriangles.size();
            }
        }

        mGpuMeshlets.clear();
        for (uint32_t instanceID = 0; instanceID < mpScene->getGeometryInstanceCount(); ++instanceID)
        {
            const auto& instance = mpScene->getGeometryInstance(instanceID);
            if (instance.getType() != GeometryType::TriangleMesh && instance.getType() != GeometryType::DisplacedTriangleMesh)
                continue;

            MeshID meshID{instance.geometryID};
            if (meshID.get() >= mMeshMeshletData.size())
                continue;

            const auto& meshData = mMeshMeshletData[meshID.get()];
            if (meshData.meshletVertexCount.empty())
                continue;

            uint32_t baseVertexOffset = meshBaseVertexOffset[meshID.get()];
            uint32_t baseTriangleOffset = meshBaseTriangleOffset[meshID.get()];
            uint32_t totalPrimitiveOffset = 0;
            expandMeshletsForInstance(
                instanceID, instance, meshData, meshID,
                baseVertexOffset, baseTriangleOffset, totalPrimitiveOffset);
        }

        mMeshletCount = (uint32_t)mGpuMeshlets.size();

        if (mMeshletCount == 0)
        {
            mpMeshletBuffer = nullptr;
            mpMeshletVertices = nullptr;
            mpMeshletTriangles = nullptr;
            mBuilt = true;
            return;
        }

        mpMeshletBuffer = mpDevice->createStructuredBuffer(
            sizeof(GpuMeshletDesc),
            mMeshletCount,
            ResourceBindFlags::ShaderResource,
            MemoryType::DeviceLocal,
            mGpuMeshlets.data());

        size_t totalVertices = 0;
        size_t totalTriangles = 0;
        for (const auto& meshData : mMeshMeshletData)
        {
            totalVertices += meshData.meshletVertices.size();
            totalTriangles += meshData.meshletTriangles.size();
        }

        if (totalVertices > 0)
        {
            std::vector<uint32_t> allVertices;
            allVertices.reserve(totalVertices);
            for (const auto& meshData : mMeshMeshletData)
            {
                allVertices.insert(allVertices.end(), meshData.meshletVertices.begin(), meshData.meshletVertices.end());
            }
            mpMeshletVertices = mpDevice->createStructuredBuffer(
                sizeof(uint32_t),
                (uint32_t)allVertices.size(),
                ResourceBindFlags::ShaderResource,
                MemoryType::DeviceLocal,
                allVertices.data());
        }

        if (totalTriangles > 0)
        {
            std::vector<uint32_t> trianglesUint32;
            trianglesUint32.reserve(totalTriangles);
            for (const auto& meshData : mMeshMeshletData)
            {
                for (uint8_t v : meshData.meshletTriangles)
                    trianglesUint32.push_back(static_cast<uint32_t>(v));
            }
            mpMeshletTriangles = mpDevice->createStructuredBuffer(
                sizeof(uint32_t),
                (uint32_t)trianglesUint32.size(),
                ResourceBindFlags::ShaderResource,
                MemoryType::DeviceLocal,
                trianglesUint32.data());
        }

        mBuilt = true;
        logInfo("SceneMeshletData: Built {} meshlets for {} mesh instances", mMeshletCount, mpScene->getGeometryInstanceCount());
    }

    void SceneMeshletData::buildMeshletsForMesh(MeshID meshID, const MeshDesc& meshDesc,
        std::vector<uint32_t>& flatIndices, std::vector<float3>& positions)
    {
        uint32_t vertexCount = meshDesc.vertexCount;
        uint32_t triangleCount = meshDesc.getTriangleCount();
        if (triangleCount == 0)
            return;

        std::map<std::string, ref<Buffer>> buffers;
        buffers["triangleIndices"] = mpDevice->createStructuredBuffer(
            sizeof(uint3), triangleCount,
            ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
            MemoryType::DeviceLocal);
        buffers["positions"] = mpDevice->createStructuredBuffer(
            sizeof(float3), vertexCount,
            ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
            MemoryType::DeviceLocal);
        buffers["texcrds"] = mpDevice->createStructuredBuffer(
            sizeof(float2), vertexCount,
            ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
            MemoryType::DeviceLocal);

        const_cast<Scene*>(mpScene)->getMeshVerticesAndIndices(meshID, buffers);

        flatIndices.resize(triangleCount * 3);
        positions.resize(vertexCount);

        flatIndices.resize(triangleCount * 3);
        buffers["triangleIndices"]->getBlob(flatIndices.data(), 0, triangleCount * sizeof(uint3));
        buffers["positions"]->getBlob(positions.data(), 0, vertexCount * sizeof(float3));

        size_t maxMeshlets = meshopt_buildMeshletsBound(
            flatIndices.size(), kMaxVerticesPerMeshlet, kMaxTrianglesPerMeshlet);

        std::vector<meshopt_Meshlet> meshlets(maxMeshlets);
        std::vector<uint32_t> meshletVertices(maxMeshlets * kMaxVerticesPerMeshlet);
        std::vector<uint8_t> meshletTriangles(maxMeshlets * kMaxTrianglesPerMeshlet * 3);

        size_t meshletCount = meshopt_buildMeshlets(
            meshlets.data(),
            meshletVertices.data(),
            meshletTriangles.data(),
            flatIndices.data(),
            flatIndices.size(),
            reinterpret_cast<const float*>(positions.data()),
            vertexCount,
            sizeof(float3),
            kMaxVerticesPerMeshlet,
            kMaxTrianglesPerMeshlet,
            kConeWeight);

        meshlets.resize(meshletCount);

        if (meshletCount > 0)
        {
            const auto& lastMeshlet = meshlets.back();
            meshletVertices.resize(lastMeshlet.vertex_offset + lastMeshlet.vertex_count);
            meshletTriangles.resize(lastMeshlet.triangle_offset + ((lastMeshlet.triangle_count * 3 + 3) & ~3));
        }

        auto& meshData = mMeshMeshletData[meshID.get()];
        meshData.meshletVertices = std::move(meshletVertices);
        meshData.meshletTriangles = std::move(meshletTriangles);
        meshData.meshletVertexOffset.resize(meshletCount);
        meshData.meshletTriangleOffset.resize(meshletCount);
        meshData.meshletVertexCount.resize(meshletCount);
        meshData.meshletTriangleCount.resize(meshletCount);
        meshData.meshletBoundCenter.resize(meshletCount);
        meshData.meshletBoundRadius.resize(meshletCount);

        for (size_t i = 0; i < meshletCount; i++)
        {
            const auto& m = meshlets[i];
            meshData.meshletVertexOffset[i] = (uint32_t)m.vertex_offset;
            meshData.meshletTriangleOffset[i] = (uint32_t)m.triangle_offset;
            meshData.meshletVertexCount[i] = m.vertex_count;
            meshData.meshletTriangleCount[i] = m.triangle_count;

            meshopt_Bounds bounds = meshopt_computeMeshletBounds(
                &meshData.meshletVertices[m.vertex_offset],
                &meshData.meshletTriangles[m.triangle_offset],
                m.triangle_count,
                reinterpret_cast<const float*>(positions.data()),
                vertexCount,
                sizeof(float3));

            meshData.meshletBoundCenter[i] = float3(bounds.center[0], bounds.center[1], bounds.center[2]);
            meshData.meshletBoundRadius[i] = bounds.radius;
        }
    }

    void SceneMeshletData::expandMeshletsForInstance(uint32_t instanceID, const GeometryInstanceData& instance,
        const MeshMeshletData& meshData, MeshID meshID,
        uint32_t& baseVertexOffset, uint32_t& baseTriangleOffset, uint32_t& totalPrimitiveOffset)
    {
        const auto& globalMatrices = mpScene->getAnimationController()->getGlobalMatrices();
        float4x4 worldMatrix = globalMatrices[instance.globalMatrixID];

        size_t meshletCount = meshData.meshletVertexCount.size();
        for (size_t i = 0; i < meshletCount; i++)
        {
            GpuMeshletDesc gpuMeshlet;
            gpuMeshlet.vertexOffset = baseVertexOffset + meshData.meshletVertexOffset[i];
            gpuMeshlet.triangleOffset = baseTriangleOffset + meshData.meshletTriangleOffset[i];
            gpuMeshlet.vertexCount = meshData.meshletVertexCount[i];
            gpuMeshlet.triangleCount = meshData.meshletTriangleCount[i];
            gpuMeshlet.instanceID = instanceID;
            gpuMeshlet.primitiveOffset = totalPrimitiveOffset;
            gpuMeshlet.meshID = meshID.get();
            gpuMeshlet._pad0 = 0;

            float3 center = meshData.meshletBoundCenter[i];
            float4 worldCenter4 = mul(worldMatrix, float4(center, 1.0f));
            gpuMeshlet.boundCenter = float3(worldCenter4.x, worldCenter4.y, worldCenter4.z);
            gpuMeshlet.boundRadius = meshData.meshletBoundRadius[i];

            mGpuMeshlets.push_back(gpuMeshlet);
            totalPrimitiveOffset += meshData.meshletTriangleCount[i];
        }

    }
}
