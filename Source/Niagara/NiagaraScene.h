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
    int16_t cone_axis[3];
    int16_t cone_cutoff;

    uint32_t dataOffset; // dataOffset..dataOffset+vertexCount-1 stores vertex indices, we store indices packed in 4b units after that
    uint32_t baseVertex;
    uint16_t vertexCount;
    uint16_t triangleCount;
    uint16_t shortRefs;
    uint16_t padding;
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

/** Mesh task command for Niagara mesh shader (1:1 from original MeshTaskCommand). */
struct NiagaraMeshTaskCommand
{
    uint32_t drawId;
    uint32_t taskOffset;
    uint32_t taskCount;
    uint32_t lateDrawVisibility;
    uint32_t meshletVisibilityOffset;
};

/** CullData for shader Globals (1:1 from original). */
struct NiagaraCullData
{
    float4x4 view;
    float P00, P11, znear, zfar;
    float frustum[4];
    float lodTarget;
    float pyramidWidth, pyramidHeight;
    uint32_t drawCount;
    int32_t cullingEnabled;
    int32_t lodEnabled;
    int32_t occlusionEnabled;
    int32_t clusterOcclusionEnabled;
    int32_t clusterBackfaceEnabled;
    uint32_t postPass;
};

/** Globals push constant for mesh shader (1:1 from original). */
struct NiagaraGlobals
{
    float4x4 projection;
    NiagaraCullData cullData;
    float screenWidth;
    float screenHeight;
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
    float4x4 viewMatrix; ///< Cached view matrix from Falcor camera
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
