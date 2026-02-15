// Match D3D12 Pipeline State Cache: SimpleVertexShader + SimplePixelShader for cube rendering

struct VSInput
{
    float4 position : POSITION;
    float3 color : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
};

cbuffer PerDraw : register(b1)
{
    float4x4 worldViewProjection;
};

PSInput VSMain(VSInput input)
{
    PSInput result;
    result.position = mul(input.position, worldViewProjection);
    result.color = input.color;
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(input.color, 1.0f);
}
