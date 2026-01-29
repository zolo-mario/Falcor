import SampleTexture;

RWTexture2D<float4> outputTexture;

SamplerState sampler;
Texture2D<float4> metallicTexture;
Texture2D<float4> roughnessTexture;

cbuffer CB
{
    int roughnessChannel;
    float roughnessValue;
    int metallicChannel;
    float metallicValue;
    uint2 outDim;
};

[numthreads(16, 16, 1)]
void createSpecularTexture(uint3 threadId: SV_DispatchThreadID)
{
    if (any(threadId.xy > outDim))
        return;

    float metallic;
    if (metallicChannel < 0)
    {
        metallic = metallicValue;
    }
    else
    {
        metallic = sampleTexture(metallicTexture, sampler, threadId.xy, outDim)[metallicChannel];
    }

    float roughness;
    if (roughnessChannel < 0)
    {
        roughness = roughnessValue;
    }
    else
    {
        roughness = sampleTexture(roughnessTexture, sampler, threadId.xy, outDim)[roughnessChannel];
    }

    outputTexture[threadId.xy] = float4(0., saturate(roughness), saturate(metallic), 1.);
}
