#include "D3D12DescriptorPool.h"
#include "D3D12DescriptorData.h"
#include "Core/API/Device.h"

namespace Falcor
{
D3D12_DESCRIPTOR_HEAP_TYPE falcorToDxDescType(D3D12DescriptorPool::Type t)
{
    switch (t)
    {
    case D3D12DescriptorPool::Type::TextureSrv:
    case D3D12DescriptorPool::Type::TextureUav:
    case D3D12DescriptorPool::Type::RawBufferSrv:
    case D3D12DescriptorPool::Type::RawBufferUav:
    case D3D12DescriptorPool::Type::TypedBufferSrv:
    case D3D12DescriptorPool::Type::TypedBufferUav:
    case D3D12DescriptorPool::Type::StructuredBufferSrv:
    case D3D12DescriptorPool::Type::StructuredBufferUav:
    case D3D12DescriptorPool::Type::AccelerationStructureSrv:
    case D3D12DescriptorPool::Type::Cbv:
        return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    case D3D12DescriptorPool::Type::Dsv:
        return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    case D3D12DescriptorPool::Type::Rtv:
        return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    case D3D12DescriptorPool::Type::Sampler:
        return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    default:
        FALCOR_UNREACHABLE();
        return D3D12_DESCRIPTOR_HEAP_TYPE(-1);
    }
}

const D3D12DescriptorPool::ApiHandle& D3D12DescriptorPool::getApiHandle(uint32_t heapIndex) const
{
    FALCOR_ASSERT(heapIndex < std::size(mpApiData->pHeaps));
    return mpApiData->pHeaps[heapIndex]->getApiHandle();
}

ref<D3D12DescriptorPool> D3D12DescriptorPool::create(Device* pDevice, const Desc& desc, ref<Fence> pFence)
{
    FALCOR_ASSERT(pDevice);
    pDevice->requireD3D12();
    return ref<D3D12DescriptorPool>(new D3D12DescriptorPool(pDevice, desc, pFence));
}

D3D12DescriptorPool::D3D12DescriptorPool(Device* pDevice, const Desc& desc, ref<Fence> pFence) : mDesc(desc), mpFence(pFence)
{
    // Find out how many heaps we need
    static_assert(D3D12DescriptorPool::kTypeCount == 13, "Unexpected desc count, make sure all desc types are supported");
    uint32_t descCount[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {0};

    descCount[D3D12_DESCRIPTOR_HEAP_TYPE_RTV] = mDesc.mDescCount[(uint32_t)Type::Rtv];
    descCount[D3D12_DESCRIPTOR_HEAP_TYPE_DSV] = mDesc.mDescCount[(uint32_t)Type::Dsv];
    descCount[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER] = mDesc.mDescCount[(uint32_t)Type::Sampler];
    descCount[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] = mDesc.mDescCount[(uint32_t)Type::Cbv];
    descCount[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] +=
        mDesc.mDescCount[(uint32_t)Type::TextureSrv] + mDesc.mDescCount[(uint32_t)Type::RawBufferSrv] +
        mDesc.mDescCount[(uint32_t)Type::TypedBufferSrv] + mDesc.mDescCount[(uint32_t)Type::StructuredBufferSrv];
    descCount[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] +=
        mDesc.mDescCount[(uint32_t)Type::TextureUav] + mDesc.mDescCount[(uint32_t)Type::RawBufferUav] +
        mDesc.mDescCount[(uint32_t)Type::TypedBufferUav] + mDesc.mDescCount[(uint32_t)Type::StructuredBufferUav];
    descCount[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] += mDesc.mDescCount[(uint32_t)Type::AccelerationStructureSrv];

    mpApiData = std::make_shared<DescriptorPoolApiData>();
    for (uint32_t i = 0; i < std::size(mpApiData->pHeaps); i++)
    {
        if (descCount[i] > 0)
        {
            mpApiData->pHeaps[i] = D3D12DescriptorHeap::create(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE(i), descCount[i], mDesc.mShaderVisible);
        }
    }
}

D3D12DescriptorPool::~D3D12DescriptorPool() = default;

void D3D12DescriptorPool::executeDeferredReleases()
{
    uint64_t currentValue = mpFence->getCurrentValue();
    while (mpDeferredReleases.size() && mpDeferredReleases.top().fenceValue < currentValue)
    {
        mpDeferredReleases.pop();
    }
}

void D3D12DescriptorPool::releaseAllocation(std::shared_ptr<DescriptorSetApiData> pData)
{
    DeferredRelease d;
    d.pData = pData;
    d.fenceValue = mpFence->getSignaledValue();
    mpDeferredReleases.push(d);
}
} // namespace Falcor
