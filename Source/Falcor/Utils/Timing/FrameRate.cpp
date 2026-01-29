#include "FrameRate.h"
#include <fmt/core.h>

namespace Falcor
{
std::string FrameRate::getMsg(bool vsyncOn) const
{
    double frameTime = getAverageFrameTime();
    std::string msg = fmt::format("{:.1f} FPS ({:.1f} ms/frame)", 1.f / frameTime, frameTime * 1000.0);
    if (vsyncOn)
        msg += std::string(", VSync");
    return msg;
}
} // namespace Falcor
