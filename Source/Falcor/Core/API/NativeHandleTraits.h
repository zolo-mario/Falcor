#pragma once

#include "NativeHandle.h"

#if FALCOR_HAS_D3D12
#include <d3d12.h>
#endif

#if FALCOR_HAS_VULKAN
#include <vulkan/vulkan.h>
#endif

#include "fstd/bit.h"

namespace Falcor
{

template<typename T>
struct NativeHandleTrait
{};

#define FALCOR_NATIVE_HANDLE(T, TYPE)                \
    template<>                                       \
    struct NativeHandleTrait<T>                      \
    {                                                \
        static const NativeHandleType type = TYPE;   \
        static uint64_t pack(T native)               \
        {                                            \
            return fstd::bit_cast<uint64_t>(native); \
        }                                            \
        static T unpack(uint64_t value)              \
        {                                            \
            return fstd::bit_cast<T>(value);         \
        }                                            \
    };

#if FALCOR_HAS_D3D12
FALCOR_NATIVE_HANDLE(ID3D12Device*, NativeHandleType::ID3D12Device);
FALCOR_NATIVE_HANDLE(ID3D12Resource*, NativeHandleType::ID3D12Resource);
FALCOR_NATIVE_HANDLE(ID3D12PipelineState*, NativeHandleType::ID3D12PipelineState);
FALCOR_NATIVE_HANDLE(ID3D12Fence*, NativeHandleType::ID3D12Fence);
FALCOR_NATIVE_HANDLE(ID3D12CommandQueue*, NativeHandleType::ID3D12CommandQueue);
FALCOR_NATIVE_HANDLE(ID3D12GraphicsCommandList*, NativeHandleType::ID3D12GraphicsCommandList);
FALCOR_NATIVE_HANDLE(D3D12_CPU_DESCRIPTOR_HANDLE, NativeHandleType::D3D12_CPU_DESCRIPTOR_HANDLE);
#endif // FALCOR_HAS_D3D12

#if FALCOR_HAS_VULKAN
FALCOR_NATIVE_HANDLE(VkInstance, NativeHandleType::VkInstance);
FALCOR_NATIVE_HANDLE(VkPhysicalDevice, NativeHandleType::VkPhysicalDevice);
FALCOR_NATIVE_HANDLE(VkDevice, NativeHandleType::VkDevice);
FALCOR_NATIVE_HANDLE(VkImage, NativeHandleType::VkImage);
FALCOR_NATIVE_HANDLE(VkImageView, NativeHandleType::VkImageView);
FALCOR_NATIVE_HANDLE(VkBuffer, NativeHandleType::VkBuffer);
FALCOR_NATIVE_HANDLE(VkBufferView, NativeHandleType::VkBufferView);
FALCOR_NATIVE_HANDLE(VkPipeline, NativeHandleType::VkPipeline);
FALCOR_NATIVE_HANDLE(VkFence, NativeHandleType::VkFence);
FALCOR_NATIVE_HANDLE(VkQueue, NativeHandleType::VkQueue);
FALCOR_NATIVE_HANDLE(VkCommandBuffer, NativeHandleType::VkCommandBuffer);
FALCOR_NATIVE_HANDLE(VkSampler, NativeHandleType::VkSampler);
#endif // FALCOR_HAS_VULKAN

#undef FALCOR_NATIVE_HANDLE
} // namespace Falcor
