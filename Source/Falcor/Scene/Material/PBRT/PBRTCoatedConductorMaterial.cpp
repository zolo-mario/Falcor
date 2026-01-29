#include "PBRTCoatedConductorMaterial.h"
#include "Utils/Scripting/ScriptBindings.h"
#include "GlobalState.h"

namespace Falcor
{
    namespace
    {
        const char kShaderFile[] = "Rendering/Materials/PBRT/PBRTCoatedConductorMaterial.slang";
    }

    PBRTCoatedConductorMaterial::PBRTCoatedConductorMaterial(ref<Device> pDevice, const std::string& name)
        : BasicMaterial(pDevice, name, MaterialType::PBRTCoatedConductor)
    {
        // Setup additional texture slots.
        mTextureSlotInfo[(uint32_t)TextureSlot::BaseColor] = { "baseColor", TextureChannelFlags::RGBA, false };
        mTextureSlotInfo[(uint32_t)TextureSlot::Transmission] = { "transmission", TextureChannelFlags::RGB, false };
        mTextureSlotInfo[(uint32_t)TextureSlot::Specular] = { "specular", TextureChannelFlags::RGBA, false };
        mTextureSlotInfo[(uint32_t)TextureSlot::Normal] = { "normal", TextureChannelFlags::RGB, false };
    }

    ProgramDesc::ShaderModuleList PBRTCoatedConductorMaterial::getShaderModules() const
    {
        return { ProgramDesc::ShaderModule::fromFile(kShaderFile) };
    }

    TypeConformanceList PBRTCoatedConductorMaterial::getTypeConformances() const
    {
        return { {{"PBRTCoatedConductorMaterial", "IMaterial"}, (uint32_t)MaterialType::PBRTCoatedConductor} };
    }

    void PBRTCoatedConductorMaterial::renderSpecularUI(Gui::Widgets& widget)
    {
        float4 roughness = getRoughness();
        if (widget.var("Interface X Roughness", roughness.x, 0.f, 1.f, 0.01f)) setRoughness(roughness);
        if (widget.var("Interface Y Roughness", roughness.y, 0.f, 1.f, 0.01f)) setRoughness(roughness);
        if (widget.var("Conductor X Roughness", roughness.z, 0.f, 1.f, 0.01f)) setRoughness(roughness);
        if (widget.var("Conductor Y Roughness", roughness.w, 0.f, 1.f, 0.01f)) setRoughness(roughness);
    }

    void PBRTCoatedConductorMaterial::setRoughness(float4 roughness)
    {
        if (mData.specular[0] != (float16_t)roughness.x || mData.specular[1] != (float16_t)roughness.y ||
            mData.specular[2] != (float16_t)roughness.z || mData.specular[3] != (float16_t)roughness.w)
        {
            mData.specular[0] = (float16_t)roughness.x;
            mData.specular[1] = (float16_t)roughness.y;
            mData.specular[2] = (float16_t)roughness.z;
            mData.specular[3] = (float16_t)roughness.w;
            markUpdates(UpdateFlags::DataChanged);
        }
    }

    FALCOR_SCRIPT_BINDING(PBRTCoatedConductorMaterial)
    {
        using namespace pybind11::literals;

        FALCOR_SCRIPT_BINDING_DEPENDENCY(BasicMaterial)

        pybind11::class_<PBRTCoatedConductorMaterial, BasicMaterial, ref<PBRTCoatedConductorMaterial>> material(m, "PBRTCoatedConductorMaterial");
        auto create = [] (const std::string& name)
        {
            return PBRTCoatedConductorMaterial::create(accessActivePythonSceneBuilder().getDevice(), name);
        };
        material.def(pybind11::init(create), "name"_a = ""); // PYTHONDEPRECATED

        material.def_property("roughness", &PBRTCoatedConductorMaterial::getRoughness, &PBRTCoatedConductorMaterial::setRoughness);
    }
}
