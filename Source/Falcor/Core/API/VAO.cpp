#include "VAO.h"
#include "GFXAPI.h"
#include "Core/ObjectPython.h"
#include "Utils/Scripting/ScriptBindings.h"

namespace Falcor
{
Vao::Vao(const BufferVec& pVBs, ref<VertexLayout> pLayout, ref<Buffer> pIB, ResourceFormat ibFormat, Topology topology)
    : mpVertexLayout(pLayout), mpVBs(pVBs), mpIB(pIB), mIbFormat(ibFormat), mTopology(topology)
{}

ref<Vao> Vao::create(Topology topology, ref<VertexLayout> pLayout, const BufferVec& pVBs, ref<Buffer> pIB, ResourceFormat ibFormat)
{
    // TODO: Check number of vertex buffers match with pLayout.
    FALCOR_CHECK(
        !pIB || (ibFormat == ResourceFormat::R16Uint || ibFormat == ResourceFormat::R32Uint), "'ibFormat' must be R16Uint or R32Uint."
    );
    return ref<Vao>(new Vao(pVBs, pLayout, pIB, ibFormat, topology));
}

Vao::ElementDesc Vao::getElementIndexByLocation(uint32_t elementLocaion) const
{
    ElementDesc desc;

    for (uint32_t bufId = 0; bufId < getVertexBuffersCount(); ++bufId)
    {
        const VertexBufferLayout* pVbLayout = mpVertexLayout->getBufferLayout(bufId).get();
        FALCOR_ASSERT(pVbLayout);

        for (uint32_t i = 0; i < pVbLayout->getElementCount(); ++i)
        {
            if (pVbLayout->getElementShaderLocation(i) == elementLocaion)
            {
                desc.vbIndex = bufId;
                desc.elementIndex = i;
                return desc;
            }
        }
    }
    return desc;
}

FALCOR_SCRIPT_BINDING(Vao)
{
    pybind11::class_<Vao, ref<Vao>>(m, "Vao");
}
} // namespace Falcor
