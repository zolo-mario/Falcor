import SampleTexture;

RWTexture2D<float4> outputTexture;

SamplerState sampler;
Texture2D<float4> baseColorTexture;
Texture2D<float4> opacityTexture;

cbuffer CB
{
    int opacityChannel;
    float opacityValue;
    int baseColorTextureValid;
    float4 baseColorValue;
    uint2 outDim;
};

[numthreads(16, 16, 1)]
void packBaseColorAlpha(uint3 threadId: SV_DispatchThreadID)
{
    if (any(threadId.xy > outDim))
        return;

    float opacity;
    if (opacityChannel < 0)
    {
        opacity = opacityValue;
    }
    else
    {
        opacity = sampleTexture(opacityTexture, sampler, threadId.xy, outDim)[opacityChannel];
    }

    float4 baseColor;
    if (!baseColorTextureValid)
    {
        baseColor = baseColorValue;
    }
    else
    {
        baseColor = sampleTexture(baseColorTexture, sampler, threadId.xy, outDim);
    }

    outputTexture[threadId.xy] = float4(baseColor.rgb, opacity);
}
