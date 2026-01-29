#include "Testing/UnitTest.h"
#include <random>

namespace Falcor
{
namespace
{
std::vector<float3> testData = {
    // Some pre-determined values to test out-of-range behavior.
    // clang-format off
    {0, 0, 0},
    {1e-30f, 1e-30f, 1e-30f},
    {1e-10f, 1e-10f, 1e-10f},
    {1e10f, 1e10f, 1e10f},
    {1e30f, 1e30f, 1e30f},
    // clang-format on
    // We'll append random data here at runtime.
};
}

GPU_TEST(LogLuvHDR)
{
    std::mt19937 rng;
    auto dist = std::uniform_real_distribution<float>();
    auto u = [&]() { return dist(rng); };

    // Generate random colors within the supported dynamic range.
    for (size_t i = 0; i < 10000; i++)
    {
        float scale = std::pow(2.f, u() * 40.f - 20.f);
        float3 c = float3(u(), u(), u()) * scale;
        testData.push_back(c);
    }

    // Setup and run GPU test.
    ctx.createProgram("Tests/Utils/PackedFormatsTests.cs.slang", "testLogLuvHDR");
    ctx.allocateStructuredBuffer("testData", (uint32_t)testData.size(), testData.data(), testData.size() * sizeof(testData[0]));
    ctx.allocateStructuredBuffer("result", (uint32_t)testData.size());
    ctx.runProgram((uint32_t)testData.size());

    // Verify results.
    std::vector<float3> result = ctx.readBuffer<float3>("result");

    // Test that small are reproduced as exactly zero.
    for (size_t i = 0; i < 3; i++)
    {
        EXPECT_EQ(result[i], float3(0));
    }

    // Test that above range values are clamped to the maximum, roughly 2^20 = 1.05e6f.
    for (size_t i = 3; i < 5; i++)
    {
        EXPECT_GE(result[i].x, 1.0e6f) << "i = " << i;
        EXPECT_GE(result[i].y, 1.0e6f) << "i = " << i;
        EXPECT_GE(result[i].z, 1.0e6f) << "i = " << i;

        EXPECT_LE(result[i].x, 1.1e6f) << "i = " << i;
        EXPECT_LE(result[i].y, 1.1e6f) << "i = " << i;
        EXPECT_LE(result[i].z, 1.1e6f) << "i = " << i;
    }

    // Test that valid colors are accurately reproduced.
    for (size_t i = 5; i < testData.size(); i++)
    {
        float threshold = std::max(std::max(testData[i].x, testData[i].y), testData[i].z) * 0.0105f;

        // Define min/max expected value based on threshold derived from the max component.
        // Note that color components with very low values may get clamped to zero.
        auto expMin = [=](float v) { return v > 1e-5f ? std::max(0.f, v - threshold) : 0.f; };
        auto expMax = [=](float v) { return v + threshold; };

        EXPECT_GE(result[i].x, expMin(testData[i].x)) << "i = " << i;
        EXPECT_GE(result[i].y, expMin(testData[i].y)) << "i = " << i;
        EXPECT_GE(result[i].z, expMin(testData[i].z)) << "i = " << i;

        EXPECT_LE(result[i].x, expMax(testData[i].x)) << "i = " << i;
        EXPECT_LE(result[i].y, expMax(testData[i].y)) << "i = " << i;
        EXPECT_LE(result[i].z, expMax(testData[i].z)) << "i = " << i;
    }
}
} // namespace Falcor
