#include "DepthStencilState.h"
#include "Core/Error.h"
#include "Core/ObjectPython.h"
#include "Utils/Scripting/ScriptBindings.h"

namespace Falcor
{
ref<DepthStencilState> DepthStencilState::create(const Desc& desc)
{
    return ref<DepthStencilState>(new DepthStencilState(desc));
}

DepthStencilState::~DepthStencilState() = default;

DepthStencilState::Desc& DepthStencilState::Desc::setStencilWriteMask(uint8_t mask)
{
    mStencilWriteMask = mask;
    return *this;
}

DepthStencilState::Desc& DepthStencilState::Desc::setStencilReadMask(uint8_t mask)
{
    mStencilReadMask = mask;
    return *this;
}

DepthStencilState::Desc& DepthStencilState::Desc::setStencilFunc(Face face, ComparisonFunc func)
{
    if (face == Face::FrontAndBack)
    {
        setStencilFunc(Face::Front, func);
        setStencilFunc(Face::Back, func);
        return *this;
    }
    StencilDesc& Desc = (face == Face::Front) ? mStencilFront : mStencilBack;
    Desc.func = func;
    return *this;
}

DepthStencilState::Desc& DepthStencilState::Desc::setStencilOp(
    Face face,
    StencilOp stencilFail,
    StencilOp depthFail,
    StencilOp depthStencilPass
)
{
    if (face == Face::FrontAndBack)
    {
        setStencilOp(Face::Front, stencilFail, depthFail, depthStencilPass);
        setStencilOp(Face::Back, stencilFail, depthFail, depthStencilPass);
        return *this;
    }
    StencilDesc& Desc = (face == Face::Front) ? mStencilFront : mStencilBack;
    Desc.stencilFailOp = stencilFail;
    Desc.depthFailOp = depthFail;
    Desc.depthStencilPassOp = depthStencilPass;

    return *this;
}

const DepthStencilState::StencilDesc& DepthStencilState::getStencilDesc(Face face) const
{
    FALCOR_ASSERT(face != Face::FrontAndBack);
    return (face == Face::Front) ? mDesc.mStencilFront : mDesc.mStencilBack;
}

FALCOR_SCRIPT_BINDING(DepthStencilState)
{
    pybind11::class_<DepthStencilState, ref<DepthStencilState>>(m, "DepthStencilState");
}
} // namespace Falcor
