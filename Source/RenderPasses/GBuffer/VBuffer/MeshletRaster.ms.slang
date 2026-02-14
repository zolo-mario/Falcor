/**
 * Mesh shader rasterization for VBuffer output.
 * Renders meshlets with mesh + pixel shader, outputting PackedHitInfo (VBuffer format).
 * Multi-instance support: meshlets are expanded per-instance in SceneMeshletData.
 * No culling. Requires ShaderModel 6.5, mesh shader support.
 */

import Scene.Scene;
import Utils.Math.MathHelpers;

// GPU meshlet structure (must match C++ GpuMeshlet)
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
    // uint2 gFrameDim;
    uint gMeshletCount;
};

StructuredBuffer<GpuMeshlet> gMeshlets;
StructuredBuffer<uint> gMeshletVertices;
StructuredBuffer<uint> gMeshletTriangles;

// Vertex output (minimal for PS - instanceID comes from MeshPrimitiveOut to avoid INSTANCE_ID overlap)
struct MeshVertexOut
{
    float2 texC : TEXCRD;
    nointerpolation uint materialID : MATERIAL_ID;
    float4 posH : SV_Position;
};

// Per-primitive output for VBuffer (instanceID, primitiveIndex)
struct MeshPrimitiveOut
{
    nointerpolation GeometryInstanceID instanceID : INSTANCE_ID;
    nointerpolation uint primitiveIndex : PRIMITIVE_ID;
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
    OutputIndices<uint3, MAX_PRIMS> triangles,
    OutputPrimitives<MeshPrimitiveOut, MAX_PRIMS> primitives)
{
    uint meshletID = groupID.x;
    uint numVertices = 0;
    uint numPrimitives = 0;
    GpuMeshlet meshlet;
    GeometryInstanceID geomInstanceID;
    float4x4 worldViewProj;
    uint vbOffset = 0;
    uint materialID = 0;

    if (meshletID < gMeshletCount)
    {
        meshlet = gMeshlets[meshletID];
        geomInstanceID.index = meshlet.instanceID;

        float4x4 worldMat = gScene.getWorldMatrix(geomInstanceID);
        float4x4 viewProj = gScene.camera.getViewProj();
        worldViewProj = mul(viewProj, worldMat);

        GeometryInstanceData instance = gScene.getGeometryInstance(geomInstanceID);
        vbOffset = instance.vbOffset;
        materialID = gScene.getMaterialID(geomInstanceID);

        numVertices = meshlet.vertexCount;
        numPrimitives = meshlet.triangleCount;
    }

    // SetMeshOutputCounts must be called exactly once per thread group
    SetMeshOutputCounts(numVertices, numPrimitives);

    // Output vertices
    if (tig < numVertices)
    {
        uint localVtxIdx = tig;
        uint globalVtxIdx = vbOffset + gMeshletVertices[meshlet.vertexOffset + localVtxIdx];
        StaticVertexData vtx = gScene.getVertex(globalVtxIdx);

        float4 clipPos = mul(worldViewProj, float4(vtx.position, 1.0));

        MeshVertexOut outVtx;
        outVtx.posH = clipPos;
        outVtx.texC = vtx.texCrd;
        outVtx.materialID = materialID;
        verts[tig] = outVtx;
    }

    // Output triangles and per-primitive data
    if (tig < numPrimitives)
    {
        uint triOffset = meshlet.triangleOffset + tig * 3;
        uint idx0 = gMeshletTriangles[triOffset + 0];
        uint idx1 = gMeshletTriangles[triOffset + 1];
        uint idx2 = gMeshletTriangles[triOffset + 2];

        triangles[tig] = uint3(idx0, idx1, idx2);

        MeshPrimitiveOut outPrim;
        outPrim.instanceID = geomInstanceID;
        outPrim.primitiveIndex = meshlet.primitiveOffset + tig;
        primitives[tig] = outPrim;
    }
}

struct VBufferPSOut
{
    PackedHitInfo packedHitInfo : SV_TARGET0;
};

VBufferPSOut psMain(
    MeshVertexOut vsOut,
    MeshPrimitiveOut primOut,
    float3 barycentrics : SV_Barycentrics)
{
    VBufferPSOut psOut;

    TriangleHit triangleHit;
    triangleHit.instanceID = primOut.instanceID;
    triangleHit.primitiveIndex = primOut.primitiveIndex;
    triangleHit.barycentrics = barycentrics.yz;
    psOut.packedHitInfo = triangleHit.pack();

    return psOut;
}
