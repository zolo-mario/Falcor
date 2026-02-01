import MeshletTypes;
import Scene.Raster;

// ============ Resource Bindings ============
StructuredBuffer<float3> gPositions;
StructuredBuffer<uint> gVisibleMeshletIDs;
StructuredBuffer<GpuMeshlet> gMeshlets;

cbuffer PerFrameCB
{
    float4x4 gViewProj;
    uint gFrameCount;
};

// ============ Visibility Buffer Output Mode ============
#ifndef VBUFFER_OUTPUT_MODE
#define VBUFFER_OUTPUT_MODE 0  // 0=Color, 1=VBuffer
#endif

// ============ VS -> PS Interpolation Data ============
struct VSOut
{
    float4 posH : SV_Position;
    nointerpolation uint meshletID : MESHLET_ID;
    nointerpolation uint localPrimitiveID : LOCAL_PRIM_ID;
};

// ============ Vertex Shader ============
VSOut vsMain(
    uint vertexID : SV_VertexID,
    uint instanceID : SV_InstanceID
)
{
    VSOut vout;

    // 1. Get original MeshletID
    uint originalMeshletID = gVisibleMeshletIDs[instanceID];

    // 2. Get Meshlet info to calculate Local Primitive ID
    GpuMeshlet meshlet = gMeshlets[originalMeshletID];

    // 3. Get vertex position from Position Buffer
    float3 posW = gPositions[vertexID];

    // 4. Transform to clip space
    vout.posH = mul(gViewProj, float4(posW, 1.0f));

    // 5. Pass ID
    vout.meshletID = originalMeshletID;

    // Calculate triangle index for current vertex within Meshlet
    // vertexID is in range [meshlet.indexStart, meshlet.indexStart + meshlet.indexCount)
    // localPrimitiveID = (vertexID - meshlet.indexStart) / 3
    // However since IB is per-triangle, SV_PrimitiveID is more accurate
    vout.localPrimitiveID = 0; // Will be obtained via SV_PrimitiveID in PS

    return vout;
}

// ============ Pixel Shader Output ============
struct PSOut
{
#if VBUFFER_OUTPUT_MODE == 1
    uint vbuffer : SV_Target0;   // R32_UINT Visibility Buffer
#else
    float4 color : SV_Target0;   // RGBA color output (for debugging)
#endif
    float depth : SV_Depth;      // Depth output (optional)
};

// ============ Pixel Shader ============
[earlydepthstencil]
PSOut psMain(
    VSOut vsIn,
    uint primitiveID : SV_PrimitiveID
)
{
    PSOut psOut;

    // primitiveID is triangle index relative to current Draw Call
    // Since each Indirect Draw corresponds to one Meshlet, this is the Local Primitive ID
    uint localPrimID = primitiveID;
    uint meshletID = vsIn.meshletID;

#if VBUFFER_OUTPUT_MODE == 1
    // Visibility Buffer mode: packed output
    psOut.vbuffer = packVisibilityBuffer(meshletID, localPrimID);
#else
    // Color mode: visualize Meshlet or Triangle
    #ifdef VISUALIZE_TRIANGLES
        psOut.color = float4(triangleIdToColor(meshletID, localPrimID), 1.0f);
    #else
        psOut.color = float4(meshletIdToColor(meshletID), 1.0f);
    #endif
#endif

    psOut.depth = vsIn.posH.z / vsIn.posH.w;

    return psOut;
}
