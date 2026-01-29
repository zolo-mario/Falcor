cbuffer GlobalCB
{
    uint gStride;
}

Texture2D<float4> gInTex;
RWBuffer<float4> gOutBuf;

[numthreads(8, 8, 1)]
void main(uint3 thrdId: SV_DispatchThreadID)
{
    uint bufIdx = thrdId.x + thrdId.y * gStride;
    gOutBuf[bufIdx] = gInTex[thrdId.xy];
}
