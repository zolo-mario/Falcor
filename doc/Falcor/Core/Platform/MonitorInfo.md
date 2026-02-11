# MonitorInfo

## Module Overview

The **MonitorInfo** class provides cross-platform monitor and display information extraction for Falcor engine. This class encapsulates platform-specific display enumeration mechanisms, using Windows Setup API and Registry on Windows and GLFW on Linux. It provides monitor descriptions including resolution, physical size, pixel density (PPI), and primary monitor identification.

## Files

- **Header**: `Source/Falcor/Core/Platform/MonitorInfo.h` (66 lines)
- **Implementation**: `Source/Falcor/Core/Platform/MonitorInfo.cpp` (286 lines)

## Class Definition

```cpp
class MonitorInfo
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
```

## Dependencies

### Internal Dependencies

- **Macros**: Core macros for platform detection (FALCOR_WINDOWS, FALCOR_LINUX)
- **StringUtils**: String utilities for wstring_2_string conversion (Windows)
- **Vector**: Math vector types (uint2, float2)

### External Dependencies

#### Windows Dependencies

- **windows.h**: Windows API headers
- **setupapi.h**: Windows Setup API headers
- **cfgmgr32.h**: Windows Configuration Manager API headers
- **EnumDisplayMonitors**: Windows display enumeration API
- **GetMonitorInfo**: Windows monitor info API
- **EnumDisplayDevices**: Windows display device enumeration API
- **SetupDiGetClassDevsEx**: Windows Setup API device enumeration
- **SetupDiEnumDeviceInfo**: Windows Setup API device info enumeration
- **SetupDiGetDeviceInstanceId**: Windows Setup API device instance ID API
- **SetupDiOpenDevRegKey**: Windows Setup API registry key API
- **RegEnumValue**: Windows registry enumeration API
- **RegCloseKey**: Windows registry closing API
- **SetupDiDestroyDeviceInfoList**: Windows Setup API cleanup API
- **MONITORINFOEX**: Windows monitor info structure
- **MONITORINFOF_PRIMARY**: Windows primary monitor flag
- **DISPLAY_DEVICE**: Windows display device structure
- **GUID_CLASS_MONITOR**: Windows monitor class GUID

#### Linux Dependencies

- **GLFW**: Cross-platform window and input library
- **glfwGetMonitors**: GLFW monitor enumeration API
- **glfwGetMonitorName**: GLFW monitor name API
- **glfwGetVideoModes**: GLFW video mode enumeration API
- **glfwGetMonitorPhysicalSize**: GLFW physical size API
- **glfwGetPrimaryMonitor**: GLFW primary monitor API
- **GLFWmonitor**: GLFW monitor structure
- **GLFWvidmode**: GLFW video mode structure

#### Common Dependencies

- **std::string**: Standard library string container
- **std::vector**: Standard library vector container
- **fmt**: Format library for console output

## Cross-Platform Considerations

- **Cross-Platform**: Works on Windows and Linux
- **Platform-Specific Code**: Conditional compilation for Windows and Linux
- **Unified Interface**: Same API across all platforms
- **Platform Detection**: Uses FALCOR_WINDOWS and FALCOR_LINUX macros
- **Unknown OS**: Compile-time error for unsupported platforms
- **Different Implementations**: Different APIs for Windows and Linux
- **GLFW Dependency**: Linux implementation requires GLFW
- **Setup API**: Windows implementation requires Setup API

## Usage Patterns

### Getting All Monitor Descriptions

```cpp
std::vector<MonitorInfo::MonitorDesc> monitors = MonitorInfo::getMonitorDescs();

for (const auto& desc : monitors)
{
    std::cout << "Monitor: " << desc.identifier << std::endl;
    std::cout << "Resolution: " << desc.resolution.x << " x " << desc.resolution.y << std::endl;
    std::cout << "Physical Size: " << desc.physicalSize.x << "\" x " << desc.physicalSize.y << "\"" << std::endl;
    std::cout << "PPI: " << desc.ppi << std::endl;
    std::cout << "Primary: " << (desc.isPrimary ? "Yes" : "No") << std::endl;
}
```

### Displaying Monitor Information

```cpp
MonitorInfo::displayMonitorInfo();
```

### Finding Primary Monitor

```cpp
std::vector<MonitorInfo::MonitorDesc> monitors = MonitorInfo::getMonitorDescs();

for (const auto& desc : monitors)
{
    if (desc.isPrimary)
    {
        std::cout << "Primary Monitor: " << desc.identifier << std::endl;
        std::cout << "Resolution: " << desc.resolution.x << " x " << desc.resolution.y << std::endl;
        break;
    }
}
```

### Finding Monitor with Highest Resolution

```cpp
std::vector<MonitorInfo::MonitorDesc> monitors = MonitorInfo::getMonitorDescs();

MonitorInfo::MonitorDesc* highestResMonitor = nullptr;
uint32_t maxPixels = 0;

for (auto& desc : monitors)
{
    uint32_t pixels = desc.resolution.x * desc.resolution.y;
    if (pixels > maxPixels)
    {
        maxPixels = pixels;
        highestResMonitor = &desc;
    }
}

if (highestResMonitor)
{
    std::cout << "Highest Resolution Monitor: " << highestResMonitor->identifier << std::endl;
    std::cout << "Resolution: " << highestResMonitor->resolution.x << " x " << highestResMonitor->resolution.y << std::endl;
}
```

### Finding Monitor with Highest PPI

```cpp
std::vector<MonitorInfo::MonitorDesc> monitors = MonitorInfo::getMonitorDescs();

MonitorInfo::MonitorDesc* highestPpiMonitor = nullptr;
float maxPpi = 0.0f;

for (auto& desc : monitors)
{
    if (desc.ppi > maxPpi)
    {
        maxPpi = desc.ppi;
        highestPpiMonitor = &desc;
    }
}

if (highestPpiMonitor)
{
    std::cout << "Highest PPI Monitor: " << highestPpiMonitor->identifier << std::endl;
    std::cout << "PPI: " << highestPpiMonitor->ppi << std::endl;
}
```

### Calculating Monitor Aspect Ratio

```cpp
std::vector<MonitorInfo::MonitorDesc> monitors = MonitorInfo::getMonitorDescs();

for (const auto& desc : monitors)
{
    float aspectRatio = float(desc.resolution.x) / float(desc.resolution.y);
    std::cout << "Monitor: " << desc.identifier << std::endl;
    std::cout << "Aspect Ratio: " << aspectRatio << ":1" << std::endl;
}
```

## Summary

**MonitorInfo** is a cross-platform monitor information utility that provides:

### MonitorDesc
- ~56 bytes total size (excluding std::string internal allocation)
- 8-byte natural alignment
- 3 bytes of padding
- Excellent cache locality (all members in single cache line)
- O(1) time complexity for direct member access
- Monitor identifier string
- Resolution in pixels (uint2)
- Physical size in inches (float2)
- Pixel density (PPI)
- Primary monitor flag

### MonitorInfo
- 0 bytes total size (no member variables)
- All methods are static
- O(N × M) time complexity for enumeration (N = monitors, M = devices/modes)
- O(N) time complexity for display (N = monitors)
- Platform-specific implementations for Windows and Linux
- Not thread-safe (explicitly documented)
- Uses global/static state for enumeration
- EDID extraction for physical size on Windows
- GLFW integration for monitor queries on Linux
- Console output for displayMonitorInfo()

The class implements a simple and efficient cross-platform monitor information utility with excellent cache locality, linear-time enumeration, comprehensive monitor description including resolution, physical size, PPI, and primary monitor identification, providing a solid foundation for display management in Falcor engine.
