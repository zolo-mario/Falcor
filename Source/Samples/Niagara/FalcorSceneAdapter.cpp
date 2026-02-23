#include "FalcorSceneAdapter.h"

#include "Scene/Scene.h"
#include "Scene/Material/BasicMaterial.h"
#include "Scene/Animation/AnimationController.h"
#include "Scene/Lights/Light.h"
#include "Utils/Math/FalcorMath.h"
#include "Core/API/Buffer.h"
#include "Core/API/Device.h"
#include "Utils/Math/Matrix.h"

#include <meshoptimizer.h>

#include <algorithm>
#include <map>

namespace Falcor
{

namespace
{

const size_t MESH_MAXVTX = 64;
const size_t MESH_MAXTRI = 96;
const size_t MESH_MINTRI = MESH_MAXTRI / 4;
const float MESHLET_CONE_WEIGHT = 0.25f;
const float MESHLET_FILL_WEIGHT = 0.5f;

static int addTexturePath(std::vector<std::string>& texturePaths, const std::filesystem::path& path)
{
    if (path.empty())
        return 0;
    std::string pathStr = path.string();
    for (size_t i = 0; i < texturePaths.size(); ++i)
    {
        if (texturePaths[i] == pathStr)
            return (int)(i + 1);
    }
    texturePaths.push_back(pathStr);
    return (int)texturePaths.size();
}

static void appendMeshlet(NiagaraFormat::Geometry& result,
    const meshopt_Meshlet& meshlet,
    const std::vector<float3>& vertices,
    const std::vector<unsigned int>& meshlet_vertices,
    const std::vector<unsigned char>& meshlet_triangles,
    uint32_t baseVertex,
    bool lod0)
{
    size_t dataOffset = result.meshletdata.size();
    unsigned int minVertex = ~0u, maxVertex = 0;
    for (unsigned int i = 0; i < meshlet.vertex_count; ++i)
    {
        minVertex = std::min(meshlet_vertices[meshlet.vertex_offset + i], minVertex);
        maxVertex = std::max(meshlet_vertices[meshlet.vertex_offset + i], maxVertex);
    }
    bool shortRefs = maxVertex - minVertex < (1 << 16);
    for (unsigned int i = 0; i < meshlet.vertex_count; ++i)
    {
        unsigned int ref = meshlet_vertices[meshlet.vertex_offset + i] - minVertex;
        if (shortRefs && i % 2)
            result.meshletdata.back() |= ref << 16;
        else
            result.meshletdata.push_back(ref);
    }
    const unsigned int* indexGroups = reinterpret_cast<const unsigned int*>(&meshlet_triangles[0] + meshlet.triangle_offset);
    unsigned int indexGroupCount = (meshlet.triangle_count * 3 + 3) / 4;
    for (unsigned int i = 0; i < indexGroupCount; ++i)
        result.meshletdata.push_back(indexGroups[i]);
    if (lod0)
    {
        for (unsigned int i = 0; i < meshlet.vertex_count; ++i)
        {
            unsigned int vtx = meshlet_vertices[meshlet.vertex_offset + i];
            const auto& v = vertices[vtx];
            result.meshletvtx0.push_back(meshopt_quantizeHalf(v.x));
            result.meshletvtx0.push_back(meshopt_quantizeHalf(v.y));
            result.meshletvtx0.push_back(meshopt_quantizeHalf(v.z));
            result.meshletvtx0.push_back(0);
        }
    }
    meshopt_Bounds bounds = meshopt_computeMeshletBounds(
        &meshlet_vertices[meshlet.vertex_offset],
        &meshlet_triangles[meshlet.triangle_offset],
        meshlet.triangle_count,
        reinterpret_cast<const float*>(vertices.data()),
        vertices.size(),
        sizeof(float3));
    NiagaraFormat::Meshlet m = {};
    m.dataOffset = uint32_t(dataOffset);
    m.baseVertex = baseVertex + minVertex;
    m.triangleCount = (uint8_t)meshlet.triangle_count;
    m.vertexCount = (uint8_t)meshlet.vertex_count;
    m.shortRefs = shortRefs ? 1u : 0u;
    m.center[0] = meshopt_quantizeHalf(bounds.center[0]);
    m.center[1] = meshopt_quantizeHalf(bounds.center[1]);
    m.center[2] = meshopt_quantizeHalf(bounds.center[2]);
    m.radius = meshopt_quantizeHalf(bounds.radius);
    m.cone_axis[0] = bounds.cone_axis_s8[0];
    m.cone_axis[1] = bounds.cone_axis_s8[1];
    m.cone_axis[2] = bounds.cone_axis_s8[2];
    m.cone_cutoff = bounds.cone_cutoff_s8;
    result.meshlets.push_back(m);
}

static size_t appendMeshlets(NiagaraFormat::Geometry& result,
    const std::vector<float3>& vertices,
    std::vector<uint32_t>& indices,
    uint32_t baseVertex,
    bool lod0)
{
    std::vector<meshopt_Meshlet> meshlets(meshopt_buildMeshletsBound(indices.size(), MESH_MAXVTX, MESH_MINTRI));
    std::vector<unsigned int> meshlet_vertices(indices.size());
    std::vector<unsigned char> meshlet_triangles(indices.size());
    meshlets.resize(meshopt_buildMeshlets(meshlets.data(), meshlet_vertices.data(), meshlet_triangles.data(),
        indices.data(), indices.size(), reinterpret_cast<const float*>(vertices.data()), vertices.size(), sizeof(float3),
        MESH_MAXVTX, MESH_MAXTRI, MESHLET_CONE_WEIGHT));
    for (auto& meshlet : meshlets)
    {
        meshopt_optimizeMeshlet(&meshlet_vertices[meshlet.vertex_offset], &meshlet_triangles[meshlet.triangle_offset],
            meshlet.triangle_count, meshlet.vertex_count);
        appendMeshlet(result, meshlet, vertices, meshlet_vertices, meshlet_triangles, baseVertex, lod0);
    }
    return meshlets.size();
}

static size_t buildMeshlets(NiagaraFormat::Geometry& geometry,
    const std::vector<float3>& positions,
    std::vector<uint32_t>& indices,
    uint32_t baseVertex,
    bool lod0)
{
    meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), positions.size());
    return appendMeshlets(geometry, positions, indices, baseVertex, lod0);
}

}

bool convertFalcorSceneToNiagaraScene(Scene* pScene, NiagaraFormat::NiagaraSceneFormat& outScene)
{
    if (!pScene || pScene->getMeshCount() == 0)
        return false;
    outScene.geometry.vertices.clear();
    outScene.geometry.indices.clear();
    outScene.geometry.meshlets.clear();
    outScene.geometry.meshletdata.clear();
    outScene.geometry.meshletvtx0.clear();
    outScene.geometry.meshes.clear();
    outScene.materials.clear();
    outScene.draws.clear();
    outScene.texturePaths.clear();
    auto pDevice = pScene->getDevice();
    auto& geometry = outScene.geometry;
    auto& materials = outScene.materials;
    auto& draws = outScene.draws;
    auto& texturePaths = outScene.texturePaths;
    materials.resize(1);
    materials[0].albedoTexture = 0;
    materials[0].normalTexture = 0;
    materials[0].specularTexture = 0;
    materials[0].emissiveTexture = 0;
    materials[0].diffuseFactor = float4(1.f);
    materials[0].specularFactor = float4(1.f);
    materials[0].emissiveFactor = float3(0.f);
    for (MaterialID materialID{0}; materialID.get() < pScene->getMaterialCount(); ++materialID)
    {
        auto pMaterial = pScene->getMaterial(materialID);
        auto pBasic = pMaterial ? pMaterial->toBasicMaterial() : nullptr;
        NiagaraFormat::Material mat = {};
        mat.albedoTexture = 0;
        mat.normalTexture = 0;
        mat.specularTexture = 0;
        mat.emissiveTexture = 0;
        mat.diffuseFactor = float4(1.f);
        mat.specularFactor = float4(1, 1, 1, 1);
        mat.emissiveFactor = float3(0.f);
        if (pBasic)
        {
            mat.diffuseFactor = pBasic->getBaseColor();
            mat.specularFactor = pBasic->getSpecularParams();
            mat.emissiveFactor = pBasic->getData().emissive * pBasic->getData().emissiveFactor;
            if (auto pTex = pBasic->getBaseColorTexture())
            {
                if (!pTex->getSourcePath().empty())
                    mat.albedoTexture = addTexturePath(texturePaths, pTex->getSourcePath());
            }
            if (auto pTex = pBasic->getNormalMap())
            {
                if (!pTex->getSourcePath().empty())
                    mat.normalTexture = addTexturePath(texturePaths, pTex->getSourcePath());
            }
            if (auto pTex = pBasic->getSpecularTexture())
            {
                if (!pTex->getSourcePath().empty())
                    mat.specularTexture = addTexturePath(texturePaths, pTex->getSourcePath());
            }
            if (auto pTex = pBasic->getEmissiveTexture())
            {
                if (!pTex->getSourcePath().empty())
                    mat.emissiveTexture = addTexturePath(texturePaths, pTex->getSourcePath());
            }
        }
        materials.push_back(mat);
    }
    for (MeshID meshID{0}; meshID.get() < pScene->getMeshCount(); ++meshID)
    {
        const auto& meshDesc = pScene->getMesh(meshID);
        uint32_t vertexCount = meshDesc.vertexCount;
        uint32_t triangleCount = meshDesc.getTriangleCount();
        if (triangleCount == 0)
            continue;
        std::map<std::string, ref<Buffer>> buffers;
        buffers["triangleIndices"] = pDevice->createStructuredBuffer(
            sizeof(uint3), triangleCount,
            ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
            MemoryType::DeviceLocal);
        buffers["positions"] = pDevice->createStructuredBuffer(
            sizeof(float3), vertexCount,
            ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
            MemoryType::DeviceLocal);
        buffers["texcrds"] = pDevice->createStructuredBuffer(
            sizeof(float2), vertexCount,
            ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
            MemoryType::DeviceLocal);
        pScene->getMeshVerticesAndIndices(meshID, buffers);
        std::vector<uint3> triIndices(triangleCount);
        std::vector<float3> positions(vertexCount);
        buffers["triangleIndices"]->getBlob(triIndices.data(), 0, triangleCount * sizeof(uint3));
        buffers["positions"]->getBlob(positions.data(), 0, vertexCount * sizeof(float3));
        std::vector<uint32_t> indices(triangleCount * 3);
        for (uint32_t i = 0; i < triangleCount; ++i)
        {
            indices[i * 3 + 0] = triIndices[i].x;
            indices[i * 3 + 1] = triIndices[i].y;
            indices[i * 3 + 2] = triIndices[i].z;
        }
        std::vector<NiagaraFormat::Vertex> vertices(vertexCount);
        for (uint32_t i = 0; i < vertexCount; ++i)
        {
            vertices[i].vx = meshopt_quantizeHalf(positions[i].x);
            vertices[i].vy = meshopt_quantizeHalf(positions[i].y);
            vertices[i].vz = meshopt_quantizeHalf(positions[i].z);
            vertices[i].tp = 0;
            vertices[i].np = (511) | (511) << 10 | (511) << 20;
            vertices[i].tu = 0;
            vertices[i].tv = 0;
        }
        std::vector<uint32_t> remap(vertexCount);
        size_t uniqueVertices = meshopt_generateVertexRemap(remap.data(), indices.data(), indices.size(),
            vertices.data(), vertexCount, sizeof(NiagaraFormat::Vertex));
        meshopt_remapVertexBuffer(vertices.data(), vertices.data(), vertexCount, sizeof(NiagaraFormat::Vertex), remap.data());
        meshopt_remapIndexBuffer(indices.data(), indices.data(), indices.size(), remap.data());
        vertices.resize(uniqueVertices);
        std::vector<float3> remappedPositions(uniqueVertices);
        for (size_t i = 0; i < vertexCount; ++i)
            remappedPositions[remap[i]] = positions[i];
        positions = std::move(remappedPositions);
        uint32_t vertexOffset = (uint32_t)geometry.vertices.size();
        geometry.vertices.insert(geometry.vertices.end(), vertices.begin(), vertices.end());
        NiagaraFormat::Mesh mesh = {};
        mesh.vertexOffset = vertexOffset;
        mesh.vertexCount = (uint32_t)vertices.size();
        mesh.center = float3(0.f);
        for (const auto& p : positions)
            mesh.center += p;
        mesh.center /= (float)positions.size();
        mesh.radius = 0.f;
        for (const auto& p : positions)
            mesh.radius = std::max(mesh.radius, math::length(p - mesh.center));
        NiagaraFormat::MeshLod& lod = mesh.lods[mesh.lodCount++];
        lod.indexOffset = (uint32_t)geometry.indices.size();
        lod.indexCount = (uint32_t)indices.size();
        geometry.indices.insert(geometry.indices.end(), indices.begin(), indices.end());
        lod.meshletOffset = (uint32_t)geometry.meshlets.size();
        lod.meshletCount = (uint32_t)buildMeshlets(geometry, positions, indices, vertexOffset, true);
        lod.error = 0.f;
        geometry.meshes.push_back(mesh);
    }
    const auto& globalMatrices = pScene->getAnimationController()->getGlobalMatrices();
    for (uint32_t instanceID = 0; instanceID < pScene->getGeometryInstanceCount(); ++instanceID)
    {
        const auto& instance = pScene->getGeometryInstance(instanceID);
        if (instance.getType() != GeometryType::TriangleMesh && instance.getType() != GeometryType::DisplacedTriangleMesh)
            continue;
        MeshID meshID{instance.geometryID};
        if (meshID.get() >= geometry.meshes.size())
            continue;
        float3 scale, translation, skew;
        float4 perspective;
        quatf orientation;
        if (!math::decompose(globalMatrices[instance.globalMatrixID], scale, orientation, translation, skew, perspective))
        {
            orientation = quatf::identity();
            scale = float3(1.f);
            translation = float3(0.f);
        }
        NiagaraFormat::MeshDraw draw = {};
        draw.position = translation;
        draw.scale = std::max({scale.x, scale.y, scale.z});
        draw.orientation = orientation;
        draw.meshIndex = meshID.get();
        draw.meshletVisibilityOffset = 0;
        draw.postPass = 0;
        draw.materialIndex = (uint32_t)(instance.materialID + 1);
        draws.push_back(draw);
    }
    if (!pScene->getCameras().empty())
    {
        const auto& pCam = pScene->getCamera();
        outScene.camera.position = pCam->getPosition();
        float4x4 invView = math::inverse(pCam->getViewMatrix());
        float3 scale, translation, skew;
        float4 perspective;
        if (!math::decompose(invView, scale, outScene.camera.orientation, translation, skew, perspective))
            outScene.camera.orientation = quatf::identity();
        outScene.camera.fovY = focalLengthToFovY(pCam->getFocalLength(), pCam->getFrameHeight());
        outScene.camera.znear = pCam->getNearPlane();
        outScene.camera.viewMatrix = pCam->getViewMatrix();
    }
    else
    {
        outScene.camera.position = float3(0.f);
        outScene.camera.orientation = quatf::identity();
        outScene.camera.fovY = math::radians(70.f);
        outScene.camera.znear = 0.1f;
        outScene.camera.viewMatrix = float4x4::identity();
    }
    outScene.sunDirection = normalize(float3(1.f, 1.f, 1.f));
    for (const auto& light : pScene->getLights())
    {
        if (light && light->getType() == LightType::Distant)
        {
            auto* distantLight = dynamic_cast<DistantLight*>(light.get());
            if (distantLight)
            {
                outScene.sunDirection = normalize(distantLight->getWorldDirection());
                break;
            }
        }
    }
    return true;
}

}
