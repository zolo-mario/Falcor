#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"
#include "RenderGraph/RenderPassHelpers.h"

using namespace Falcor;

class ImageLoader : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(ImageLoader, "ImageLoader", "Load an image into a texture.");

    static ref<ImageLoader> create(ref<Device> pDevice, const Properties& props) { return make_ref<ImageLoader>(pDevice, props); }

    ImageLoader(ref<Device> pDevice, const Properties& props);

    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual Properties getProperties() const override;

private:
    bool loadImage(const std::filesystem::path& path);

    /// Selected output size.
    RenderPassHelpers::IOSize mOutputSizeSelection = RenderPassHelpers::IOSize::Default;
    /// Current output resource format.
    ResourceFormat mOutputFormat = ResourceFormat::Unknown;
    /// Current output size in pixels.
    uint2 mOutputSize = {};

    ref<Texture> mpTex;
    std::filesystem::path mImagePath;
    uint32_t mArraySlice = 0;
    uint32_t mMipLevel = 0;
    bool mGenerateMips = false;
    bool mLoadSRGB = true;
};
