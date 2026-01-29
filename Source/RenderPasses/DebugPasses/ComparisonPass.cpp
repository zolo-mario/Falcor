#include "ComparisonPass.h"

namespace
{
const std::string kSplitLocation = "splitLocation";
const std::string kShowTextLabels = "showTextLabels";
const std::string kLeftLabel = "leftLabel";
const std::string kRightLabel = "rightLabel";

const std::string kLeftInput = "leftInput";
const std::string kRightInput = "rightInput";
const std::string kOutput = "output";
} // namespace

ComparisonPass::ComparisonPass(ref<Device> pDevice) : RenderPass(pDevice)
{
    mpTextRenderer = std::make_unique<TextRenderer>(mpDevice);
}

bool ComparisonPass::parseKeyValuePair(const std::string key, const Properties::ConstValue& val)
{
    if (key == kSplitLocation)
    {
        mSplitLoc = val;
        return true;
    }
    else if (key == kShowTextLabels)
    {
        mShowLabels = val;
        return true;
    }
    else if (key == kLeftLabel)
    {
        std::string str = val;
        mLeftLabel = str;
        return true;
    }
    else if (key == kRightLabel)
    {
        std::string str = val;
        mRightLabel = str;
        return true;
    }
    else
        return false;
}

Properties ComparisonPass::getProperties() const
{
    Properties props;
    props[kSplitLocation] = mSplitLoc;
    props[kShowTextLabels] = mShowLabels;
    props[kLeftLabel] = mLeftLabel;
    props[kRightLabel] = mRightLabel;
    return props;
}

RenderPassReflection ComparisonPass::reflect(const CompileData& compileData)
{
    RenderPassReflection r;
    r.addInput(kLeftInput, "Left side image").bindFlags(Falcor::ResourceBindFlags::ShaderResource).texture2D(0, 0);
    r.addInput(kRightInput, "Right side image").bindFlags(Falcor::ResourceBindFlags::ShaderResource).texture2D(0, 0);
    r.addOutput(kOutput, "Output image").bindFlags(Falcor::ResourceBindFlags::RenderTarget).texture2D(0, 0);
    return r;
}

void ComparisonPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // Get references to our input, output, and temporary accumulation texture
    pLeftSrcTex = renderData.getTexture(kLeftInput);
    pRightSrcTex = renderData.getTexture(kRightInput);
    pDstFbo = Fbo::create(mpDevice, {renderData.getTexture(kOutput)});

    // If we haven't initialized the split location, split the screen in half by default
    if (mSplitLoc < 0)
        mSplitLoc = 0.5f;

    // Set shader parameters
    auto var = mpSplitShader->getRootVar();
    var["GlobalCB"]["gSplitLocation"] = int32_t(mSplitLoc * renderData.getDefaultTextureDims().x);
    var["GlobalCB"]["gDividerSize"] = mDividerSize;
    var["gLeftInput"] = mSwapSides ? pRightSrcTex : pLeftSrcTex;
    var["gRightInput"] = mSwapSides ? pLeftSrcTex : pRightSrcTex;

    // Execute the accumulation shader
    mpSplitShader->execute(pRenderContext, pDstFbo);

    // Render some labels
    if (mShowLabels)
    {
        const int32_t screenLocX = int32_t(mSplitLoc * renderData.getDefaultTextureDims().x);
        const int32_t screenLocY = int32_t(renderData.getDefaultTextureDims().y - 32);

        // Draw text labeling the right side image
        std::string rightSide = mSwapSides ? mLeftLabel : mRightLabel;
        mpTextRenderer->render(pRenderContext, rightSide, pDstFbo, float2(screenLocX + 16, screenLocY));

        // Draw text labeling the left side image
        std::string leftSide = mSwapSides ? mRightLabel : mLeftLabel;
        uint32_t leftLength = uint32_t(leftSide.length()) * 9;
        mpTextRenderer->render(pRenderContext, leftSide, pDstFbo, float2(screenLocX - 16 - leftLength, screenLocY));
    }
}

void ComparisonPass::renderUI(Gui::Widgets& widget)
{
    widget.checkbox("Swap Sides", mSwapSides);
    widget.checkbox("Show Labels", mShowLabels);
}
