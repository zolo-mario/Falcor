#include "ClothMaterial.h"
#include "GlobalState.h"
#include "Utils/Scripting/ScriptBindings.h"

namespace Falcor
{
    namespace
    {
        const char kShaderFile[] = "Rendering/Materials/ClothMaterial.slang";
    }

    ClothMaterial::ClothMaterial(ref<Device> pDevice, const std::string& name)
        : BasicMaterial(pDevice, name, MaterialType::Cloth)
    {
        // Setup additional texture slots.
        mTextureSlotInfo[(uint32_t)TextureSlot::BaseColor] = { "baseColor", TextureChannelFlags::RGBA, true };
        mTextureSlotInfo[(uint32_t)TextureSlot::Specular] = { "specular", TextureChannelFlags::Green, false };
        mTextureSlotInfo[(uint32_t)TextureSlot::Normal] = { "normal", TextureChannelFlags::RGB, false };
    }

    ProgramDesc::ShaderModuleList ClothMaterial::getShaderModules() const
    {
        return { ProgramDesc::ShaderModule::fromFile(kShaderFile) };
    }

    TypeConformanceList ClothMaterial::getTypeConformances() const
    {
        return { {{"ClothMaterial", "IMaterial"}, (uint32_t)MaterialType::Cloth} };
    }

    void ClothMaterial::renderSpecularUI(Gui::Widgets& widget)
    {
        float roughness = getRoughness();
        if (widget.var("Roughness", roughness, 0.f, 1.f, 0.01f)) setRoughness(roughness);
    }

    void ClothMaterial::setRoughness(float roughness)
    {
        if (mData.specular[1] != (float16_t)roughness)
        {
            mData.specular[1] = (float16_t)roughness;
            markUpdates(UpdateFlags::DataChanged);
        }
    }

    FALCOR_SCRIPT_BINDING(ClothMaterial)
    {
        using namespace pybind11::literals;

        FALCOR_SCRIPT_BINDING_DEPENDENCY(BasicMaterial)

        pybind11::class_<ClothMaterial, BasicMaterial, ref<ClothMaterial>> material(m, "ClothMaterial");
        auto create = [] (const std::string& name)
        {
            return ClothMaterial::create(accessActivePythonSceneBuilder().getDevice(), name);
        };
        material.def(pybind11::init(create), "name"_a = ""); // PYTHONDEPRECATED

        material.def_property("roughness", &ClothMaterial::getRoughness, &ClothMaterial::setRoughness);
    }
}
