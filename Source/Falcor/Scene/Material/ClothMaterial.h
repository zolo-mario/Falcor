#pragma once
#include "BasicMaterial.h"

namespace Falcor
{
    /** Class representing a cloth material.

        Texture channel layout:

            BaseColor
                - RGB - Base Color
                - A   - Opacity
            Specular
                - R - Unused
                - G - Roughness
                - B - Unused
                - A - Unused
            Normal
                - 3-Channel standard normal map, or 2-Channel BC5 format

        See additional texture channels defined in BasicMaterial.
    */
    class FALCOR_API ClothMaterial : public BasicMaterial
    {
        FALCOR_OBJECT(ClothMaterial)
    public:
        static ref<ClothMaterial> create(ref<Device> pDevice, const std::string& name) { return make_ref<ClothMaterial>(pDevice, name); };

        ClothMaterial(ref<Device> pDevice, const std::string& name);

        ProgramDesc::ShaderModuleList getShaderModules() const override;
        TypeConformanceList getTypeConformances() const override;

        /** Set the roughness.
        */
        void setRoughness(float roughness);

        /** Get the roughness.
        */
        float getRoughness() const { return (float)mData.specular[1]; }

    protected:
        void renderSpecularUI(Gui::Widgets& widget) override;
    };
}
