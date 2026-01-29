// This code is based on pbrt:
// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

#pragma once
#include "Utils/Math/Vector.h"
#include "Utils/Color/Spectrum.h"
#include <fmt/format.h>
#include <filesystem>
#include <functional>
#include <string_view>
#include <string>
#include <variant>

namespace Falcor::pbrt
{

using Float = float;

/**
 * File location.
 */
struct FileLoc
{
    FileLoc() = default;
    FileLoc(const std::string_view filename) : filename(filename) {}
    std::string toString() const { return fmt::format("{}:{}:{}", filename, line, column); }

    std::string_view filename;
    uint32_t line = 1;
    uint32_t column = 0;
};

/**
 * Placeholder for representing RGB color space.
 * RGB in Rec.709. is currently always used.
 */
struct RGBColorSpace;

/**
 * Resolve a relative file path.
 */
using Resolver = std::function<std::filesystem::path(const std::filesystem::path& path)>;

/**
 * Spectrum types.
 */
enum class SpectrumType
{
    Illuminant,
    Albedo,
    Unbounded
};

/**
 * Spectrum holder.
 */
using Spectrum = std::variant<
    float3, // RGB value
    PiecewiseLinearSpectrum,
    BlackbodySpectrum>;

} // namespace Falcor::pbrt
