#pragma once
#include "Scene/Material/BasicMaterial.h"

namespace Falcor
{
    /** This class implements a dielectric material. This means the
        surface has both (uncolored) reflection and refraction lobes.
        The material can be act like an ideal smooth surface
        (if roughness is 0), or a (potentially anisotropic) GGX microfacet.

        This class perfectly matches the PBRT "dielectric" material.

        Texture channel layout:

            Specular
                - R - X Roughness
                - G - Y Roughness
                - B - Unused
                - A - Unused
            Normal
                - 3-Channel standard normal map, or 2-Channel BC5 format

        See additional texture channels defined in BasicMaterial.
    */
    class FALCOR_API PBRTDielectricMaterial : public BasicMaterial
    {
        FALCOR_OBJECT(PBRTDielectricMaterial)
    public:
        static ref<PBRTDielectricMaterial> create(ref<Device> pDevice, const std::string& name) { return make_ref<PBRTDielectricMaterial>(pDevice, name); }

        PBRTDielectricMaterial(ref<Device> pDevice, const std::string& name);

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
