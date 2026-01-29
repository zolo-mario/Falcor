#pragma once
#include "D3D12DescriptorHeap.h"
#include <memory>
#include <vector>
#include <cstdint>

namespace Falcor
{
struct DescriptorPoolApiData
{
    static constexpr size_t kHeapCount = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
    ref<D3D12DescriptorHeap> pHeaps[kHeapCount];
};

struct DescriptorSetApiData
{
    // The heap-allocation. We always allocate a single contiguous block, even if there are multiple ranges.
    D3D12DescriptorHeap::Allocation::SharedPtr pAllocation;

    // For each range, we store the base offset into the allocation. We need it because many set calls accept a range index.
    std::vector<uint32_t> rangeBaseOffset;

    uint32_t descriptorCount;
};
} // namespace Falcor
