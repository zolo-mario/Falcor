#pragma once
#include "BasicMaterial.h"

namespace Falcor
{
    /** Class representing a hair material.

        Texture channel layout:

            BaseColor
                - RGB - Absorption coefficient, sigmaA
                - A   - Opacity
            Specular
                - R   - Longitudinal roughness, betaM
                - G   - Azimuthal roughness, betaN
                - B   - The angle that the small scales on the surface of hair are offset from the base cylinder (in degrees).
                - A   - Unused

        See additional texture channels defined in BasicMaterial.
    */
    class FALCOR_API HairMaterial : public BasicMaterial
    {
        FALCOR_OBJECT(HairMaterial)
    public:
        static ref<HairMaterial> create(ref<Device> pDevice, const std::string& name) { return make_ref<HairMaterial>(pDevice, name); };

        HairMaterial(ref<Device> pDevice, const std::string& name);

        ProgramDesc::ShaderModuleList getShaderModules() const override;
        TypeConformanceList getTypeConformances() const override;

        /** Compute sigmaA from eumelanin and pheomelanin concentration.
        */
        static float3 sigmaAFromConcentration(float ce, float cp);

        /** Compute sigmaA from RGB color.
        */
        static float3 sigmaAFromColor(float3 color, float betaN);

        /** Compute RGB color from sigmaA (inverse of sigmaAFromColor).
        */
        static float3 colorFromSigmaA(float3 sigmaA, float betaN);
    };
}
