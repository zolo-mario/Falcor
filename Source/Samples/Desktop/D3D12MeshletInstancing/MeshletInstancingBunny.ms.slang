/**
 * D3D12 Meshlet Instancing - Falcor bunny scene with SceneMeshletData.
 * Mesh shader + Pixel shader pipeline with user-defined instancing.
 * Requires Shader Model 6.5.
 * Uses Falcor Scene (bunny), SceneMeshletData, user Instance buffer, and Scene camera.
 */

import Scene.Scene;

// GPU meshlet structure (must match C++ GpuMeshletDesc)
struct GpuMeshlet
{
    uint vertexOffset;
    uint triangleOffset;
    uint vertexCount;
    uint triangleCount;
    float3 boundCenter;
    float boundRadius;
    uint instanceID;
    uint primitiveOffset;
    uint meshID;
    uint _pad0;
};

struct Instance
{
    float4x4 World;
    float4x4 WorldInvTranspose;
};

struct DrawParams
{
    uint InstanceCount;
    uint InstanceOffset;
};

struct MeshInfo
{
    uint MeshletCount;
    uint MeshletOffset;
};

cbuffer CB : register(b0)
{
    uint gDrawMeshlets;
    uint gDebugInstanceColor;  // 1 = color by instance index (debug)
};

cbuffer DrawParamsCB : register(b1)
{
    DrawParams DrawParams_data;
};

cbuffer MeshInfoCB : register(b2)
{
    MeshInfo MeshInfo_data;
};

StructuredBuffer<GpuMeshlet> gMeshlets;
StructuredBuffer<uint> gMeshletVertices;
StructuredBuffer<uint> gMeshletTriangles;
StructuredBuffer<Instance> Instances;

struct MeshVertexOut
{
    float4 posH : SV_Position;
    float3 positionVS : POSITION0;
    float3 normal : NORMAL0;
    nointerpolation uint meshletIndex : COLOR0;
    nointerpolation uint instanceIndex : COLOR1;  // debug: which user instance
};

static const uint MAX_VERTS = 64;
static const uint MAX_PRIMS = 124;

[outputtopology("triangle")]
[numthreads(128, 1, 1)]
void meshMain(
    in uint tig : SV_GroupIndex,
    in uint3 groupID : SV_GroupID,
    OutputVertices<MeshVertexOut, MAX_VERTS> verts,
    OutputIndices<uint3, MAX_PRIMS> triangles)
{
    uint gid = groupID.x;
    uint meshletIndex = gid / DrawParams_data.InstanceCount;
    uint startInstance = gid % DrawParams_data.InstanceCount;
    uint instanceCount = 1;

    GpuMeshlet meshlet;
    meshlet.vertexCount = 0;
    meshlet.triangleCount = 0;

    if (meshletIndex < MeshInfo_data.MeshletCount)
    {
        meshlet = gMeshlets[MeshInfo_data.MeshletOffset + meshletIndex];

        // Packing for last meshlet: multiple instances per group (disabled when InstanceCount<=32 to avoid "飞面" bug)
        if (meshletIndex == MeshInfo_data.MeshletCount - 1 && DrawParams_data.InstanceCount > 32)
        {
            uint instancesPerGroup = min(MAX_VERTS / max(1u, meshlet.vertexCount), MAX_PRIMS / max(1u, meshlet.triangleCount));
            if (instancesPerGroup > 1)
            {
                uint unpackedGroupCount = (MeshInfo_data.MeshletCount - 1) * DrawParams_data.InstanceCount;
                uint packedIndex = gid - unpackedGroupCount;
                startInstance = packedIndex * instancesPerGroup;
                instanceCount = min(DrawParams_data.InstanceCount - startInstance, instancesPerGroup);
            }
        }
    }

    uint vertCount = meshlet.vertexCount * instanceCount;
    uint primCount = meshlet.triangleCount * instanceCount;

    SetMeshOutputCounts(vertCount, primCount);

    if (vertCount == 0 || primCount == 0)
        return;

    GeometryInstanceID geomInstanceID;
    geomInstanceID.index = meshlet.instanceID;
    GeometryInstanceData instanceData = gScene.getGeometryInstance(geomInstanceID);
    uint vbOffset = instanceData.vbOffset;

    float4x4 viewProj = gScene.camera.getViewProj();
    float4x4 viewMat = gScene.camera.data.viewMat;

    // Output vertices
    if (tig < vertCount)
    {
        uint readIndex = tig % meshlet.vertexCount;
        uint instanceId = tig / meshlet.vertexCount;
        uint instanceIndex = startInstance + instanceId;
        Instance inst = Instances[DrawParams_data.InstanceOffset + instanceIndex];

        uint globalVtxIdx = vbOffset + gMeshletVertices[meshlet.vertexOffset + readIndex];
        StaticVertexData vtx = gScene.getVertex(globalVtxIdx);

        // Match original DX sample: mul(position, World) = row vector * matrix
        float4 worldPos = mul(float4(vtx.position, 1.0), inst.World);
        float4 clipPos = mul(viewProj, worldPos);
        float4 viewPos = mul(viewMat, worldPos);

        MeshVertexOut outVtx;
        outVtx.posH = clipPos;
        outVtx.positionVS = viewPos.xyz;
        outVtx.normal = mul(float4(vtx.normal, 0.0), inst.WorldInvTranspose).xyz;
        outVtx.meshletIndex = meshletIndex;
        outVtx.instanceIndex = DrawParams_data.InstanceOffset + instanceIndex;  // debug
        verts[tig] = outVtx;
    }

    // Output triangles
    if (tig < primCount)
    {
        uint readIndex = tig % meshlet.triangleCount;
        uint instanceId = tig / meshlet.triangleCount;
        uint triOffset = meshlet.triangleOffset + readIndex * 3;
        uint idx0 = gMeshletTriangles[triOffset + 0];
        uint idx1 = gMeshletTriangles[triOffset + 1];
        uint idx2 = gMeshletTriangles[triOffset + 2];
        triangles[tig] = uint3(idx0, idx1, idx2) + (meshlet.vertexCount * instanceId);
    }
}

float4 psMain(MeshVertexOut input) : SV_TARGET
{
    float ambientIntensity = 0.1;
    float3 lightDir = -normalize(float3(1, -1, 1));

    float3 diffuseColor;
    float shininess;
    if (gDebugInstanceColor != 0)
    {
        uint idx = input.instanceIndex % 3u;
        diffuseColor = float3(idx == 0u ? 1.0 : 0.0, idx == 1u ? 1.0 : 0.0, idx == 2u ? 1.0 : 0.0);
        shininess = 16.0;
    }
    else if (gDrawMeshlets != 0)
    {
        uint meshletIndex = input.meshletIndex;
        diffuseColor = float3(
            float(meshletIndex & 1),
            float(meshletIndex & 3) / 4,
            float(meshletIndex & 7) / 8);
        shininess = 16.0;
    }
    else
    {
        diffuseColor = 0.8;
        shininess = 64.0;
    }

    float3 normal = normalize(input.normal);
    float cosAngle = saturate(dot(normal, lightDir));
    float3 viewDir = -normalize(input.positionVS);
    float3 halfAngle = normalize(lightDir + viewDir);
    float blinnTerm = saturate(dot(normal, halfAngle));
    blinnTerm = cosAngle != 0.0 ? blinnTerm : 0.0;
    blinnTerm = pow(blinnTerm, shininess);

    float3 finalColor = (cosAngle + blinnTerm + ambientIntensity) * diffuseColor;

    return float4(finalColor, 1);
}
