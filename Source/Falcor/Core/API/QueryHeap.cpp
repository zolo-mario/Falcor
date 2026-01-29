#include "QueryHeap.h"
#include "Device.h"
#include "GFXAPI.h"

namespace Falcor
{

ref<QueryHeap> QueryHeap::create(ref<Device> pDevice, Type type, uint32_t count)
{
    return ref<QueryHeap>(new QueryHeap(pDevice, type, count));
}

QueryHeap::QueryHeap(ref<Device> pDevice, Type type, uint32_t count) : mpDevice(pDevice), mCount(count), mType(type)
{
    FALCOR_ASSERT(pDevice);
    gfx::IQueryPool::Desc desc = {};
    desc.count = count;
    switch (type)
    {
    case Type::Timestamp:
        desc.type = gfx::QueryType::Timestamp;
        break;
    default:
        FALCOR_UNREACHABLE();
        break;
    }
    FALCOR_GFX_CALL(pDevice->getGfxDevice()->createQueryPool(desc, mGfxQueryPool.writeRef()));
}

void QueryHeap::breakStrongReferenceToDevice()
{
    mpDevice.breakStrongReference();
}

} // namespace Falcor
