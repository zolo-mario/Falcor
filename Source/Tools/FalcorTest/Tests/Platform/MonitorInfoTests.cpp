#include "Testing/UnitTest.h"
#include "Core/Platform/MonitorInfo.h"

namespace Falcor
{
CPU_TEST(MonitorInfo)
{
    auto monitorDescs = MonitorInfo::getMonitorDescs();
}
} // namespace Falcor
