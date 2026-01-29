#include "BlitContext.h"
#include "Core/Error.h"
#include "Core/API/Device.h"
#include "Core/Program/Program.h"
#include "Core/Pass/FullScreenPass.h"

namespace Falcor
{
BlitContext::BlitContext(Device* pDevice)
{
    FALCOR_ASSERT(pDevice);

    // Init the blit data.
    DefineList defines = {
        {"SAMPLE_COUNT", "1"},
        {"COMPLEX_BLIT", "0"},
        {"SRC_INT", "0"},
        {"DST_INT", "0"},
    };
    ProgramDesc d;
    d.addShaderLibrary("Core/API/BlitReduction.3d.slang").vsEntry("vsMain").psEntry("psMain");
    pPass = FullScreenPass::create(ref<Device>(pDevice), d, defines);
    pPass->breakStrongReferenceToDevice();
    pFbo = Fbo::create(ref<Device>(pDevice));
    pFbo->breakStrongReferenceToDevice();
    FALCOR_ASSERT(pPass && pFbo);

    pBlitParamsBuffer = pPass->getVars()->getParameterBlock("BlitParamsCB");
    offsetVarOffset = pBlitParamsBuffer->getVariableOffset("gOffset");
    scaleVarOffset = pBlitParamsBuffer->getVariableOffset("gScale");
    prevSrcRectOffset = float2(-1.0f);
    prevSrcReftScale = float2(-1.0f);

    Sampler::Desc desc;
    desc.setAddressingMode(TextureAddressingMode::Clamp, TextureAddressingMode::Clamp, TextureAddressingMode::Clamp);
    desc.setReductionMode(TextureReductionMode::Standard);
    desc.setFilterMode(TextureFilteringMode::Linear, TextureFilteringMode::Linear, TextureFilteringMode::Point);
    pLinearSampler = pDevice->createSampler(desc);
    pLinearSampler->breakStrongReferenceToDevice();
    desc.setFilterMode(TextureFilteringMode::Point, TextureFilteringMode::Point, TextureFilteringMode::Point);
    pPointSampler = pDevice->createSampler(desc);
    pPointSampler->breakStrongReferenceToDevice();
    // Min reductions.
    desc.setReductionMode(TextureReductionMode::Min);
    desc.setFilterMode(TextureFilteringMode::Linear, TextureFilteringMode::Linear, TextureFilteringMode::Point);
    pLinearMinSampler = pDevice->createSampler(desc);
    pLinearMinSampler->breakStrongReferenceToDevice();
    desc.setFilterMode(TextureFilteringMode::Point, TextureFilteringMode::Point, TextureFilteringMode::Point);
    pPointMinSampler = pDevice->createSampler(desc);
    pPointMinSampler->breakStrongReferenceToDevice();
    // Max reductions.
    desc.setReductionMode(TextureReductionMode::Max);
    desc.setFilterMode(TextureFilteringMode::Linear, TextureFilteringMode::Linear, TextureFilteringMode::Point);
    pLinearMaxSampler = pDevice->createSampler(desc);
    pLinearMaxSampler->breakStrongReferenceToDevice();
    desc.setFilterMode(TextureFilteringMode::Point, TextureFilteringMode::Point, TextureFilteringMode::Point);
    pPointMaxSampler = pDevice->createSampler(desc);
    pPointMaxSampler->breakStrongReferenceToDevice();

    const auto& pDefaultBlockReflection = pPass->getProgram()->getReflector()->getDefaultParameterBlock();
    texBindLoc = pDefaultBlockReflection->getResourceBinding("gTex");

    // Complex blit parameters

    compTransVarOffset[0] = pBlitParamsBuffer->getVariableOffset("gCompTransformR");
    compTransVarOffset[1] = pBlitParamsBuffer->getVariableOffset("gCompTransformG");
    compTransVarOffset[2] = pBlitParamsBuffer->getVariableOffset("gCompTransformB");
    compTransVarOffset[3] = pBlitParamsBuffer->getVariableOffset("gCompTransformA");
    prevComponentsTransform[0] = float4(1.0f, 0.0f, 0.0f, 0.0f);
    prevComponentsTransform[1] = float4(0.0f, 1.0f, 0.0f, 0.0f);
    prevComponentsTransform[2] = float4(0.0f, 0.0f, 1.0f, 0.0f);
    prevComponentsTransform[3] = float4(0.0f, 0.0f, 0.0f, 1.0f);
    for (uint32_t i = 0; i < 4; i++)
        pBlitParamsBuffer->setVariable(compTransVarOffset[i], prevComponentsTransform[i]);
}
} // namespace Falcor
