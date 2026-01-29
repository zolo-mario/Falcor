#pragma once
#include "BasicMaterial.h"

namespace Falcor
{
    /** Class representing the standard material.

        Texture channel layout:

        ShadingModel::MetalRough
            BaseColor
                - RGB - Base Color
                - A   - Opacity
            Specular
                - R   - Unused
                - G   - Roughness
                - B   - Metallic
                - A   - Unused

        ShadingModel::SpecGloss
            BaseColor
                - RGB - Diffuse Color
                - A   - Opacity
            Specular
                - RGB - Specular Color
                - A   - Gloss

        In all shading models:

            Normal
                - 3-Channel standard normal map, or 2-Channel BC5 format
            Emissive
                - RGB - Emissive Color
                - A   - Unused
            Transmission
                - RGB - Transmission color
                - A   - Unused

        See additional texture channels defined in BasicMaterial.
    */
    class FALCOR_API StandardMaterial : public BasicMaterial
    {
        FALCOR_OBJECT(StandardMaterial)
    public:
        static ref<StandardMaterial> create(ref<Device> pDevice, const std::string& name = "", ShadingModel shadingModel = ShadingModel::MetalRough)
        {
            return make_ref<StandardMaterial>(pDevice, name, shadingModel);
        }

        StandardMaterial(ref<Device> pDevice, const std::string& name, ShadingModel shadingModel);

        /** Render the UI.
            \return True if the material was modified.
        */
        bool renderUI(Gui::Widgets& widget) override;

        ProgramDesc::ShaderModuleList getShaderModules() const override;
        TypeConformanceList getTypeConformances() const override;

        /** Get the shading model.
        */
        ShadingModel getShadingModel() const { return mData.getShadingModel(); }

        /** Set the roughness.
            Only available for metallic/roughness shading model.
        */
        void setRoughness(float roughness);

        /** Get the roughness.
            Only available for metallic/roughness shading model.
        */
        float getRoughness() const { return getShadingModel() == ShadingModel::MetalRough ? (float)mData.specular[1] : 0.f; }

        /** Set the metallic value.
            Only available for metallic/roughness shading model.
        */
        void setMetallic(float metallic);

        /** Get the metallic value.
            Only available for metallic/roughness shading model.
        */
        float getMetallic() const { return getShadingModel() == ShadingModel::MetalRough ? (float)mData.specular[2] : 0.f; }

        /** Set the emissive color.
        */
        void setEmissiveColor(const float3& color) override;

        /** Set the emissive factor.
        */
        void setEmissiveFactor(float factor);

        /** Get the emissive color.
        */
        float3 getEmissiveColor() const { return mData.emissive; }

        /** Get the emissive factor.
        */
        float getEmissiveFactor() const { return mData.emissiveFactor; }

        // DEMO21: The mesh will use the global IES profile (LightProfile) to modulate its emission
        void setLightProfileEnabled( bool enabled )
        {
            mHeader.setEnableLightProfile( enabled );
        }

        void setHasEntryPointVolumeProperties(bool hasEntryPointVolumeProperties);

        bool getHasEntryPointVolumeProperties() const;

        DefineList getDefines() const override;

        const MaterialParamLayout& getParamLayout() const override;
        SerializedMaterialParams serializeParams() const override;
        void deserializeParams(const SerializedMaterialParams& params) override;

    protected:
        void updateDeltaSpecularFlag() override;

        void renderSpecularUI(Gui::Widgets& widget) override;
        void setShadingModel(ShadingModel model);
    };
}
