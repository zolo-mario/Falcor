#include "SampleGenerator.h"

namespace Falcor
{
static std::map<uint32_t, std::function<ref<SampleGenerator>(ref<Device>)>> sFactory;
static Gui::DropdownList sGuiDropdownList;

ref<SampleGenerator> SampleGenerator::create(ref<Device> pDevice, uint32_t type)
{
    if (auto it = sFactory.find(type); it != sFactory.end())
    {
        return it->second(pDevice);
    }
    else
    {
        FALCOR_THROW("Can't create SampleGenerator. Unknown type");
    }
}

DefineList SampleGenerator::getDefines() const
{
    DefineList defines;
    defines.add("SAMPLE_GENERATOR_TYPE", std::to_string(mType));
    return defines;
}

const Gui::DropdownList& SampleGenerator::getGuiDropdownList()
{
    return sGuiDropdownList;
}

void SampleGenerator::registerType(uint32_t type, const std::string& name, std::function<ref<SampleGenerator>(ref<Device>)> createFunc)
{
    sGuiDropdownList.push_back({type, name});
    sFactory[type] = createFunc;
}

void SampleGenerator::registerAll()
{
    registerType(
        SAMPLE_GENERATOR_TINY_UNIFORM,
        "Tiny uniform (32-bit)",
        [](ref<Device> pDevice) { return ref<SampleGenerator>(new SampleGenerator(pDevice, SAMPLE_GENERATOR_TINY_UNIFORM)); }
    );
    registerType(
        SAMPLE_GENERATOR_UNIFORM,
        "Uniform (128-bit)",
        [](ref<Device> pDevice) { return ref<SampleGenerator>(new SampleGenerator(pDevice, SAMPLE_GENERATOR_UNIFORM)); }
    );
}

// Automatically register basic sampler types.
static struct RegisterSampleGenerators
{
    RegisterSampleGenerators() { SampleGenerator::registerAll(); }
} sRegisterSampleGenerators;
} // namespace Falcor
