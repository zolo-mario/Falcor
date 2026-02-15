// D3D12 Hello Texture - 1:1 mapping from shaders.hlsl
// VS: pass-through position and UV
// PS: sample texture

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D g_texture;
SamplerState g_sampler;

PSInput VSMain(float3 position : POSITION, float2 uv : TEXCOORD)
{
    PSInput result;
    result.position = float4(position, 1.0);
    result.uv = uv;
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return g_texture.Sample(g_sampler, input.uv);
}
