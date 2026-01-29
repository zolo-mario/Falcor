#pragma once
#include "CpuTimer.h"
#include "Core/Macros.h"
#include <string>
#include <utility>
#include <vector>

namespace Falcor
{
/**
 * Utility class to record a number of timing measurements and print them afterwards.
 * This is mainly intended for measuring longer running tasks on the CPU.
 */
class FALCOR_API TimeReport
{
public:
    TimeReport();

    /**
     * Resets the recorded measurements and the internal timer.
     */
    void reset();

    /**
     * Resets the the internal timer but not the recoreded measurements.
     */
    void resetTimer();

    /**
     * Prints the recorded measurements to the logfile.
     */
    void printToLog();

    /**
     * Records a time measurement.
     * Measures time since last call to reset() or measure(), whichever happened more recently.
     * @param[in] name Name of the record.
     */
    void measure(const std::string& name);

    /**
     * Add a record containing the total of all measurements.
     * @param[in] name Name of the record.
     */
    void addTotal(const std::string name = "Total");

private:
    CpuTimer::TimePoint mLastMeasureTime;
    std::vector<std::pair<std::string, double>> mMeasurements;
    double mTotal = 0.0;
};
} // namespace Falcor
