#pragma once
#include "D3D12ConstantBufferView.h"
#include "Core/API/Device.h"
#include "Core/API/NativeHandleTraits.h"

namespace Falcor
{
ref<D3D12DescriptorSet> createCbvDescriptor(ref<Device> pDevice, const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc)
{
    pDevice->requireD3D12();

    D3D12DescriptorSetLayout layout;
    layout.addRange(ShaderResourceType::Cbv, 0, 1);
    ref<D3D12DescriptorSet> handle = D3D12DescriptorSet::create(pDevice, pDevice->getD3D12CpuDescriptorPool(), layout);
    pDevice->getNativeHandle().as<ID3D12Device*>()->CreateConstantBufferView(&desc, handle->getCpuHandle(0));

    return handle;
}

ref<D3D12ConstantBufferView> D3D12ConstantBufferView::create(ref<Device> pDevice, uint64_t gpuAddress, uint32_t byteSize)
{
    FALCOR_ASSERT(pDevice);
    pDevice->requireD3D12();

    D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
    desc.BufferLocation = gpuAddress;
    desc.SizeInBytes = byteSize;

    return ref<D3D12ConstantBufferView>(new D3D12ConstantBufferView(nullptr, createCbvDescriptor(pDevice, desc)));
}

ref<D3D12ConstantBufferView> D3D12ConstantBufferView::create(ref<Device> pDevice, ref<Buffer> pBuffer)
{
    FALCOR_ASSERT(pDevice);
    pDevice->requireD3D12();

    FALCOR_ASSERT(pBuffer);
    D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
    desc.BufferLocation = pBuffer->getGpuAddress();
    desc.SizeInBytes = (uint32_t)pBuffer->getSize();

    return ref<D3D12ConstantBufferView>(new D3D12ConstantBufferView(pBuffer, createCbvDescriptor(pDevice, desc)));
}

ref<D3D12ConstantBufferView> D3D12ConstantBufferView::create(ref<Device> pDevice)
{
    FALCOR_ASSERT(pDevice);
    pDevice->requireD3D12();

    // GFX doesn't support constant buffer view.
    // We provide a raw D3D12 implementation for applications
    // that wish to use the raw D3D12DescriptorSet API.

    // Create a null view.
    D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
    return ref<D3D12ConstantBufferView>(new D3D12ConstantBufferView(nullptr, createCbvDescriptor(pDevice, desc)));
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12ConstantBufferView::getD3D12CpuHeapHandle() const
{
    return mpDescriptorSet->getCpuHandle(0);
}
} // namespace Falcor
