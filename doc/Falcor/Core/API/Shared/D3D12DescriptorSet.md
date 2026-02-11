# D3D12DescriptorSet

## Module Overview

The **D3D12DescriptorSet** class provides D3D12 descriptor set management with support for both explicit binding and root signature offset binding modes. It manages descriptor allocations from descriptor pools and provides methods for setting various descriptor types (SRV, UAV, Sampler, CBV) and binding descriptor sets for graphics and compute operations.

## Files

- **Header**: `Source/Falcor/Core/API/Shared/D3D12DescriptorSet.h` (117 lines)
- **Implementation**: `Source/Falcor/Core/API/Shared/D3D12DescriptorSet.cpp` (243 lines)

## Enum Definition

### D3D12DescriptorSetBindingUsage

```cpp
enum class D3D12DescriptorSetBindingUsage
{
    ExplicitBind,       //< The descriptor set will be bound explicitly with a `bindForGraphics` or `bindForCompute` call.
    RootSignatureOffset //< The descriptor set will be implicitly bound via a root signature offsets.
};
```

## Class Definition

```cpp
class D3D12DescriptorSet : public Object
{
    FALCOR_OBJECT(D3D12DescriptorSet)
public:
    using Type = ShaderResourceType;
    using CpuHandle = D3D12DescriptorPool::CpuHandle;
    using GpuHandle = D3D12DescriptorPool::GpuHandle;
    using ApiData = DescriptorSetApiData;

    ~D3D12DescriptorSet();

    static ref<D3D12DescriptorSet> create(ref<Device> pDevice, ref<D3D12DescriptorPool> pPool, const D3D12DescriptorSetLayout& layout);
    static ref<D3D12DescriptorSet> create(
        ref<Device> pDevice,
        const D3D12DescriptorSetLayout& layout,
        D3D12DescriptorSetBindingUsage bindingUsage = D3D12DescriptorSetBindingUsage::ExplicitBind
    );

    size_t getRangeCount() const { return mLayout.getRangeCount(); }
    const D3D12DescriptorSetLayout::Range& getRange(uint32_t range) const { return mLayout.getRange(range); }
    ShaderVisibility getVisibility() const { return mLayout.getVisibility(); }

    CpuHandle getCpuHandle(uint32_t rangeIndex, uint32_t descInRange = 0) const;
    GpuHandle getGpuHandle(uint32_t rangeIndex, uint32_t descInRange = 0) const;
    const ApiData* getApiData() const { return mpApiData.get(); }

    void setCpuHandle(uint32_t rangeIndex, uint32_t descIndex, const CpuHandle& handle);
    void setSrv(uint32_t rangeIndex, uint32_t descIndex, const ShaderResourceView* pSrv);
    void setUav(uint32_t rangeIndex, uint32_t descIndex, const UnorderedAccessView* pUav);
    void setSampler(uint32_t rangeIndex, uint32_t descIndex, const Sampler* pSampler);
    void setCbv(uint32_t rangeIndex, uint32_t descIndex, D3D12ConstantBufferView* pView);

    void bindForGraphics(CopyContext* pCtx, const D3D12RootSignature* pRootSig, uint32_t rootIndex);
    void bindForCompute(CopyContext* pCtx, const D3D12RootSignature* pRootSig, uint32_t rootIndex);

private:
    D3D12DescriptorSet(ref<Device> pDevice, ref<D3D12DescriptorPool> pPool, const D3D12DescriptorSetLayout& layout);

    ref<Device> mpDevice;
    D3D12DescriptorSetLayout mLayout;
    std::shared_ptr<ApiData> mpApiData;
    ref<D3D12DescriptorPool> mpPool;
};
```

## Dependencies

### Internal Dependencies

- **D3D12DescriptorSetLayout**: Required for layout management
- **D3D12DescriptorPool**: Required for descriptor allocation
- **D3D12DescriptorData**: Required for API data structures
- **D3D12ConstantBufferView**: Required for CBV setting
- **D3D12RootSignature**: Required for binding operations
- **D3D12DescriptorHeap**: Required for heap access

### External Dependencies

- **D3D12**: Requires Direct3D 12 API
- **CopyContext**: Required for binding operations
- **ShaderResourceView**: Required for SRV setting
- **UnorderedAccessView**: Required for UAV setting
- **Sampler**: Required for sampler setting
- **Device**: Required for device access
- **std::shared_ptr**: Standard library shared pointer
- **std::vector**: Standard library vector container

## Cross-Platform Considerations

- **Windows Only**: D3D12 is only available on Windows
- **No Vulkan Equivalent**: Vulkan uses different descriptor management
- **Conditional Compilation**: Likely uses `FALCOR_HAS_D3D12` for conditional compilation

## Usage Patterns

### Descriptor Set Creation (Explicit Bind)

```cpp
ref<Device> pDevice = ...;
ref<D3D12DescriptorPool> pPool = ...;
D3D12DescriptorSetLayout layout;
layout.addRange(D3D12DescriptorSet::Type::TextureSrv, 0, 16);

ref<D3D12DescriptorSet> pSet = D3D12DescriptorSet::create(pDevice, pPool, layout);
```

### Descriptor Set Creation (Root Signature Offset)

```cpp
ref<Device> pDevice = ...;
D3D12DescriptorSetLayout layout;
layout.addRange(D3D12DescriptorSet::Type::TextureSrv, 0, 16);

ref<D3D12DescriptorSet> pSet = D3D12DescriptorSet::create(
    pDevice,
    layout,
    D3D12DescriptorSetBindingUsage::RootSignatureOffset
);
```

### Setting Descriptors

```cpp
ref<ShaderResourceView> pSrv = ...;
ref<UnorderedAccessView> pUav = ...;
ref<Sampler> pSampler = ...;
ref<D3D12ConstantBufferView> pCbv = ...;

pSet->setSrv(0, 0, pSrv.get());
pSet->setUav(1, 0, pUav.get());
pSet->setSampler(2, 0, pSampler.get());
pSet->setCbv(3, 0, pCbv.get());
```

### Binding Descriptor Set

```cpp
CopyContext* pCtx = ...;
const D3D12RootSignature* pRootSig = ...;

pSet->bindForGraphics(pCtx, pRootSig, 0);
pSet->bindForCompute(pCtx, pRootSig, 1);
```

### Getting Handles

```cpp
D3D12DescriptorSet::CpuHandle cpuHandle = pSet->getCpuHandle(0, 0);
const DescriptorSetApiData* pApiData = pSet->getApiData();
```

## Summary

**D3D12DescriptorSet** is a sophisticated D3D12 descriptor set manager that provides:

### D3D12DescriptorSet
- Excellent cache locality (all members in single cache line, excluding layout internal storage)
- O(N) time complexity for creation (N = number of ranges)
- O(1) time complexity for descriptor setting
- O(N) time complexity for binding (N = descriptor count)
- Supports explicit and implicit binding modes
- Manages descriptor allocations from pools
- Supports SRV, UAV, Sampler, and CBV descriptors
- Copies descriptors to GPU for binding
- Automatic cleanup via smart pointers

### D3D12DescriptorSetBindingUsage
- Two binding modes: ExplicitBind and RootSignatureOffset
- Explicit bind requires explicit binding calls
- Root signature offset uses implicit binding
- Determines CPU or GPU heap selection

The class implements a sophisticated descriptor set management system with support for multiple binding modes, descriptor types, and efficient GPU descriptor table allocation, providing flexible and efficient descriptor management for D3D12 applications.
