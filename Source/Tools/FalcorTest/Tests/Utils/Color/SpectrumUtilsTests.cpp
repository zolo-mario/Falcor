#include "Testing/UnitTest.h"
#include "Utils/Color/SpectrumUtils.h"
#include <random>

namespace Falcor
{
namespace
{
const float kTestMinWavelength = 300.f;
const float kTestMaxWavelength = 900.f;
} // namespace

GPU_TEST(WavelengthToXYZ)
{
    std::mt19937 rng;
    auto dist = std::uniform_real_distribution<float>();
    auto u = [&]() { return dist(rng); };

    const uint32_t n = 20000;
    std::vector<float> wavelengths(n);

    for (uint32_t i = 0; i < n; i++)
    {
        float w = ((float)i + u()) / n;
        wavelengths[i] = kTestMinWavelength + w * (kTestMaxWavelength - kTestMinWavelength);
    }

    // Run GPU test.
    ctx.createProgram("Tests/Utils/Color/SpectrumUtilsTests.cs.slang", "testWavelengthToXYZ");
    ctx.allocateStructuredBuffer("result", n);
    ctx.allocateStructuredBuffer("wavelengths", n, wavelengths.data());
    ctx["CB"]["n"] = n;
    ctx.runProgram(uint3(n, 1, 1));

    // Verify results.
    float3 maxSqrError = {};
    std::vector<float3> result = ctx.readBuffer<float3>("result");
    for (uint32_t i = 0; i < n; i++)
    {
        float lambda = wavelengths[i];
        float3 res = result[i];
        float3 ref = SpectrumUtils::wavelengthToXYZ_CIE1931(wavelengths[i]);

        EXPECT_GE(res.x, 0.f);
        EXPECT_GE(res.y, 0.f);
        EXPECT_GE(res.z, 0.f);

        float3 e = ref - res;
        maxSqrError = max(maxSqrError, e * e);
    }

    EXPECT_LE(maxSqrError.x, 2.0e-4f);
    EXPECT_LE(maxSqrError.y, 6.6e-5f);
    EXPECT_LE(maxSqrError.z, 5.2e-4f);
}
} // namespace Falcor
