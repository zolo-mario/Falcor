// D3D12HDR - Gradient strip (SDR 0-1, HDR 0-9)

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(float3 position : POSITION, float3 color : COLOR)
{
    PSInput result;
    result.position = float4(position, 1.0);
    result.color = float4(color, 1.0);
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    // Perceptual gradient: pow to gamma 2.2 (match gradientPS.hlsl)
    return float4(pow(abs(input.color.rgb), 2.2), input.color.a);
}
