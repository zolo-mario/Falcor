#include "BlendState.h"
#include "FBO.h"
#include "Core/ObjectPython.h"
#include "Utils/Scripting/ScriptBindings.h"

namespace Falcor
{
ref<BlendState> BlendState::create(const Desc& desc)
{
    return ref<BlendState>(new BlendState(desc));
}

BlendState::Desc::Desc()
{
    mRtDesc.resize(Fbo::getMaxColorTargetCount());
}

BlendState::~BlendState() = default;

BlendState::Desc& BlendState::Desc::setRtParams(
    uint32_t rtIndex,
    BlendOp rgbOp,
    BlendOp alphaOp,
    BlendFunc srcRgbFunc,
    BlendFunc dstRgbFunc,
    BlendFunc srcAlphaFunc,
    BlendFunc dstAlphaFunc
)
{
    FALCOR_CHECK(rtIndex < mRtDesc.size(), "'rtIndex' ({}) is out of range.  Must be smaller than {}.", rtIndex, mRtDesc.size());

    mRtDesc[rtIndex].rgbBlendOp = rgbOp;
    mRtDesc[rtIndex].alphaBlendOp = alphaOp;
    mRtDesc[rtIndex].srcRgbFunc = srcRgbFunc;
    mRtDesc[rtIndex].dstRgbFunc = dstRgbFunc;
    mRtDesc[rtIndex].srcAlphaFunc = srcAlphaFunc;
    mRtDesc[rtIndex].dstAlphaFunc = dstAlphaFunc;
    return *this;
}

BlendState::Desc& BlendState::Desc::setRenderTargetWriteMask(
    uint32_t rtIndex,
    bool writeRed,
    bool writeGreen,
    bool writeBlue,
    bool writeAlpha
)
{
    FALCOR_CHECK(rtIndex < mRtDesc.size(), "'rtIndex' ({}) is out of range.  Must be smaller than {}.", rtIndex, mRtDesc.size());

    mRtDesc[rtIndex].writeMask.writeRed = writeRed;
    mRtDesc[rtIndex].writeMask.writeGreen = writeGreen;
    mRtDesc[rtIndex].writeMask.writeBlue = writeBlue;
    mRtDesc[rtIndex].writeMask.writeAlpha = writeAlpha;
    return *this;
}

FALCOR_SCRIPT_BINDING(BlendState)
{
    pybind11::class_<BlendState, ref<BlendState>>(m, "BlendState");
}
} // namespace Falcor
