#include "Fence.h"
#include "Device.h"
#include "GFXAPI.h"
#include "NativeHandleTraits.h"
#include "Core/Error.h"

namespace Falcor
{
Fence::Fence(ref<Device> pDevice, FenceDesc desc) : mpDevice(pDevice), mDesc(desc)
{
    FALCOR_ASSERT(mpDevice);
    gfx::IFence::Desc gfxDesc = {};
    mSignaledValue = mDesc.initialValue;
    gfxDesc.isShared = mDesc.shared;
    FALCOR_GFX_CALL(mpDevice->getGfxDevice()->createFence(gfxDesc, mGfxFence.writeRef()));
}

Fence::~Fence() = default;

uint64_t Fence::signal(uint64_t value)
{
    uint64_t signalValue = updateSignaledValue(value);
    FALCOR_GFX_CALL(mGfxFence->setCurrentValue(signalValue));
    return signalValue;
}

void Fence::wait(uint64_t value, uint64_t timeoutNs)
{
    uint64_t waitValue = value == kAuto ? mSignaledValue : value;
    uint64_t currentValue = getCurrentValue();
    if (currentValue >= waitValue)
        return;
    gfx::IFence* fences[] = {mGfxFence};
    uint64_t waitValues[] = {waitValue};
    FALCOR_GFX_CALL(mpDevice->getGfxDevice()->waitForFences(1, fences, waitValues, true, timeoutNs));
}

uint64_t Fence::getCurrentValue()
{
    uint64_t value;
    FALCOR_GFX_CALL(mGfxFence->getCurrentValue(&value));
    return value;
}

uint64_t Fence::updateSignaledValue(uint64_t value)
{
    mSignaledValue = value == kAuto ? mSignaledValue + 1 : value;
    return mSignaledValue;
}

SharedResourceApiHandle Fence::getSharedApiHandle() const
{
    gfx::InteropHandle sharedHandle;
    FALCOR_GFX_CALL(mGfxFence->getSharedHandle(&sharedHandle));
    return (SharedResourceApiHandle)sharedHandle.handleValue;
}

NativeHandle Fence::getNativeHandle() const
{
    gfx::InteropHandle gfxNativeHandle = {};
    FALCOR_GFX_CALL(mGfxFence->getNativeHandle(&gfxNativeHandle));
#if FALCOR_HAS_D3D12
    if (mpDevice->getType() == Device::Type::D3D12)
        return NativeHandle(reinterpret_cast<ID3D12Fence*>(gfxNativeHandle.handleValue));
#endif
#if FALCOR_HAS_VULKAN
        // currently not supported
#endif
    return {};
}

void Fence::breakStrongReferenceToDevice()
{
    mpDevice.breakStrongReference();
}

} // namespace Falcor
