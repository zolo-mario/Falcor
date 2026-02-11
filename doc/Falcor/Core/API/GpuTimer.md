# GpuTimer

## Overview
[`GpuTimer`](Source/Falcor/Core/API/GpuTimer.h:41) is a GPU timing and profiling class that provides a mechanism to measure elapsed time in milliseconds between a pair of [`begin()`](Source/Falcor/Core/API/GpuTimer.h:60)/[`end()`](Source/Falcor/Core/API/GpuTimer.h:66) calls. It uses GPU timestamp queries to accurately measure GPU execution time.

## Source Files
- Header: [`Source/Falcor/Core/API/GpuTimer.h`](Source/Falcor/Core/API/GpuTimer.h) (105 lines)
- Implementation: [`Source/Falcor/Core/API/GpuTimer.cpp`](Source/Falcor/Core/API/GpuTimer.cpp) (180 lines)

## Class Hierarchy
```
Object (base class)
  └── GpuTimer
```

## Dependencies

### Direct Dependencies
- [`fwd.h`](Source/Falcor/Core/API/fwd.h) - Forward declarations
- [`QueryHeap.h`](Source/Falcor/Core/API/QueryHeap.h) - Query heap types
- [`Buffer.h`](Source/Falcor/Core/API/Buffer.h) - Buffer types
- [`Core/Macros.h`](Source/Falcor/Core/Macros.h) - Macros
- [`Core/Object.h`](Source/Falcor/Core/Object.h) - Base class
- [`Device.h`](Source/Falcor/Core/API/Device.h) - Device (implementation)
- [`RenderContext.h`](Source/Falcor/Core/API/RenderContext.h) - Render context (implementation)
- [`GFXAPI.h`](Source/Falcor/Core/API/GFXAPI.h) - GFX API (implementation)
- [`Core/Error.h`](Source/Falcor/Core/Error.h) - Error handling (implementation)
- [`Core/ObjectPython.h`](Source/Falcor/Core/ObjectPython.h) - Python bindings (implementation)
- [`Utils/Logger.h`](Source/Falcor/Utils/Logger.h) - Logging (implementation)
- [`Utils/Scripting/ScriptBindings.h`](Source/Falcor/Utils/Scripting/ScriptBindings.h) - Script bindings (implementation)

### Indirect Dependencies
- Object base class (from Core/Object.h)
- Device class (from Device.h)
- Buffer class (from Buffer.h)
- QueryHeap class (from QueryHeap.h)
- LowLevelContextData class (from RenderContext.h)
- ResourceCommandEncoder class (from LowLevelContextData.h)
- Logger functions (from Utils/Logger.h)

## Known Issues and Limitations

1. **Thread Safety**: Not thread-safe for concurrent access
2. **Inefficient Resolution**: Resolves each timer individually (should be batched)
3. **No Validation**: No validation of timestamp query allocation
4. **No Error Handling**: No error handling for buffer map/unmap failures
5. **No Multiple Measurements**: Only supports a single measurement at a time
6. **No Auto-Resolve**: Does not automatically resolve after end()
7. **No Timestamp Frequency Validation**: Does not validate GPU timestamp frequency
8. **No Precision Control**: No control over timestamp query precision
9. **No Timestamp Units**: Always returns elapsed time in milliseconds
10. **No Timestamp Range Validation**: Does not validate timestamp range

## Usage Example

```cpp
// Create a GPU timer
ref<GpuTimer> pTimer = GpuTimer::create(pDevice);

// Begin timing
pTimer->begin();

// <perform GPU work>

// End timing
pTimer->end();

// Insert GPU synchronization between begin() and end()
// <insert GPU synchronization>

// Resolve timestamps
pTimer->resolve();

// Get elapsed time in milliseconds
double elapsedTimeMs = pTimer->getElapsedTime();

// Begin a new measurement
pTimer->begin();

// <perform more GPU work>

// End timing
pTimer->end();

// Insert GPU synchronization between begin() and end()
// <insert GPU synchronization>

// Resolve timestamps
pTimer->resolve();

// Get elapsed time in milliseconds
double elapsedTimeMs = pTimer->getElapsedTime();

// Break strong reference to device
pTimer->breakStrongReferenceToDevice();
```

## Conclusion

GpuTimer provides a straightforward and efficient GPU timing mechanism using timestamp queries. The implementation is clean with good cache locality but has several performance and thread safety limitations.

**Strengths**:
- Clean and simple API for GPU timing
- Good cache locality (members fit in a single cache line)
- Lightweight begin and end operations
- Accurate GPU timing using timestamp queries
- State machine for validating usage patterns
- Automatic resource management using smart pointers
- BreakableReference pattern to avoid circular references
- Python bindings

**Weaknesses**:
- Not thread-safe for concurrent access
- Inefficient resolution (resolves each timer individually)
- No validation of timestamp query allocation
- No error handling for buffer map/unmap failures
- Only supports a single measurement at a time
- Does not automatically resolve after end()
- No validation of GPU timestamp frequency
- No control over timestamp query precision
- Always returns elapsed time in milliseconds
- No validation of timestamp range

**Recommendations**:
1. Add thread safety using mutex or atomic operations
2. Implement batched resolution across all active timers (as noted in TODO comment)
3. Add validation for timestamp query allocation
4. Add error handling for buffer map/unmap failures
5. Consider supporting multiple measurements
6. Consider adding auto-resolve after end()
7. Add validation for GPU timestamp frequency
8. Consider adding control over timestamp query precision
9. Consider supporting different timestamp units (microseconds, nanoseconds)
10. Add validation for timestamp range
11. Consider using a single buffer for resolve and staging to reduce memory overhead
12. Consider packing status, start, end, and dataPending into a single 32-bit value
