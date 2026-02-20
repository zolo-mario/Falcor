// Build draw args (3, visibleCount, 0, 0) for single drawInstanced from visible indices counter
// Counter buffer has UnorderedAccess only (no ShaderResource), so bind as UAV

RWByteAddressBuffer counter : register(u0);  // UAV counter (4 bytes)
RWStructuredBuffer<uint> drawArgs : register(u1); // Output: vertexCount=3, instanceCount, firstVertex=0, firstInstance=0

[shader("compute")]
[numthreads(1, 1, 1)]
void CSMain()
{
    uint count = counter.Load(0);
    drawArgs[0] = 3;       // vertexCount
    drawArgs[1] = count;   // instanceCount
    drawArgs[2] = 0;       // firstVertex
    drawArgs[3] = 0;       // firstInstance
}
