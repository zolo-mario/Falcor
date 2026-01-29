#pragma once
#include "Core/Macros.h"
#include "Core/Error.h"

#include <exception>
#include <string>
#include <type_traits>
#include <array>

#include <nlohmann/json.hpp>

#include <fmt/format.h>

namespace Falcor
{
namespace settings::detail
{

namespace
{
void flattenDictionary(const nlohmann::json& dict, const std::string& prefix, nlohmann::json& flattened)
{
    FALCOR_ASSERT(flattened.is_object());
    if (!dict.is_object())
    {
        flattened[prefix] = dict;
        return;
    }

    for (auto& it : dict.items())
    {
        std::string name = fmt::format("{}{}{}", prefix, prefix.empty() ? "" : ":", it.key());
        flattenDictionary(it.value(), name, flattened);
    }
}
} // namespace

/// Flattens nested dictionaries into colon separated name,
/// e.g. {"foo":{"bar":4}} becomes {"foo:bar":4}
inline nlohmann::json flattenDictionary(const nlohmann::json& dict)
{
    nlohmann::json flattened = nlohmann::json::object();
    flattenDictionary(dict, "", flattened);
    return flattened;
}

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
inline bool isType(const nlohmann::json& json)
{
    return json.is_number() || json.is_boolean();
}

template<typename T, std::enable_if_t<std::is_same_v<T, std::string>, bool> = true>
inline bool isType(const nlohmann::json& json)
{
    return json.is_string();
}

template<typename T, typename U, size_t N, std::enable_if_t<std::is_same_v<T, std::array<U, N>>, bool> = true>
inline bool isType(const nlohmann::json& json)
{
    if (!json.is_array())
        return false;
    if (json.size() != N)
        return false;
    for (size_t i = 0; i < N; ++i)
        if (!isType<U>(json[i]))
            return false;
    return true;
}

// The "gccfix" parameter is used to avoid "explicit specialization in non-namespace scope" in gcc.
// See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85282
template<typename T, typename gccfix = void>
struct TypeChecker
{
    static bool validType(const nlohmann::json& json) { return isType<T>(json); }
};

template<typename U, size_t N, typename gccfix>
struct TypeChecker<std::array<U, N>, gccfix>
{
    using ArrayType = std::array<U, N>;
    static bool validType(const nlohmann::json& json) { return isType<ArrayType, U, N>(json); }
};

class TypeError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

} // namespace settings::detail
} // namespace Falcor
