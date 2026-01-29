#include "GpuMemoryHeap.h"
#include "Fence.h"
#include "Buffer.h"
#include "Device.h"
#include "GFXAPI.h"
#include "Core/Error.h"
#include "Utils/Math/Common.h"

namespace Falcor
{
GpuMemoryHeap::~GpuMemoryHeap()
{
    mDeferredReleases = decltype(mDeferredReleases)();
}

GpuMemoryHeap::GpuMemoryHeap(ref<Device> pDevice, MemoryType memoryType, size_t pageSize, ref<Fence> pFence)
    : mpDevice(pDevice), mMemoryType(memoryType), mpFence(pFence), mPageSize(pageSize)
{
    allocateNewPage();
}

ref<GpuMemoryHeap> GpuMemoryHeap::create(ref<Device> pDevice, MemoryType memoryType, size_t pageSize, ref<Fence> pFence)
{
    return ref<GpuMemoryHeap>(new GpuMemoryHeap(pDevice, memoryType, pageSize, pFence));
}

void GpuMemoryHeap::allocateNewPage()
{
    if (mpActivePage)
    {
        mUsedPages[mCurrentPageId] = std::move(mpActivePage);
    }

    if (mAvailablePages.size())
    {
        mpActivePage = std::move(mAvailablePages.front());
        mAvailablePages.pop();
        mpActivePage->allocationsCount = 0;
        mpActivePage->currentOffset = 0;
    }
    else
    {
        mpActivePage = std::make_unique<PageData>();
        initBasePageData((*mpActivePage), mPageSize);
    }

    mpActivePage->currentOffset = 0;
    mCurrentPageId++;
}

GpuMemoryHeap::Allocation GpuMemoryHeap::allocate(size_t size, size_t alignment)
{
    Allocation data;
    if (size > mPageSize)
    {
        data.pageID = GpuMemoryHeap::Allocation::kMegaPageId;
        initBasePageData(data, size);
    }
    else
    {
        // Calculate the start
        size_t currentOffset = align_to(alignment, mpActivePage->currentOffset);
        if (currentOffset + size > mPageSize)
        {
            currentOffset = 0;
            allocateNewPage();
        }

        data.pageID = mCurrentPageId;
        data.size = size;
        data.offset = currentOffset;
        data.pData = mpActivePage->pData + currentOffset;
        data.gfxBufferResource = mpActivePage->gfxBufferResource;
        mpActivePage->currentOffset = currentOffset + size;
        mpActivePage->allocationsCount++;
    }

    data.fenceValue = mpFence->getSignaledValue();
    return data;
}

GpuMemoryHeap::Allocation GpuMemoryHeap::allocate(size_t size, ResourceBindFlags bindFlags)
{
    size_t alignment = mpDevice->getBufferDataAlignment(bindFlags);
    return allocate(align_to(alignment, size), alignment);
}

void GpuMemoryHeap::release(Allocation& data)
{
    FALCOR_ASSERT(data.gfxBufferResource);
    mDeferredReleases.push(data);
}

void GpuMemoryHeap::executeDeferredReleases()
{
    uint64_t currentValue = mpFence->getCurrentValue();
    while (mDeferredReleases.size() && mDeferredReleases.top().fenceValue < currentValue)
    {
        const Allocation& data = mDeferredReleases.top();
        if (data.pageID == mCurrentPageId)
        {
            mpActivePage->allocationsCount--;
            if (mpActivePage->allocationsCount == 0)
            {
                mpActivePage->currentOffset = 0;
            }
        }
        else
        {
            if (data.pageID != Allocation::kMegaPageId)
            {
                auto& pData = mUsedPages[data.pageID];
                pData->allocationsCount--;
                if (pData->allocationsCount == 0)
                {
                    mAvailablePages.push(std::move(pData));
                    mUsedPages.erase(data.pageID);
                }
            }
            // else it's a mega-page. Popping it will release the resource
        }
        mDeferredReleases.pop();
    }
}

Slang::ComPtr<gfx::IBufferResource> createBufferResource(
    ref<Device> pDevice,
    Buffer::State initState,
    size_t size,
    size_t elementSize,
    ResourceFormat format,
    ResourceBindFlags bindFlags,
    MemoryType memoryType
);

namespace
{
Buffer::State getInitState(MemoryType memoryType)
{
    switch (memoryType)
    {
    case MemoryType::DeviceLocal:
        return Buffer::State::Common;
    case MemoryType::Upload:
        return Buffer::State::GenericRead;
    case MemoryType::ReadBack:
        return Buffer::State::CopyDest;
    default:
        FALCOR_UNREACHABLE();
        return Buffer::State::Undefined;
    }
}
} // namespace

void GpuMemoryHeap::initBasePageData(BaseData& data, size_t size)
{
    data.gfxBufferResource = createBufferResource(
        mpDevice,
        getInitState(mMemoryType),
        size,
        0,
        ResourceFormat::Unknown,
        ResourceBindFlags::Vertex | ResourceBindFlags::Index | ResourceBindFlags::Constant,
        mMemoryType
    );
    data.size = size;
    data.offset = 0;
    FALCOR_GFX_CALL(data.gfxBufferResource->map(nullptr, (void**)&data.pData));
}

void GpuMemoryHeap::breakStrongReferenceToDevice()
{
    mpDevice.breakStrongReference();
}

} // namespace Falcor
