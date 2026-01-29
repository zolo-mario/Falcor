#include "TermColor.h"
#include "Core/Macros.h"

#include <iostream>
#include <unordered_map>

#if FALCOR_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <io.h>
#define ISATTY _isatty
#define FILENO _fileno
#elif FALCOR_LINUX
#include <unistd.h>
#define ISATTY isatty
#define FILENO fileno
#endif

namespace Falcor
{
#if FALCOR_WINDOWS
/**
 * The Windows console does not have ANSI support by default,
 * but it can be enabled through SetConsoleMode().
 * We use static initialization to do so.
 */
struct EnableVirtualTerminal
{
    EnableVirtualTerminal()
    {
        auto enableVirtualTerminal = [](DWORD handle)
        {
            HANDLE console = GetStdHandle(handle);
            if (console == INVALID_HANDLE_VALUE)
                return;
            DWORD mode;
            GetConsoleMode(console, &mode);
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(console, mode);
        };
        enableVirtualTerminal(STD_OUTPUT_HANDLE);
        enableVirtualTerminal(STD_ERROR_HANDLE);
    }
};

static EnableVirtualTerminal sEnableVirtualTerminal;
#endif // FALCOR_WINDOWS

static const std::unordered_map<TermColor, std::string> kBeginTag = {
    // clang-format off
    { TermColor::Gray,    "\33[90m" },
    { TermColor::Red,     "\33[91m" },
    { TermColor::Green,   "\33[92m" },
    { TermColor::Yellow,  "\33[93m" },
    { TermColor::Blue,    "\33[94m" },
    { TermColor::Magenta, "\33[95m" },
    // clang-format on
};

static const std::string kEndTag = "\033[0m";

inline bool isTTY(const std::ostream& stream)
{
    if (&stream == &std::cout && ISATTY(FILENO(stdout)))
        return true;
    if (&stream == &std::cerr && ISATTY(FILENO(stderr)))
        return true;
    return false;
}

std::string colored(const std::string& str, TermColor color, const std::ostream& stream)
{
    return isTTY(stream) ? (kBeginTag.at(color) + str + kEndTag) : str;
}
} // namespace Falcor
