// D3D12 ExecuteIndirect - Culling path: VS uses gVisibleIndices[instanceId] to index gSceneCB
// Single drawInstanced(3, visibleCount, 0, 0) with instanceId 0..visibleCount-1

struct SceneConstantBuffer
{
    float4 velocity;
    float4 offset;
    float4 color;
    float4x4 projection;
};

StructuredBuffer<SceneConstantBuffer> gSceneCB : register(t0);
StructuredBuffer<uint> gVisibleIndices : register(t1);

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(float3 position : POSITION, uint instanceId : SV_InstanceID)
{
    PSInput result;
    uint index = gVisibleIndices[instanceId];
    SceneConstantBuffer cb = gSceneCB[index];
    // D3D12 原版用 XMMatrixPerspectiveFovLH（+Z 朝向相机），Falcor math::perspective 为 RH（-Z 朝向相机），需取反 z
    float4 worldPos = float4(position + cb.offset.xyz, 1.0f);
    worldPos.z = -worldPos.z;
    result.position = mul(cb.projection, worldPos);

    float intensity = saturate((4.0f - result.position.z) / 2.0f);
    result.color = float4(cb.color.xyz * intensity, 1.0f);

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
