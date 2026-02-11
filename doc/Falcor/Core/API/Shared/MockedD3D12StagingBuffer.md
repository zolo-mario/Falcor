# MockedD3D12StagingBuffer

## Module Overview

The **MockedD3D12StagingBuffer** class provides a mocked `ID3D12Resource` that supports mapped write and allows reading contents directly from CPU memory. This is a temporary solution used to call `DDGIVolume::Update` to receive contents to write into a ParameterBlock later without reading back from GPU memory. The class provides both CPU and GPU buffers, with the CPU buffer used for SDK writes and the GPU buffer used for SDK passes.

## Files

- **Header**: `Source/Falcor/Core/API/Shared/MockedD3D12StagingBuffer.h` (101 lines)
- **Implementation**: `Source/Falcor/Core/API/Shared/MockedD3D12StagingBuffer.cpp` (133 lines)

## Class Definition

```cpp
class MockedD3D12StagingBuffer : public ID3D12Resource
{
public:
    void resize(ref<Device> pDevice, size_t size);

    size_t getSize() const { return mData.size(); }
    const void* getData() const { return mData.data(); }

    // Inherited via ID3D12Resource
    virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    virtual ULONG __stdcall AddRef(void) override;
    virtual ULONG __stdcall Release(void) override;
    virtual HRESULT __stdcall GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) override;
    virtual HRESULT __stdcall SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) override;
    virtual HRESULT __stdcall SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) override;
    virtual HRESULT __stdcall SetName(LPCWSTR Name) override;
    virtual HRESULT __stdcall GetDevice(REFIID riid, void** ppvDevice) override;
    virtual HRESULT __stdcall Map(UINT Subresource, const D3D12_RANGE* pReadRange, void** ppData) override;
    virtual void __stdcall Unmap(UINT Subresource, const D3D12_RANGE* pWrittenRange) override;
    virtual D3D12_RESOURCE_DESC __stdcall GetDesc(void) override;
    virtual D3D12_GPU_VIRTUAL_ADDRESS __stdcall GetGPUVirtualAddress(void) override;
    virtual HRESULT __stdcall WriteToSubresource(
        UINT DstSubresource,
        const D3D12_BOX* pDstBox,
        const void* pSrcData,
        UINT SrcRowPitch,
        UINT SrcDepthPitch
    ) override;
    virtual HRESULT __stdcall ReadFromSubresource(
        void* pDstData,
        UINT DstRowPitch,
        UINT DstDepthPitch,
        UINT SrcSubresource,
        const D3D12_BOX* pSrcBox
    ) override;
    virtual HRESULT __stdcall GetHeapProperties(D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS* pHeapFlags) override;

private:
    std::vector<uint8_t> mData; // CPU Buffer.
    ref<Buffer> mpGpuBuffer;    // GPU Buffer.
};
```

## Dependencies

### Internal Dependencies

- **Buffer**: Required for GPU buffer management
- **Device**: Required for buffer creation

### External Dependencies

- **D3D12**: Requires Direct3D 12 API
- **windows.h**: Windows API definitions
- **std::vector**: Standard library vector container

## Cross-Platform Considerations

- **Windows Only**: D3D12 is only available on Windows
- **Conditional Compilation**: Only compiles when `FALCOR_HAS_D3D12` is defined
- **No Vulkan Equivalent**: Vulkan uses different staging buffer management

## Usage Patterns

### Buffer Creation and Resize

```cpp
ref<Device> pDevice = ...;
size_t bufferSize = 1024;

MockedD3D12StagingBuffer stagingBuffer;
stagingBuffer.resize(pDevice, bufferSize);
```

### Accessing Buffer Data

```cpp
size_t size = stagingBuffer.getSize();
const void* pData = stagingBuffer.getData();
```

### Using with DDGIVolume

```cpp
// SDK writes to CPU buffer via Map
void* pCpuData;
stagingBuffer.Map(0, nullptr, &pCpuData);
// SDK writes data to pCpuData
stagingBuffer.Unmap(0, nullptr);

// GPU buffer now has updated data
// Can be used for SDK passes
```

### Getting GPU Virtual Address

```cpp
D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = stagingBuffer.GetGPUVirtualAddress();
```

## Summary

**MockedD3D12StagingBuffer** is a specialized D3D12 resource mock that provides:

### MockedD3D12StagingBuffer
- Excellent cache locality (all members in single cache line, excluding vector elements)
- O(N) time complexity for resize (N = buffer size)
- O(1) time complexity for access and COM operations
- O(N) time complexity for unmap (N = buffer size)
- Dual buffer architecture (CPU + GPU)
- COM interface implementation with delegation
- Map operation returns CPU buffer pointer
- Unmap operation copies CPU to GPU buffer
- Temporary solution for DDGIVolume integration

The class implements a sophisticated dual-buffer staging system with COM interface delegation, providing a temporary workaround for DDGIVolume integration that allows SDK writes to CPU memory without requiring GPU readback, with excellent cache locality and efficient memory management.
