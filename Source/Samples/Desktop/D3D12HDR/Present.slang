// D3D12HDR - Present pass: composite scene + UI, apply display curve

import Color;

#define DISPLAY_CURVE_SRGB 0
#define DISPLAY_CURVE_ST2084 1
#define DISPLAY_CURVE_LINEAR 2

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

cbuffer RootConstants : register(b0)
{
    float standardNits;  // Reference brightness (paper white)
    uint displayCurve;   // sRGB / ST.2084 / Linear
};

Texture2D g_scene : register(t0);
SamplerState g_sampler : register(s0);

PSInput VSMain(float3 position : POSITION, float2 uv : TEXCOORD)
{
    PSInput result;
    result.position = float4(position, 1.0);
    result.uv = uv;
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    // Scene: linear gamma, Rec.709 primaries (ImGui drawn by framework after)
    float3 result = g_scene.Sample(g_sampler, input.uv).rgb;

    if (displayCurve == DISPLAY_CURVE_SRGB)
    {
        result = LinearToSRGB(result);
    }
    else if (displayCurve == DISPLAY_CURVE_ST2084)
    {
        const float st2084max = 10000.0;
        float hdrScalar = standardNits / st2084max;
        result = Rec709ToRec2020(result);
        result = LinearToST2084(result * hdrScalar);
    }
    // else: DISPLAY_CURVE_LINEAR - pass through

    return float4(result, 1.0);
}
