#pragma once

#include "Core/Error.h"

#include <cstdint>

namespace Falcor
{

enum class NativeHandleType
{
    Unknown,

    ID3D12Device,
    ID3D12Resource,
    ID3D12PipelineState,
    ID3D12Fence,
    ID3D12CommandQueue,
    ID3D12GraphicsCommandList,
    D3D12_CPU_DESCRIPTOR_HANDLE,

    VkInstance,
    VkPhysicalDevice,
    VkDevice,
    VkImage,
    VkImageView,
    VkBuffer,
    VkBufferView,
    VkPipeline,
    VkFence,
    VkQueue,
    VkCommandBuffer,
    VkSampler,
};

template<typename T>
struct NativeHandleTrait;

/// Represents a native graphics API handle (e.g. D3D12 or Vulkan).
/// Native handles are expected to fit into 64 bits.
/// Type information and conversion from/to native handles is done
/// using type traits from NativeHandleTraits.h which needs to be
/// included when creating and accessing NativeHandle.
/// This separation is done so we don't expose the heavy D3D12/Vulkan
/// headers everywhere.
class NativeHandle
{
public:
    NativeHandle() = default;

    template<typename T>
    explicit NativeHandle(T native)
    {
        mType = NativeHandleTrait<T>::type;
        mValue = NativeHandleTrait<T>::pack(native);
    }

    NativeHandleType getType() const { return mType; }

    bool isValid() const { return mType != NativeHandleType::Unknown; }

    template<typename T>
    T as() const
    {
        FALCOR_ASSERT(mType == NativeHandleTrait<T>::type);
        return NativeHandleTrait<T>::unpack(mValue);
    }

private:
    NativeHandleType mType{NativeHandleType::Unknown};
    uint64_t mValue{0};
};

} // namespace Falcor
