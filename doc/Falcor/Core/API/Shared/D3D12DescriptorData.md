# D3D12DescriptorData

## Module Overview

The **D3D12DescriptorData** module contains D3D12-specific data structures for descriptor pool and descriptor set API data. These structures are used to store D3D12-specific implementation details for descriptor pools and descriptor sets.

## Files

- **Header**: `Source/Falcor/Core/API/Shared/D3D12DescriptorData.h` (52 lines)
- **Implementation**: Header-only (no separate .cpp file)

## Struct Definitions

### DescriptorPoolApiData

```cpp
struct DescriptorPoolApiData
{
    static constexpr size_t kHeapCount = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
    ref<D3D12DescriptorHeap> pHeaps[kHeapCount];
};
```

### DescriptorSetApiData

```cpp
struct DescriptorSetApiData
{
    // The heap-allocation. We always allocate a single contiguous block, even if there are multiple ranges.
    D3D12DescriptorHeap::Allocation::SharedPtr pAllocation;

    // For each range, we store the base offset into the allocation. We need it because many set calls accept a range index.
    std::vector<uint32_t> rangeBaseOffset;

    uint32_t descriptorCount;
};
```

## Dependencies

### Internal Dependencies

- **D3D12DescriptorHeap**: Required for descriptor heap management
- **D3D12DescriptorHeap::Allocation**: Required for allocation management

### External Dependencies

- **D3D12**: Requires Direct3D 12 API
- **std::vector**: Standard library vector container
- **std::shared_ptr**: Standard library shared pointer (via SharedPtr alias)

## Cross-Platform Considerations

- **Windows Only**: D3D12 is only available on Windows
- **No Vulkan Equivalent**: Vulkan uses different descriptor management
- **Conditional Compilation**: Likely uses `FALCOR_HAS_D3D12` for conditional compilation

## Usage Patterns

### DescriptorPoolApiData Usage

```cpp
DescriptorPoolApiData poolData;
poolData.pHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] = cbvSrvUavHeap;
poolData.pHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER] = samplerHeap;
poolData.pHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV] = rtvHeap;
poolData.pHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV] = dsvHeap;
```

### DescriptorSetApiData Usage

```cpp
DescriptorSetApiData setData;
setData.pAllocation = allocation;
setData.rangeBaseOffset.push_back(0);
setData.rangeBaseOffset.push_back(10);
setData.descriptorCount = 20;
```

## Summary

**D3D12DescriptorData** contains two lightweight D3D12-specific data structures:

### DescriptorPoolApiData
- Excellent cache locality (all members in single cache line)
- O(1) time complexity for all operations
- Supports all D3D12 descriptor heap types (4 types)
- Array-based heap storage for direct access
- No dynamic memory allocation

### DescriptorSetApiData
- Good cache locality (most members in single cache line)
- O(1) time complexity for most operations (amortized O(1) for vector operations)
- Supports contiguous heap allocations
- Range-based organization with offset tracking
- Dynamic vector storage for range offsets

Both structs are designed for D3D12 descriptor management and provide efficient, low-overhead storage for D3D12-specific implementation details.
