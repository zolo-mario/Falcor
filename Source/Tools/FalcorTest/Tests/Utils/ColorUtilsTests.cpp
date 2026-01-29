#include "Testing/UnitTest.h"
#include "Utils/Color/ColorUtils.h"
#include <random>

namespace Falcor
{
// Some shared test utils.
namespace
{
const float kMaxError = 1e-5f;

auto maxAbsDiff = [](float3 a, float3 b) -> float
{
    float3 d = abs(a - b);
    return std::max(std::max(d.x, d.y), d.z);
};
} // namespace

CPU_TEST(ColorTransforms)
{
    const uint32_t n = 10000;

    // Prepare for tests.
    std::default_random_engine rng;
    auto dist = std::uniform_real_distribution<float>();
    auto u = [&]() -> float { return dist(rng); };

    const float3x3 LMS_CAT02 = mul(kColorTransform_LMStoXYZ_CAT02, kColorTransform_XYZtoLMS_CAT02);
    const float3x3 LMS_Bradford = mul(kColorTransform_LMStoXYZ_Bradford, kColorTransform_XYZtoLMS_Bradford);

    // Run test code that transforms random colors between different spaces.
    for (uint32_t i = 0; i < n; i++)
    {
        const float3 c = {u(), u(), u()};

        // Test RGB<->XYZ by transforming random colors back and forth.
        float3 res1 = XYZtoRGB_Rec709(RGBtoXYZ_Rec709(c));
        EXPECT_LE(maxAbsDiff(res1, c), kMaxError);

        // Test XYZ<->LMS using the CAT02 transform.
        float3 res2 = mul(LMS_CAT02, c);
        EXPECT_LE(maxAbsDiff(res2, c), kMaxError);

        // Test XYZ<->LMS using the Bradford transform
        float3 res3 = mul(LMS_Bradford, c);
        EXPECT_LE(maxAbsDiff(res3, c), kMaxError);
    }
}

CPU_TEST(WhiteBalance)
{
    const float3 white = {1, 1, 1};

    // The white point should be 6500K. Verify that we get pure white back.
    float3 wbWhite = mul(calculateWhiteBalanceTransformRGB_Rec709(6500.f), white);
    EXPECT_LE(maxAbsDiff(wbWhite, white), kMaxError);

    // Test white balance transform at a few different color temperatures.
    // This is a  very crude test just to see we're not entirely off.
    //
    // Color correcting white @ 6500K to these targets should yield:
    // - Cloudy (7000K) => yellowish tint (r > g > b)
    // - Sunny  (5500K) => blueish tint (r < g < b)
    // - Indoor (3000K) => stronger bluish tint (r < g < b)
    float3 wbCloudy = mul(calculateWhiteBalanceTransformRGB_Rec709(7000.f), white);
    float3 wbSunny = mul(calculateWhiteBalanceTransformRGB_Rec709(5500.f), white);
    float3 wbIndoor = mul(calculateWhiteBalanceTransformRGB_Rec709(3000.f), white);

    EXPECT_GE(wbCloudy.r, wbCloudy.g);
    EXPECT_GE(wbCloudy.g, wbCloudy.b);

    EXPECT_LE(wbSunny.r, wbSunny.g);
    EXPECT_LE(wbSunny.g, wbSunny.b);

    EXPECT_LE(wbIndoor.r, wbIndoor.g);
    EXPECT_LE(wbIndoor.g, wbIndoor.b);

    // Normalize the returned RGB to max 1.0 to be able to compare the scale.
    wbSunny /= wbSunny.b;
    wbIndoor /= wbIndoor.b;

    EXPECT_LE(wbIndoor.r, wbSunny.r);
    EXPECT_LE(wbIndoor.g, wbSunny.g);
}
} // namespace Falcor
