// Fullscreen triangle: present CPU ray-traced RGBA texture (same pattern as TinyRenderer).

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
