#include "FullScreenPass.h"
#include "Core/API/RenderContext.h"
#include "Utils/SharedCache.h"

namespace Falcor
{
namespace
{
struct Vertex
{
    float2 screenPos;
    float2 texCoord;
};

const Vertex kVertices[] = {
    {float2(-1, 1), float2(0, 0)},
    {float2(-1, -1), float2(0, 1)},
    {float2(1, 1), float2(1, 0)},
    {float2(1, -1), float2(1, 1)},
};
} // namespace

struct FullScreenPass::SharedData
{
    ref<Buffer> pVertexBuffer;
    ref<Vao> pVao;
    uint64_t objectCount = 0;

    SharedData(ref<Device> pDevice)
    {
        const uint32_t vbSize = (uint32_t)(sizeof(Vertex) * std::size(kVertices));
        pVertexBuffer = pDevice->createBuffer(vbSize, ResourceBindFlags::Vertex, MemoryType::Upload, (void*)kVertices);
        pVertexBuffer->breakStrongReferenceToDevice();

        ref<VertexLayout> pLayout = VertexLayout::create();
        ref<VertexBufferLayout> pBufLayout = VertexBufferLayout::create();
        pBufLayout->addElement("POSITION", 0, ResourceFormat::RG32Float, 1, 0);
        pBufLayout->addElement("TEXCOORD", 8, ResourceFormat::RG32Float, 1, 1);
        pLayout->addBufferLayout(0, pBufLayout);

        Vao::BufferVec buffers{pVertexBuffer};
        pVao = Vao::create(Vao::Topology::TriangleStrip, pLayout, buffers);
    }
};

static SharedCache<FullScreenPass::SharedData, Device*> sSharedCache;

FullScreenPass::FullScreenPass(ref<Device> pDevice, const ProgramDesc& progDesc, const DefineList& programDefines)
    : BaseGraphicsPass(pDevice, progDesc, programDefines)
{
    // Get shared VB and VAO.
    mpSharedData = sSharedCache.acquire(mpDevice, [this]() { return std::make_shared<SharedData>(mpDevice); });

    // Create depth stencil state
    FALCOR_ASSERT(mpState);
    auto pDsState = DepthStencilState::create(DepthStencilState::Desc().setDepthEnabled(false));
    mpState->setDepthStencilState(pDsState);

    mpState->setVao(mpSharedData->pVao);
}

FullScreenPass::~FullScreenPass() = default;

ref<FullScreenPass> FullScreenPass::create(ref<Device> pDevice, const ProgramDesc& desc, const DefineList& defines, uint32_t viewportMask)
{
    ProgramDesc d = desc;
    DefineList defs = defines;
    std::string gs;

    if (viewportMask)
    {
        defs.add("_VIEWPORT_MASK", std::to_string(viewportMask));
        defs.add("_OUTPUT_VERTEX_COUNT", std::to_string(3 * popcount(viewportMask)));
        d.addShaderLibrary("Core/Pass/FullScreenPass.gs.slang").gsEntry("main");
    }
    if (!d.hasEntryPoint(ShaderType::Vertex))
        d.addShaderLibrary("Core/Pass/FullScreenPass.vs.slang").vsEntry("main");

    return ref<FullScreenPass>(new FullScreenPass(pDevice, d, defs));
}

ref<FullScreenPass> FullScreenPass::create(
    ref<Device> pDevice,
    const std::filesystem::path& path,
    const DefineList& defines,
    uint32_t viewportMask
)
{
    ProgramDesc desc;
    desc.addShaderLibrary(path).psEntry("main");
    return create(pDevice, desc, defines, viewportMask);
}

void FullScreenPass::execute(RenderContext* pRenderContext, const ref<Fbo>& pFbo, bool autoSetVpSc) const
{
    mpState->setFbo(pFbo, autoSetVpSc);
    pRenderContext->draw(mpState.get(), mpVars.get(), (uint32_t)std::size(kVertices), 0);
}
} // namespace Falcor
