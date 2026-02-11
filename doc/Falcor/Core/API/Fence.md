# Fence

## Overview
[`Fence`](Source/Falcor/Core/API/Fence.h:62) is a synchronization primitive used to synchronize host (CPU) and device (GPU) execution. It represents a fence on the device as a 64-bit integer and maintains a copy of the last signaled value on the host. The fence value is monotonically incremented every time it is signaled.

## Source Files
- Header: [`Source/Falcor/Core/API/Fence.h`](Source/Falcor/Core/API/Fence.h) (139 lines)
- Implementation: [`Source/Falcor/Core/API/Fence.cpp`](Source/Falcor/Core/API/Fence.cpp) (104 lines)

## Class Hierarchy
```
Object (base class)
  └── Fence
```

## Dependencies

### Direct Dependencies
- [`fwd.h`](Source/Falcor/Core/API/fwd.h) - Forward declarations
- [`Handles.h`](Source/Falcor/Core/API/Handles.h) - Handle types
- [`NativeHandle.h`](Source/Falcor/Core/API/NativeHandle.h) - Native handle types
- [`Core/Macros.h`](Source/Falcor/Core/Macros.h) - Macros
- [`Core/Object.h`](Source/Falcor/Core/Object.h) - Base class
- [`Device.h`](Source/Falcor/Core/API/Device.h) - Device (implementation)
- [`GFXAPI.h`](Source/Falcor/Core/API/GFXAPI.h) - GFX API (implementation)
- [`NativeHandleTraits.h`](Source/Falcor/Core/API/NativeHandleTraits.h) - Native handle traits (implementation)
- [`Core/Error.h`](Source/Falcor/Core/Error.h) - Error handling (implementation)

### Indirect Dependencies
- Object base class (from Core/Object.h)
- Device class (from Device.h)
- Slang gfx interfaces (gfx::IFence, gfx::IFence::Desc)
- NativeHandle type (from NativeHandle.h)
- SharedResourceApiHandle type (from Handles.h)

## Known Issues and Limitations

1. **Thread Safety**: Not thread-safe for concurrent access
2. **No Atomic Operations**: `mSignaledValue` is not atomic
3. **No Validation**: No validation of fence value
4. **No Timeout Handling**: No error handling for timeout
5. **No Cancel Support**: No support for canceling wait operations
6. **Vulkan Support**: Native handle access not supported for Vulkan
7. **No Multiple Fence Wait**: Only supports waiting on a single fence
8. **No Event Support**: No support for event-based signaling

## Usage Example

```cpp
// Create a fence
FenceDesc desc = {};
desc.initialValue = 0;
desc.shared = false;

ref<Fence> pFence = pDevice->createFence(desc);

// Schedule device work 1
// <schedule device work 1>

// Signal fence once we have finished all the above work on device
pRenderContext->signal(pFence);

// Schedule device work 2
// <schedule device work 2>

// Wait on host until <device work 1> is finished
pFence->wait();

// Signal fence with auto-increment
uint64_t signaledValue = pFence->signal(); // Auto-increments signaled value

// Wait for specific value
pFence->wait(signaledValue);

// Wait with timeout
pFence->wait(signaledValue, 1000000000); // 1 second timeout

// Get current value
uint64_t currentValue = pFence->getCurrentValue();

// Get signaled value
uint64_t signaledValue = pFence->getSignaledValue();

// Update signaled value
uint64_t newValue = pFence->updateSignaledValue(); // Auto-increments

// Get native handle (D3D12 only)
NativeHandle nativeHandle = pFence->getNativeHandle();

// Get shared API handle
SharedResourceApiHandle sharedHandle = pFence->getSharedApiHandle();

// Break strong reference to device
pFence->breakStrongReferenceToDevice();
```

## Conclusion

Fence provides a straightforward and efficient synchronization primitive for host-device synchronization. The implementation is clean with excellent cache locality but lacks thread safety.

**Strengths**:
- Clean and simple API for host-device synchronization
- Excellent cache locality (all members fit in a single cache line)
- Lightweight construction and operations
- Auto-increment mode for automatic value management
- Support for shared fences
- Native handle access for D3D12
- Automatic resource management using smart pointers
- BreakableReference pattern to avoid circular references

**Weaknesses**:
- Not thread-safe for concurrent access
- No atomic operations for `mSignaledValue`
- No validation of fence value
- No error handling for timeout
- No support for canceling wait operations
- Native handle access not supported for Vulkan
- Only supports waiting on a single fence
- No support for event-based signaling

**Recommendations**:
1. Add thread safety using atomic operations for `mSignaledValue`
2. Add validation for fence value
3. Add error handling for timeout
4. Consider adding support for canceling wait operations
5. Consider adding support for waiting on multiple fences
6. Consider adding support for event-based signaling
7. Add native handle support for Vulkan
8. Consider using atomic operations for all mutable state to enable thread-safe usage
