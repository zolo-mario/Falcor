#include "TimeReport.h"
#include "Utils/Logger.h"
#include "Utils/StringUtils.h"
#include <numeric>

namespace Falcor
{
TimeReport::TimeReport()
{
    reset();
}

void TimeReport::reset()
{
    mLastMeasureTime = CpuTimer::getCurrentTimePoint();
    mMeasurements.clear();
    mTotal = 0.0;
}

void TimeReport::resetTimer()
{
    mLastMeasureTime = CpuTimer::getCurrentTimePoint();
    mTotal = 0.0;
}

void TimeReport::printToLog()
{
    for (const auto& [task, duration] : mMeasurements)
    {
        logInfo(
            padStringToLength(task + ":", 25) + " " + std::to_string(duration) + " s" +
            (mTotal > 0.0 && !mMeasurements.empty() ? ", " + std::to_string(100.0 * duration / mTotal) + "% of total" : "")
        );
    }
}

void TimeReport::measure(const std::string& name)
{
    auto currentTime = CpuTimer::getCurrentTimePoint();
    std::chrono::duration<double> duration = currentTime - mLastMeasureTime;
    mLastMeasureTime = currentTime;
    mMeasurements.push_back({name, duration.count()});
}

void TimeReport::addTotal(const std::string name)
{
    mTotal = std::accumulate(mMeasurements.begin(), mMeasurements.end(), 0.0, [](double t, auto&& m) { return t + m.second; });
    mMeasurements.push_back({"Total", mTotal});
}
} // namespace Falcor
