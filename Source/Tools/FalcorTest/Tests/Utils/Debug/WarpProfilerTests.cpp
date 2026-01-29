#include "Testing/UnitTest.h"
#include "Utils/Debug/WarpProfiler.h"

namespace Falcor
{
GPU_TEST(WarpProfiler, Device::Type::D3D12)
{
    WarpProfiler profiler(ctx.getDevice(), 4);

    ProgramDesc desc;
    desc.addShaderLibrary("Tests/Utils/Debug/WarpProfilerTests.cs.slang").csEntry("main");
    desc.setShaderModel(ShaderModel::SM6_5); // Minimum required shader model.
    ctx.createProgram(desc);

    auto var = ctx.vars().getRootVar();
    profiler.bindShaderData(var);
    profiler.begin(ctx.getRenderContext());

    ctx.runProgram(256, 256, 16); // Launch 2^20 threads = 32768 warps.

    profiler.end(ctx.getRenderContext());

    // Utilization
    {
        auto histogram = profiler.getWarpHistogram(0);
        EXPECT_EQ(histogram.size(), WarpProfiler::kWarpSize);

        size_t warpCount = 0;
        for (auto h : histogram)
        {
            warpCount += h;
        }
        EXPECT_EQ(histogram[31], 32768);
        EXPECT_EQ(warpCount, 32768);
    }

    {
        auto histogram = profiler.getWarpHistogram(1);
        EXPECT_EQ(histogram.size(), WarpProfiler::kWarpSize);

        size_t warpCount = 0;
        for (auto h : histogram)
        {
            warpCount += h;
        }
        EXPECT_EQ(histogram[7], 16384);
        EXPECT_EQ(warpCount, 16384);
    }

    {
        auto histogram = profiler.getWarpHistogram(0, 2);
        EXPECT_EQ(histogram.size(), WarpProfiler::kWarpSize);

        EXPECT_EQ(histogram[7], 16384);
        EXPECT_EQ(histogram[31], 32768);
    }

    // Divergence
    {
        auto histogram = profiler.getWarpHistogram(2);
        EXPECT_EQ(histogram.size(), WarpProfiler::kWarpSize);

        size_t warpCount = 0;
        for (auto h : histogram)
        {
            warpCount += h;
        }
        EXPECT_EQ(histogram[3], 32768);
        EXPECT_EQ(warpCount, 32768);
    }

    {
        auto histogram = profiler.getWarpHistogram(3);
        EXPECT_EQ(histogram.size(), WarpProfiler::kWarpSize);

        size_t warpCount = 0;
        for (auto h : histogram)
        {
            warpCount += h;
        }
        EXPECT_EQ(histogram[7], 8192);
        EXPECT_EQ(warpCount, 8192);
    }
}
} // namespace Falcor
