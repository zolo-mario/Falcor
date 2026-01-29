#include "Utils/Math/MathConstants.slangh"

import Utils.Math.MathHelpers;
__exported import Rendering.Materials.IBSDF;

/**
 * Lambertian diffuse reflection.
 * f_r(wi, wo) = albedo / pi
 */
struct LambertDiffuseBRDF : IBSDF, IDifferentiable
{
    float3 albedo; ///< Diffuse albedo.

    [Differentiable]
    __init(float3 albedo) { this.albedo = albedo; }

    [Differentiable]
    float3 eval<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, BSDFContext bc)
    {
        if (min(wi.z, wo.z) < kMinCosTheta)
            return float3(0.f);

        return M_1_PI * albedo * wo.z;
    }

    bool sample<S : ISampleGenerator>(
        const float3 wi,
        out float3 wo,
        out float pdf,
        out float3 weight,
        out uint lobeType,
        inout S sg,
        BSDFContext bc
    )
    {
        wo = sample_cosine_hemisphere_concentric(sampleNext2D(sg), pdf);
        lobeType = (uint)LobeType::DiffuseReflection;

        if (min(wi.z, wo.z) < kMinCosTheta)
        {
            weight = {};
            return false;
        }

        weight = albedo;
        return true;
    }

    float evalPdf(const float3 wi, const float3 wo, BSDFContext bc)
    {
        if (min(wi.z, wo.z) < kMinCosTheta)
            return 0.f;

        return M_1_PI * wo.z;
    }

    AlbedoContributions evalAlbedo(const float3 wi, const LobeType lobetype)
    {
        return AlbedoContributions(albedo, 1.0f - albedo, 0.0f, 0.0f);
    }

    float3 getIorAsReflectance() { return float3(1.f); }

    RoughnessInformation getRoughnessInformation(const float3 wi)
    {
        RoughnessInformation r;
        r.roughnessBSDFNotation = float2(0.5f, 0.5f);
        return r;
    }
};
