// D3D12HDR - Rec.2020 color space triangles

import Color;

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

PSInput VSMain(float3 position : POSITION, float2 uv : TEXCOORD)
{
    PSInput result;
    result.position = float4(position, 1.0);
    result.uv = uv;
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(xyYToRec2020(input.uv), 1.0);
}
