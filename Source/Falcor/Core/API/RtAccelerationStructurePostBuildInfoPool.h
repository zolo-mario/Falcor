#pragma once
#include "fwd.h"
#include "Core/Macros.h"
#include "Core/Object.h"
#include <slang-gfx.h>

namespace Falcor
{
enum class RtAccelerationStructurePostBuildInfoQueryType
{
    CompactedSize,
    SerializationSize,
    CurrentSize,
};

class FALCOR_API RtAccelerationStructurePostBuildInfoPool : public Object
{
    FALCOR_OBJECT(RtAccelerationStructurePostBuildInfoPool)
public:
    struct Desc
    {
        RtAccelerationStructurePostBuildInfoQueryType queryType;
        uint32_t elementCount;
    };
    static ref<RtAccelerationStructurePostBuildInfoPool> create(Device* pDevice, const Desc& desc);
    ~RtAccelerationStructurePostBuildInfoPool();
    uint64_t getElement(CopyContext* pContext, uint32_t index);
    void reset(CopyContext* pContext);
    gfx::IQueryPool* getGFXQueryPool() const { return mpGFXQueryPool.get(); }

protected:
    RtAccelerationStructurePostBuildInfoPool(Device* pDevice, const Desc& desc);

private:
    Desc mDesc;
    Slang::ComPtr<gfx::IQueryPool> mpGFXQueryPool;
    bool mNeedFlush = true;
};

struct RtAccelerationStructurePostBuildInfoDesc
{
    RtAccelerationStructurePostBuildInfoQueryType type;
    RtAccelerationStructurePostBuildInfoPool* pool;
    uint32_t index;
};

} // namespace Falcor
