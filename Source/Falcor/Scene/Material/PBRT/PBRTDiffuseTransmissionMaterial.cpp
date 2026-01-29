#include "PBRTDiffuseTransmissionMaterial.h"
#include "Utils/Scripting/ScriptBindings.h"
#include "GlobalState.h"

namespace Falcor
{
    namespace
    {
        const char kShaderFile[] = "Rendering/Materials/PBRT/PBRTDiffuseTransmissionMaterial.slang";
    }

    PBRTDiffuseTransmissionMaterial::PBRTDiffuseTransmissionMaterial(ref<Device> pDevice, const std::string& name)
        : BasicMaterial(pDevice, name, MaterialType::PBRTDiffuseTransmission)
    {
        // Setup additional texture slots.
        mTextureSlotInfo[(uint32_t)TextureSlot::BaseColor] = { "baseColor", TextureChannelFlags::RGBA, true };
        mTextureSlotInfo[(uint32_t)TextureSlot::Transmission] = { "transmission", TextureChannelFlags::RGB, true };
        mTextureSlotInfo[(uint32_t)TextureSlot::Normal] = { "normal", TextureChannelFlags::RGB, false };
    }

    ProgramDesc::ShaderModuleList PBRTDiffuseTransmissionMaterial::getShaderModules() const
    {
        return { ProgramDesc::ShaderModule::fromFile(kShaderFile) };
    }

    TypeConformanceList PBRTDiffuseTransmissionMaterial::getTypeConformances() const
    {
        return { {{"PBRTDiffuseTransmissionMaterial", "IMaterial"}, (uint32_t)MaterialType::PBRTDiffuseTransmission} };
    }

    FALCOR_SCRIPT_BINDING(PBRTDiffuseTransmissionMaterial)
    {
        using namespace pybind11::literals;

        FALCOR_SCRIPT_BINDING_DEPENDENCY(BasicMaterial)

        pybind11::class_<PBRTDiffuseTransmissionMaterial, BasicMaterial, ref<PBRTDiffuseTransmissionMaterial>> material(m, "PBRTDiffuseTransmissionMaterial");
        auto create = [] (const std::string& name)
        {
            return PBRTDiffuseTransmissionMaterial::create(accessActivePythonSceneBuilder().getDevice(), name);
        };
        material.def(pybind11::init(create), "name"_a = ""); // PYTHONDEPRECATED
    }
}
