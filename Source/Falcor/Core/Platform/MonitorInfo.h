#pragma once
#include "Core/Macros.h"
#include "Utils/Math/Vector.h"
#include <string>
#include <vector>

namespace Falcor
{
/**
 * A class to extract information about displays
 */
class FALCOR_API MonitorInfo
{
public:
    /**
     * Description data structure
     */
    struct MonitorDesc
    {
        std::string identifier; ///< Monitor identifier.
        uint2 resolution;       ///< Resolution in pixels.
        float2 physicalSize;    ///< Physical size in inches.
        float ppi;              ///< Pixel density (points per inch).
        bool isPrimary;         ///< True if primary monitor.
    };

    /**
     * Get a list of all monitors.
     * This function is _not_ thread-safe
     * @return Returns a list of monitor configurations.
     */
    static std::vector<MonitorDesc> getMonitorDescs();

    /**
     * Display information on currently connected monitors.
     */
    static void displayMonitorInfo();
};
} // namespace Falcor
