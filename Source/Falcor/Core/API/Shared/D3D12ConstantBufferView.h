#pragma once
#include "D3D12DescriptorSet.h"
#include "Core/API/Buffer.h"
#include "Core/Macros.h"
#include "Core/Object.h"

#include <memory>

namespace Falcor
{
// GFX doesn't need constant buffer view.
// We provide a raw D3D12 implementation for applications
// that wish to use the raw D3D12DescriptorSet API.
class FALCOR_API D3D12ConstantBufferView : public Object
{
    FALCOR_OBJECT(D3D12ConstantBufferView)
public:
    static ref<D3D12ConstantBufferView> create(ref<Device> pDevice, uint64_t gpuAddress, uint32_t byteSize);
    static ref<D3D12ConstantBufferView> create(ref<Device> pDevice, ref<Buffer> pBuffer);
    static ref<D3D12ConstantBufferView> create(ref<Device> pDevice);

    /**
     * Get the D3D12 CPU descriptor handle representing this resource view.
     * Valid only when D3D12 is the underlying API.
     */
    D3D12_CPU_DESCRIPTOR_HANDLE getD3D12CpuHeapHandle() const;

private:
    D3D12ConstantBufferView(ref<Buffer> pBuffer, ref<D3D12DescriptorSet> pDescriptorSet)
        : mpBuffer(pBuffer), mpDescriptorSet(pDescriptorSet)
    {}

    ref<Buffer> mpBuffer;
    ref<D3D12DescriptorSet> mpDescriptorSet;
};
} // namespace Falcor
