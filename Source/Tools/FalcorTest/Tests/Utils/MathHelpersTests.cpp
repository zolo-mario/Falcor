#include "Testing/UnitTest.h"
#include "Utils/Math/ScalarMath.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <vector>

namespace Falcor
{
GPU_TEST(MathHelpers_SphericalCoordinates)
{
    ctx.createProgram("Tests/Utils/MathHelpersTests.cs.slang", "testSphericalCoordinates");
    constexpr int32_t n = 1024 * 1024;
    ctx.allocateStructuredBuffer("result", n);
    // The shader runs threadgroups of 1024 threads.
    ctx.runProgram(n);

    // The shader generates a bunch of random vectors, converts them to
    // spherical coordinates and back, and computes the dot product with
    // the original vector.  Here, we'll check that the dot product is
    // pretty close to one.
    std::vector<float> r = ctx.readBuffer<float>("result");
    for (int32_t i = 0; i < n; ++i)
    {
        EXPECT_GT(r[i], .999f) << "i = " << i;
        EXPECT_LT(r[i], 1.001f) << "i = " << i;
    }
}

GPU_TEST(MathHelpers_SphericalCoordinatesRad)
{
    ctx.createProgram("Tests/Utils/MathHelpersTests.cs.slang", "testSphericalCoordinatesRad");
    constexpr int32_t n = 1024 * 1024;
    ctx.allocateStructuredBuffer("result", n);
    // The shader runs threadgroups of 1024 threads.
    ctx.runProgram(n);

    // The shader generates a bunch of random vectors, converts them to
    // spherical coordinates and back, and computes the dot product with
    // the original vector.  Here, we'll check that the dot product is
    // pretty close to one.
    std::vector<float> r = ctx.readBuffer<float>("result");
    for (int32_t i = 0; i < n; ++i)
    {
        EXPECT_GT(r[i], .999f) << "i = " << i;
        EXPECT_LT(r[i], 1.001f) << "i = " << i;
    }
}

GPU_TEST(MathHelpers_ErrorFunction)
{
    // Test the approximate implementation of `erf` against
    // the C++ standard library.
    ctx.createProgram("Tests/Utils/MathHelpersTests.cs.slang", "testErrorFunction");
    constexpr int32_t n = 25;
    std::vector<float> input(n);
    std::vector<float> ref(n);
    for (int32_t i = 0; i < n; ++i)
    {
        float t = i / (float)(n - 1);
        float x = math::lerp<float>(-5, 5, t);
        input[i] = x;
        ref[i] = std::erf(x);
    }

    ctx.allocateStructuredBuffer("result", n);
    ctx.allocateStructuredBuffer("input", (uint32_t)input.size(), input.data());

    ctx.runProgram(n);

    std::vector<float> r = ctx.readBuffer<float>("result");
    float epsilon = 1e-6f;
    for (int32_t i = 0; i < n; ++i)
    {
        EXPECT_GE(r[i], ref[i] - epsilon) << "i = " << i;
        EXPECT_LE(r[i], ref[i] + epsilon) << "i = " << i;
    }
}

GPU_TEST(MathHelpers_InverseErrorFunction)
{
    // The C++ standard library does not have a reference for `erfinv`,
    // but we can test erf(erfinv(x)) = x instead.
    ctx.createProgram("Tests/Utils/MathHelpersTests.cs.slang", "testInverseErrorFunction");
    constexpr int32_t n = 25;
    std::vector<float> input(n);
    for (int32_t i = 0; i < n; ++i)
    {
        float t = i / (float)(n - 1);
        input[i] = math::lerp<float>(-1, 1, t);
    }

    ctx.allocateStructuredBuffer("result", n);
    ctx.allocateStructuredBuffer("input", (uint32_t)input.size(), input.data());

    ctx.runProgram(n);

    std::vector<float> r = ctx.readBuffer<float>("result");
    float epsilon = 1e-6f;
    for (int32_t i = 0; i < n; ++i)
    {
        EXPECT_GE(std::erf(r[i]), input[i] - epsilon) << "i = " << i;
        EXPECT_LE(std::erf(r[i]), input[i] + epsilon) << "i = " << i;
    }
}
} // namespace Falcor
