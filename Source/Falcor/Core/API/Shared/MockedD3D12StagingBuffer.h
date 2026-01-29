#pragma once
#if FALCOR_HAS_D3D12
#include "Core/Macros.h"
#include "Core/API/Buffer.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <d3d12.h>
#include <vector>

namespace Falcor
{
// A mocked `ID3D12Resource` that supports mapped write and allow reading contents directly
// from CPU memory.
// This is currently used to call `DDGIVolume::Update` to receive the contents to write into
// a ParameterBlock later without reading back from GPU memory.
// Since DDGIVolume will use the buffer passed into `DDGIVolume::Update` to run its internal
// passes, this buffer implementation also provides an actual GPU resource for those passes.
// The only methods that matter here are `Map` `Unmap` and `GetGPUVirtualAddress`.
// In `Map`, we just return a CPU memory allocation so the SDK can write update-to-date data
// into it.
// In `Unmap`, we update our internal GPU buffer with the contents that the SDK just wrote into.
// In `GetGPUVirtualAddress`, we return the address of the GPU buffer, so the SDK can use it to
// run its internal passes.
//
// With this class, we have a temporary solution that avoids the hackery around
// `ParameterBlock::getUnderlyingConstantBuffer`.
// When `DDGIVolume` provides a better interface to allow us to get the constant buffer data without
// GPU readback in the future, this class can be removed.
//
class FALCOR_API MockedD3D12StagingBuffer : public ID3D12Resource
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
} // namespace Falcor
#endif // FALCOR_HAS_D3D12
