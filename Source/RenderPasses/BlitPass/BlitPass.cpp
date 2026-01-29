#include "BlitPass.h"

namespace
{
const char kDst[] = "dst";
const char kSrc[] = "src";
const char kFilter[] = "filter";
const char kOutputFormat[] = "outputFormat";

void regBlitPass(pybind11::module& m)
{
    pybind11::class_<BlitPass, RenderPass, ref<BlitPass>> pass(m, "BlitPass");
    pass.def_property(
        "filter",
        [](const BlitPass& self) { return enumToString(self.getFilter()); },
        [](BlitPass& self, const std::string& value) { self.setFilter(stringToEnum<TextureFilteringMode>(value)); }
    );
}
} // namespace

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, BlitPass>();
    ScriptBindings::registerBinding(regBlitPass);
}

BlitPass::BlitPass(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    parseProperties(props);
}

RenderPassReflection BlitPass::reflect(const CompileData& compileData)
{
    RenderPassReflection r;
    r.addOutput(kDst, "The destination texture").format(mOutputFormat);
    r.addInput(kSrc, "The source texture");
    return r;
}

void BlitPass::parseProperties(const Properties& props)
{
    for (const auto& [key, value] : props)
    {
        if (key == kFilter)
            setFilter(value);
        if (key == kOutputFormat)
            mOutputFormat = value;
        else
            logWarning("Unknown property '{}' in a BlitPass properties.", key);
    }
}

Properties BlitPass::getProperties() const
{
    Properties props;
    props[kFilter] = mFilter;
    if (mOutputFormat != ResourceFormat::Unknown)
        props[kOutputFormat] = mOutputFormat;
    return props;
}

void BlitPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    const auto& pSrcTex = renderData.getTexture(kSrc);
    const auto& pDstTex = renderData.getTexture(kDst);

    if (pSrcTex && pDstTex)
    {
        pRenderContext->blit(pSrcTex->getSRV(), pDstTex->getRTV(), RenderContext::kMaxRect, RenderContext::kMaxRect, mFilter);
    }
    else
    {
        logWarning("BlitPass::execute() - missing an input or output resource");
    }
}

void BlitPass::renderUI(Gui::Widgets& widget)
{
    if (auto filter = mFilter; widget.dropdown("Filter", filter))
        setFilter(filter);
}
