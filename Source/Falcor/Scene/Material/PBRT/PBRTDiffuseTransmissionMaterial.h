#pragma once
#include "Scene/Material/BasicMaterial.h"

namespace Falcor
{
    /** This class implements a Lambertian diffuse transmission material.
        The material has both reflective and transmissive lobes, both of
        which are invariant to wo. The albedo of the reflective and transmissive
        lobe are specified by BaseColor and Transmission, respectively.

        This class perfectly matches the PBRT "diffusetransmission" material.

        Texture channel layout:

            BaseColor
                - RGB - Base Color
                - A   - Opacity
            Transmission
                - RGB - Transmission color
            Normal
                - 3-Channel standard normal map, or 2-Channel BC5 format

        See additional texture channels defined in BasicMaterial.
    */
    class FALCOR_API PBRTDiffuseTransmissionMaterial : public BasicMaterial
    {
        FALCOR_OBJECT(PBRTDiffuseTransmissionMaterial)
    public:
        static ref<PBRTDiffuseTransmissionMaterial> create(ref<Device> pDevice, const std::string& name) { return make_ref<PBRTDiffuseTransmissionMaterial>(pDevice, name); }

        PBRTDiffuseTransmissionMaterial(ref<Device> pDevice, const std::string& name);

        ProgramDesc::ShaderModuleList getShaderModules() const override;
        TypeConformanceList getTypeConformances() const override;
    };
}
