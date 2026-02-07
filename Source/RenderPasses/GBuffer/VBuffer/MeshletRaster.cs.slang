/***************************************************************************
 # Copyright (c) 2015-23, NVIDIA CORPORATION. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions
 # are met:
 #  * Redistributions of source code must retain the above copyright
 #    notice, this list of conditions and the following disclaimer.
 #  * Redistributions in binary form must reproduce the above copyright
 #    notice, this list of conditions and the following disclaimer in the
 #    documentation and/or other materials provided with the distribution.
 #  * Neither the name of NVIDIA CORPORATION nor the names of its
 #    contributors may be used to endorse or promote products derived
 #    from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY
 # EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 # PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 # PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 # PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 # OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/
#include "Utils/Math/MathConstants.slangh"

import Scene.Scene;

// GPU meshlet structure (must match C++ definition)
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

// Constant buffer
cbuffer CB
{
    uint2 gFrameDim;
    uint gMeshletCount;
};

// Input buffers
StructuredBuffer<GpuMeshlet> gMeshlets;
StructuredBuffer<uint> gMeshletVertices;
StructuredBuffer<uint> gMeshletTriangles;

// Output: Visibility buffers (separate depth and payload)
RWTexture2D<uint> gVisBufferDepth;
RWTexture2D<uint> gVisBufferPayload;

// Helper functions
float cross2D(float2 a, float2 b)
{
    return a.x * b.y - a.y * b.x;
}

bool isInsideTriangle(float2 p, float2 v0, float2 v1, float2 v2)
{
    float2 e0 = v1 - v0;
    float2 e1 = v2 - v1;
    float2 e2 = v0 - v2;

    float c0 = cross2D(e0, p - v0);
    float c1 = cross2D(e1, p - v1);
    float c2 = cross2D(e2, p - v2);

    return (c0 >= 0 && c1 >= 0 && c2 >= 0) || (c0 <= 0 && c1 <= 0 && c2 <= 0);
}

float interpolateDepth(float2 p, float2 v0, float2 v1, float2 v2, float d0, float d1, float d2)
{
    // Compute barycentric coordinates
    float2 v0v1 = v1 - v0;
    float2 v0v2 = v2 - v0;
    float2 v0p = p - v0;

    float d00 = dot(v0v1, v0v1);
    float d01 = dot(v0v1, v0v2);
    float d11 = dot(v0v2, v0v2);
    float d20 = dot(v0p, v0v1);
    float d21 = dot(v0p, v0v2);

    float denom = d00 * d11 - d01 * d01;
    if (abs(denom) < 1e-8)
        return d0;

    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;

    return u * d0 + v * d1 + w * d2;
}

[numthreads(32, 1, 1)]
void main(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID)
{
    uint meshletID = groupID.x;
    uint threadID = groupThreadID.x;

    if (meshletID >= gMeshletCount)
        return;

    // Load meshlet data
    GpuMeshlet meshlet = gMeshlets[meshletID];

    // Get camera and transform matrices
    GeometryInstanceID geomInstanceID;
    geomInstanceID.index = meshlet.instanceID;

    // Get world matrix using Scene API
    float4x4 worldMat = gScene.getWorldMatrix(geomInstanceID);

    // Get camera view-projection matrix
    float4x4 viewProj = gScene.camera.getViewProj();
    float4x4 worldViewProj = mul(viewProj, worldMat);

    // Process all triangles in the meshlet
    for (uint triIdx = 0; triIdx < meshlet.triangleCount; triIdx++)
    {
        // Load triangle indices (packed as uint8)
        uint triOffset = meshlet.triangleOffset + triIdx * 3;
        uint idx0 = gMeshletTriangles[triOffset + 0];
        uint idx1 = gMeshletTriangles[triOffset + 1];
        uint idx2 = gMeshletTriangles[triOffset + 2];

        // Get vertex indices
        uint v0Idx = gMeshletVertices[meshlet.vertexOffset + idx0];
        uint v1Idx = gMeshletVertices[meshlet.vertexOffset + idx1];
        uint v2Idx = gMeshletVertices[meshlet.vertexOffset + idx2];

        // Load vertex positions from scene using Scene API
        float3 pos0 = gScene.getVertex(v0Idx).position;
        float3 pos1 = gScene.getVertex(v1Idx).position;
        float3 pos2 = gScene.getVertex(v2Idx).position;

        // Transform to clip space
        float4 clipPos0 = mul(worldViewProj, float4(pos0, 1.0));
        float4 clipPos1 = mul(worldViewProj, float4(pos1, 1.0));
        float4 clipPos2 = mul(worldViewProj, float4(pos2, 1.0));

        // Perspective divide and convert to screen space
        float3 ndc0 = clipPos0.xyz / clipPos0.w;
        float3 ndc1 = clipPos1.xyz / clipPos1.w;
        float3 ndc2 = clipPos2.xyz / clipPos2.w;

        float2 screenPos0 = float2((ndc0.x * 0.5 + 0.5) * gFrameDim.x, (1.0 - (ndc0.y * 0.5 + 0.5)) * gFrameDim.y);
        float2 screenPos1 = float2((ndc1.x * 0.5 + 0.5) * gFrameDim.x, (1.0 - (ndc1.y * 0.5 + 0.5)) * gFrameDim.y);
        float2 screenPos2 = float2((ndc2.x * 0.5 + 0.5) * gFrameDim.x, (1.0 - (ndc2.y * 0.5 + 0.5)) * gFrameDim.y);

        // Compute bounding box
        int2 bbMin = int2(
            floor(min(min(screenPos0.x, screenPos1.x), screenPos2.x)),
            floor(min(min(screenPos0.y, screenPos1.y), screenPos2.y))
        );
        int2 bbMax = int2(
            ceil(max(max(screenPos0.x, screenPos1.x), screenPos2.x)),
            ceil(max(max(screenPos0.y, screenPos1.y), screenPos2.y))
        );

        // Clamp to screen bounds
        bbMin = max(bbMin, int2(0, 0));
        bbMax = min(bbMax, int2(gFrameDim) - int2(1, 1));

        // Rasterize: each thread processes subset of pixels
        for (int y = bbMin.y; y <= bbMax.y; y++)
        {
            for (int x = bbMin.x + threadID; x <= bbMax.x; x += 32)
            {
                float2 pixelCenter = float2(x, y) + float2(0.5, 0.5);

                // Edge function coverage test
                if (!isInsideTriangle(pixelCenter, screenPos0, screenPos1, screenPos2))
                    continue;

                // Interpolate depth
                float depth = interpolateDepth(pixelCenter, screenPos0, screenPos1, screenPos2, ndc0.z, ndc1.z, ndc2.z);

                // Pack payload: (instanceID << 20) | primitiveID
                uint primitiveID = meshlet.primitiveOffset + triIdx;
                uint payload = (meshlet.instanceID << 20) | (primitiveID & 0xFFFFF);

                // Convert depth to uint for atomic comparison
                uint depthUint = asuint(depth);

                // Atomic depth test and write - use Min to keep nearest depth
                InterlockedMin(gVisBufferDepth[uint2(x, y)], depthUint);

                // Always write payload (not perfectly atomic, but simpler)
                gVisBufferPayload[uint2(x, y)] = payload;
            }
        }
    }
}

