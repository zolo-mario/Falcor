#include "PBRTConductorMaterial.h"
#include "PBRTConductorMaterialParamLayout.slang"
#include "Utils/Scripting/ScriptBindings.h"
#include "GlobalState.h"

namespace Falcor
{
    namespace
    {
        const char kShaderFile[] = "Rendering/Materials/PBRT/PBRTConductorMaterial.slang";
    }

    PBRTConductorMaterial::PBRTConductorMaterial(ref<Device> pDevice, const std::string& name)
        : BasicMaterial(pDevice, name, MaterialType::PBRTConductor)
    {
        // Setup additional texture slots.
        mTextureSlotInfo[(uint32_t)TextureSlot::BaseColor] = { "baseColor", TextureChannelFlags::RGBA, false };
        mTextureSlotInfo[(uint32_t)TextureSlot::Transmission] = { "transmission", TextureChannelFlags::RGB, false };
        mTextureSlotInfo[(uint32_t)TextureSlot::Specular] = { "specular", TextureChannelFlags::Red | TextureChannelFlags::Green, false };
        mTextureSlotInfo[(uint32_t)TextureSlot::Normal] = { "normal", TextureChannelFlags::RGB, false };
    }

    ProgramDesc::ShaderModuleList PBRTConductorMaterial::getShaderModules() const
    {
        return { ProgramDesc::ShaderModule::fromFile(kShaderFile) };
    }

    TypeConformanceList PBRTConductorMaterial::getTypeConformances() const
    {
        return { {{"PBRTConductorMaterial", "IMaterial"}, (uint32_t)MaterialType::PBRTConductor} };
    }

    void PBRTConductorMaterial::renderSpecularUI(Gui::Widgets& widget)
    {
        float2 roughness = getRoughness();
        if (widget.var("X Roughness", roughness.x, 0.f, 1.f, 0.01f)) setRoughness(roughness);
        if (widget.var("Y Roughness", roughness.y, 0.f, 1.f, 0.01f)) setRoughness(roughness);
    }

    void PBRTConductorMaterial::setRoughness(float2 roughness)
    {
        if (mData.specular[0] != (float16_t)roughness.x || mData.specular[1] != (float16_t)roughness.y)
        {
            mData.specular[0] = (float16_t)roughness.x;
            mData.specular[1] = (float16_t)roughness.y;
            markUpdates(UpdateFlags::DataChanged);
        }
    }

    const MaterialParamLayout& PBRTConductorMaterial::getParamLayout() const
    {
        return PBRTConductorMaterialParamLayout::layout();
    }

    SerializedMaterialParams PBRTConductorMaterial::serializeParams() const
    {
        return PBRTConductorMaterialParamLayout::serialize(this);
    }

    void PBRTConductorMaterial::deserializeParams(const SerializedMaterialParams& params)
    {
        PBRTConductorMaterialParamLayout::deserialize(this, params);
    }

    FALCOR_SCRIPT_BINDING(PBRTConductorMaterial)
    {
        using namespace pybind11::literals;

        FALCOR_SCRIPT_BINDING_DEPENDENCY(BasicMaterial)

        pybind11::class_<PBRTConductorMaterial, BasicMaterial, ref<PBRTConductorMaterial>> material(m, "PBRTConductorMaterial");
        auto create = [] (const std::string& name)
        {
            return PBRTConductorMaterial::create(accessActivePythonSceneBuilder().getDevice(), name);
        };
        material.def(pybind11::init(create), "name"_a = ""); // PYTHONDEPRECATED

        material.def_property("roughness", &PBRTConductorMaterial::getRoughness, &PBRTConductorMaterial::setRoughness);
    }
}
