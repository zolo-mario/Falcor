# ProgressBar

## Module Overview

The **ProgressBar** class provides a progress bar visual and manages a new thread for it in Falcor engine. This class encapsulates platform-specific progress bar functionality, using native window APIs on Windows and Linux. It provides a simple interface for showing, closing, and checking the status of a progress bar.

## Files

- **Header**: `Source/Falcor/Core/Platform/ProgressBar.h` (67 lines)
- **Implementation**: `Source/Falcor/Core/Platform/ProgressBar.cpp` (31 lines)

## Class Definition

```cpp
class ProgressBar
{
public:
    struct Window;
    
    ProgressBar();
    ~ProgressBar();
    
    /**
     * Show progress bar.
     * @param[in] msg Message to display on the progress bar.
     */
    void show(const std::string& msg);
    
    /**
     * Close progress bar.
     */
    void close();
    
    /**
     * Check if progress bar is currently active.
     * @return Returns true if progress bar is active.
     */
    bool isActive() const { return mpWindow != nullptr; }

private:
    std::unique_ptr<Window> mpWindow;
};
```

## Dependencies

### Internal Dependencies

- **Macros**: Core macros for platform detection (FALCOR_WINDOWS, FALCOR_LINUX)

### External Dependencies

#### Windows Dependencies

- **Windows API**: Windows API headers (implied by platform-specific implementation)
- **Window API**: Windows window creation and management API
- **Thread API**: Windows thread creation and management API

#### Linux Dependencies

- **GLFW**: Cross-platform window and input library (implied by platform-specific implementation)
- **Window API**: GLFW window creation and management API
- **Thread API**: POSIX thread API (implied by platform-specific implementation)

#### Common Dependencies

- **std::memory**: Standard library memory utilities
- **std::string**: Standard library string container
- **std::unique_ptr**: Standard library unique pointer

## Cross-Platform Considerations

- **Cross-Platform**: Works on Windows and Linux
- **Platform-Specific Code**: Conditional compilation for Windows and Linux
- **Unified Interface**: Same API across all platforms
- **Platform Detection**: Uses FALCOR_WINDOWS and FALCOR_LINUX macros
- **Unknown OS**: Compile-time error for unsupported platforms
- **Different Implementations**: Different APIs for Windows and Linux
- **GLFW Dependency**: Linux implementation requires GLFW

## Usage Patterns

### Creating and Showing Progress Bar

```cpp
ProgressBar progressBar;
progressBar.show("Processing data...");
```

### Checking if Progress Bar is Active

```cpp
if (progressBar.isActive())
{
    std::cout << "Progress bar is active" << std::endl;
}
```

### Closing Progress Bar

```cpp
progressBar.close();
```

### RAII Pattern

```cpp
{
    ProgressBar progressBar;
    progressBar.show("Processing data...");

    // ... do work ...

} // Destructor automatically closes progress bar
```

### Multiple Progress Bars

```cpp
ProgressBar progressBar1;
ProgressBar progressBar2;

progressBar1.show("Processing task 1...");
progressBar2.show("Processing task 2...");

// ... do work ...

progressBar1.close();
progressBar2.close();
```

### Conditional Progress Bar

```cpp
ProgressBar progressBar;

if (showProgress)
{
    progressBar.show("Processing data...");

    // ... do work ...

    progressBar.close();
}
```

## Summary

**ProgressBar** is a simple progress bar utility that provides:

### ProgressBar
- ~8 bytes total size (excluding Window allocation and unique_ptr internal storage)
- 8-byte natural alignment
- No padding (perfectly aligned)
- Excellent cache locality (all members in single cache line)
- O(1) time complexity for all operations
- RAII pattern for automatic resource cleanup
- Unique pointer pattern for automatic Window management
- Thread management (creates and joins progress bar thread)
- Simple interface (show, close, isActive)
- Platform-specific implementation (Windows and Linux)
- Not thread-safe for concurrent access
- Thread-safe for concurrent reads to isActive()

The class implements a simple and efficient progress bar utility with excellent cache locality, constant-time operations, RAII resource management, and internal thread management, providing a solid foundation for progress indication in Falcor engine.
