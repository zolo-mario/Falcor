#include "BitonicSort.h"
#include "Core/Macros.h"
#include "Core/API/RenderContext.h"
#include "Utils/Math/Common.h"
#include "Utils/Timing/Profiler.h"

namespace Falcor
{
static const char kShaderFilename[] = "Utils/Algorithm/BitonicSort.cs.slang";

BitonicSort::BitonicSort(ref<Device> pDevice) : mpDevice(pDevice)
{
#if !FALCOR_NVAPI_AVAILABLE
    FALCOR_THROW("BitonicSort requires NVAPI. See installation instructions in README.");
#endif
    mSort.pState = ComputeState::create(mpDevice);

    // Create shaders
    DefineList defines;
    defines.add("CHUNK_SIZE", "256"); // Dummy values just so we can get reflection data. We'll set the actual values in execute().
    defines.add("GROUP_SIZE", "256");
    mSort.pProgram = Program::createCompute(mpDevice, kShaderFilename, "main", defines);
    mSort.pState->setProgram(mSort.pProgram);
    mSort.pVars = ProgramVars::create(mpDevice, mSort.pProgram.get());
}

bool BitonicSort::execute(RenderContext* pRenderContext, ref<Buffer> pData, uint32_t totalSize, uint32_t chunkSize, uint32_t groupSize)
{
    FALCOR_PROFILE(pRenderContext, "BitonicSort::execute");

    // Validate inputs.
    FALCOR_ASSERT(pRenderContext);
    FALCOR_ASSERT(pData);
    FALCOR_ASSERT(chunkSize >= 1 && chunkSize <= groupSize && isPowerOf2(chunkSize));
    FALCOR_ASSERT(groupSize >= 1 && groupSize <= 1024 && isPowerOf2(groupSize));

    // Early out if there is nothing to be done.
    if (totalSize == 0 || chunkSize <= 1)
        return true;

    // Configure the shader for the specified chunk size.
    // This will trigger a re-compile if a new chunk size is encountered.
    mSort.pProgram->addDefine("CHUNK_SIZE", std::to_string(chunkSize));
    mSort.pProgram->addDefine("GROUP_SIZE", std::to_string(groupSize));

    // Determine dispatch dimensions.
    const uint32_t numGroups = div_round_up(totalSize, groupSize);
    const uint32_t groupsX = std::max((uint32_t)sqrt(numGroups), 1u);
    const uint32_t groupsY = div_round_up(numGroups, groupsX);
    FALCOR_ASSERT(groupsX * groupsY * groupSize >= totalSize);

    // Constants. The buffer size as a runtime constant as it may be variable and we don't want to recompile each time it changes.
    auto var = mSort.pVars->getRootVar();
    var["CB"]["gTotalSize"] = totalSize;
    var["CB"]["gDispatchX"] = groupsX;

    // Bind the data.
    var["gData"] = pData;

    // Execute.
    pRenderContext->dispatch(mSort.pState.get(), mSort.pVars.get(), {groupsX, groupsY, 1});

    return true;
}
} // namespace Falcor
