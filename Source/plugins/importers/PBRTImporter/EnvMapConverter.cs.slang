import Utils.Math.MathHelpers;

/**
 * Converts env map from equal-area octahedral mapping to lat-long mapping.
 */
struct EnvMapConverter
{
    Texture2D<float4> src;
    SamplerState srcSampler;

    RWTexture2D<float4> dst;

    uint2 dstDim;

    void execute(uint2 threadID)
    {
        const uint2 pixel = threadID;
        if (any(pixel >= dstDim))
            return;

        const float2 latlong = (float2(pixel) + 0.5f) / dstDim;
        const float3 dir = latlong_map_to_world(latlong);
        const float2 oct = ndir_to_oct_equal_area_unorm(dir);

        const float3 L = src.SampleLevel(srcSampler, oct, 0).rgb;
        dst[pixel].rgb = L;
    }
};

ParameterBlock<EnvMapConverter> gEnvMapConverter;

[numthreads(16, 16, 1)]
void main(uint2 threadID: SV_DispatchThreadID)
{
    gEnvMapConverter.execute(threadID);
}
