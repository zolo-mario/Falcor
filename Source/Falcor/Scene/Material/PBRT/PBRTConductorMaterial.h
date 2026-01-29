#pragma once
#include "Scene/Material/BasicMaterial.h"

namespace Falcor
{
    /** This class implements a conductor material. This means the
        surface is purely reflective, and its albedo is dictated by
        the conductor Fresnel equations (i.e. by a complex IoR).
        The material can be act like a mirror (if roughness is 0),
        or a (potentially anisotropic) GGX microfacet.

        This class perfectly matches the PBRT "conductor" material.

        Texture channel layout:

            BaseColor
                - RGB - Complex Eta
                - A   - Opacity
            Transmission
                - RGB - Complex k
            Specular
                - R - X Roughness
                - G - Y Roughness
                - B - Unused
                - A - Unused
            Normal
                - 3-Channel standard normal map, or 2-Channel BC5 format

        See additional texture channels defined in BasicMaterial.
    */
    class FALCOR_API PBRTConductorMaterial : public BasicMaterial
    {
        FALCOR_OBJECT(PBRTConductorMaterial)
    public:
        static ref<PBRTConductorMaterial> create(ref<Device> pDevice, const std::string& name) { return make_ref<PBRTConductorMaterial>(pDevice, name); }

        PBRTConductorMaterial(ref<Device> pDevice, const std::string& name);

        ProgramDesc::ShaderModuleList getShaderModules() const override;
        TypeConformanceList getTypeConformances() const override;

        /** Set the roughness.
        */
        void setRoughness(float2 roughness);

        /** Get the roughness.
        */
        float2 getRoughness() const { return float2(mData.specular[0], mData.specular[1]); }

        const MaterialParamLayout& getParamLayout() const override;
        SerializedMaterialParams serializeParams() const override;
        void deserializeParams(const SerializedMaterialParams& params) override;

    protected:
        void renderSpecularUI(Gui::Widgets& widget) override;
    };
}
