#include "RenderPass.h"

namespace Falcor
{
RenderData::RenderData(
    const std::string& passName,
    ResourceCache& resources,
    Dictionary& dictionary,
    const uint2& defaultTexDims,
    ResourceFormat defaultTexFormat
)
    : mName(passName), mResources(resources), mDictionary(dictionary), mDefaultTexDims(defaultTexDims), mDefaultTexFormat(defaultTexFormat)
{}

const ref<Resource>& RenderData::getResource(const std::string_view name) const
{
    return mResources.getResource(fmt::format("{}.{}", mName, name));
}

ref<Texture> RenderData::getTexture(const std::string_view name) const
{
    auto pResource = getResource(name);
    return pResource ? pResource->asTexture() : nullptr;
}

ref<RenderPass> RenderPass::create(std::string_view type, ref<Device> pDevice, const Properties& props, PluginManager& pm)
{
    // Try to load a plugin of the same name, if render pass class is not registered yet.
    if (!pm.hasClass<RenderPass>(type))
        pm.loadPluginByName(type);

    return pm.createClass<RenderPass>(type, pDevice, props);
}
} // namespace Falcor
