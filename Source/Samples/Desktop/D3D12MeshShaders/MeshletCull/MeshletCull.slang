/**
 * MeshletCull - AS + MS + PS pipeline with SceneMeshletData.
 * AS: Frustum culling per meshlet (using boundCenter, boundRadius from SceneMeshletData).
 * MS: Output meshlet vertices/primitives from gScene.
 * PS: Blinn-Phong shading, meshlet visualization.
 * Demonstrates culling meshlests before ever dispatching a mesh shader.
 */

import Scene.Scene;

#define AS_GROUP_SIZE 32

// GPU meshlet structure (must match Falcor GpuMeshletDesc)
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

struct CBData
{
    float4x4 viewProj;
    float4 planes[6];
    float3 viewPosition;
    uint _pad;
    uint drawMeshlets;
    uint meshletCount;
};
StructuredBuffer<CBData> gCB : register(t0);

StructuredBuffer<GpuMeshlet> gMeshlets : register(t1);
StructuredBuffer<uint> gMeshletVertices : register(t2);
StructuredBuffer<uint> gMeshletTriangles : register(t3);

struct Payload
{
    uint meshletIndices[AS_GROUP_SIZE];
};

struct MeshVertexOut
{
    float4 posH : SV_Position;
    float3 positionVS : POSITION0;
    float3 normal : NORMAL0;
    nointerpolation uint meshletIndex : COLOR0;
};

// Frustum culling: sphere (center, radius) vs 6 planes
bool isVisible(float3 center, float radius)
{
    for (int i = 0; i < 6; ++i)
    {
        if (dot(float4(center, 1), gCB[0].planes[i]) < -radius)
            return false;
    }
    return true;
}

groupshared Payload s_Payload;

[numthreads(AS_GROUP_SIZE, 1, 1)]
void ampMain(
    in uint gtid : SV_GroupThreadID,
    in uint dtid : SV_DispatchThreadID,
    in uint gid : SV_GroupID)
{
    bool visible = false;
    if (dtid < gCB[0].meshletCount)
    {
        GpuMeshlet m = gMeshlets[dtid];
        visible = isVisible(m.boundCenter, m.boundRadius);
    }

    if (visible)
    {
        uint index = WavePrefixCountBits(visible);
        s_Payload.meshletIndices[index] = dtid;
    }

    GroupMemoryBarrierWithGroupSync();
    uint visibleCount = WaveActiveCountBits(visible);
    DispatchMesh(visibleCount, 1, 1, s_Payload);
}

static const uint MAX_VERTS = 64;
static const uint MAX_PRIMS = 124;

[outputtopology("triangle")]
[numthreads(128, 1, 1)]
void meshMain(
    in uint tig : SV_GroupIndex,
    in uint3 groupID : SV_GroupID,
    in payload Payload payload,
    OutputVertices<MeshVertexOut, MAX_VERTS> verts,
    OutputIndices<uint3, MAX_PRIMS> triangles)
{
    uint meshletID = payload.meshletIndices[groupID.x];
    GpuMeshlet meshlet = gMeshlets[meshletID];
    GeometryInstanceID geomInstanceID;
    geomInstanceID.index = meshlet.instanceID;

    float4x4 worldMat = gScene.getWorldMatrix(geomInstanceID);
    float4x4 worldViewProj = mul(gCB[0].viewProj, worldMat);
    GeometryInstanceData instance = gScene.getGeometryInstance(geomInstanceID);
    uint vbOffset = instance.vbOffset;

    uint numVertices = meshlet.vertexCount;
    uint numPrimitives = meshlet.triangleCount;

    SetMeshOutputCounts(numVertices, numPrimitives);

    if (tig < numVertices)
    {
        uint localVtxIdx = tig;
        uint globalVtxIdx = vbOffset + gMeshletVertices[meshlet.vertexOffset + localVtxIdx];
        StaticVertexData vtx = gScene.getVertex(globalVtxIdx);

        float4 worldPos = mul(worldMat, float4(vtx.position, 1.0));
        float4 clipPos = mul(worldViewProj, float4(vtx.position, 1.0));
        float4 viewPos = mul(gScene.camera.data.viewMat, worldPos);

        MeshVertexOut outVtx;
        outVtx.posH = clipPos;
        outVtx.positionVS = viewPos.xyz;
        outVtx.normal = mul(float4(vtx.normal, 0), worldMat).xyz;
        outVtx.meshletIndex = meshletID;
        verts[tig] = outVtx;
    }

    if (tig < numPrimitives)
    {
        uint triOffset = meshlet.triangleOffset + tig * 3;
        uint idx0 = gMeshletTriangles[triOffset + 0];
        uint idx1 = gMeshletTriangles[triOffset + 1];
        uint idx2 = gMeshletTriangles[triOffset + 2];
        triangles[tig] = uint3(idx0, idx1, idx2);
    }
}

float3 luminosity(float3 color)
{
    return dot(color, float3(0.2126, 0.7152, 0.0722)).xxx;
}

float4 psMain(MeshVertexOut input) : SV_TARGET
{
    float ambientIntensity = 0.1;
    float3 lightDir = -normalize(float3(1, -1, 1));

    float3 color;
    float shininess;
    if (gCB[0].drawMeshlets)
    {
        uint meshletIndex = input.meshletIndex;
        color = float3(
            float(meshletIndex & 1),
            float(meshletIndex & 3) / 4,
            float(meshletIndex & 7) / 8);
        shininess = 16.0;
    }
    else
    {
        color = 0.8;
        shininess = 64.0;
    }

    float3 normal = normalize(input.normal);
    float cosAngle = saturate(dot(normal, lightDir));
    float3 viewDir = -normalize(input.positionVS);
    float3 halfAngle = normalize(lightDir + viewDir);
    float blinnTerm = saturate(dot(normal, halfAngle));
    blinnTerm = cosAngle != 0.0 ? blinnTerm : 0.0;
    blinnTerm = pow(blinnTerm, shininess);

    float3 finalColor = (cosAngle + blinnTerm + ambientIntensity) * color;
    return float4(finalColor, 1);
}
