# D3D12ConstantBufferView

## Module Overview

The **D3D12ConstantBufferView** class provides a D3D12-specific constant buffer view management for applications that wish to use the raw D3D12DescriptorSet API. This class wraps D3D12 constant buffer views and provides a convenient interface for creating and managing them.

## Files

- **Header**: `Source/Falcor/Core/API/Shared/D3D12ConstantBufferView.h` (63 lines)
- **Implementation**: `Source/Falcor/Core/API/Shared/D3D12ConstantBufferView.cpp` (90 lines)

## Class Definition

```cpp
class D3D12ConstantBufferView : public Object
{
    FALCOR_OBJECT(D3D12ConstantBufferView)
public:
    static ref<D3D12ConstantBufferView> create(ref<Device> pDevice, uint64_t gpuAddress, uint32_t byteSize);
    static ref<D3D12ConstantBufferView> create(ref<Device> pDevice, ref<Buffer> pBuffer);
    static ref<D3D12ConstantBufferView> create(ref<Device> pDevice);
    D3D12_CPU_DESCRIPTOR_HANDLE getD3D12CpuHeapHandle() const;
private:
    D3D12ConstantBufferView(ref<Buffer> pBuffer, ref<D3D12DescriptorSet> pDescriptorSet)
        : mpBuffer(pBuffer), mpDescriptorSet(pDescriptorSet)
    {}
    ref<Buffer> mpBuffer;
    ref<D3D12DescriptorSet> mpDescriptorSet;
};
```

## Dependencies

### Internal Dependencies

- **D3D12DescriptorSet**: Required for descriptor set management
- **Buffer**: Optional dependency for buffer-based constant buffer views
- **Device**: Required for device access and validation

### External Dependencies

- **D3D12**: Requires Direct3D 12 API
- **Core/API/Buffer**: Buffer class for buffer-based views
- **Core/API/Device**: Device class for device access
- **Core/Object**: Base Object class for reference counting

## Cross-Platform Considerations

- **Windows Only**: D3D12 is only available on Windows
- **No Vulkan Equivalent**: GFX API doesn't support constant buffer views
- **Conditional Compilation**: Likely uses `FALCOR_HAS_D3D12` for conditional compilation

## Usage Patterns

### GPU Address-Based Creation

```cpp
ref<Device> pDevice = ...;
uint64_t gpuAddress = ...;
uint32_t byteSize = ...;
ref<D3D12ConstantBufferView> pCbv = D3D12ConstantBufferView::create(pDevice, gpuAddress, byteSize);
```

### Buffer-Based Creation

```cpp
ref<Device> pDevice = ...;
ref<Buffer> pBuffer = ...;
ref<D3D12ConstantBufferView> pCbv = D3D12ConstantBufferView::create(pDevice, pBuffer);
```

### Null View Creation

```cpp
ref<Device> pDevice = ...;
ref<D3D12ConstantBufferView> pCbv = D3D12ConstantBufferView::create(pDevice);
```

### Native Handle Access

```cpp
D3D12_CPU_DESCRIPTOR_HANDLE handle = pCbv->getD3D12CpuHeapHandle();
```

## Summary

**D3D12ConstantBufferView** is a lightweight D3D12-specific wrapper for constant buffer views that provides:
- Excellent cache locality (all members in single cache line)
- O(1) time complexity for all operations
- Multiple creation methods for flexibility
- Direct access to D3D12 CPU descriptor handle
- Automatic memory management via smart pointers
- Support for GPU address-based and buffer-based views
- Null view support for flexibility

The class is designed for applications that need raw D3D12 API access and provides a convenient interface for creating and managing D3D12 constant buffer views.
