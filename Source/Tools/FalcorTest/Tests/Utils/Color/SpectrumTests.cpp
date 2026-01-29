#include "Testing/UnitTest.h"
#include "Utils/Color/Spectrum.h"

namespace Falcor
{
CPU_TEST(SpectrumXYZ)
{
    // Make sure the integral of the CIE matching functions is 1.
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;
    for (float lambda = 360.f; lambda <= 830.f; lambda += 1.f)
    {
        x += Spectra::kCIE_X.eval(lambda);
        y += Spectra::kCIE_Y.eval(lambda);
        z += Spectra::kCIE_Z.eval(lambda);
    }
    x /= Spectra::kCIE_Y_Integral;
    y /= Spectra::kCIE_Y_Integral;
    z /= Spectra::kCIE_Y_Integral;
    EXPECT_LT(std::abs(1.f - x), 0.005f);
    EXPECT_LT(std::abs(1.f - y), 0.005f);
    EXPECT_LT(std::abs(1.f - z), 0.005f);
}
} // namespace Falcor
