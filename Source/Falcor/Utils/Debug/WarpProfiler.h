#pragma once

#include "Core/Macros.h"
#include "Core/API/Buffer.h"
#include "Core/API/Fence.h"

#include <filesystem>
#include <vector>

namespace Falcor
{

class RenderContext;
struct ShaderVar;

/**
 * @brief Utility class for warp-level profiling.
 */
class FALCOR_API WarpProfiler
{
public:
    static constexpr size_t kWarpSize = 32; // Do not change

    /**
     * @brief Construct new warp profiler object.
     * @param[in] pDevice GPU device.
     * @param[in] binCount Number of profiling bins.
     */
    WarpProfiler(ref<Device> pDevice, const uint32_t binCount);

    /**
     * @brief Binds the profiler data to shader vars.
     * This function must be called before the profiler can be used.
     * @param[in] var Shader vars of the program to set data into.
     */
    void bindShaderData(const ShaderVar& var) const;

    /**
     * @brief Begin profiling.
     * @param[in] pRenderContext The context.
     */
    void begin(RenderContext* pRenderContext);

    /**
     * @brief End profiling.
     * @param[in] pRenderContext The context.
     */
    void end(RenderContext* pRenderContext);

    /**
     * @brief Compute warp histogram over a range of profiling bins.
     * @param[in] binIndex Index of first profiling bin for histogram.
     * @param[in] binCount Number of profiling bins to include in histogram.
     * @return Histogram with `kWarpSize` buckets. The first bucket number of warps with 1 counted element, the last
     * bucket represents number of warps with `kWarpSize` counted elements.
     */
    std::vector<uint32_t> getWarpHistogram(const uint32_t binIndex, const uint32_t binCount = 1);

    /**
     * Save warp histograms for all profiling bins to file in CSV format.
     * @param[in] path File path.
     * @return True if successful, false otherwise.
     */
    bool saveWarpHistogramsAsCSV(const std::filesystem::path& path);

private:
    void readBackData();

    ref<Fence> mpFence;
    ref<Buffer> mpHistogramBuffer;
    ref<Buffer> mpHistogramStagingBuffer;

    const uint32_t mBinCount;          ///< Number of profiling bins.
    std::vector<uint32_t> mHistograms; ///< Histograms for all profiling bins.

    bool mActive = false;      ///< True while inside a begin()/end() section.
    bool mDataWaiting = false; ///< True when data is waiting for readback in the staging buffer.
};
} // namespace Falcor
