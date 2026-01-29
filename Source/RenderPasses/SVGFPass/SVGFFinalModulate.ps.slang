import SVGFCommon;

cbuffer PerImageCB
{
    Texture2D gAlbedo;
    Texture2D gEmission;
    Texture2D gIllumination;
};

float4 main(FullScreenPassVsOut vsOut) : SV_TARGET0
{
    const int2 ipos = int2(vsOut.posH.xy);

    return gAlbedo[ipos] * gIllumination[ipos] + gEmission[ipos];
}
