/**
 * Niagara depth pyramid - 2x2 min reduction.
 * D3D12 has no VK_SAMPLER_REDUCTION_MODE_MIN, so we manually sample 4 texels.
 * Used for level 0 (depth->L0) and L(i-1)->L(i).
 */

Texture2D<float> gInputDepth : register(t0);
RWTexture2D<float> gOutputPyramid : register(u0);

cbuffer CB : register(b0)
{
    uint2 gInputSize;
    uint2 gOutputSize;
};

[numthreads(8, 8, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    uint2 pos = dtid.xy;
    if (pos.x >= gOutputSize.x || pos.y >= gOutputSize.y)
        return;

    uint2 src0 = pos * 2;
    uint2 src1 = min(src0 + uint2(1, 0), gInputSize - 1);
    uint2 src2 = min(src0 + uint2(0, 1), gInputSize - 1);
    uint2 src3 = min(src0 + uint2(1, 1), gInputSize - 1);

    float d00 = gInputDepth[src0];
    float d10 = gInputDepth[src1];
    float d01 = gInputDepth[src2];
    float d11 = gInputDepth[src3];

    float depth = min(min(d00, d10), min(d01, d11));
    gOutputPyramid[pos] = depth;
}
