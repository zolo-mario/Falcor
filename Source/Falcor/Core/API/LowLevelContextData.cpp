#include "LowLevelContextData.h"
#include "Device.h"
#include "GFXAPI.h"
#include "NativeHandleTraits.h"

#if FALCOR_HAS_CUDA
#include "Utils/CudaUtils.h"
#endif

#include <slang-gfx.h>

namespace Falcor
{
LowLevelContextData::LowLevelContextData(Device* pDevice, gfx::ICommandQueue* pQueue) : mpDevice(pDevice), mpGfxCommandQueue(pQueue)
{
    mpFence = mpDevice->createFence();
    mpFence->breakStrongReferenceToDevice();

#if FALCOR_HAS_CUDA
    // GFX currently doesn't support shared fences on Vulkan.
    if (mpDevice->getType() == Device::Type::D3D12)
    {
        mpDevice->initCudaDevice();
        mpCudaFence = mpDevice->createFence(true);
        mpCudaFence->breakStrongReferenceToDevice();
        mpCudaSemaphore = make_ref<cuda_utils::ExternalSemaphore>(mpCudaFence);
    }
#endif

    openCommandBuffer();
}

LowLevelContextData::~LowLevelContextData()
{
    if (mIsCommandBufferOpen)
    {
        closeCommandBuffer();
    }
}

NativeHandle LowLevelContextData::getCommandQueueNativeHandle() const
{
    gfx::InteropHandle gfxNativeHandle = {};
    FALCOR_GFX_CALL(mpGfxCommandQueue->getNativeHandle(&gfxNativeHandle));
#if FALCOR_HAS_D3D12
    if (mpDevice->getType() == Device::Type::D3D12)
        return NativeHandle(reinterpret_cast<ID3D12CommandQueue*>(gfxNativeHandle.handleValue));
#endif
#if FALCOR_HAS_VULKAN
    if (mpDevice->getType() == Device::Type::Vulkan)
        return NativeHandle(reinterpret_cast<VkQueue>(gfxNativeHandle.handleValue));
#endif
    return {};
}

NativeHandle LowLevelContextData::getCommandBufferNativeHandle() const
{
    gfx::InteropHandle gfxNativeHandle = {};
    FALCOR_GFX_CALL(mGfxCommandBuffer->getNativeHandle(&gfxNativeHandle));
#if FALCOR_HAS_D3D12
    if (mpDevice->getType() == Device::Type::D3D12)
        return NativeHandle(reinterpret_cast<ID3D12GraphicsCommandList*>(gfxNativeHandle.handleValue));
#endif
#if FALCOR_HAS_VULKAN
    if (mpDevice->getType() == Device::Type::Vulkan)
        return NativeHandle(reinterpret_cast<VkCommandBuffer>(gfxNativeHandle.handleValue));
#endif
    return {};
}

void LowLevelContextData::closeCommandBuffer()
{
    mIsCommandBufferOpen = false;
    closeEncoders();
    mGfxCommandBuffer->close();
}

void LowLevelContextData::openCommandBuffer()
{
    mIsCommandBufferOpen = true;
    FALCOR_GFX_CALL(mpDevice->getCurrentTransientResourceHeap()->createCommandBuffer(mGfxCommandBuffer.writeRef()));
    mpCommandBuffer = mGfxCommandBuffer.get();
}

void LowLevelContextData::submitCommandBuffer()
{
    closeCommandBuffer();
    mpGfxCommandQueue->executeCommandBuffers(1, mGfxCommandBuffer.readRef(), mpFence->getGfxFence(), mpFence->updateSignaledValue());
    openCommandBuffer();
}

gfx::IResourceCommandEncoder* LowLevelContextData::getResourceCommandEncoder()
{
    if (mpResourceCommandEncoder)
    {
        return mpResourceCommandEncoder;
    }
    if (mpComputeCommandEncoder)
    {
        return mpComputeCommandEncoder;
    }
    if (mpRayTracingCommandEncoder)
    {
        return mpRayTracingCommandEncoder;
    }
    closeEncoders();
    mpResourceCommandEncoder = mpCommandBuffer->encodeResourceCommands();
    return mpResourceCommandEncoder;
}

gfx::IComputeCommandEncoder* LowLevelContextData::getComputeCommandEncoder()
{
    if (mpComputeCommandEncoder)
    {
        return mpComputeCommandEncoder;
    }
    closeEncoders();
    mpComputeCommandEncoder = mpCommandBuffer->encodeComputeCommands();
    return mpComputeCommandEncoder;
}

gfx::IRenderCommandEncoder* LowLevelContextData::getRenderCommandEncoder(
    gfx::IRenderPassLayout* renderPassLayout,
    gfx::IFramebuffer* framebuffer,
    bool& newEncoder
)
{
    if (mpRenderCommandEncoder && mpRenderPassLayout == renderPassLayout && mpFramebuffer == framebuffer)
    {
        newEncoder = false;
        return mpRenderCommandEncoder;
    }
    closeEncoders();
    mpRenderCommandEncoder = mpCommandBuffer->encodeRenderCommands(renderPassLayout, framebuffer);
    mpRenderPassLayout = renderPassLayout;
    mpFramebuffer = framebuffer;
    newEncoder = true;
    return mpRenderCommandEncoder;
}

gfx::IRayTracingCommandEncoder* LowLevelContextData::getRayTracingCommandEncoder()
{
    if (mpRayTracingCommandEncoder)
    {
        return mpRayTracingCommandEncoder;
    }
    closeEncoders();
    mpRayTracingCommandEncoder = mpCommandBuffer->encodeRayTracingCommands();
    return mpRayTracingCommandEncoder;
}

void LowLevelContextData::closeEncoders()
{
    if (mpResourceCommandEncoder)
    {
        mpResourceCommandEncoder->endEncoding();
        mpResourceCommandEncoder = nullptr;
    }
    if (mpRenderCommandEncoder)
    {
        mpRenderCommandEncoder->endEncoding();
        mpRenderCommandEncoder = nullptr;
    }
    if (mpComputeCommandEncoder)
    {
        mpComputeCommandEncoder->endEncoding();
        mpComputeCommandEncoder = nullptr;
    }
    if (mpRayTracingCommandEncoder)
    {
        mpRayTracingCommandEncoder->endEncoding();
        mpRayTracingCommandEncoder = nullptr;
    }
}

void LowLevelContextData::beginDebugEvent(const char* name)
{
    float blackColor[3] = {0.0f, 0.0f, 0.0f};
    getResourceCommandEncoder()->beginDebugEvent(name, blackColor);
}

void LowLevelContextData::endDebugEvent()
{
    getResourceCommandEncoder()->endDebugEvent();
}
} // namespace Falcor
