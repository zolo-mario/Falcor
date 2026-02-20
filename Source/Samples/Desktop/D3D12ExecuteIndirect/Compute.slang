// D3D12 ExecuteIndirect - 1:1 mapping from compute.hlsl
// Culls triangles outside horizontal bounds, outputs draw args for visible ones

#define threadBlockSize 128

struct SceneConstantBuffer
{
    float4 velocity;
    float4 offset;
    float4 color;
    float4x4 projection;
};

cbuffer RootConstants : register(b0)
{
    float xOffset;     // Half the width of the triangles
    float zOffset;     // The z offset for the triangle vertices
    float cullOffset;  // The culling plane offset in homogenous space
    float commandCount; // The number of commands to be processed
};

StructuredBuffer<SceneConstantBuffer> cbv : register(t0);           // SRV: Constant buffers
AppendStructuredBuffer<uint> outputVisibleIndices : register(u0);    // UAV: Visible triangle indices (for gSceneCB[index])

[shader("compute")]
[numthreads(threadBlockSize, 1, 1)]
void CSMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    uint index = (groupId.x * threadBlockSize) + groupIndex;

    if (index < (uint)commandCount)
    {
        // Project the left and right bounds of the triangle into homogenous space
        // 与 Shaders.slang 一致：z 取反以匹配 Falcor RH 投影
        float4 left = float4(-xOffset, 0.0f, -zOffset, 1.0f) + float4(cbv[index].offset.xy, -cbv[index].offset.z, cbv[index].offset.w);
        left = mul(cbv[index].projection, left);
        left /= left.w;

        float4 right = float4(xOffset, 0.0f, -zOffset, 1.0f) + float4(cbv[index].offset.xy, -cbv[index].offset.z, cbv[index].offset.w);
        right = mul(cbv[index].projection, right);
        right /= right.w;

        // Only draw triangles that are within the culling space.
        // Append index for VS to use gSceneCB[gVisibleIndices[instanceId]]
        if (-cullOffset < right.x && left.x < cullOffset)
        {
            outputVisibleIndices.Append(index);
        }
    }
}
