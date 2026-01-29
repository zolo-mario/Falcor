import Utils.Debug.WarpProfiler;

[numthreads(16, 16, 1)]
void main(uint3 threadId: SV_DispatchThreadID)
{
    // Utilization
    warpUtilization(0);

    if (all(threadId % 2 == 0))
    {
        warpUtilization(1);
    }

    // Divergence
    uint value = threadId.x & 3;
    warpDivergence(2, value);

    if ((threadId.y & 1) == 1 && (threadId.z & 3) == 0)
    {
        uint value = threadId.x & 7;
        warpDivergence(3, value);
    }
}
