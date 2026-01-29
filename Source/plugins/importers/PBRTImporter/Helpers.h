#pragma once
#include "Types.h"
#include "Core/Error.h"
#include "Utils/Logger.h"
#include <fmt/format.h>
#include <string_view>

namespace Falcor::pbrt
{

template<typename... Args>
[[noreturn]] inline void throwError(fmt::format_string<Args...> format, Args&&... args)
{
    FALCOR_THROW(format, std::forward<Args>(args)...);
}

template<typename... Args>
[[noreturn]] inline void throwError(const FileLoc& loc, fmt::format_string<Args...> format, Args&&... args)
{
    auto msg = fmt::format(format, std::forward<Args>(args)...);
    FALCOR_THROW("{}: {}", loc.toString(), msg);
}

template<typename... Args>
inline void logWarning(const FileLoc& loc, fmt::format_string<Args...> format, Args&&... args)
{
    auto msg = fmt::format(format, std::forward<Args>(args)...);
    Falcor::logWarning("{}: {}", loc.toString(), msg);
}

} // namespace Falcor::pbrt
