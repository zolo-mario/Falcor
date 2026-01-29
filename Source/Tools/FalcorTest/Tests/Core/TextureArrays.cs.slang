#define I (1 << BITS_I)
#define J (1 << BITS_J)
#define K (1 << BITS_K)

RWStructuredBuffer<float> result;
RWTexture2D<float> tex[I][J][K];

[numthreads(16, 16, 1)]
void testWrite(uint3 threadId: SV_DispatchThreadID)
{
    if (threadId.x >= 16 || threadId.y >= 16 || threadId.z >= 8)
        return;

    uint i = threadId.z & ((I)-1);
    uint j = (threadId.z >> (BITS_I)) & ((J)-1);
    uint k = (threadId.z >> ((BITS_I) + (BITS_J))) & ((K)-1);
    uint2 p = threadId.xy;

    tex[i][j][k][p] = (float)threadId.x * threadId.y + threadId.z;
}

[numthreads(16, 16, 1)]
void testRead(uint3 threadId: SV_DispatchThreadID)
{
    if (threadId.x >= 16 || threadId.y >= 16 || threadId.z >= 8)
        return;

    uint i = threadId.z & ((I)-1);
    uint j = (threadId.z >> (BITS_I)) & ((J)-1);
    uint k = (threadId.z >> ((BITS_I) + (BITS_J))) & ((K)-1);
    uint2 p = threadId.xy;

    uint idx = threadId.z * 256 + threadId.y * 16 + threadId.x;
    result[idx] = tex[i][j][k][p];
}
