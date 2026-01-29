#pragma once
#include "Sampler.h"
#include "FBO.h"
#include "ParameterBlock.h"
#include "Utils/Math/Vector.h"
#include <memory>

namespace Falcor
{
class Device;
class FullScreenPass;

struct BlitContext
{
    ref<FullScreenPass> pPass;
    ref<Fbo> pFbo;

    ref<Sampler> pLinearSampler;
    ref<Sampler> pPointSampler;
    ref<Sampler> pLinearMinSampler;
    ref<Sampler> pPointMinSampler;
    ref<Sampler> pLinearMaxSampler;
    ref<Sampler> pPointMaxSampler;

    ref<ParameterBlock> pBlitParamsBuffer;
    float2 prevSrcRectOffset = float2(0, 0);
    float2 prevSrcReftScale = float2(0, 0);

    // Variable offsets in constant buffer
    TypedShaderVarOffset offsetVarOffset;
    TypedShaderVarOffset scaleVarOffset;
    ProgramReflection::BindLocation texBindLoc;

    // Parameters for complex blit
    float4 prevComponentsTransform[4] = {float4(0), float4(0), float4(0), float4(0)};
    TypedShaderVarOffset compTransVarOffset[4];

    BlitContext(Device* pDevice);
};
} // namespace Falcor
