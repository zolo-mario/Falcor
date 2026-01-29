// Sample the given texture over a footprint corresponding to the given output pixel
float4 sampleTexture(Texture2D<float4> tex, SamplerState sampler, int2 pos, int2 outDim)
{
    uint width;
    uint height;
    tex.GetDimensions(width, height);

    float2 samplePos = (float2(pos) + float2(0.5)) / float2(outDim);

    // Scale factor between destination pixels and source pixels
    float2 pixelScale = float2((float)width / (float)outDim.x, (float)height / (float)outDim.y);

    float ddX = pixelScale.x / outDim.x;
    float ddY = pixelScale.y / outDim.y;

    return tex.SampleGrad(sampler, samplePos, ddX, ddY);
}
