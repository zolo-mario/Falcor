// D3D12 Hello Frame Buffering - 1:1 mapping from shaders.hlsl
// Same triangle as HelloTriangle; frame buffering (fences, command allocators) abstracted by Falcor
// VS: pass-through position and color
// PS: output interpolated color

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
