#include "Testing/UnitTest.h"
#include "Utils/Color/SampledSpectrum.h"

namespace Falcor
{
CPU_TEST(SampledSpectrum)
{
    auto s = SampledSpectrum<float>(400.f, 800.f, 5); // 400, 500, 600, 700, 800nm

    std::vector<float> samples = {0.f, 0.25f, 1.f, 0.75f, 0.f};
    s.set(samples);

    std::vector<std::pair<float, float>> testData = {
        // clang-format off
        {200.f, 0.f},
        {450.f, 0.125f},
        {600.f, 1.f},
        {725.f, 0.5625f},
        {800.f, 0.f},
        {810.f, 0.f},
        // clang-format on
    };

    for (auto t : testData)
    {
        float expected = t.second;
        float result = s.eval(t.first);
        EXPECT_EQ(result, expected);
    }
}
} // namespace Falcor
