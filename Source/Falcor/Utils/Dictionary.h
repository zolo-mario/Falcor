#pragma once
#include "Core/Error.h"
#include <unordered_map>
#include <any>
#include <memory>
#include <string>

namespace Falcor
{
class Dictionary
{
public:
    class Value
    {
    public:
        Value() = default;
        Value(std::any& value) : mValue(value){};

        template<typename T>
        void operator=(const T& t)
        {
            mValue = t;
        }

        template<typename T>
        operator T() const
        {
            return std::any_cast<T>(mValue);
        }

    private:
        std::any mValue;
    };

    using Container = std::unordered_map<std::string, Value>;

    Dictionary() = default;
    Dictionary(const Dictionary& d) : mContainer(d.mContainer) {}

    Value& operator[](const std::string& key) { return mContainer[key]; }
    const Value& operator[](const std::string& key) const { return mContainer.at(key); }

    Container::const_iterator begin() const { return mContainer.begin(); }
    Container::const_iterator end() const { return mContainer.end(); }

    Container::iterator begin() { return mContainer.begin(); }
    Container::iterator end() { return mContainer.end(); }

    size_t size() const { return mContainer.size(); }

    /// Check if a key exists.
    bool keyExists(const std::string& key) const { return mContainer.find(key) != mContainer.end(); }

    /// Get value by key. Throws an exception if key does not exist.
    template<typename T>
    T getValue(const std::string& key)
    {
        auto it = mContainer.find(key);
        FALCOR_CHECK(it != mContainer.end(), "Key '{}' does not exist", key);
        return it->second;
    }

    /// Get value by key. Returns the specified default value if key does not exist.
    template<typename T>
    T getValue(const std::string& key, const T& defaultValue)
    {
        auto it = mContainer.find(key);
        return it != mContainer.end() ? it->second : defaultValue;
    }

private:
    Container mContainer;
};
} // namespace Falcor
