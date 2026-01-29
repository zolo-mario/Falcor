#pragma once
#include "fwd.h"
#include "Handles.h"
#include "Core/Macros.h"
#include "Core/Error.h"
#include "Core/Object.h"
#include <deque>

namespace Falcor
{
class FALCOR_API QueryHeap : public Object
{
    FALCOR_OBJECT(QueryHeap)
public:
    enum class Type
    {
        Timestamp,
        Occlusion,
        PipelineStats
    };

    static constexpr uint32_t kInvalidIndex = 0xffffffff;

    /**
     * Create a new query heap.
     * @param[in] type Type of queries.
     * @param[in] count Number of queries.
     * @return New object, or throws an exception if creation failed.
     */
    static ref<QueryHeap> create(ref<Device> pDevice, Type type, uint32_t count);

    gfx::IQueryPool* getGfxQueryPool() const { return mGfxQueryPool; }
    uint32_t getQueryCount() const { return mCount; }
    Type getType() const { return mType; }

    /**
     * Allocates a new query.
     * @return Query index, or kInvalidIndex if out of queries.
     */
    uint32_t allocate()
    {
        if (mFreeQueries.size())
        {
            uint32_t entry = mFreeQueries.front();
            mFreeQueries.pop_front();
            return entry;
        }
        if (mCurrentObject < mCount)
            return mCurrentObject++;
        else
            return kInvalidIndex;
    }

    void release(uint32_t entry)
    {
        FALCOR_ASSERT(entry != kInvalidIndex);
        mFreeQueries.push_back(entry);
    }

    void breakStrongReferenceToDevice();

private:
    QueryHeap(ref<Device> pDevice, Type type, uint32_t count);

    BreakableReference<Device> mpDevice;
    Slang::ComPtr<gfx::IQueryPool> mGfxQueryPool;
    uint32_t mCount = 0;
    uint32_t mCurrentObject = 0;
    std::deque<uint32_t> mFreeQueries;
    Type mType;
};
} // namespace Falcor
