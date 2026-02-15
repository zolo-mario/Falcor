// D3D12 Hello Bundles - 1:1 mapping from shaders.hlsl (same as HelloTriangle)
// VS: pass-through position and color
// PS: output interpolated color
// Note: Original uses ExecuteBundle; Falcor maps to direct draw() - same visual output

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(float3 position : POSITION, float4 color : COLOR)
{
    PSInput result;
    result.position = float4(position, 1.0);
    result.color = color;
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
