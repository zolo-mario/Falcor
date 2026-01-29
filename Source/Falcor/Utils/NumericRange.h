#pragma once
#include "Core/Error.h"
#include <utility>
#include <stdexcept>

namespace Falcor
{
template<typename T, typename Enable = void>
class NumericRange final
{};

/**
 * Numeric range that can be iterated over.
 * Should be replaced with C++20 std::views::iota when available.
 */
template<typename T>
class NumericRange<T, typename std::enable_if<std::is_integral<T>::value>::type> final
{
public:
    class Iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = T;
        using pointer = const T*;
        using reference = T;

        explicit Iterator(const T& value = T(0)) : mValue(value) {}
        const Iterator& operator++()
        {
            ++mValue;
            return *this;
        }
        bool operator!=(const Iterator& other) const { return other.mValue != mValue; }
        T operator*() const { return mValue; }

    private:
        T mValue;
    };

    explicit NumericRange(const T& begin, const T& end) : mBegin(begin), mEnd(end) { FALCOR_CHECK(begin <= end, "Invalid range"); }
    NumericRange() = delete;
    NumericRange(const NumericRange&) = delete;
    NumericRange(NumericRange&& other) = delete;

    Iterator begin() const { return Iterator(mBegin); }
    Iterator end() const { return Iterator(mEnd); }

private:
    T mBegin, mEnd;
};
}; // namespace Falcor
