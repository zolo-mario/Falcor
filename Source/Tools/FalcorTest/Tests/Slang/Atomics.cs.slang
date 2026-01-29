import Utils.NVAPI; // TODO: This is currently needed for using FP atomics on D3D12. It's safe to import on Vulkan.

StructuredBuffer<float> data;
RWByteAddressBuffer resultBuf;
[format("r32f")]
RWTexture2D<float> resultTex;

[numthreads(256, 1, 1)]
void testBufferAddF16(uint3 threadID: SV_DispatchThreadID)
{
    uint x = asuint(data[threadID.x]);
    uint y = x ^ 0x80008000; // Flip signs

    // This should work on both D3D12 and Vulkan.
    // NOTE: The SPIR-V backend is required for RWByteAddressBuffer.InterlockedAddF16() on Vulkan!
    float16_t origVal;
    resultBuf.InterlockedAddF16(0, asfloat16((uint16_t)x), origVal);
    resultBuf.InterlockedAddF16(2, asfloat16((uint16_t)(x >> 16)), origVal);
    resultBuf.InterlockedAddF16(4, asfloat16((uint16_t)y), origVal);
    resultBuf.InterlockedAddF16(6, asfloat16((uint16_t)(y >> 16)), origVal);
}

[numthreads(256, 1, 1)]
void testBufferAddF16_2(uint3 threadID: SV_DispatchThreadID)
{
    uint x = asuint(data[threadID.x]);
    uint y = x ^ 0x80008000; // Flip signs

    // This should work on both D3D12 and Vulkan.
    // NOTE: The SPIR-V backend is required for RWByteAddressBuffer._NvInterlockedAddFp16x2() on Vulkan!
    resultBuf._NvInterlockedAddFp16x2(0, x);
    resultBuf._NvInterlockedAddFp16x2(4, y);
}

[numthreads(256, 1, 1)]
void testBufferAddF32(uint3 threadID: SV_DispatchThreadID)
{
    float x = data[threadID.x];

    // This should work on both D3D12 and Vulkan.
    resultBuf.InterlockedAddF32(0, x);
    resultBuf.InterlockedAddF32(4, -x);
}

[numthreads(256, 1, 1)]
void testTextureAddF32(uint3 threadID: SV_DispatchThreadID)
{
    float x = data[threadID.x];

    // This should work on both D3D12 and Vulkan.
    // NOTE: The SPIR-V backend is required for RWTexture2D.InterlockedAddF32() on Vulkan!
    resultTex.InterlockedAddF32(uint2(0, 0), x);
    resultTex.InterlockedAddF32(uint2(1, 0), -x);
}

[numthreads(256, 1, 1)]
void copyResult(uint3 threadID: SV_DispatchThreadID)
{
    uint i = threadID.x;
    if (i < 2)
    {
        float value = resultTex[uint2(i, 0)];
        resultBuf.Store(i * 4, value);
    }
}
