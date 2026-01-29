#pragma once
#include "SettingsUtils.h"
#include "Attributes.h"
#include "Core/Macros.h"
#include "Core/Error.h"
#include "Utils/Logger.h"

#include <type_traits>
#include <optional>
#include <regex>
#include <string>
#include <string_view>

#include <nlohmann/json.hpp>

namespace pybind11
{
class dict;
}

namespace Falcor
{

namespace settings
{

class AttributeFilter
{
    struct Record
    {
        std::string name;
        std::regex regex;
        nlohmann::json attributes;
    };

public:
    void add(const nlohmann::json& json) { addJson(json); }
    void clear() { mAttributes.clear(); }

    Attributes getAttributes(std::string_view shapeName_) const;

    template<typename T>
    std::optional<T> getAttribute(std::string_view shapeName, std::string_view attrName) const
    {
        nlohmann::json attribute = nullptr;

        for (const Record& recordIt : mAttributes)
        {
            if (!std::regex_match(shapeName.begin(), shapeName.end(), recordIt.regex))
                continue;
            auto attrIt = recordIt.attributes.find(attrName);
            if (attrIt != recordIt.attributes.end())
                attribute = attrIt.value();
        }

        if (attribute.is_null())
            return {};

        if (!detail::TypeChecker<T>::validType(attribute))
            throw detail::TypeError("Attribute's type does not match the requested type.");

        if constexpr (std::is_arithmetic_v<T>)
        {
            if (attribute.is_boolean())
                return attribute.get<bool>() ? T(1) : T(0);
        }

        return attribute.get<T>();
    }

    template<typename T>
    T getAttribute(std::string_view shapeName, std::string_view attrName, const T& def) const
    {
        auto result = getAttribute<T>(shapeName, attrName);
        return result ? *result : def;
    }

private:
    void addJson(const nlohmann::json& json);
    void addArray(const nlohmann::json& array);
    void addDictionary(const nlohmann::json& dict);

private:
    /// Filters out all attributes using the deprecated `name.filter` syntax,
    /// processes into filters, and returns the remaining attributes
    nlohmann::json processDeprecatedFilters(std::string_view name, nlohmann::json flattened, const std::string& regexStr);

private:
    std::vector<Record> mAttributes;
};

} // namespace settings

} // namespace Falcor
