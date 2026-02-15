// Match D3D12 Bundles shader_mesh_alt_pixel.hlsl (alternate PSO with color filter)

struct VSInput
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
    float3 tangent  : TANGENT;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
};

cbuffer cb0 : register(b0)
{
    float4x4 g_mWorldViewProj;
}

PSInput VSMain(VSInput input)
{
    PSInput result;
    result.position = mul(float4(input.position, 1.0f), g_mWorldViewProj);
    result.uv = input.uv;
    return result;
}

Texture2D g_txDiffuse : register(t0);
SamplerState g_sampler : register(s0);

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 color = g_txDiffuse.Sample(g_sampler, input.uv).rgb;
    // Reduce R and B contributions - match D3D12 sample filter (0.25, 1.0, 0.25)
    float3 filter = float3(0.25f, 1.0f, 0.25f);
    return float4(color.xyz * filter, 1.0f);
}
