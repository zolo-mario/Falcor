#pragma once
#include <iostream>
#include <string>

namespace Falcor
{
enum class TermColor
{
    Gray,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta
};

/**
 * Colorize a string for writing to a terminal. Return original string if stream is not a terminal.
 * @param[in] str String to colorize
 * @param[in] color Color
 * @param[in] stream Output stream
 * @return Returns string wrapped in color codes if stream is not a terminal, original string otherwise.
 */
std::string colored(const std::string& str, TermColor color, const std::ostream& stream = std::cout);
} // namespace Falcor
