# PlatformHandles

## Module Overview

The **PlatformHandles** module provides platform-specific handle type definitions for Falcor engine. This header-only module defines type aliases for platform-specific handles, including shared library handles, window handles, and display handles. It provides a unified interface across Windows and Linux platforms using conditional compilation.

## Files

- **Header**: `Source/Falcor/Core/Platform/PlatformHandles.h` (46 lines)
- **Implementation**: Header-only (no separate .cpp file)

## Module Structure

### Type Aliases

#### Windows Platform

```cpp
#if FALCOR_WINDOWS
using SharedLibraryHandle = void*; // HANDLE
using WindowHandle = void*;        // HWND
#endif
```

#### Linux Platform

```cpp
#elif FALCOR_LINUX
using SharedLibraryHandle = void*;
struct WindowHandle
{
    void* pDisplay;
    unsigned long window;
};
#endif
```

#### Unknown Platform

```cpp
#else
#error "Platform not specified!"
#endif
```

## Dependencies

### Internal Dependencies

- **Macros**: Core macros for platform detection (FALCOR_WINDOWS, FALCOR_LINUX)

### External Dependencies

#### Windows Dependencies

- **Windows API**: Windows API headers (implied by platform-specific implementation)
- **HANDLE**: Windows handle type (implied by void* type alias)
- **HWND**: Windows window handle type (implied by void* type alias)

#### Linux Dependencies

- **X11 API**: X11 API headers (implied by platform-specific implementation)
- **Display**: X11 display pointer (void*)
- **Window**: X11 window ID (unsigned long)

#### Common Dependencies

- **None**: No external dependencies (header-only file)

## Cross-Platform Considerations

- **Cross-Platform**: Works on Windows and Linux
- **Platform-Specific Code**: Conditional compilation for Windows and Linux
- **Unified Interface**: Same type names across all platforms
- **Platform Detection**: Uses FALCOR_WINDOWS and FALCOR_LINUX macros
- **Unknown OS**: Compile-time error for unsupported platforms
- **Zero Runtime Overhead**: Type aliases are compile-time substitutions
- **Different Implementations**: Different memory layout for WindowHandle on different platforms

## Usage Patterns

### Using Type Aliases

```cpp
// Shared library handle
PlatformHandles::SharedLibraryHandle library = loadSharedLibrary("library.so");
void* funcPtr = getProcAddress(library, "functionName");
releaseSharedLibrary(library);

// Window handle (Windows)
PlatformHandles::WindowHandle window = getNativeWindowHandle();

// Window handle (Linux)
PlatformHandles::WindowHandle window;
window.pDisplay = getDisplay();
window.window = createWindow();
```

### Conditional Compilation

```cpp
#if FALCOR_WINDOWS
// Windows-specific code
PlatformHandles::SharedLibraryHandle library = LoadLibrary("library.dll");
PlatformHandles::WindowHandle window = CreateWindow(...);
#elif FALCOR_LINUX
// Linux-specific code
PlatformHandles::SharedLibraryHandle library = dlopen("library.so", RTLD_LAZY);
PlatformHandles::WindowHandle window;
window.pDisplay = XOpenDisplay(NULL);
window.window = XCreateSimpleWindow(window.pDisplay, ...);
#endif
```

### Type Safety

```cpp
// Type-safe handle management
PlatformHandles::SharedLibraryHandle library = loadLibrary(...);
if (library != nullptr)
{
    // Use library
    releaseSharedLibrary(library);
}

// Type-safe window handle (Linux)
PlatformHandles::WindowHandle window;
window.pDisplay = XOpenDisplay(NULL);
window.window = XCreateSimpleWindow(window.pDisplay, ...);
```

### Platform-Specific Code

```cpp
// Windows-specific code
#if FALCOR_WINDOWS
PlatformHandles::SharedLibraryHandle library = LoadLibrary("library.dll");
PlatformHandles::WindowHandle window = CreateWindowEx(..., ...);
#endif

// Linux-specific code
#if FALCOR_LINUX
PlatformHandles::WindowHandle window;
window.pDisplay = XOpenDisplay(NULL);
window.window = XCreateSimpleWindow(window.pDisplay, ...);
#endif
```

## Summary

**PlatformHandles** is a header-only module that provides platform-specific handle type definitions:

### Type Aliases
- Zero runtime memory footprint
- Zero runtime overhead (compile-time substitutions)
- 8-byte natural alignment
- Thread-safe for concurrent reads and writes
- Platform-specific type aliases (void* for both platforms)
- Unified interface across Windows and Linux

### WindowHandle Struct (Linux)
- 16 bytes total size
- 8-byte natural alignment
- No padding (perfectly aligned)
- Excellent cache locality (all members in single cache line)
- O(1) time complexity for all operations
- Platform-specific implementation (X11 display and window handles)
- Not thread-safe for concurrent writes (no synchronization)

### Module Characteristics
- Header-only (no implementation file)
- Conditional compilation for Windows and Linux
- Compile-time error for unsupported platforms
- Zero-cost type alias abstraction
- Platform-specific handle definitions
- Type-safe handle management
- Cross-platform unified interface

The module provides a zero-cost type alias abstraction for platform-specific handles with excellent cache locality for the WindowHandle struct, thread-safe type aliases, and cross-platform support for Windows and Linux, providing a solid foundation for platform-specific handle management in Falcor engine.
