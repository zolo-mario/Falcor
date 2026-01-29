// This code is based on pbrt:
// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

#pragma once

#include "Types.h"
#include "Utils/Math/Vector.h"
#include "Utils/Color/Spectrum.h"
#include <string>
#include <string_view>
#include <vector>

namespace Falcor::pbrt
{

class ParsedParameter
{
public:
    ParsedParameter(FileLoc loc) : loc(loc) {}

    void addFloat(Float f);
    void addInt(int i);
    void addString(const std::string_view s);
    void addBool(bool b);

    std::string toString() const;

    // ParsedParameter Public Members
    std::string type;
    std::string name;
    FileLoc loc;
    std::vector<Float> floats;
    std::vector<int> ints;
    std::vector<std::string> strings;
    std::vector<uint8_t> bools;
    mutable bool lookedUp = false;
    mutable const RGBColorSpace* colorSpace = nullptr;
    bool mayBeUnused = false;
};

using ParsedParameterVector = std::vector<ParsedParameter>;

enum class ParameterType
{
    Float,
    Int,
    String,
    Bool,
    Point2,
    Vector2,
    Point3,
    Vector3,
    Normal,
    Spectrum,
    Texture
};

template<ParameterType PT>
struct ParameterTypeTraits
{};

class ParameterDictionary
{
public:
    // ParameterDictionary Public Methods
    ParameterDictionary() = default;
    ParameterDictionary(ParsedParameterVector params, const RGBColorSpace* pColorSpace);
    ParameterDictionary(ParsedParameterVector params1, ParsedParameterVector params2, const RGBColorSpace* pColorSpace);

    const ParsedParameterVector& getParameters() const { return mParams; }
    FileLoc getParameterLoc(const std::string& name) const;

    bool hasParameter(const std::string& name) const;

    template<ParameterType PT>
    bool hasParameterWithType(const std::string& name) const;

    bool hasFloat(const std::string& name) const;
    bool hasInt(const std::string& name) const;
    bool hasString(const std::string& name) const;
    bool hasBool(const std::string& name) const;
    bool hasPoint2(const std::string& name) const;
    bool hasVector2(const std::string& name) const;
    bool hasPoint3(const std::string& name) const;
    bool hasVector3(const std::string& name) const;
    bool hasNormal(const std::string& name) const;
    bool hasSpectrum(const std::string& name) const;
    bool hasTexture(const std::string& name) const;

    Float getFloat(const std::string& name, Float def) const;
    int getInt(const std::string& name, int def) const;
    std::string getString(const std::string& name, const std::string& def) const;
    bool getBool(const std::string& name, bool def) const;
    float2 getPoint2(const std::string& name, float2 def) const;
    float2 getVector2(const std::string& name, float2 def) const;
    float3 getPoint3(const std::string& name, float3 def) const;
    float3 getVector3(const std::string& name, float3 def) const;
    float3 getNormal(const std::string& name, float3 def) const;
    Spectrum getSpectrum(const std::string& name, const Spectrum& def, Resolver resolver) const;
    std::string getTexture(const std::string& name) const;

    std::vector<Float> getFloatArray(const std::string& name) const;
    std::vector<int> getIntArray(const std::string& name) const;
    std::vector<std::string> getStringArray(const std::string& name) const;
    std::vector<uint8_t> getBoolArray(const std::string& name) const;
    std::vector<float2> getPoint2Array(const std::string& name) const;
    std::vector<float2> getVector2Array(const std::string& name) const;
    std::vector<float3> getPoint3Array(const std::string& name) const;
    std::vector<float3> getVector3Array(const std::string& name) const;
    std::vector<float3> getNormalArray(const std::string& name) const;
    std::vector<Spectrum> getSpectrumArray(const std::string& name, Resolver resolver) const;

    std::string toString() const;

private:
    const ParsedParameter* findParameter(const std::string& name) const;

    template<ParameterType PT>
    typename ParameterTypeTraits<PT>::ReturnType lookupSingle(
        const std::string& name,
        typename ParameterTypeTraits<PT>::ReturnType defaultValue
    ) const;

    template<ParameterType PT>
    std::vector<typename ParameterTypeTraits<PT>::ReturnType> lookupArray(const std::string& name) const;

    std::vector<Spectrum> extractSpectrumArray(const ParsedParameter& param, Resolver resolver) const;

    ParsedParameterVector mParams;
    const RGBColorSpace* mpColorSpace;
};

} // namespace Falcor::pbrt
