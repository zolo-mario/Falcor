import Utils.Math.PackedFormats;

StructuredBuffer<float3> testData;
RWStructuredBuffer<float3> result;

[numthreads(256, 1, 1)]
void testLogLuvHDR(uint3 threadId: SV_DispatchThreadID)
{
    const uint idx = threadId.x;

    float3 color = testData[idx];

    uint packed = encodeLogLuvHDR(color);
    result[idx] = decodeLogLuvHDR(packed);
}
