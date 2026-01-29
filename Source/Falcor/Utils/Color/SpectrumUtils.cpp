#include "SpectrumUtils.h"
#include "Utils/Color/ColorUtils.h"

#include <xyzcurves/ciexyzCurves1931_1nm.h>
#include <illuminants/D65_5nm.h>

namespace Falcor
{
// Initialize static data.
// clang-format off
const SampledSpectrum<float3> SpectrumUtils::sCIE_XYZ_1931_1nm(360.0f, 830.0f, 471, reinterpret_cast<const float3*>(xyz1931_1nm));  // 1 nm between samples.
const SampledSpectrum<float> SpectrumUtils::sD65_5nm(300.0f, 830.0f, 107, reinterpret_cast<const float*>(D65_1nm));                 // 5 nm between samples.
// clang-format on

float3 SpectrumUtils::wavelengthToXYZ_CIE1931(float lambda)
{
    return sCIE_XYZ_1931_1nm.eval(lambda);
}

float SpectrumUtils::wavelengthToD65(float lambda)
{
    return sD65_5nm.eval(lambda);
}

float3 SpectrumUtils::wavelengthToRGB_Rec709(const float lambda)
{
    float3 XYZ = wavelengthToXYZ_CIE1931(lambda);
    return XYZtoRGB_Rec709(XYZ);
}
} // namespace Falcor
