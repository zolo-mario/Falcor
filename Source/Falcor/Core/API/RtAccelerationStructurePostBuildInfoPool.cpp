#include "RtAccelerationStructurePostBuildInfoPool.h"
#include "RtAccelerationStructure.h"
#include "Device.h"
#include "CopyContext.h"
#include "GFXAPI.h"

namespace Falcor
{
ref<RtAccelerationStructurePostBuildInfoPool> RtAccelerationStructurePostBuildInfoPool::create(Device* pDevice, const Desc& desc)
{
    return ref<RtAccelerationStructurePostBuildInfoPool>(new RtAccelerationStructurePostBuildInfoPool(pDevice, desc));
}

RtAccelerationStructurePostBuildInfoPool::RtAccelerationStructurePostBuildInfoPool(Device* pDevice, const Desc& desc) : mDesc(desc)
{
    gfx::IQueryPool::Desc queryPoolDesc = {};
    queryPoolDesc.count = desc.elementCount;
    queryPoolDesc.type = getGFXAccelerationStructurePostBuildQueryType(desc.queryType);
    FALCOR_GFX_CALL(pDevice->getGfxDevice()->createQueryPool(queryPoolDesc, mpGFXQueryPool.writeRef()));
}

RtAccelerationStructurePostBuildInfoPool::~RtAccelerationStructurePostBuildInfoPool() {}

uint64_t RtAccelerationStructurePostBuildInfoPool::getElement(CopyContext* pContext, uint32_t index)
{
    if (mNeedFlush)
    {
        pContext->submit(true);
        mNeedFlush = false;
    }
    uint64_t result = 0;
    FALCOR_GFX_CALL(mpGFXQueryPool->getResult(index, 1, &result));
    return result;
}

void RtAccelerationStructurePostBuildInfoPool::reset(CopyContext* pContext)
{
    FALCOR_GFX_CALL(mpGFXQueryPool->reset());
    mNeedFlush = true;
}
} // namespace Falcor
