import Utils.Sampling.LowDiscrepancy.HammersleySequence;

RWStructuredBuffer<float> result;

cbuffer TestCB
{
    int resultSize;
};

[numthreads(1, 1, 1)]
void testRadicalInverse()
{
    for (int i = 0; i < resultSize; ++i)
    {
        result[i] = radicalInverse(i);
    }
}
