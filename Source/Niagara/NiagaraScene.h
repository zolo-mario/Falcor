#pragma once

#include "Falcor.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Falcor
{

struct alignas(8) NiagaraMeshlet
{
    uint16_t center[3];
    uint16_t radius;
    int8_t cone_axis[3];
    int8_t cone_cutoff;

    uint32_t dataOffset; // dataOffset..dataOffset+vertexCount-1 stores vertex indices, we store indices packed in 4b units after that
    uint32_t baseVertex;
    uint8_t vertexCount;
    uint8_t triangleCount;
    uint8_t shortRefs;
    uint8_t padding;
};

struct alignas(16) NiagaraMaterial
{
    int albedoTexture;
    int normalTexture;
    int specularTexture;
    int emissiveTexture;

    float4 diffuseFactor;
    float4 specularFactor;
    float3 emissiveFactor;
};

struct alignas(16) NiagaraMeshDraw
{
    float3 position;
    float scale;
    quatf orientation;

    uint32_t meshIndex;
    uint32_t meshletVisibilityOffset;
    uint32_t postPass;
    uint32_t materialIndex;
};

struct NiagaraVertex
{
    uint16_t vx, vy, vz;
    uint16_t tp; // packed tangent: 8-8 octahedral
    uint32_t np; // packed normal: 10-10-10-2 vector + bitangent sign
    uint16_t tu, tv;
};

struct NiagaraMeshLod
{
    uint32_t indexOffset;
    uint32_t indexCount;
    uint32_t meshletOffset;
    uint32_t meshletCount;
    float error;
};

struct alignas(16) NiagaraMesh
{
    float3 center;
    float radius;

    uint32_t vertexOffset;
    uint32_t vertexCount;

    uint32_t lodCount;
    NiagaraMeshLod lods[8];
};

struct NiagaraGeometry
{
    std::vector<NiagaraVertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<NiagaraMeshlet> meshlets;
    std::vector<uint32_t> meshletdata;
    std::vector<uint16_t> meshletvtx0; // 4 position components per vertex referenced by meshlets in lod 0, packed tightly
    std::vector<NiagaraMesh> meshes;
};

struct NiagaraCamera
{
    float3 position;
    quatf orientation;
    float fovY;
    float znear;
};

struct NiagaraScene
{
    NiagaraGeometry geometry;
    std::vector<NiagaraMaterial> materials;
    std::vector<NiagaraMeshDraw> draws;
    std::vector<std::string> texturePaths;
    NiagaraCamera camera;
    float3 sunDirection;
};

/** Build meshlets from positions and indices, append to geometry.
 *  Performs vertex cache optimization, then meshlet generation (fast/default/spatial mode).
 *  \param geometry Target geometry to append meshlets to
 *  \param positions Vertex positions (float3)
 *  \param indices Triangle indices (modified in-place by vertex cache optimization)
 *  \param baseVertex Base vertex offset for this mesh
 *  \param lod0 If true, also writes meshletvtx0 for ray tracing
 *  \param fast Use fast scan mode (meshopt_buildMeshletsScan)
 *  \param clrt Cluster ray tracing: use spatial mode for lod0
 *  \return Number of meshlets appended
 */
size_t buildMeshlets(NiagaraGeometry& geometry,
    const std::vector<float3>& positions,
    std::vector<uint32_t>& indices,
    uint32_t baseVertex,
    bool lod0,
    bool fast = false,
    bool clrt = false);

/** Convert Falcor Scene to NiagaraScene, building meshlets for each mesh.
 *  \param pScene Falcor scene (must have meshes)
 *  \param outScene Output NiagaraScene
 *  \param doBuildMeshlets Enable meshlet generation
 *  \param fast Use fast meshlet build mode
 *  \param clrt Use cluster RT spatial mode for LOD 0
 *  \return true if conversion succeeded
 */
bool convertFalcorSceneToNiagaraScene(Scene* pScene,
                                      NiagaraScene& outScene,
                                      bool doBuildMeshletsParam = true,
                                      bool fast = false,
                                      bool clrt = false);

} // namespace Falcor
