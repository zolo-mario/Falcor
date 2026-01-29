#define NRD_COMPILER_DXC
#include "nrd/Shaders/Include/NRD.hlsli"

import Utils.Color.ColorHelpers;

#ifndef NRD_METHOD
// NRD_METHOD must be defined in order to use this module.
#error "NRD_METHOD not defined!"
#endif

#define NRD_METHOD_RELAX_DIFFUSE_SPECULAR 0
#define NRD_METHOD_REBLUR_DIFFUSE_SPECULAR 1

cbuffer PerImageCB
{
    float4 gHitDistParams;
    float gMaxIntensity;

    Texture2D<float> gViewZ;
    Texture2D<float4> gNormalRoughness;

    RWTexture2D<float4> gDiffuseRadianceHitDist;
    RWTexture2D<float4> gSpecularRadianceHitDist;
};

void clampRadiance(inout float3 diffuseRadiance, inout float3 specularRadiance)
{
    static const float kEpsilon = 1e-6f;

    float lDiff = luminance(diffuseRadiance);
    if (lDiff > kEpsilon)
    {
        diffuseRadiance *= min(gMaxIntensity / lDiff, 1.f);
    }

    float lSpec = luminance(specularRadiance);
    if (lSpec > kEpsilon)
    {
        specularRadiance *= min(gMaxIntensity / lSpec, 1.f);
    }
}

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadId: SV_DispatchThreadID)
{
    int2 ipos = dispatchThreadId.xy;

    float4 diffuseRadianceHitDist = gDiffuseRadianceHitDist[ipos];
    float4 specularRadianceHitDist = gSpecularRadianceHitDist[ipos];

    clampRadiance(diffuseRadianceHitDist.rgb, specularRadianceHitDist.rgb);

#if NRD_METHOD == NRD_METHOD_RELAX_DIFFUSE_SPECULAR

    diffuseRadianceHitDist = RELAX_FrontEnd_PackRadianceAndHitDist(diffuseRadianceHitDist.rgb, diffuseRadianceHitDist.a);
    specularRadianceHitDist = RELAX_FrontEnd_PackRadianceAndHitDist(specularRadianceHitDist.rgb, specularRadianceHitDist.a);

#else // NRD_METHOD == NRD_METHOD_REBLUR_DIFFUSE_SPECULAR

    float viewZ = gViewZ[ipos];
    float linearRoughness = gNormalRoughness[ipos].z;

    diffuseRadianceHitDist.a = REBLUR_FrontEnd_GetNormHitDist(diffuseRadianceHitDist.a, viewZ, gHitDistParams, linearRoughness);
    REBLUR_FrontEnd_PackRadianceAndHitDist(diffuseRadianceHitDist.rgb, diffuseRadianceHitDist.a);

    specularRadianceHitDist.a = REBLUR_FrontEnd_GetNormHitDist(specularRadianceHitDist.a, viewZ, gHitDistParams, linearRoughness);
    REBLUR_FrontEnd_PackRadianceAndHitDist(specularRadianceHitDist.rgb, specularRadianceHitDist.a);

#endif

    gDiffuseRadianceHitDist[ipos] = diffuseRadianceHitDist;
    gSpecularRadianceHitDist[ipos] = specularRadianceHitDist;
}
