__exported import Rendering.Materials.IBSDF;

/**
 * Unrefracted dirac transmission for absorption
 * The color is determeined by Beer's law. Absorption and thickness are folded together
 */
struct BeerBTDF : IBSDF
{
    float3 absorption;    ///< Absorption times thickness
    float pathshortening; ///< Grazing angle saturation control; typically calculated with an IOR

    __init(float3 absorption, float pathshortening)
    {
        this.absorption = absorption;
        this.pathshortening = pathshortening;
    }

    float3 eval<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, BSDFContext bc) { return float3(0.f); }

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
        // Default initialization to avoid divergence at returns.
        wo = {};
        weight = {};
        pdf = {};
        lobeType = {};

        if (wi.z > kMinCosTheta)
            return false;

        wo = float3(-wi.x, -wi.y, -wi.z);
        pdf = 0.f;

        float pathlength = 1.f / (1.f - pathshortening + wi.z * pathshortening);
        float3 transmittance = exp(-absorption * pathlength);
        weight = transmittance;
        lobeType = (uint)LobeType::DeltaTransmission;
        return true;
    }

    float evalPdf(const float3 wi, const float3 wo, BSDFContext bc) { return 0.f; }

    AlbedoContributions evalAlbedo(const float3 wi, const LobeType lobetype)
    {
        float pathlength = 1.f / (1.f - pathshortening + wi.z * pathshortening);
        float3 transmittance = exp(-absorption * pathlength);

        return AlbedoContributions(0.0f, 0.0f, transmittance, 1.0f - transmittance);
    }

    float3 getIorAsReflectance() { return float3(0.f); }

    RoughnessInformation getRoughnessInformation(const float3 wi)
    {
        RoughnessInformation r;
        r.roughnessBSDFNotation = float2(0.f, 0.f);
        return r;
    }
};
