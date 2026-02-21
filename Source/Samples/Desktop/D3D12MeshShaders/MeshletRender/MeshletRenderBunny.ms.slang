/**
 * MeshletRender - Falcor bunny scene with SceneMeshletData.
 * Mesh shader + Pixel shader pipeline. No amplification shader.
 * Requires Shader Model 6.5.
 * Uses Falcor Scene (bunny), SceneMeshletData, and Scene camera.
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

cbuffer CB
{
    uint gMeshletCount;
    uint gDrawMeshlets;  // 1 = color by meshlet, 0 = solid
};

StructuredBuffer<GpuMeshlet> gMeshlets;
StructuredBuffer<uint> gMeshletVertices;
StructuredBuffer<uint> gMeshletTriangles;

// Vertex output for lighting (match original MeshletRender)
struct MeshVertexOut
{
    float4 posH : SV_Position;
    float3 positionVS : POSITION0;
    float3 normal : NORMAL0;
    nointerpolation uint meshletIndex : COLOR0;
};

// Mesh shader limits (meshopt defaults)
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
    uint meshletID = groupID.x;
    uint numVertices = 0;
    uint numPrimitives = 0;
    GpuMeshlet meshlet;
    GeometryInstanceID geomInstanceID;
    float4x4 worldMat;
    float4x4 worldViewProj;
    uint vbOffset = 0;

    if (meshletID < gMeshletCount)
    {
        meshlet = gMeshlets[meshletID];
        geomInstanceID.index = meshlet.instanceID;

        worldMat = gScene.getWorldMatrix(geomInstanceID);
        float4x4 viewProj = gScene.camera.getViewProj();
        worldViewProj = mul(viewProj, worldMat);

        GeometryInstanceData instance = gScene.getGeometryInstance(geomInstanceID);
        vbOffset = instance.vbOffset;

        numVertices = meshlet.vertexCount;
        numPrimitives = meshlet.triangleCount;
    }

    SetMeshOutputCounts(numVertices, numPrimitives);

    // Output vertices
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

    // Output triangles
    if (tig < numPrimitives)
    {
        uint triOffset = meshlet.triangleOffset + tig * 3;
        uint idx0 = gMeshletTriangles[triOffset + 0];
        uint idx1 = gMeshletTriangles[triOffset + 1];
        uint idx2 = gMeshletTriangles[triOffset + 2];

        triangles[tig] = uint3(idx0, idx1, idx2);
    }
}

float4 psMain(MeshVertexOut input) : SV_TARGET
{
    float ambientIntensity = 0.1;
    float3 lightColor = float3(1, 1, 1);
    float3 lightDir = -normalize(float3(1, -1, 1));

    float3 diffuseColor;
    float shininess;
    if (gDrawMeshlets)
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
