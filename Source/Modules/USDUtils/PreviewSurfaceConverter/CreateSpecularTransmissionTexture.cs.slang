RWTexture2D<float4> outputTexture;

Texture2D<float4> opacityTexture;

cbuffer CB
{
    int opacityChannel;
    uint2 outDim;
};

[numthreads(16, 16, 1)]
void createSpecularTransmissionTexture(uint3 threadId: SV_DispatchThreadID)
{
    if (any(threadId.xy > outDim))
        return;
    float oVal = opacityTexture[threadId.xy][opacityChannel];

    float val = saturate(1.f - oVal);
    outputTexture[threadId.xy] = float4(val, val, val, 1.);
}
