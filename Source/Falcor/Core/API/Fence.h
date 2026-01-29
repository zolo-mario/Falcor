#pragma once
#include "fwd.h"
#include "Handles.h"
#include "NativeHandle.h"
#include "Core/Macros.h"
#include "Core/Object.h"
#include <limits>

namespace Falcor
{

struct FenceDesc
{
    bool initialValue{0};
    bool shared{false};
};

/**
 * This class represents a fence on the device.
 * It is used to synchronize host and device execution.
 * On the device, the fence is represented by a 64-bit integer.
 * On the host, we keep a copy of the last signaled value.
 * By default, the fence value is monotonically incremented every time it is signaled.
 *
 * To synchronize the host with the device, we can do the following:
 *
 * ref<Fence> fence = device->createFence();
 * <schedule device work 1>
 * // Signal the fence once we have finished all the above work on the device.
 * renderContext->signal(fence);
 * <schedule device work 2>
 * // Wait on the host until <device work 1> is finished.
 * fence->wait();
 */
class FALCOR_API Fence : public Object
{
    FALCOR_OBJECT(Fence)
public:
    static constexpr uint64_t kAuto = std::numeric_limits<uint64_t>::max();
    static constexpr uint64_t kTimeoutInfinite = std::numeric_limits<uint64_t>::max();

    /// Constructor.
    /// Do not use directly, use Device::createFence instead.
    Fence(ref<Device> pDevice, FenceDesc desc);

    ~Fence();

    /// Returns the description.
    const FenceDesc& getDesc() const { return mDesc; }

    /**
     * Signal the fence.
     * This signals the fence from the host.
     * @param value The value to signal. If kAuto, the signaled value will be auto-incremented.
     * @return Returns the signaled value.
     */
    uint64_t signal(uint64_t value = kAuto);

    /**
     * Wait for the fence to be signaled on the host.
     * Blocks the host until the fence reaches or exceeds the specified value.
     * @param value The value to wait for. If kAuto, wait for the last signaled value.
     * @param timeoutNs The timeout in nanoseconds. If kTimeoutInfinite, the function will block indefinitely.
     */
    void wait(uint64_t value = kAuto, uint64_t timeoutNs = kTimeoutInfinite);

    /// Returns the current value on the device.
    uint64_t getCurrentValue();

    /// Returns the latest signaled value (after auto-increment).
    uint64_t getSignaledValue() const { return mSignaledValue; }

    /**
     * Updates or increments the signaled value.
     * This is used before signaling a fence (from the host, on the device or
     * from an external source), to update the internal state.
     * The passed value is stored, or if value == kAuto, the last signaled
     * value is auto-incremented by one. The returned value is what the caller
     * should signal to the fence.
     * @param value The value to signal. If kAuto, the signaled value will be auto-incremented.
     * @return Returns the value to signal to the fence.
     */
    uint64_t updateSignaledValue(uint64_t value = kAuto);

    /**
     * Get the internal API handle
     */
    gfx::IFence* getGfxFence() const { return mGfxFence; }

    /**
     * Returns the native API handle:
     * - D3D12: ID3D12Fence*
     * - Vulkan: currently not supported
     */
    NativeHandle getNativeHandle() const;

    /**
     * Creates a shared fence API handle.
     */
    SharedResourceApiHandle getSharedApiHandle() const;

    Device* getDevice() const { return mpDevice.get(); }

    void breakStrongReferenceToDevice();

private:
    BreakableReference<Device> mpDevice;
    FenceDesc mDesc;
    Slang::ComPtr<gfx::IFence> mGfxFence;
    uint64_t mSignaledValue{0};
};
} // namespace Falcor
