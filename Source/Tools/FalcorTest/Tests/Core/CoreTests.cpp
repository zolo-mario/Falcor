#include "Testing/UnitTest.h"

namespace Falcor
{
GPU_TEST(TransientHeapRecycling)
{
    ref<Device> pDevice = ctx.getDevice();
    RenderContext* pRenderContext = pDevice->getRenderContext();

    size_t M = 1024 * 1024 * 1024;
    std::vector<uint8_t> cpuBuf(M, 0);
    ref<Buffer> A = pDevice->createBuffer(M, ResourceBindFlags::None, MemoryType::DeviceLocal, cpuBuf.data());
    ref<Buffer> B = pDevice->createBuffer(4, ResourceBindFlags::None, MemoryType::DeviceLocal);

    // Progress through N frames (and transient heaps), ending up using the
    // same transient heap as is used for uploading the data to buffer A.
    // Before the fix, this leads to a validation error as the buffer for
    // uplading to buffer A is still in flight.
    for (uint32_t i = 0; i < Device::kInFlightFrameCount; ++i)
        pDevice->endFrame();

    // The following commands will trigger a TDR even if the validation error
    // is missed.
    pRenderContext->copyBufferRegion(B.get(), 0, A.get(), 0, 4);
    pRenderContext->submit(true);
    // A->map();
    // A->unmap();
}
} // namespace Falcor
