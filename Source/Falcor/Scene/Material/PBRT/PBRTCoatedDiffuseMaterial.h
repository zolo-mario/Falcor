#pragma once
#include "Scene/Material/BasicMaterial.h"

namespace Falcor
{
    /** This class implements a coated diffuse material, i.e. a
        dielectric coating on top of a Lamebrtian base.
        The coating can be smooth or rough, isotropic or anisotropic.
        The material simulates interreflection between the layers
        using a Monte Carlo random walk.

        This class perfectly matches the PBRT "coateddiffuse" material.

        Texture channel layout:

            BaseColor
                - RGB - Diffuse albedo
                - A   - Opacity
            Specular
                - R - Interface X Roughness
                - G - Interface Y Roughness
            Normal
                - 3-Channel standard normal map, or 2-Channel BC5 format

        See additional texture channels defined in BasicMaterial.
    */
    class FALCOR_API PBRTCoatedDiffuseMaterial : public BasicMaterial
    {
        FALCOR_OBJECT(PBRTCoatedDiffuseMaterial)
    public:
        static ref<PBRTCoatedDiffuseMaterial> create(ref<Device> pDevice, const std::string& name) { return make_ref<PBRTCoatedDiffuseMaterial>(pDevice, name); }

        PBRTCoatedDiffuseMaterial(ref<Device> pDevice, const std::string& name);

        ProgramDesc::ShaderModuleList getShaderModules() const override;
        TypeConformanceList getTypeConformances() const override;

        /** Set the roughness.
        */
        void setRoughness(float2 roughness);

        /** Get the roughness.
        */
        float2 getRoughness() const { return float2(mData.specular[0], mData.specular[1]); }

    protected:
        void renderSpecularUI(Gui::Widgets& widget) override;
    };
}
