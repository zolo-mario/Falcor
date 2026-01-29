#pragma once
#include "Scene/Material/BasicMaterial.h"

namespace Falcor
{
    /** This class implements a coated conductor material, i.e. a
        dielectric coating on top of a conductor base. Both conductor
        and coating can be smooth or rough, isotropic or anisotropic.
        The material simulates interreflection between the layers
        using a Monte Carlo random walk.

        This class perfectly matches the PBRT "coatedconductor" material.

        Texture channel layout:

            BaseColor
                - RGB - Conductor complex Eta
                - A   - Opacity
            Transmission
                - RGB - Conductor complex k
            Specular
                - R - Interface X Roughness
                - G - Interface Y Roughness
                - B - Conductor X Roughness
                - A - Conductor Y Roughness
            Normal
                - 3-Channel standard normal map, or 2-Channel BC5 format

        See additional texture channels defined in BasicMaterial.
    */
    class FALCOR_API PBRTCoatedConductorMaterial : public BasicMaterial
    {
        FALCOR_OBJECT(PBRTCoatedConductorMaterial)
    public:
        static ref<PBRTCoatedConductorMaterial> create(ref<Device> pDevice, const std::string& name) { return make_ref<PBRTCoatedConductorMaterial>(pDevice, name); }

        PBRTCoatedConductorMaterial(ref<Device> pDevice, const std::string& name);

        ProgramDesc::ShaderModuleList getShaderModules() const override;
        TypeConformanceList getTypeConformances() const override;

        /** Set the roughness.
        */
        void setRoughness(float4 roughness);

        /** Get the roughness.
        */
        float4 getRoughness() const { return float4(mData.specular[0], mData.specular[1], mData.specular[2], mData.specular[3]); }

    protected:
        void renderSpecularUI(Gui::Widgets& widget) override;
    };
}
