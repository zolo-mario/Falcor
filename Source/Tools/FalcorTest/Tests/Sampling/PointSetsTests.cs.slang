import Internal.Utils.Sampling.LowDiscrepancy.PointSets;

RWStructuredBuffer<float2> result;

cbuffer CB
{
    uint m;
};

[numthreads(256, 1, 1)]
void testPermutedTms(uint3 threadId: SV_DispatchThreadID)
{
    const uint i = threadId.x;
    result[i] = generatePermutedTms(i, m);
}
