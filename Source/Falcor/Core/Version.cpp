#include "Version.h"

#include <fmt/format.h>

namespace Falcor
{
const std::string& getVersionString()
{
    static std::string str{fmt::format("{}.{}", FALCOR_MAJOR_VERSION, FALCOR_MINOR_VERSION)};
    return str;
}

const std::string& getLongVersionString()
{
    return getVersionString();
}
} // namespace Falcor
