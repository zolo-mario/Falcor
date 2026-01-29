#pragma once
#include "fwd.h"
#include "Handles.h"
#include "Resource.h"
#include "Buffer.h"
#include "Fence.h"
#include "Core/Macros.h"
#include "Core/Object.h"
#include <queue>
#include <unordered_map>

namespace Falcor
{
class FALCOR_API GpuMemoryHeap : public Object
{
    FALCOR_OBJECT(GpuMemoryHeap)
public:
    struct BaseData
    {
        Slang::ComPtr<gfx::IBufferResource> gfxBufferResource;
        uint32_t size = 0;
        GpuAddress offset = 0;
        uint8_t* pData = nullptr;

        uint64_t getGpuAddress() const { return gfxBufferResource->getDeviceAddress() + offset; }
    };

    struct Allocation : public BaseData
    {
        uint64_t pageID = 0;
        uint64_t fenceValue = 0;

        static constexpr uint64_t kMegaPageId = -1;
        bool operator<(const Allocation& other) const { return fenceValue > other.fenceValue; }
    };

    ~GpuMemoryHeap();

    /**
     * Create a new GPU memory heap.
     * @param[in] memoryType The memory type of heap.
     * @param[in] pageSize Page size in bytes.
     * @param[in] pFence Fence to use for synchronization.
     * @return A new object, or throws an exception if creation failed.
     */
    static ref<GpuMemoryHeap> create(ref<Device> pDevice, MemoryType memoryType, size_t pageSize, ref<Fence> pFence);

    Allocation allocate(size_t size, size_t alignment = 1);
    Allocation allocate(size_t size, ResourceBindFlags bindFlags);
    void release(Allocation& data);
    size_t getPageSize() const { return mPageSize; }
    void executeDeferredReleases();

    void breakStrongReferenceToDevice();

private:
    GpuMemoryHeap(ref<Device> pDevice, MemoryType memoryType, size_t pageSize, ref<Fence> pFence);

    struct PageData : public BaseData
    {
        uint32_t allocationsCount = 0;
        size_t currentOffset = 0;

        using UniquePtr = std::unique_ptr<PageData>;
    };

    BreakableReference<Device> mpDevice;
    MemoryType mMemoryType;
    ref<Fence> mpFence;
    size_t mPageSize = 0;
    size_t mCurrentPageId = 0;
    PageData::UniquePtr mpActivePage;

    std::priority_queue<Allocation> mDeferredReleases;
    std::unordered_map<size_t, PageData::UniquePtr> mUsedPages;
    std::queue<PageData::UniquePtr> mAvailablePages;

    void allocateNewPage();
    void initBasePageData(BaseData& data, size_t size);
};
} // namespace Falcor
