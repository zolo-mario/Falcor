#include "InvalidPixelDetectionPass.h"

namespace
{
const std::string kSrc = "src";
const std::string kDst = "dst";
const std::string kFormatWarning = "Non-float format can't represent Inf/NaN values. Expect black output.";
} // namespace

InvalidPixelDetectionPass::InvalidPixelDetectionPass(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    mpInvalidPixelDetectPass =
        FullScreenPass::create(mpDevice, "RenderPasses/DebugPasses/InvalidPixelDetectionPass/InvalidPixelDetection.ps.slang");
    mpFbo = Fbo::create(mpDevice);
}

RenderPassReflection InvalidPixelDetectionPass::reflect(const CompileData& compileData)
{
    RenderPassReflection r;
    mReady = false;
    if (compileData.connectedResources.getFieldCount() > 0)
    {
        const RenderPassReflection::Field* edge = compileData.connectedResources.getField(kSrc);
        RenderPassReflection::Field::Type srcType = edge->getType();
        ResourceFormat srcFormat = edge->getFormat();
        uint32_t srcWidth = edge->getWidth();
        uint32_t srcHeight = edge->getHeight();
        uint32_t srcDepth = edge->getDepth();
        uint32_t srcSampleCount = edge->getSampleCount();
        uint32_t srcMipCount = edge->getMipCount();
        uint32_t srcArraySize = edge->getArraySize();

        auto formatField = [=](RenderPassReflection::Field& f)
        { return f.resourceType(srcType, srcWidth, srcHeight, srcDepth, srcSampleCount, srcMipCount, srcArraySize); };

        formatField(r.addInput(kSrc, "Input image to be checked")).format(srcFormat);
        formatField(r.addOutput(kDst, "Output where pixels are red if NaN, green if Inf, and black otherwise"));
        mReady = true;
    }
    else
    {
        r.addInput(kSrc, "Input image to be checked");
        r.addOutput(kDst, "Output where pixels are red if NaN, green if Inf, and black otherwise");
    }
    return r;
}

void InvalidPixelDetectionPass::compile(RenderContext* pRenderContext, const CompileData& compileData)
{
    FALCOR_CHECK(mReady, "InvalidPixelDetectionPass: Missing incoming reflection data");
}

void InvalidPixelDetectionPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    const auto& pSrc = renderData.getTexture(kSrc);
    mFormat = ResourceFormat::Unknown;
    if (pSrc)
    {
        mFormat = pSrc->getFormat();
        if (getFormatType(mFormat) != FormatType::Float)
        {
            logWarning("InvalidPixelDetectionPass::execute() - {}", kFormatWarning);
        }
    }

    mpInvalidPixelDetectPass->getRootVar()["gTexture"] = pSrc;
    mpFbo->attachColorTarget(renderData.getTexture(kDst), 0);
    mpInvalidPixelDetectPass->getState()->setFbo(mpFbo);
    mpInvalidPixelDetectPass->execute(pRenderContext, mpFbo);
}

void InvalidPixelDetectionPass::renderUI(Gui::Widgets& widget)
{
    widget.textWrapped("Pixels are colored red if NaN, green if Inf, and black otherwise.");

    if (mFormat != ResourceFormat::Unknown)
    {
        widget.dummy("#space", {1, 10});
        widget.text("Input format: " + to_string(mFormat));
        if (getFormatType(mFormat) != FormatType::Float)
        {
            widget.textWrapped("Warning: " + kFormatWarning);
        }
    }
}
