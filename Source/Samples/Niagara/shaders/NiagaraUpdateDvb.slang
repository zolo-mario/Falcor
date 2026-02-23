/**
 * Update draw visibility buffer from meshlet visibility buffer.
 * For each draw: dvb[drawId] = 1 if any meshlet in the draw's range has mvb bit set.
 */

struct MeshTaskCommand
{
    uint drawId;
    uint taskOffset;
    uint taskCount;
    uint meshletVisibilityOffset;
};

StructuredBuffer<MeshTaskCommand> gTaskCommands : register(t0);
StructuredBuffer<uint> gMeshletVisibility : register(t1);
RWStructuredBuffer<uint> gDrawVisibility : register(u0);

cbuffer CB : register(b0)
{
    uint gDrawCount;
};

[numthreads(64, 1, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    uint drawId = dtid.x;
    if (drawId >= gDrawCount)
        return;

    MeshTaskCommand cmd = gTaskCommands[drawId];
    uint mvbOffset = cmd.meshletVisibilityOffset;
    uint taskCount = cmd.taskCount;

    bool anyVisible = false;
    for (uint i = 0; i < taskCount && !anyVisible; ++i)
    {
        uint mvi = mvbOffset + i;
        uint bit = (gMeshletVisibility[mvi >> 5] >> (mvi & 31)) & 1u;
        if (bit != 0)
            anyVisible = true;
    }
    gDrawVisibility[drawId] = anyVisible ? 1u : 0u;
}
