#pragma once
#include "Scene/Material/BasicMaterial.h"

namespace Falcor
{
    /** This class implements a Lambertian diffuse material, where
        reflectance does not depend on wo.

        This class perfectly matches the PBRT "diffuse" material.

        Texture channel layout:

            BaseColor
                - RGB - Base Color
                - A   - Opacity
            Normal
                - 3-Channel standard normal map, or 2-Channel BC5 format

        See additional texture channels defined in BasicMaterial.
    */
    class FALCOR_API PBRTDiffuseMaterial : public BasicMaterial
    {
        FALCOR_OBJECT(PBRTDiffuseMaterial)
    public:
        static ref<PBRTDiffuseMaterial> create(ref<Device> pDevice, const std::string& name) { return make_ref<PBRTDiffuseMaterial>(pDevice, name); }

        PBRTDiffuseMaterial(ref<Device> pDevice, const std::string& name);

        ProgramDesc::ShaderModuleList getShaderModules() const override;
        TypeConformanceList getTypeConformances() const override;

        const MaterialParamLayout& getParamLayout() const override;
        SerializedMaterialParams serializeParams() const override;
        void deserializeParams(const SerializedMaterialParams& params) override;
    };
}
