#pragma once
#include "Fence.h"
#include "Handles.h"
#include "NativeHandle.h"
#include "Core/Macros.h"

namespace Falcor
{

class FALCOR_API LowLevelContextData
{
public:
    /**
     * Constructor. Throws an exception if creation failed.
     * @param[in] pDevice Device.
     * @param[in] pQueue Command queue.
     */
    LowLevelContextData(Device* pDevice, gfx::ICommandQueue* pQueue);
    ~LowLevelContextData();

    gfx::ICommandQueue* getGfxCommandQueue() const { return mpGfxCommandQueue; }
    gfx::ICommandBuffer* getGfxCommandBuffer() const { return mGfxCommandBuffer; }

    /**
     * Returns the native API handle for the command queue:
     * - D3D12: ID3D12CommandQueue*
     * - Vulkan: VkQueue (Vulkan)
     */
    NativeHandle getCommandQueueNativeHandle() const;

    /**
     * Returns the native API handle for the command buffer:
     * - D3D12: ID3D12GraphicsCommandList*
     * - Vulkan: VkCommandBuffer
     */
    NativeHandle getCommandBufferNativeHandle() const;

    const ref<Fence>& getFence() const { return mpFence; }

    void closeCommandBuffer();
    void openCommandBuffer();
    void submitCommandBuffer();

    gfx::IResourceCommandEncoder* getResourceCommandEncoder();
    gfx::IComputeCommandEncoder* getComputeCommandEncoder();
    gfx::IRenderCommandEncoder* getRenderCommandEncoder(
        gfx::IRenderPassLayout* renderPassLayout,
        gfx::IFramebuffer* framebuffer,
        bool& newEncoder
    );
    gfx::IRayTracingCommandEncoder* getRayTracingCommandEncoder();
    void closeEncoders();

    void beginDebugEvent(const char* name);
    void endDebugEvent();

private:
    Device* mpDevice;
    gfx::ICommandQueue* mpGfxCommandQueue;
    Slang::ComPtr<gfx::ICommandBuffer> mGfxCommandBuffer;
    ref<Fence> mpFence;

    gfx::ICommandBuffer* mpCommandBuffer = nullptr;
    bool mIsCommandBufferOpen = false;

    gfx::IFramebuffer* mpFramebuffer = nullptr;
    gfx::IRenderPassLayout* mpRenderPassLayout = nullptr;
    gfx::IResourceCommandEncoder* mpResourceCommandEncoder = nullptr;
    gfx::IComputeCommandEncoder* mpComputeCommandEncoder = nullptr;
    gfx::IRenderCommandEncoder* mpRenderCommandEncoder = nullptr;
    gfx::IRayTracingCommandEncoder* mpRayTracingCommandEncoder = nullptr;
};
} // namespace Falcor
