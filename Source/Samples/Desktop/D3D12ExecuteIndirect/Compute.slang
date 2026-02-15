// D3D12 ExecuteIndirect - 1:1 mapping from compute.hlsl
// Culls triangles outside horizontal bounds, outputs draw args for visible ones

#define threadBlockSize 128

struct SceneConstantBuffer
{
    float4 velocity;
    float4 offset;
    float4 color;
    float4x4 projection;
    float4 padding[9];
};

struct DrawIndirectCommand
{
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
};

cbuffer RootConstants : register(b0)
{
    float xOffset;     // Half the width of the triangles
    float zOffset;     // The z offset for the triangle vertices
    float cullOffset;  // The culling plane offset in homogenous space
    float commandCount; // The number of commands to be processed
};

StructuredBuffer<SceneConstantBuffer> cbv : register(t0);           // SRV: Constant buffers
AppendStructuredBuffer<DrawIndirectCommand> outputCommands : register(u0); // UAV: Processed (visible) commands

[shader("compute")]
[numthreads(threadBlockSize, 1, 1)]
void CSMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    uint index = (groupId.x * threadBlockSize) + groupIndex;

    if (index < (uint)commandCount)
    {
        // Project the left and right bounds of the triangle into homogenous space
        float4 left = float4(-xOffset, 0.0f, zOffset, 1.0f) + cbv[index].offset;
        left = mul(left, cbv[index].projection);
        left /= left.w;

        float4 right = float4(xOffset, 0.0f, zOffset, 1.0f) + cbv[index].offset;
        right = mul(right, cbv[index].projection);
        right /= right.w;

        // Only draw triangles that are within the culling space.
        // Output (3, 1, 0, index): firstInstance = index for VS to fetch cbv[index]
        if (-cullOffset < right.x && left.x < cullOffset)
        {
            DrawIndirectCommand cmd;
            cmd.vertexCount = 3;
            cmd.instanceCount = 1;
            cmd.firstVertex = 0;
            cmd.firstInstance = index;
            outputCommands.Append(cmd);
        }
    }
}
