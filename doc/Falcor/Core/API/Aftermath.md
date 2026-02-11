# Aftermath

## Overview
[`Aftermath`](Source/Falcor/Core/API/Aftermath.h) is a wrapper class for NVIDIA Aftermath SDK integration. It provides GPU crash dump tracking and debugging capabilities for application crashes.

## Source Files
- Header: [`Source/Falcor/Core/API/Aftermath.h`](Source/Falcor/Core/API/Aftermath.h) (99 lines)
- Implementation: [`Source/Falcor/Core/API/Aftermath.cpp`](Source/Falcor/Core/API/Aftermath.cpp) (98 lines)

## Class Hierarchy
```
Object (base class)
  └── Aftermath
```

## Dependencies

### Direct Dependencies
- [`Core/Macros.h`](Source/Falcor/Core/Macros.h) - Macros
- [`Core/Object.h`](Source/Falcor/Core/Object.h) - Base class
- [`Device.h`](Source/Falcor/Core/API/Device.h) - Device type (forward declaration)
- [`LowLevelContextData.h`](Source/Falcor/Core/API/LowLevelContextData.h) - Low-level context data type (forward declaration)
- [`string_view`](Source/Falcor/Core/API/string_view) - String view type

### Indirect Dependencies
- Object base class (from Core/Object.h)
- Device class (from Device.h)
- LowLevelContextData class (from LowLevelContextData.h)
- Aftermath SDK (external dependency, only compiled with FALCOR_HAS_AFTERMATH)

## Known Issues and Limitations

1. **Thread Safety**: Not thread-safe for concurrent access
2. **No Validation**: No validation of initialization state
3. **No Error Handling**: No error handling for Aftermath SDK failures
4. **No Marker Validation**: No validation of marker names
5. **No Resource Tracking Control**: No control over resource tracking granularity
6. **No Call Stack Control**: No control over call stack depth
7. **No Shader Debug Info Control**: No control over shader debug info detail level
8. **No Error Reporting Control**: No control over error reporting detail level
9. **No Timeout Control**: Fixed timeout of 5 seconds in [`waitForAftermathDumps()`](Source/Falcor/Core/API/Aftermath.cpp:95)
10. **Platform-Specific**: Only available on Windows (FALCOR_HAS_AFTERMATH)

## Usage Example

```cpp
// Create an Aftermath context (only compiled with FALCOR_HAS_AFTERMATH)
AftermathContext aftermathContext(pDevice, pLowLevelContextData);

// Initialize Aftermath with default flags
aftermathContext.initialize(AftermathFlags::Defaults);

// Add a marker to the command list
aftermathContext.addMarker(pLowLevelContextData, "MyMarker");

// Enable Aftermath with custom flags
aftermathContext.enableAftermath(AftermathFlags::EnableMarkers | AftermathFlags::CallStackCapturing);

// Wait for GPU crash dumps to be generated
aftermathContext.waitForAftermathDumps(5);

// Disable Aftermath
aftermathContext.disableAftermath();
```

## Conclusion

Aftermath provides a straightforward and efficient wrapper for NVIDIA Aftermath SDK integration. The implementation is clean with excellent cache locality but lacks thread safety and advanced features.

**Strengths**:
- Clean and simple API for Aftermath SDK integration
- Excellent cache locality (all members fit in single cache lines)
- Lightweight construction and operations
- Support for GPU crash dump tracking
- Support for markers, resource tracking, call stack capturing
- Support for shader debug info and error reporting
- Automatic resource management via Aftermath SDK
- Device integration for GPU operations
- Low-level context data integration

**Weaknesses**:
- Not thread-safe for concurrent access
- No validation of initialization state
- No error handling for Aftermath SDK failures
- No validation of marker names
- No control over resource tracking granularity
- No control over call stack depth
- No control over shader debug info detail level
- No control over error reporting detail level
- Fixed timeout of 5 seconds for dump waiting
- Platform-specific (only available on Windows)

**Recommendations**:
1. Add thread safety using mutex or atomic operations
2. Add validation for initialization state
3. Add error handling for Aftermath SDK failures
4. Add validation of marker names
5. Consider adding control over resource tracking granularity
6. Consider adding control over call stack depth
7. Consider adding control over shader debug info detail level
8. Consider adding control over error reporting detail level
9. Consider making timeout configurable
10. Consider adding cross-platform support for other operating systems
