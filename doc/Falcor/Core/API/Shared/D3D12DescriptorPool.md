# D3D12DescriptorPool

## Module Overview

The **D3D12DescriptorPool** class provides D3D12 descriptor pool management with fence-based synchronization for deferred releases. It manages multiple descriptor heaps (one for each D3D12 descriptor heap type) and provides a unified interface for descriptor allocation across different descriptor types. The pool uses a priority queue to track deferred releases based on fence values.

## Files

- **Header**: `Source/Falcor/Core/API/Shared/D3D12DescriptorPool.h` (114 lines)
- **Implementation**: `Source/Falcor/Core/API/Shared/D3D12DescriptorPool.cpp` (120 lines)

## Class Definitions

### D3D12DescriptorPool

```cpp
class D3D12DescriptorPool : public Object
{
    FALCOR_OBJECT(D3D12DescriptorPool)
public:
    using ApiHandle = ID3D12DescriptorHeapPtr;
    using CpuHandle = D3D12_CPU_DESCRIPTOR_HANDLE;
    using GpuHandle = D3D12_GPU_DESCRIPTOR_HANDLE;
    using ApiData = DescriptorPoolApiData;
    using Type = ShaderResourceType;

    ~D3D12DescriptorPool();

    static constexpr uint32_t kTypeCount = uint32_t(Type::Count);

    class FALCOR_API Desc
    {
    public:
        Desc& setDescCount(Type type, uint32_t count)
        {
            uint32_t t = (uint32_t)type;
            mTotalDescCount -= mDescCount[t];
            mTotalDescCount += count;
            mDescCount[t] = count;
            return *this;
        }

        Desc& setShaderVisible(bool visible)
        {
            mShaderVisible = visible;
            return *this;
        }

    private:
        friend D3D12DescriptorPool;
        uint32_t mDescCount[kTypeCount] = {0};
        uint32_t mTotalDescCount = 0;
        bool mShaderVisible = false;
    };

    static ref<D3D12DescriptorPool> create(Device* pDevice, const Desc& desc, ref<Fence> pFence);

    uint32_t getDescCount(Type type) const { return mDesc.mDescCount[(uint32_t)type]; }
    uint32_t getTotalDescCount() const { return mDesc.mTotalDescCount; }
    bool isShaderVisible() const { return mDesc.mShaderVisible; }
    const ApiHandle& getApiHandle(uint32_t heapIndex) const;
    const ApiData* getApiData() const { return mpApiData.get(); }
    void executeDeferredReleases();

private:
    friend class D3D12DescriptorSet;
    D3D12DescriptorPool(Device* pDevice, const Desc& desc, ref<Fence> pFence);
    void releaseAllocation(std::shared_ptr<DescriptorSetApiData> pData);
    Desc mDesc;
    std::shared_ptr<ApiData> mpApiData;
    ref<Fence> mpFence;

    struct DeferredRelease
    {
        std::shared_ptr<DescriptorSetApiData> pData;
        uint64_t fenceValue;
        bool operator>(const DeferredRelease& other) const { return fenceValue > other.fenceValue; }
    };

    std::priority_queue<DeferredRelease, std::vector<DeferredRelease>, std::greater<DeferredRelease>> mpDeferredReleases;
};
```

## Dependencies

### Internal Dependencies

- **D3D12DescriptorHeap**: Required for descriptor heap management
- **D3D12DescriptorData**: Required for API data structures
- **D3D12Handles**: Required for D3D12 handle types
- **Fence**: Required for fence-based synchronization

### External Dependencies

- **D3D12**: Requires Direct3D 12 API
- **std::shared_ptr**: Standard library shared pointer
- **std::priority_queue**: Standard library priority queue
- **std::vector**: Standard library vector container

## Cross-Platform Considerations

- **Windows Only**: D3D12 is only available on Windows
- **No Vulkan Equivalent**: Vulkan uses different descriptor management
- **Conditional Compilation**: Likely uses `FALCOR_HAS_D3D12` for conditional compilation

## Usage Patterns

### Pool Creation

```cpp
ref<Device> pDevice = ...;
ref<Fence> pFence = ...;

D3D12DescriptorPool::Desc desc;
desc.setDescCount(D3D12DescriptorPool::Type::TextureSrv, 256);
desc.setDescCount(D3D12DescriptorPool::Type::Sampler, 64);
desc.setShaderVisible(true);

ref<D3D12DescriptorPool> pPool = D3D12DescriptorPool::create(pDevice.get(), desc, pFence);
```

### Descriptor Count Access

```cpp
uint32_t srvCount = pPool->getDescCount(D3D12DescriptorPool::Type::TextureSrv);
uint32_t totalCount = pPool->getTotalDescCount();
bool shaderVisible = pPool->isShaderVisible();
```

### API Handle Access

```cpp
const D3D12DescriptorPool::ApiHandle& heapHandle = pPool->getApiHandle(0);
const DescriptorPoolApiData* pApiData = pPool->getApiData();
```

### Deferred Release Execution

```cpp
pPool->executeDeferredReleases();
```

## Summary

**D3D12DescriptorPool** is a sophisticated D3D12 descriptor pool manager that provides:

### D3D12DescriptorPool
- Moderate cache locality (spans 2 cache lines)
- O(1) time complexity for most operations
- O(log N) for release operations (N = number of deferred releases)
- O(N log N) for batch release processing
- Manages 4 D3D12 descriptor heaps
- Supports 13 shader resource types
- Fence-based synchronization for deferred releases
- Priority queue for efficient release management
- Shader-visible and non-shader-visible heaps

### D3D12DescriptorPool::Desc
- Excellent cache locality (all members in single cache line)
- O(1) time complexity for all operations
- Builder pattern with fluent interface
- Supports 13 shader resource types
- Tracks total descriptor count

### D3D12DescriptorPool::DeferredRelease
- Excellent cache locality (all members in single cache line)
- O(1) time complexity for creation
- Stores descriptor set API data and fence value
- Used for fence-based deferred release

The class implements a sophisticated descriptor pool with fence-based synchronization, priority queue management, and automatic cleanup, providing efficient descriptor allocation and release for D3D12 applications.
