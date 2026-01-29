#include "EmissiveUniformSampler.h"

namespace Falcor
{
    EmissiveUniformSampler::EmissiveUniformSampler(RenderContext* pRenderContext, ref<ILightCollection> pLightCollection, const Options& options)
        : EmissiveLightSampler(EmissiveLightSamplerType::Uniform, std::move(pLightCollection))
        , mOptions(options)
    {
    }
}
