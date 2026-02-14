#include "GraphicsState.h"
#include "Core/ObjectPython.h"
#include "Core/API/Device.h"
#include "Core/Program/ProgramVars.h"
#include "Utils/Scripting/ScriptBindings.h"

namespace Falcor
{
static GraphicsStateObjectDesc::PrimitiveType topology2Type(Vao::Topology t)
{
    switch (t)
    {
    case Vao::Topology::PointList:
        return GraphicsStateObjectDesc::PrimitiveType::Point;
    case Vao::Topology::LineList:
    case Vao::Topology::LineStrip:
        return GraphicsStateObjectDesc::PrimitiveType::Line;
    case Vao::Topology::TriangleList:
    case Vao::Topology::TriangleStrip:
        return GraphicsStateObjectDesc::PrimitiveType::Triangle;
    default:
        FALCOR_UNREACHABLE();
        return GraphicsStateObjectDesc::PrimitiveType::Undefined;
    }
}

ref<GraphicsState> GraphicsState::create(ref<Device> pDevice)
{
    return ref<GraphicsState>(new GraphicsState(pDevice));
}

GraphicsState::GraphicsState(ref<Device> pDevice) : mpDevice(pDevice)
{
    uint32_t vpCount = getMaxViewportCount();

    // Create the viewports
    mViewports.resize(vpCount);
    mScissors.resize(vpCount);
    mVpStack.resize(vpCount);
    mScStack.resize(vpCount);
    for (uint32_t i = 0; i < vpCount; i++)
    {
        setViewport(i, mViewports[i], true);
    }

    mpGsoGraph = std::make_unique<GraphicsStateGraph>();
}

GraphicsState::~GraphicsState() = default;

ref<GraphicsStateObject> GraphicsState::getGSO(const ProgramVars* pVars)
{
    auto pProgramKernels = mpProgram ? mpProgram->getActiveVersion()->getKernels(mpDevice, pVars) : nullptr;
    bool newProgVersion = pProgramKernels.get() != mCachedData.pProgramKernels;
    if (newProgVersion)
    {
        mCachedData.pProgramKernels = pProgramKernels.get();
        mpGsoGraph->walk((void*)pProgramKernels.get());
    }

    const Fbo::Desc* pFboDesc = mpFbo ? &mpFbo->getDesc() : nullptr;
    if (mCachedData.pFboDesc != pFboDesc)
    {
        mpGsoGraph->walk((void*)pFboDesc);
        mCachedData.pFboDesc = pFboDesc;
    }

    ref<GraphicsStateObject> pGso = mpGsoGraph->getCurrentNode();
    if (pGso == nullptr)
    {
        mDesc.pProgramKernels = pProgramKernels;
        mDesc.fboDesc = mpFbo ? mpFbo->getDesc() : Fbo::Desc();
        mDesc.pVertexLayout = mpVao ? mpVao->getVertexLayout() : nullptr;
        mDesc.primitiveType = mpVao ? topology2Type(mpVao->getPrimitiveTopology()) : GraphicsStateObjectDesc::PrimitiveType::Triangle;

        GraphicsStateGraph::CompareFunc cmpFunc = [&desc = mDesc](ref<GraphicsStateObject> pGso) -> bool
        { return pGso && (desc == pGso->getDesc()); };

        if (mpGsoGraph->scanForMatchingNode(cmpFunc))
        {
            pGso = mpGsoGraph->getCurrentNode();
        }
        else
        {
            pGso = mpDevice->createGraphicsStateObject(mDesc);
            mDesc = pGso->getDesc();
            pGso->breakStrongReferenceToDevice();
            mpGsoGraph->setCurrentNodeData(pGso);
        }
    }
    return pGso;
}

GraphicsState& GraphicsState::setFbo(const ref<Fbo>& pFbo, bool setVp0Sc0)
{
    mpFbo = pFbo;

    if (setVp0Sc0 && pFbo)
    {
        uint32_t w = pFbo->getWidth();
        uint32_t h = pFbo->getHeight();
        GraphicsState::Viewport vp(0, 0, float(w), float(h), 0, 1);
        setViewport(0, vp, true);
    }
    return *this;
}

void GraphicsState::pushFbo(const ref<Fbo>& pFbo, bool setVp0Sc0)
{
    mFboStack.push(mpFbo);
    setFbo(pFbo, setVp0Sc0);
}

void GraphicsState::popFbo(bool setVp0Sc0)
{
    FALCOR_CHECK(!mFboStack.empty(), "Empty stack.");

    setFbo(mFboStack.top(), setVp0Sc0);
    mFboStack.pop();
}

GraphicsState& GraphicsState::setVao(const ref<Vao>& pVao)
{
    if (mpVao != pVao)
    {
        mpVao = pVao;
        mpGsoGraph->walk(pVao ? (void*)pVao->getVertexLayout().get() : nullptr);
    }
    return *this;
}

GraphicsState& GraphicsState::setBlendState(ref<BlendState> pBlendState)
{
    if (mDesc.pBlendState != pBlendState)
    {
        mDesc.pBlendState = pBlendState;
        mpGsoGraph->walk((void*)pBlendState.get());
    }
    return *this;
}

GraphicsState& GraphicsState::setRasterizerState(ref<RasterizerState> pRasterizerState)
{
    if (mDesc.pRasterizerState != pRasterizerState)
    {
        mDesc.pRasterizerState = pRasterizerState;
        mpGsoGraph->walk((void*)pRasterizerState.get());
    }
    return *this;
}

GraphicsState& GraphicsState::setSampleMask(uint32_t sampleMask)
{
    if (mDesc.sampleMask != sampleMask)
    {
        mDesc.sampleMask = sampleMask;
        mpGsoGraph->walk((void*)(uint64_t)sampleMask);
    }
    return *this;
}

GraphicsState& GraphicsState::setDepthStencilState(ref<DepthStencilState> pDepthStencilState)
{
    if (mDesc.pDepthStencilState != pDepthStencilState)
    {
        mDesc.pDepthStencilState = pDepthStencilState;
        mpGsoGraph->walk((void*)pDepthStencilState.get());
    }
    return *this;
}

void GraphicsState::pushViewport(uint32_t index, const GraphicsState::Viewport& vp, bool setScissors)
{
    FALCOR_CHECK(index < mVpStack.size(), "'index' is out of range.");

    mVpStack[index].push(mViewports[index]);
    setViewport(index, vp, setScissors);
}

void GraphicsState::popViewport(uint32_t index, bool setScissors)
{
    FALCOR_CHECK(index < mVpStack.size(), "'index' is out of range.");
    FALCOR_CHECK(!mVpStack[index].empty(), "Empty stack.");

    const auto& VP = mVpStack[index].top();
    setViewport(index, VP, setScissors);
    mVpStack[index].pop();
}

void GraphicsState::pushScissors(uint32_t index, const GraphicsState::Scissor& sc)
{
    FALCOR_CHECK(index < mScStack.size(), "'index' is out of range.");

    mScStack[index].push(mScissors[index]);
    setScissors(index, sc);
}

void GraphicsState::popScissors(uint32_t index)
{
    FALCOR_CHECK(index < mScStack.size(), "'index' is out of range.");
    FALCOR_CHECK(!mScStack[index].empty(), "Empty stack.");

    const auto& sc = mScStack[index].top();
    setScissors(index, sc);
    mScStack[index].pop();
}

void GraphicsState::setViewport(uint32_t index, const GraphicsState::Viewport& vp, bool setScissors)
{
    mViewports[index] = vp;

    if (setScissors)
    {
        GraphicsState::Scissor sc;
        sc.left = (int32_t)vp.originX;
        sc.right = sc.left + (int32_t)vp.width;
        sc.top = (int32_t)vp.originY;
        sc.bottom = sc.top + (int32_t)vp.height;
        this->setScissors(index, sc);
    }
}

void GraphicsState::setScissors(uint32_t index, const GraphicsState::Scissor& sc)
{
    mScissors[index] = sc;
}

void GraphicsState::breakStrongReferenceToDevice()
{
    mpDevice.breakStrongReference();
}

FALCOR_SCRIPT_BINDING(GraphicsState)
{
    pybind11::class_<GraphicsState, ref<GraphicsState>>(m, "GraphicsState");
}
} // namespace Falcor
