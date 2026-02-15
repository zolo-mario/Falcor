// D3D12 Hello Constant Buffers - 1:1 mapping from shaders.hlsl
// VS: position + offset (animated), PS: output interpolated color

cbuffer SceneConstantBuffer : register(b0)
{
    float4 offset;
    float4 padding[15];
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(float3 position : POSITION, float4 color : COLOR)
{
    PSInput result;
    result.position = float4(position, 1.0) + offset;
    result.color = color;
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
