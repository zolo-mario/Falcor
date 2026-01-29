import Utils.Color.SpectrumUtils;

cbuffer CB
{
    uint n;
};

StructuredBuffer<float> wavelengths;
RWStructuredBuffer<float3> result;

[numthreads(256, 1, 1)]
void testWavelengthToXYZ(uint3 threadId: SV_DispatchThreadID)
{
    uint i = threadId.x;
    if (i >= n)
        return;

    result[i] = SpectrumUtils.wavelengthToXYZ(wavelengths[i]);
}
