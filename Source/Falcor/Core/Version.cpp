#include "Version.h"

#include "git_version.h"

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
    auto gitVersionString = []()
    {
        if (GIT_VERSION_AVAILABLE)
        {
            return fmt::format(
                "commit: {}, branch: {}{}",
                GIT_VERSION_COMMIT,
                GIT_VERSION_BRANCH,
                GIT_VERSION_DIRTY ? ", contains uncommitted changes" : ""
            );
        }
        else
        {
            return std::string{"git version unknown, git shell not installed"};
        }
    };
    static std::string str{fmt::format("{}.{} ({})", FALCOR_MAJOR_VERSION, FALCOR_MINOR_VERSION, gitVersionString())};
    return str;
}
} // namespace Falcor
