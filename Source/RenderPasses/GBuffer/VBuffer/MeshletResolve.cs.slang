#include "Utils/Math/MathConstants.slangh"

import Scene.Scene;
import Scene.HitInfo;
import Utils.Math.MathHelpers;

// Define is_valid flags for optional outputs
#define is_valid_gViewW 1
#define is_valid_gDepth 1
#define is_valid_gMotionVector 1
#define is_valid(name) (is_valid_##name != 0)

// Constant buffer
cbuffer CB
{
    uint2 gFrameDim;
};

// Input: Visibility buffers (separate depth and payload)
Texture2D<uint> gVisBufferDepth;
Texture2D<uint> gVisBufferPayload;

// Outputs
RWTexture2D<PackedHitInfo> gVBuffer;
RWTexture2D<float4> gViewW;
RWTexture2D<float> gDepth;
RWTexture2D<float2> gMotionVector;

// Ray-triangle intersection using Möller-Trumbore algorithm
float2 rayTriangleIntersection(float3 rayOrigin, float3 rayDir, float3 v0, float3 v1, float3 v2)
{
    float3 edge1 = v1 - v0;
    float3 edge2 = v2 - v0;
    float3 h = cross(rayDir, edge2);
    float a = dot(edge1, h);

    if (abs(a) < 1e-8)
        return float2(0, 0);

    float f = 1.0 / a;
    float3 s = rayOrigin - v0;
    float u = f * dot(s, h);

    float3 q = cross(s, edge1);
    float v = f * dot(rayDir, q);

    return float2(u, v);
}

// Calculate motion vector
float2 calcMotionVector(float2 pixelPos, float4 prevPosH, uint2 frameDim)
{
    float2 prevPixelPos = prevPosH.xy / prevPosH.w;
    prevPixelPos = (prevPixelPos * float2(0.5, -0.5) + 0.5) * frameDim;
    return prevPixelPos - pixelPos;
}

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 pixel = dispatchThreadID.xy;

    if (any(pixel >= gFrameDim))
        return;

    // Read visibility buffers
    uint depthUint = gVisBufferDepth[pixel];
    uint payload = gVisBufferPayload[pixel];

    // Check if background
    if (payload == 0 && depthUint == 0)
    {
        // Background
        gVBuffer[pixel] = 0;
        if (is_valid(gViewW))
        {
            float2 pixelCenter = float2(pixel) + float2(0.5, 0.5);
            Ray ray = gScene.camera.computeRayPinhole(pixel, gFrameDim);
            gViewW[pixel] = float4(ray.dir, 0.0);
        }
        if (is_valid(gDepth))
            gDepth[pixel] = 1.0f;
        if (is_valid(gMotionVector))
            gMotionVector[pixel] = float2(0, 0);
        return;
    }

    // Decode payload
    uint instanceID = payload >> 20;
    uint primitiveID = payload & 0xFFFFF;
    float depth = asfloat(depthUint);

    // Get geometry instance
    GeometryInstanceID geomInstanceID;
    geomInstanceID.index = instanceID;

    // Load triangle vertices from scene
    uint3 vtxIndices = gScene.getIndices(geomInstanceID, primitiveID);

    StaticVertexData vtx0 = gScene.getVertex(vtxIndices[0]);
    StaticVertexData vtx1 = gScene.getVertex(vtxIndices[1]);
    StaticVertexData vtx2 = gScene.getVertex(vtxIndices[2]);

    float3 positions[3];
    positions[0] = vtx0.position;
    positions[1] = vtx1.position;
    positions[2] = vtx2.position;

    // Transform to world space
    float4x4 worldMat = gScene.getWorldMatrix(geomInstanceID);
    positions[0] = mul(worldMat, float4(positions[0], 1.0)).xyz;
    positions[1] = mul(worldMat, float4(positions[1], 1.0)).xyz;
    positions[2] = mul(worldMat, float4(positions[2], 1.0)).xyz;

    // Reconstruct ray from pixel
    float2 pixelCenter = float2(pixel) + float2(0.5, 0.5);
    Ray ray = gScene.camera.computeRayPinhole(pixel, gFrameDim);

    // Compute barycentrics via ray-triangle intersection
    float2 barycentrics = rayTriangleIntersection(ray.origin, ray.dir, positions[0], positions[1], positions[2]);

    // Pack and write outputs
    TriangleHit triangleHit;
    triangleHit.instanceID = geomInstanceID;
    triangleHit.primitiveIndex = primitiveID;
    triangleHit.barycentrics = barycentrics;

    gVBuffer[pixel] = triangleHit.pack();

    // Write view direction
    if (is_valid(gViewW))
    {
        gViewW[pixel] = float4(ray.dir, 0.0);
    }

    // Write depth
    if (is_valid(gDepth))
    {
        gDepth[pixel] = depth;
    }

    // Compute motion vector
    if (is_valid(gMotionVector))
    {
        float3 prevPosW = gScene.getPrevPosW(triangleHit);
        float2 pixelPos = pixelCenter;
        float4 prevPosH = mul(gScene.camera.data.prevViewProjMatNoJitter, float4(prevPosW, 1.0));
        float2 motionVector = calcMotionVector(pixelPos, prevPosH, gFrameDim) +
                              float2(gScene.camera.data.jitterX, -gScene.camera.data.jitterY);
        gMotionVector[pixel] = motionVector;
    }
}

