# Color - Color and Spectrum Utilities

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **ColorHelpers** - Color helper functions
- [x] **ColorMap** - Color mapping utilities
- [x] **ColorUtils** - Color conversion utilities
- [x] **SampledSpectrum** - Sampled spectrum representation
- [x] **Spectrum** - Spectrum representation
- [x] **SpectrumUtils** - Spectrum utilities

### External Dependencies

- **Utils/Math** - Mathematical utilities (Vector, Matrix, Common)
- **Core/Error** - Error handling

## Module Overview

The Color module provides comprehensive color and spectrum management for the Falcor rendering framework. It includes color space conversions (RGB, XYZ, LMS), spectrum representations (piecewise linear, densely sampled, blackbody), color matching functions (CIE 1931), illuminant data (D65), and color temperature calculations. These utilities are essential for physically-based rendering, spectral rendering, and accurate color reproduction.

## Component Specifications

### ColorUtils

**Files**:
- [`ColorUtils.h`](Source/Falcor/Utils/Color/ColorUtils.h:1) - Color utilities header

**Purpose**: Color space conversion and color temperature utilities.

**Key Features**:
- RGB to XYZ conversion (Rec.709)
- XYZ to RGB conversion (Rec.709)
- XYZ to LMS conversion (CAT02 and Bradford transforms)
- Color temperature to XYZ conversion
- White balance transform calculation
- Chromaticity conversions (xyY to XYZ)

**Core Functions**:

**Color Space Conversions**:
- [`RGBtoXYZ_Rec709(float3 c)`](Source/Falcor/Utils/Color/ColorUtils.h:123) - Transform RGB color in Rec.709 to CIE XYZ
- [`XYZtoRGB_Rec709(float3 c)`](Source/Falcor/Utils/Color/ColorUtils.h:131) - Transform XYZ color to RGB in Rec.709
- [`xyYtoXYZ(float x, float y, float Y)`](Source/Falcor/Utils/Color/ColorUtils.h:139) - Convert (chromaticities, luminance) to XYZ color

**Color Temperature**:
- [`colorTemperatureToXYZ(float T, float Y = 1.f)`](Source/Falcor/Utils/Color/ColorUtils.h:154) - Transform color temperature to CIE XYZ
  - Parameters:
    - `T` - Color temperature in degrees Kelvin (1667K to 25000K)
    - `Y` - Luminance (default 1.0)
  - Uses piecewise rational polynomial approximation from Kang et al. (2002)

**White Balance**:
- [`calculateWhiteBalanceTransformRGB_Rec709(float T)`](Source/Falcor/Utils/Color/ColorUtils.h:209) - Calculate white balance transform matrix
  - Parameters:
    - `T` - Target color temperature in Kelvin
  - Returns: 3x3 transform matrix M for c' = M * c
  - Uses von Kries transform with CAT02 LMS space
  - Preserves D65 white point at T=6500K
  - May produce out-of-gamut RGB values

**Data Structures**:

**Transform Matrices**:
- [`kColorTransform_RGBtoXYZ_Rec709`](Source/Falcor/Utils/Color/ColorUtils.h:67) - RGB Rec.709 to CIE XYZ transform
- [`kColorTransform_XYZtoRGB_Rec709`](Source/Falcor/Utils/Color/ColorUtils.h:76) - CIE XYZ to RGB Rec.709 transform
- [`kColorTransform_XYZtoLMS_CAT02`](Source/Falcor/Utils/Color/ColorUtils.h:85) - CIE XYZ to LMS using CAT02 transform
- [`kColorTransform_LMStoXYZ_CAT02`](Source/Falcor/Utils/Color/ColorUtils.h:94) - LMS to CIE XYZ using inverse CAT02 transform
- [`kColorTransform_XYZtoLMS_Bradford`](Source/Falcor/Utils/Color/ColorUtils.h:103) - CIE XYZ to LMS using Bradford transform
- [`kColorTransform_LMStoXYZ_Bradford`](Source/Falcor/Utils/Color/ColorUtils.h:112) - LMS to CIE XYZ using inverse Bradford transform

**Technical Details**:

**Color Space Assumptions**:
- All input/outputs are in sRGB
- sRGB uses ITU-R Rec. BT.709 (Rec.709) color space
- D65 white point (6500K) is the reference

**Color Temperature Approximation**:
- Piecewise rational polynomial approximation
- Range: 1667K to 25000K
- Two regimes: T < 4000K and T >= 4000K
- Uses double precision for intermediate calculations

**White Balance Algorithm**:
1. Convert RGB to LMS space (CAT02 transform)
2. Compute source white point in LMS space
3. Compute destination white point (D65) in LMS space
4. Calculate diagonal scaling matrix in LMS space
5. Transform back to RGB space

**Use Cases**:
- Color space conversions for rendering
- White balance adjustment
- Color temperature correction
- Physically-based lighting calculations
- Color grading operations

### SampledSpectrum

**Files**:
- [`SampledSpectrum.h`](Source/Falcor/Utils/Color/SampledSpectrum.h:1) - Sampled spectrum header

**Purpose**: Uniformly sampled spectrum representation with template-based value types.

**Key Features**:
- Template-based value type (float, float2, float3, float4)
- Uniform wavelength sampling
- Linear interpolation evaluation
- Zero outside wavelength range
- CIE 1931 XYZ conversion support

**Core Methods**:

**Constructors**:
- [`SampledSpectrum(float lambdaStart, float lambdaEnd, size_t sampleCount)`](Source/Falcor/Utils/Color/SampledSpectrum.h:68) - Create zero-initialized spectrum
  - Parameters:
    - `lambdaStart` - First sampled wavelength in nm
    - `lambdaEnd` - Last sampled wavelength in nm
    - `sampleCount` - Number of wavelength samples

- [`SampledSpectrum(float lambdaStart, float lambdaEnd, size_t sampleCount, const value_type* pSamples)`](Source/Falcor/Utils/Color/SampledSpectrum.h:82) - Create from sample array
  - Parameters:
    - `lambdaStart` - First sampled wavelength in nm
    - `lambdaEnd` - Last sampled wavelength in nm
    - `sampleCount` - Number of wavelength samples
    - `pSamples` - Spectral samples

**Sample Management**:
- [`set(const size_t sampleCount, const value_type* pSamples)`](Source/Falcor/Utils/Color/SampledSpectrum.h:93) - Set spectrum samples from array
- [`set(const std::vector<value_type>& samples)`](Source/Falcor/Utils/Color/SampledSpectrum.h:104) - Set spectrum samples from vector
- [`set(const size_t sampleCount, const value_type* pSamples, const float* pLambdas)`](Source/Falcor/Utils/Color/SampledSpectrum.h:117) - Set from unsorted spectral data (unimplemented)
- [`set(const std::vector<value_type>& samples, const std::vector<float>& lambdas)`](Source/Falcor/Utils/Color/SampledSpectrum.h:129) - Set from unsorted spectral data (unimplemented)

**Evaluation**:
- [`eval(const float lambda, const SpectrumInterpolation interpolationType = SpectrumInterpolation::Linear) const`](Source/Falcor/Utils/Color/SampledSpectrum.h:143) - Evaluate spectrum at wavelength
  - Parameters:
    - `lambda` - Wavelength in nm
    - `interpolationType` - Interpolation type (Linear only supported)
  - Returns: Interpolated value (zero outside range)

**Conversion**:
- [`toXYZ_CIE1931() const`](Source/Falcor/Utils/Color/SampledSpectrum.h:149) - Return CIE 1931 tristimulus values (unimplemented)

**Accessors**:
- [`size() const`](Source/Falcor/Utils/Color/SampledSpectrum.h:155) - Return number of samples
- [`get(size_t index) const`](Source/Falcor/Utils/Color/SampledSpectrum.h:162) - Get sample by index
- [`set(size_t index, T value)`](Source/Falcor/Utils/Color/SampledSpectrum.h:173) - Set sample by index
- [`getWavelengthRange() const`](Source/Falcor/Utils/Color/SampledSpectrum.h:183) - Return wavelength range

**Data Structures**:

**SampledSpectrum Class**:
- [`mLambdaStart`](Source/Falcor/Utils/Color/SampledSpectrum.h:186) - First wavelength sample in nm
- [`mLambdaEnd`](Source/Falcor/Utils/Color/SampledSpectrum.h:187) - Last wavelength sample in nm
- [`mSamples`](Source/Falcor/Utils/Color/SampledSpectrum.h:188) - Sample values vector

**SpectrumInterpolation Enum**:
- [`Linear`](Source/Falcor/Utils/Color/SampledSpectrum.h:40) - Piecewise linear interpolation of two nearest samples, zero outside endpoints

**Technical Details**:

**Uniform Sampling**:
- First sample centered at lambdaStart
- Last sample centered at lambdaEnd
- Example: lambdaStart=400nm, lambdaEnd=700nm, sampleCount=4
  - Bins centered at 400, 500, 600, 700 nm
  - Bins: [400,450], [450,550], [550,650], [650,700]

**Evaluation Algorithm**:
1. Check if wavelength is outside range (return 0)
2. Compute normalized position: x = ((lambda - lambdaStart) / (lambdaEnd - lambdaStart)) * (size - 1)
3. Find lower index: i = floor(x)
4. Compute interpolation weight: w = x - i
5. Linear interpolation: lerp(mSamples[i], mSamples[i+1], w)

**Type Constraints**:
- Value type must be floating point scalar or vector
- Supported: float, float2, float3, float4

**Use Cases**:
- Spectral rendering
- Color matching functions
- Illuminant data storage
- Material spectral properties
- Light spectral distributions

### Spectrum

**Files**:
- [`Spectrum.h`](Source/Falcor/Utils/Color/Spectrum.h:1) - Spectrum header
- [`Spectrum.cpp`](Source/Falcor/Utils/Color/Spectrum.cpp:1) - Spectrum implementation
- [`Spectra.inl`](Source/Falcor/Utils/Color/Spectra.inl:1) - Spectra data inline file

**Purpose**: Various spectrum representations (piecewise linear, densely sampled, blackbody).

**Key Features**:
- Piecewise linear spectrum
- Densely sampled spectrum
- Blackbody emission spectrum
- CIE 1931 color matching functions
- Spectrum to XYZ/RGB conversion
- File I/O for spectrum data

**Core Classes**:

**PiecewiseLinearSpectrum**:
- [`PiecewiseLinearSpectrum(fstd::span<const float> wavelengths, fstd::span<const float> values)`](Source/Falcor/Utils/Color/Spectrum.h:53) - Create from wavelength and value arrays
- [`fromInterleaved(fstd::span<const float> interleaved, bool normalize)`](Source/Falcor/Utils/Color/Spectrum.h:62) - Create from interleaved data [w0,v0,w1,v1,...]
- [`fromFile(const std::filesystem::path& path)`](Source/Falcor/Utils/Color/Spectrum.h:70) - Create from text file
- [`scale(float factor)`](Source/Falcor/Utils/Color/Spectrum.h:76) - Scale all values by constant
- [`eval(float wavelength) const`](Source/Falcor/Utils/Color/Spectrum.h:84) - Evaluate at wavelength (linear interpolation, zero outside range)
- [`getWavelengthRange() const`](Source/Falcor/Utils/Color/Spectrum.h:108) - Return wavelength range
- [`getMaxValue() const`](Source/Falcor/Utils/Color/Spectrum.h:114) - Return maximum value

**DenseleySampledSpectrum**:
- [`DenseleySampledSpectrum(float minWavelength, float maxWavelength, fstd::span<const float> values)`](Source/Falcor/Utils/Color/Spectrum.h:128) - Create from value array
- [`DenseleySampledSpectrum(const S& spectrum, float wavelengthStep = 1.f)`](Source/Falcor/Utils/Color/Spectrum.h:137) - Create from another spectrum with given wavelength step
- [`eval(float wavelength) const`](Source/Falcor/Utils/Color/Spectrum.h:159) - Evaluate at wavelength (nearest neighbor, zero outside range)
- [`getWavelengthRange() const`](Source/Falcor/Utils/Color/Spectrum.h:171) - Return wavelength range
- [`getMaxValue() const`](Source/Falcor/Utils/Color/Spectrum.h:177) - Return maximum value

**BlackbodySpectrum**:
- [`BlackbodySpectrum(float temperature, bool normalize = true)`](Source/Falcor/Utils/Color/Spectrum.h:206) - Create blackbody emission spectrum
  - Parameters:
    - `temperature` - Temperature in Kelvin
    - `normalize` - Normalize so peak value is 1
- [`eval(float wavelength) const`](Source/Falcor/Utils/Color/Spectrum.h:213) - Evaluate at wavelength
- [`getWavelengthRange() const`](Source/Falcor/Utils/Color/Spectrum.h:219) - Return wavelength range (infinite)
- [`getMaxValue() const`](Source/Falcor/Utils/Color/Spectrum.h:225) - Return maximum value

**Data Structures**:

**PiecewiseLinearSpectrum**:
- [`mWavelengths`](Source/Falcor/Utils/Color/Spectrum.h:117) - Wavelengths in nm (increasing order)
- [`mValues`](Source/Falcor/Utils/Color/Spectrum.h:118) - Values at each wavelength
- [`mMaxValue`](Source/Falcor/Utils/Color/Spectrum.h:119) - Maximum value in mValues

**DenseleySampledSpectrum**:
- [`mMinWavelength`](Source/Falcor/Utils/Color/Spectrum.h:180) - Minimum wavelength
- [`mMaxWavelength`](Source/Falcor/Utils/Color/Spectrum.h:181) - Maximum wavelength
- [`mWavelengthStep`](Source/Falcor/Utils/Color/Spectrum.h:182) - Wavelength step between samples
- [`mValues`](Source/Falcor/Utils/Color/Spectrum.h:183) - Sample values
- [`mMaxValue`](Source/Falcor/Utils/Color/Spectrum.h:184) - Maximum value

**BlackbodySpectrum**:
- [`mTemperature`](Source/Falcor/Utils/Color/Spectrum.h:228) - Temperature in Kelvin
- [`mNormalizationFactor`](Source/Falcor/Utils/Color/Spectrum.h:229) - Normalization factor
- [`mMaxValue`](Source/Falcor/Utils/Color/Spectrum.h:230) - Maximum value

**Spectra Collection**:
- [`kCIE_X`](Source/Falcor/Utils/Color/Spectrum.h:239) - CIE 1931 X color matching function (densely sampled)
- [`kCIE_Y`](Source/Falcor/Utils/Color/Spectrum.h:240) - CIE 1931 Y color matching function (densely sampled)
- [`kCIE_Z`](Source/Falcor/Utils/Color/Spectrum.h:241) - CIE 1931 Z color matching function (densely sampled)
- [`kCIE_Y_Integral`](Source/Falcor/Utils/Color/Spectrum.h:242) - CIE Y integral constant (106.856895f)

**Technical Details**:

**Piecewise Linear Interpolation**:
1. Check if wavelength outside range (return 0)
2. Find upper bound using binary search
3. Compute interpolation parameter: t = (lambda - lambda[i]) / (lambda[i+1] - lambda[i])
4. Linear interpolation: lerp(values[i], values[i+1], t)

**Dense Sampling**:
- Uniform wavelength step
- Nearest neighbor evaluation
- Index calculation: round((wavelength - minWavelength) / step)
- Efficient lookup without binary search

**Blackbody Emission**:
- Planck's law for blackbody radiation
- Wavelength-dependent emission
- Optional normalization to peak value
- Infinite wavelength range

**Spectrum Conversion**:
- [`innerProduct<A,B>(const A& a, const B& b)`](Source/Falcor/Utils/Color/Spectrum.h:256) - Compute inner product of two spectra
- [`spectrumToXYZ<S>(const S& s)`](Source/Falcor/Utils/Color/Spectrum.h:274) - Convert spectrum to CIE 1931 XYZ
- [`spectrumToRGB<S>(const S& s)`](Source/Falcor/Utils/Color/Spectrum.h:284) - Convert spectrum to RGB in Rec.709

**Use Cases**:
- Physically-based rendering
- Spectral light sources
- Material spectral properties
- Color matching
- Illuminant data

### SpectrumUtils

**Files**:
- [`SpectrumUtils.h`](Source/Falcor/Utils/Color/SpectrumUtils.h:1) - Spectrum utilities header
- [`SpectrumUtils.cpp`](Source/Falcor/Utils/Color/SpectrumUtils.cpp:1) - Spectrum utilities implementation
- [`SpectrumUtils.slang`](Source/Falcor/Utils/Color/SpectrumUtils.slang:1) - Spectrum utilities shader

**Purpose**: Spectrum integration and conversion utilities.

**Key Features**:
- CIE 1931 color matching functions
- D65 standard illuminant
- Wavelength to XYZ conversion
- Wavelength to RGB conversion
- Spectrum integration
- Spectrum to XYZ/RGB conversion

**Core Methods**:

**Color Matching Functions**:
- [`wavelengthToXYZ_CIE1931(float lambda)`](Source/Falcor/Utils/Color/SpectrumUtils.h:55) - Evaluate CIE 1931 XYZ color matching curves
  - Uses 1nm sampling
  - Returns XYZ tristimulus values
  - Linearly interpolated from two nearest samples

- [`wavelengthToD65(float lambda)`](Source/Falcor/Utils/Color/SpectrumUtils.h:63) - Evaluate D65 standard illuminant
  - Uses 5nm sampling
  - Returns D65 value
  - Linearly interpolated from two nearest samples

- [`wavelengthToRGB_Rec709(const float lambda)`](Source/Falcor/Utils/Color/SpectrumUtils.h:70) - Convert wavelength to RGB Rec.709
  - Converts wavelength to XYZ_CIE1931 then to RGB Rec709

**Spectrum Integration**:
- [`integrate<T,ReturnType>(SampledSpectrum<T>& spectrum, const SpectrumInterpolation interpolationType, std::function<ReturnType(float)> func, const uint32_t componentIndex = 0, const uint32_t integrationSteps = 1)`](Source/Falcor/Utils/Color/SpectrumUtils.h:82) - Integrate over spectrum with user function
  - Parameters:
    - `spectrum` - The spectrum to integrate
    - `interpolationType` - Interpolation type
    - `func` - Function applied at each integration step
    - `componentIndex` - Component index for vector types
    - `integrationSteps` - Integration steps per sample
  - Uses Riemann sum approximation
  - Trapezoidal rule at endpoints

**Spectrum Conversion**:
- [`toXYZ<T>(SampledSpectrum<T>& spectrum, const SpectrumInterpolation interpolationType = SpectrumInterpolation::Linear, const uint32_t componentIndex = 0, const uint32_t integrationSteps = 1)`](Source/Falcor/Utils/Color/SpectrumUtils.h:124) - Convert spectrum to XYZ
  - Integrates spectrum with CIE 1931 color matching functions

- [`toXYZ_D65<T>(SampledSpectrum<T>& spectrum, const SpectrumInterpolation interpolationType = SpectrumInterpolation::Linear, const uint32_t componentIndex = 0, const uint32_t integrationSteps = 1)`](Source/Falcor/Utils/Color/SpectrumUtils.h:149) - Convert spectrum to XYZ times D65
  - Integrates spectrum with CIE 1931 functions multiplied by D65

- [`toRGB_D65<T>(SampledSpectrum<T>& spectrum, const SpectrumInterpolation interpolationType, const uint32_t componentIndex = 0, const uint32_t integrationSteps = 1)`](Source/Falcor/Utils/Color/SpectrumUtils.h:175) - Convert spectrum to RGB under D65 illuminant
  - Uses Equation 8 from "An OpenEXR Layout for Spectral Images" (JCGT)
  - Converts to XYZ_D65 then to RGB Rec.709
  - Normalizes by Y_D65 = 10567.0762f

**Data Structures**:

**Static Data**:
- [`sCIE_XYZ_1931_1nm`](Source/Falcor/Utils/Color/SpectrumUtils.h:46) - CIE 1931 XYZ color matching functions sampled at 1nm
- [`sD65_5nm`](Source/Falcor/Utils/Color/SpectrumUtils.h:47) - D65 standard illuminant sampled at 5nm

**Technical Details**:

**Integration Algorithm**:
1. Compute wavelength range
2. Calculate number of evaluations: size + (integrationSteps - 1) * (size - 1)
3. Compute wavelength delta: (max - min) / (evaluations - 1)
4. Riemann sum: sum(func(wavelength) * spectrum.eval(wavelength) * delta)
5. Trapezoidal rule at endpoints (0.5 weight)

**Conversion Pipeline**:
- Spectrum → XYZ: Integrate with CIE 1931 color matching functions
- Spectrum → XYZ_D65: Integrate with CIE 1931 × D65
- Spectrum → RGB_D65: XYZ_D65 → RGB Rec.709, normalize by Y_D65

**Use Cases**:
- Spectral rendering
- Color conversion
- Illuminant integration
- Material spectral evaluation
- Light spectral calculations

### ColorHelpers

**Files**:
- [`ColorHelpers.slang`](Source/Falcor/Utils/Color/ColorHelpers.slang:1) - Color helpers shader

**Purpose**: Shader-side color helper functions.

**Key Features**:
- Color space conversions in shaders
- Tone mapping helpers
- Color space utilities

**Technical Details**:

**Shader Functions**:
- RGB to XYZ conversion
- XYZ to RGB conversion
- Tone mapping functions
- Color space utilities

**Use Cases**:
- Shader-based color operations
- Tone mapping in shaders
- Color space conversion in shaders

### ColorMap

**Files**:
- [`ColorMap.slang`](Source/Falcor/Utils/Color/ColorMap.slang:1) - Color mapping shader

**Purpose**: Color mapping utilities for visualization.

**Key Features**:
- Color gradient generation
- Heat map generation
- Color mapping functions

**Technical Details**:

**Color Mapping**:
- Gradient interpolation
- Color palette generation
- Visualization color maps

**Use Cases**:
- Heat map visualization
- Gradient generation
- Data visualization

## Architecture Patterns

### Template-Based Design
- Generic spectrum representations
- Type-safe operations
- Compile-time optimization
- Zero-overhead abstractions

### Color Space Conversion
- Matrix-based transformations
- Standard color spaces (Rec.709, XYZ, LMS)
- Bidirectional conversions
- Standard illuminants (D65)

### Spectrum Representation
- Multiple representations (piecewise linear, dense, blackbody)
- Efficient evaluation
- Interpolation support
- Conversion utilities

## Technical Details

### Color Space Transformations

**RGB Rec.709 to XYZ**:
- Linear transformation matrix
- Derived from primaries and D65 whitepoint
- Standard sRGB color space

**XYZ to LMS (CAT02)**:
- Part of CIECAM02 color appearance model
- Cone response space
- Used for white balance calculations

**XYZ to LMS (Bradford)**:
- Original CIECAM97 model
- Alternative cone response space
- Chromatic adaptation transform

### Spectrum Evaluation

**Piecewise Linear**:
- Binary search for interval
- Linear interpolation within interval
- Zero outside defined range

**Dense Sampling**:
- Direct index calculation
- Nearest neighbor lookup
- Efficient for uniform sampling

**Blackbody**:
- Planck's law evaluation
- Wavelength-dependent
- Infinite wavelength range

### Integration

**Riemann Sum**:
- Numerical integration
- Trapezoidal rule at endpoints
- Configurable integration steps
- User-supplied integration function

### Color Temperature

**Approximation**:
- Piecewise rational polynomials
- Two regimes (T < 4000K, T >= 4000K)
- Range: 1667K to 25000K
- Double precision for accuracy

### White Balance

**Von Kries Transform**:
- Diagonal scaling in LMS space
- Preserves D65 white point
- May produce out-of-gamut values
- CAT02 transform recommended

## Progress Log

- **2026-01-07T19:10:06Z**: Color sub-module analysis completed. Analyzed ColorUtils, SampledSpectrum, Spectrum, and SpectrumUtils classes. Documented color space conversions (RGB, XYZ, LMS), spectrum representations (piecewise linear, dense, blackbody), color matching functions (CIE 1931), illuminant data (D65), and color temperature calculations. Created comprehensive technical specification with detailed code patterns, data structures, and use cases.

## Next Steps

Proceed to analyze Debug sub-module to understand debugging utilities (DebugConsole, PixelDebug, WarpProfiler).
