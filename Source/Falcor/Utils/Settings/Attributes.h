#pragma once
#include "SettingsUtils.h"
#include "Core/Macros.h"
#include "Core/Error.h"

#include <type_traits>
#include <optional>
#include <string>
#include <string_view>

#include <nlohmann/json.hpp>

namespace Falcor
{
namespace settings
{

class Attributes
{
public:
    Attributes() = default;
    Attributes(nlohmann::json jsonDict) : mJsonDict(jsonDict) {}

    void overrideWith(const Attributes& other) { addDict(other.mJsonDict); }

    template<typename T>
    std::optional<T> get(std::string_view attrName) const
    {
        nlohmann::json attribute = nullptr;

        auto attrIt = mJsonDict.find(attrName);
        if (attrIt != mJsonDict.end())
            attribute = attrIt.value();

        if (attribute.is_null())
            return {};

        if (!detail::TypeChecker<T>::validType(attribute))
            throw detail::TypeError("Attribute's type does not match the requested type.");

        // Handle return value of bool, if the actual is convertible to bool (from int, usually)
        if constexpr (std::is_same_v<T, bool>)
        {
            if (attribute.is_boolean())
                return attribute.get<bool>();
            return attribute.get<int>() != 0;
        }

        // Handle return value of int, if the actual is a boolean
        if constexpr (std::is_arithmetic_v<T>)
        {
            if (attribute.is_boolean())
                return attribute.get<bool>() ? T(1) : T(0);
        }

        return attribute.get<T>();
    }

    template<typename T>
    T get(std::string_view attrName, const T& def) const
    {
        auto result = get<T>(attrName);
        return result ? *result : def;
    }

    bool has(std::string_view attrName) const
    {
        auto attrIt = mJsonDict.find(attrName);
        return attrIt != mJsonDict.end();
    }

    void addDict(nlohmann::json jsonDict)
    {
        for (auto& it : jsonDict.items())
            mJsonDict[it.key()] = it.value();
    }

    void clear() { mJsonDict = nlohmann::json::object(); }

    void removePrefix(std::string_view prefix)
    {
        nlohmann::json filtered;
        for (auto& it : mJsonDict.items())
        {
            if (it.key().find(prefix) != 0)
                filtered[it.key()] = it.value();
        }
        mJsonDict = std::move(filtered);
    }

    void removeExact(std::string_view name) { mJsonDict.erase(name); }

    std::string to_string() const { return mJsonDict.dump(); }

private:
    nlohmann::json mJsonDict;
};

} // namespace settings
} // namespace Falcor
