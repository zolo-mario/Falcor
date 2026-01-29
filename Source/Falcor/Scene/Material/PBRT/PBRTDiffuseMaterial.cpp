#include "PBRTDiffuseMaterial.h"
#include "PBRTDiffuseMaterialParamLayout.slang"
#include "Utils/Scripting/ScriptBindings.h"
#include "GlobalState.h"

namespace Falcor
{
    namespace
    {
        const char kShaderFile[] = "Rendering/Materials/PBRT/PBRTDiffuseMaterial.slang";
    }

    PBRTDiffuseMaterial::PBRTDiffuseMaterial(ref<Device> pDevice, const std::string& name)
        : BasicMaterial(pDevice, name, MaterialType::PBRTDiffuse)
    {
        // Setup additional texture slots.
        mTextureSlotInfo[(uint32_t)TextureSlot::BaseColor] = { "baseColor", TextureChannelFlags::RGBA, true };
        mTextureSlotInfo[(uint32_t)TextureSlot::Normal] = { "normal", TextureChannelFlags::RGB, false };
    }

    ProgramDesc::ShaderModuleList PBRTDiffuseMaterial::getShaderModules() const
    {
        return { ProgramDesc::ShaderModule::fromFile(kShaderFile) };
    }

    TypeConformanceList PBRTDiffuseMaterial::getTypeConformances() const
    {
        return { {{"PBRTDiffuseMaterial", "IMaterial"}, (uint32_t)MaterialType::PBRTDiffuse} };
    }

    const MaterialParamLayout& PBRTDiffuseMaterial::getParamLayout() const
    {
        return PBRTDiffuseMaterialParamLayout::layout();
    }

    SerializedMaterialParams PBRTDiffuseMaterial::serializeParams() const
    {
        return PBRTDiffuseMaterialParamLayout::serialize(this);
    }

    void PBRTDiffuseMaterial::deserializeParams(const SerializedMaterialParams& params)
    {
        PBRTDiffuseMaterialParamLayout::deserialize(this, params);
    }

    FALCOR_SCRIPT_BINDING(PBRTDiffuseMaterial)
    {
        using namespace pybind11::literals;

        FALCOR_SCRIPT_BINDING_DEPENDENCY(BasicMaterial)

        pybind11::class_<PBRTDiffuseMaterial, BasicMaterial, ref<PBRTDiffuseMaterial>> material(m, "PBRTDiffuseMaterial");
        auto create = [] (const std::string& name)
        {
            return PBRTDiffuseMaterial::create(accessActivePythonSceneBuilder().getDevice(), name);
        };
        material.def(pybind11::init(create), "name"_a = ""); // PYTHONDEPRECATED
        material.def(pybind11::init(
            [](ref<Device> device, const std::string& name)
            {
                return PBRTDiffuseMaterial::create(device, name);
            }),
            "device"_a,
            "name"_a = ""
        ); // PYTHONDEPRECATED
    }
}
