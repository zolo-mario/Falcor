#include "GaussianBlur.h"
#include <cmath>

namespace
{
const char kSrc[] = "src";
const char kDst[] = "dst";

const char kKernelWidth[] = "kernelWidth";
const char kSigma[] = "sigma";

const char kShaderFilename[] = "RenderPasses/Utils/GaussianBlur/GaussianBlur.ps.slang";
} // namespace

void GaussianBlur::registerBindings(pybind11::module& m)
{
    pybind11::class_<GaussianBlur, RenderPass, ref<GaussianBlur>> pass(m, "GaussianBlur");
    pass.def_property(kKernelWidth, &GaussianBlur::getKernelWidth, &GaussianBlur::setKernelWidth);
    pass.def_property(kSigma, &GaussianBlur::getSigma, &GaussianBlur::setSigma);
}

GaussianBlur::GaussianBlur(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    mpFbo = Fbo::create(mpDevice);
    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(TextureFilteringMode::Linear, TextureFilteringMode::Linear, TextureFilteringMode::Point)
        .setAddressingMode(TextureAddressingMode::Clamp, TextureAddressingMode::Clamp, TextureAddressingMode::Clamp);
    mpSampler = mpDevice->createSampler(samplerDesc);

    for (const auto& [key, value] : props)
    {
        if (key == kKernelWidth)
            mKernelWidth = value;
        else if (key == kSigma)
            mSigma = value;
        else
            logWarning("Unknown property '{}' in a GaussianBlur properties.", key);
    }
}

Properties GaussianBlur::getProperties() const
{
    Properties props;
    props[kKernelWidth] = mKernelWidth;
    props[kSigma] = mSigma;
    return props;
}

RenderPassReflection GaussianBlur::reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
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
        { return f.format(srcFormat).resourceType(srcType, srcWidth, srcHeight, srcDepth, srcSampleCount, srcMipCount, srcArraySize); };

        formatField(reflector.addInput(kSrc, "input image to be blurred"));
        formatField(reflector.addOutput(kDst, "output blurred image"));
        mReady = true;
    }
    else
    {
        reflector.addInput(kSrc, "input image to be blurred");
        reflector.addOutput(kDst, "output blurred image");
    }
    return reflector;
}

void GaussianBlur::compile(RenderContext* pRenderContext, const CompileData& compileData)
{
    FALCOR_CHECK(mReady, "GaussianBlur: Missing incoming reflection information");

    uint32_t arraySize = compileData.connectedResources.getField(kSrc)->getArraySize();
    DefineList defines;
    defines.add("_KERNEL_WIDTH", std::to_string(mKernelWidth));
    if (arraySize > 1)
        defines.add("_USE_TEX2D_ARRAY");

    uint32_t layerMask = (arraySize > 1) ? ((1 << arraySize) - 1) : 0;
    defines.add("_HORIZONTAL_BLUR");
    mpHorizontalBlur = FullScreenPass::create(mpDevice, kShaderFilename, defines, layerMask);
    defines.remove("_HORIZONTAL_BLUR");
    defines.add("_VERTICAL_BLUR");
    mpVerticalBlur = FullScreenPass::create(mpDevice, kShaderFilename, defines, layerMask);

    // Make the programs share the vars
    mpVerticalBlur->setVars(mpHorizontalBlur->getVars());

    updateKernel();
}

void GaussianBlur::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    auto pSrc = renderData.getTexture(kSrc);
    mpFbo->attachColorTarget(renderData.getTexture(kDst), 0);
    createTmpFbo(pSrc.get());

    // Horizontal pass
    {
        auto var = mpHorizontalBlur->getRootVar();
        var["gSampler"] = mpSampler;
        var["gSrcTex"] = pSrc;
        mpHorizontalBlur->execute(pRenderContext, mpTmpFbo);
    }

    // Vertical pass
    {
        auto var = mpVerticalBlur->getRootVar();
        var["gSrcTex"] = mpTmpFbo->getColorTexture(0);
        mpVerticalBlur->execute(pRenderContext, mpFbo);
    }
}

void GaussianBlur::createTmpFbo(const Texture* pSrc)
{
    bool createFbo = mpTmpFbo == nullptr;
    ResourceFormat srcFormat = pSrc->getFormat();

    if (createFbo == false)
    {
        createFbo = (pSrc->getWidth() != mpTmpFbo->getWidth()) || (pSrc->getHeight() != mpTmpFbo->getHeight()) ||
                    (srcFormat != mpTmpFbo->getColorTexture(0)->getFormat()) ||
                    pSrc->getArraySize() != mpTmpFbo->getColorTexture(0)->getArraySize();
    }

    if (createFbo)
    {
        Fbo::Desc fboDesc;
        fboDesc.setColorTarget(0, srcFormat);
        mpTmpFbo = Fbo::create2D(mpDevice, pSrc->getWidth(), pSrc->getHeight(), fboDesc, pSrc->getArraySize());
    }
}

void GaussianBlur::renderUI(Gui::Widgets& widget)
{
    if (widget.var("Kernel Width", (int&)mKernelWidth, 1, 15, 2))
        setKernelWidth(mKernelWidth);
    if (widget.slider("Sigma", mSigma, 0.001f, mKernelWidth / 2.f))
        setSigma(mSigma);
}

void GaussianBlur::setKernelWidth(uint32_t kernelWidth)
{
    mKernelWidth = kernelWidth | 1; // Make sure the kernel width is an odd number
    requestRecompile();
}

void GaussianBlur::setSigma(float sigma)
{
    mSigma = sigma;
    requestRecompile();
}

float getCoefficient(float sigma, float kernelWidth, float x)
{
    float sigmaSquared = sigma * sigma;
    float p = -(x * x) / (2 * sigmaSquared);
    float e = std::exp(p);

    float a = 2 * (float)M_PI * sigmaSquared;
    return e / a;
}

void GaussianBlur::updateKernel()
{
    uint32_t center = mKernelWidth / 2;
    float sum = 0;
    std::vector<float> weights(center + 1);
    for (uint32_t i = 0; i <= center; i++)
    {
        weights[i] = getCoefficient(mSigma, (float)mKernelWidth, (float)i);
        sum += (i == 0) ? weights[i] : 2 * weights[i];
    }

    ref<Buffer> pBuf = mpDevice->createTypedBuffer<float>(mKernelWidth, ResourceBindFlags::ShaderResource);

    for (uint32_t i = 0; i <= center; i++)
    {
        float w = weights[i] / sum;
        pBuf->setElement(center + i, w);
        pBuf->setElement(center - i, w);
    }

    mpHorizontalBlur->getRootVar()["weights"] = pBuf;
}
