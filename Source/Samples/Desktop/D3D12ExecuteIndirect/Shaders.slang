// D3D12 ExecuteIndirect - 1:1 mapping from shaders.hlsl
// VS uses StructuredBuffer indexed by instance ID (firstInstance from draw indirect)

struct SceneConstantBuffer
{
    float4 velocity;
    float4 offset;
    float4 color;
    float4x4 projection;
};

StructuredBuffer<SceneConstantBuffer> gSceneCB : register(t0);

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(float3 position : POSITION, uint instanceId : SV_InstanceID)
{
    PSInput result;
    SceneConstantBuffer cb = gSceneCB[instanceId];
    result.position = mul(float4(position + cb.offset.xyz, 1.0f), cb.projection);

    float intensity = saturate((4.0f - result.position.z) / 2.0f);
    result.color = float4(cb.color.xyz * intensity, 1.0f);

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
