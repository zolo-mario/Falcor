#include "Testing/UnitTest.h"

namespace Falcor
{
#if FALCOR_HAS_AFTERMATH
GPU_TEST(AftermathCatchTDR)
{
    ref<Device> pDevice = ctx.getDevice();

    if (!pDevice->getAftermathContext())
        ctx.skip("Aftermath is not enabled");

    ctx.createProgram("Tests/Core/AftermathTests.cs.slang", "main");

    std::vector<uint32_t> data(1024, 1);
    ctx.allocateStructuredBuffer("result", 1024, data.data());

    ctx.getRenderContext()->addAftermathMarker("before");

    // This should force a TDR (timeout detection & recovery).
    ctx.runProgram(32 * 1024, 1024);

    ctx.getRenderContext()->addAftermathMarker("after");

    pDevice->wait();

    // At this point we have lost the device, so submitting another dispatch should terminate the application.
    ctx.runProgram(1024, 1024);
}
#endif

} // namespace Falcor
