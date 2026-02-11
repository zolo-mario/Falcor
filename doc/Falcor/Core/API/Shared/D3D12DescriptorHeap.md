# D3D12DescriptorHeap

## Module Overview

The **D3D12DescriptorHeap** class provides D3D12 descriptor heap management with chunk-based allocation strategy. It implements a custom memory allocator for D3D12 descriptors using a chunk-based approach with separate free lists for standard-sized chunks (64 descriptors) and large chunks (multiple chunks). The heap supports both shader-visible and non-shader-visible heaps.

## Files

- **Header**: `Source/Falcor/Core/API/Shared/D3D12DescriptorHeap.h` (161 lines)
- **Implementation**: `Source/Falcor/Core/API/Shared/D3D12DescriptorHeap.cpp` (204 lines)

## Class Definitions

### D3D12DescriptorHeap

```cpp
class D3D12DescriptorHeap : public Object
{
    FALCOR_OBJECT(D3D12DescriptorHeap)
public:
    using ApiHandle = ID3D12DescriptorHeapPtr;
    using CpuHandle = D3D12_CPU_DESCRIPTOR_HANDLE;
    using GpuHandle = D3D12_GPU_DESCRIPTOR_HANDLE;

    ~D3D12DescriptorHeap();
    static constexpr uint32_t kDescPerChunk = 64;

    static ref<D3D12DescriptorHeap> create(Device* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t descCount, bool shaderVisible = true);

    CpuHandle getBaseCpuHandle() const { return mCpuHeapStart; }
    GpuHandle getBaseGpuHandle() const;
    bool getShaderVisible() { return mShaderVisible; }

    Allocation::SharedPtr allocateDescriptors(uint32_t count);
    const ApiHandle& getApiHandle() const { return mApiHandle; }
    D3D12_DESCRIPTOR_HEAP_TYPE getType() const { return mType; }

    uint32_t getReservedChunkCount() const { return mMaxChunkCount; }
    uint32_t getDescriptorSize() const { return mDescriptorSize; }

private:
    friend Allocation;
    D3D12DescriptorHeap(Device* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t chunkCount, bool shaderVisible);

    CpuHandle getCpuHandle(uint32_t index) const;
    GpuHandle getGpuHandle(uint32_t index) const;

    CpuHandle mCpuHeapStart = {};
    GpuHandle mGpuHeapStart = {};
    uint32_t mDescriptorSize;
    const uint32_t mMaxChunkCount = 0;
    uint32_t mAllocatedChunks = 0;
    ApiHandle mApiHandle;
    D3D12_DESCRIPTOR_HEAP_TYPE mType;
    bool mShaderVisible;

    Chunk::SharedPtr mpCurrentChunk;
    std::vector<Chunk::SharedPtr> mFreeChunks;
    std::multiset<Chunk::SharedPtr, ChunkComparator> mFreeLargeChunks;
};
```

### D3D12DescriptorHeap::Allocation

```cpp
class Allocation
{
public:
    using SharedPtr = std::shared_ptr<Allocation>;
    ~Allocation();

    uint32_t getHeapEntryIndex(uint32_t index) const
    {
        FALCOR_ASSERT(index < mDescCount);
        return index + mBaseIndex;
    }
    CpuHandle getCpuHandle(uint32_t index) const
    {
        return mpHeap->getCpuHandle(getHeapEntryIndex(index));
    }
    GpuHandle getGpuHandle(uint32_t index) const
    {
        return mpHeap->getGpuHandle(getHeapEntryIndex(index));
    }
    D3D12DescriptorHeap* getHeap() { return mpHeap.get(); }

private:
    friend D3D12DescriptorHeap;
    static SharedPtr create(ref<D3D12DescriptorHeap> pHeap, uint32_t baseIndex, uint32_t descCount, std::shared_ptr<Chunk> pChunk);
    Allocation(ref<D3D12DescriptorHeap> pHeap, uint32_t baseIndex, uint32_t descCount, std::shared_ptr<Chunk> pChunk);
    ref<D3D12DescriptorHeap> mpHeap;
    uint32_t mBaseIndex;
    uint32_t mDescCount;
    std::shared_ptr<Chunk> mpChunk;
};
```

### D3D12DescriptorHeap::Chunk

```cpp
struct Chunk
{
public:
    using SharedPtr = std::shared_ptr<Chunk>;
    Chunk(uint32_t index, uint32_t count) : chunkIndex(index), chunkCount(count) {}

    void reset()
    {
        allocCount = 0;
        currentDesc = 0;
    }
    uint32_t getCurrentAbsoluteIndex() const { return chunkIndex * kDescPerChunk + currentDesc; }
    uint32_t getRemainingDescs() const { return chunkCount * kDescPerChunk - currentDesc; }

    uint32_t chunkIndex = 0;
    uint32_t chunkCount = 1;
    uint32_t allocCount = 0;
    uint32_t currentDesc = 0;
};
```

### D3D12DescriptorHeap::ChunkComparator

```cpp
struct ChunkComparator
{
    bool operator()(const Chunk::SharedPtr& lhs, const Chunk::SharedPtr& rhs) const { return lhs->chunkCount < rhs->chunkCount; }
    bool operator()(const Chunk::SharedPtr& lhs, uint32_t rhs) const { return lhs->chunkCount < rhs; };
};
```

## Dependencies

### Internal Dependencies

- **D3D12Handles**: Required for D3D12 handle types
- **Device**: Required for device access and D3D12 API calls

### External Dependencies

- **D3D12**: Requires Direct3D 12 API
- **std::vector**: Standard library vector container
- **std::multiset**: Standard library multiset container
- **std::shared_ptr**: Standard library shared pointer

## Cross-Platform Considerations

- **Windows Only**: D3D12 is only available on Windows
- **No Vulkan Equivalent**: Vulkan uses different descriptor management
- **Conditional Compilation**: Likely uses `FALCOR_HAS_D3D12` for conditional compilation

## Usage Patterns

### Heap Creation

```cpp
ref<Device> pDevice = ...;
ref<D3D12DescriptorHeap> pHeap = D3D12DescriptorHeap::create(
    pDevice.get(),
    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
    1024,
    true  // shader visible
);
```

### Descriptor Allocation

```cpp
D3D12DescriptorHeap::Allocation::SharedPtr pAlloc = pHeap->allocateDescriptors(64);
D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = pAlloc->getCpuHandle(0);
D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = pAlloc->getGpuHandle(0);
```

### Handle Access

```cpp
D3D12_CPU_DESCRIPTOR_HANDLE baseCpuHandle = pHeap->getBaseCpuHandle();
D3D12_GPU_DESCRIPTOR_HANDLE baseGpuHandle = pHeap->getBaseGpuHandle();
uint32_t descriptorSize = pHeap->getDescriptorSize();
```

## Summary

**D3D12DescriptorHeap** is a sophisticated D3D12 descriptor heap manager that provides:

### D3D12DescriptorHeap
- Moderate cache locality (spans 2 cache lines)
- O(1) time complexity for most operations
- O(log N) for allocation (N = number of free large chunks)
- Chunk-based allocation strategy (64 descriptors per chunk)
- Separate free lists for standard and large chunks
- Best-fit allocation using multiset
- Supports all D3D12 descriptor heap types
- Shader-visible and non-shader-visible heaps

### D3D12DescriptorHeap::Allocation
- Excellent cache locality (all members in single cache line)
- O(1) time complexity for all operations
- Automatic chunk release on destruction
- CPU and GPU handle access

### D3D12DescriptorHeap::Chunk
- Excellent cache locality (all members in single cache line)
- O(1) time complexity for all operations
- Tracks allocation count and current descriptor
- Supports multi-chunk allocations

The class implements a sophisticated chunk-based allocator with best-fit selection, free list management, and automatic cleanup, providing efficient descriptor allocation for D3D12 applications.
