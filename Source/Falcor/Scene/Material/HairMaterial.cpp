#include "HairMaterial.h"
#include "GlobalState.h"
#include "Utils/Scripting/ScriptBindings.h"

namespace Falcor
{
    namespace
    {
        const char kShaderFile[] = "Rendering/Materials/HairMaterial.slang";
    }

    HairMaterial::HairMaterial(ref<Device> pDevice, const std::string& name)
        : BasicMaterial(pDevice, name, MaterialType::Hair)
    {
        // Setup additional texture slots.
        mTextureSlotInfo[(uint32_t)TextureSlot::BaseColor] = { "baseColor", TextureChannelFlags::RGB, true }; // Note: No alpha support
        mTextureSlotInfo[(uint32_t)TextureSlot::Specular] = { "specular", TextureChannelFlags::RGB, false };
    }

    ProgramDesc::ShaderModuleList HairMaterial::getShaderModules() const
    {
        return { ProgramDesc::ShaderModule::fromFile(kShaderFile) };
    }

    TypeConformanceList HairMaterial::getTypeConformances() const
    {
        return { {{"HairMaterial", "IMaterial"}, (uint32_t)MaterialType::Hair} };
    }

    float3 HairMaterial::sigmaAFromConcentration(float ce, float cp)
    {
        float3 eumelaninSigmaA(0.419f, 0.697f, 1.37f);
        float3 pheomelaninSigmaA(0.187f, 0.4f, 1.05f);
        return ce * eumelaninSigmaA + cp * pheomelaninSigmaA;
    }

    float3 HairMaterial::sigmaAFromColor(float3 color, float betaN)
    {
        const float tmp = 5.969f - 0.215f * betaN + 2.532f * betaN * betaN - 10.73f * std::pow(betaN, 3) + 5.574f * std::pow(betaN, 4) + 0.245f * std::pow(betaN, 5);
        float3 sqrtSigmaA = log(max(color, float3(1e-4f))) / tmp;
        return sqrtSigmaA * sqrtSigmaA;
    }

    float3 HairMaterial::colorFromSigmaA(float3 sigmaA, float betaN)
    {
        const float tmp = 5.969f - 0.215f * betaN + 2.532f * betaN * betaN - 10.73f * std::pow(betaN, 3) + 5.574f * std::pow(betaN, 4) + 0.245f * std::pow(betaN, 5);
        return exp(sqrt(sigmaA) * tmp);
    }

    FALCOR_SCRIPT_BINDING(HairMaterial)
    {
        using namespace pybind11::literals;
        FALCOR_SCRIPT_BINDING_DEPENDENCY(BasicMaterial)

        pybind11::class_<HairMaterial, BasicMaterial, ref<HairMaterial>> material(m, "HairMaterial");
        auto create = [] (const std::string& name)
        {
            return HairMaterial::create(accessActivePythonSceneBuilder().getDevice(), name);
        };
        material.def(pybind11::init(create), "name"_a = ""); // PYTHONDEPRECATED
    }
}
