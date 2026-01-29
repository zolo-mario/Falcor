#include "PBRTDielectricMaterial.h"
#include "Utils/Scripting/ScriptBindings.h"
#include "GlobalState.h"

namespace Falcor
{
    namespace
    {
        const char kShaderFile[] = "Rendering/Materials/PBRT/PBRTDielectricMaterial.slang";
    }

    PBRTDielectricMaterial::PBRTDielectricMaterial(ref<Device> pDevice, const std::string& name)
        : BasicMaterial(pDevice, name, MaterialType::PBRTDielectric)
    {
        // Setup additional texture slots.
        mTextureSlotInfo[(uint32_t)TextureSlot::Specular] = { "specular", TextureChannelFlags::Red | TextureChannelFlags::Green, false };
        mTextureSlotInfo[(uint32_t)TextureSlot::Normal] = { "normal", TextureChannelFlags::RGB, false };
    }

    ProgramDesc::ShaderModuleList PBRTDielectricMaterial::getShaderModules() const
    {
        return { ProgramDesc::ShaderModule::fromFile(kShaderFile) };
    }

    TypeConformanceList PBRTDielectricMaterial::getTypeConformances() const
    {
        return { {{"PBRTDielectricMaterial", "IMaterial"}, (uint32_t)MaterialType::PBRTDielectric} };
    }

    void PBRTDielectricMaterial::renderSpecularUI(Gui::Widgets& widget)
    {
        float2 roughness = getRoughness();
        if (widget.var("X Roughness", roughness.x, 0.f, 1.f, 0.01f)) setRoughness(roughness);
        if (widget.var("Y Roughness", roughness.y, 0.f, 1.f, 0.01f)) setRoughness(roughness);
    }

    void PBRTDielectricMaterial::setRoughness(float2 roughness)
    {
        if (mData.specular[0] != (float16_t)roughness.x || mData.specular[1] != (float16_t)roughness.y)
        {
            mData.specular[0] = (float16_t)roughness.x;
            mData.specular[1] = (float16_t)roughness.y;
            markUpdates(UpdateFlags::DataChanged);
        }
    }

    FALCOR_SCRIPT_BINDING(PBRTDielectricMaterial)
    {
        using namespace pybind11::literals;

        FALCOR_SCRIPT_BINDING_DEPENDENCY(BasicMaterial)

        pybind11::class_<PBRTDielectricMaterial, BasicMaterial, ref<PBRTDielectricMaterial>> material(m, "PBRTDielectricMaterial");
        auto create = [] (const std::string& name)
        {
            return PBRTDielectricMaterial::create(accessActivePythonSceneBuilder().getDevice(), name);
        };
        material.def(pybind11::init(create), "name"_a = ""); // PYTHONDEPRECATED

        material.def_property("roughness", &PBRTDielectricMaterial::getRoughness, &PBRTDielectricMaterial::setRoughness);
    }
}
