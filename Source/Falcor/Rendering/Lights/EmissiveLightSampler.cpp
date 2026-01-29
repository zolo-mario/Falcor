#include "EmissiveLightSampler.h"
#include "Core/Program/Program.h"
#include "Utils/Scripting/ScriptBindings.h"

namespace Falcor
{
EmissiveLightSampler::EmissiveLightSampler(EmissiveLightSamplerType type, ref<ILightCollection> pLightCollection)
    : mType(type)
    , mpDevice(pLightCollection->getDevice())
{
    setLightCollection(std::move(pLightCollection));
}

void EmissiveLightSampler::setLightCollection(ref<ILightCollection> pLightCollection)
{
    if (mpLightCollection == pLightCollection)
        return;

    mUpdateFlagsConnection.reset();

    mpLightCollection = std::move(pLightCollection);

    if (mpLightCollection)
        mUpdateFlagsConnection = mpLightCollection->getUpdateFlagsSignal().connect([&](ILightCollection::UpdateFlags flags) { mLightCollectionUpdateFlags |= flags; });
}

DefineList EmissiveLightSampler::getDefines() const
{
    return {{"_EMISSIVE_LIGHT_SAMPLER_TYPE", std::to_string((uint32_t)mType)}};
}
} // namespace Falcor
