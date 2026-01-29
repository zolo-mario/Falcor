#pragma once

#include <type_traits>

namespace Falcor
{
/// Returns whether an integer number is a power of two.
template<typename T>
constexpr typename std::enable_if<std::is_integral<T>::value, bool>::type isPowerOf2(T a)
{
    return (a & (a - (T)1)) == 0;
}

/// Divide an a by b and round up to the next integer.
template<typename T>
constexpr T div_round_up(T a, T b)
{
    return (a + b - T(1)) / b;
}

/// Helper to align an integer value to a given alignment.
template<typename T>
constexpr typename std::enable_if<std::is_integral<T>::value, T>::type align_to(T alignment, T value)
{
    return ((value + alignment - T(1)) / alignment) * alignment;
}
} // namespace Falcor
