# LockFile

## Module Overview

The **LockFile** class provides a cross-platform file locking abstraction for the Falcor engine. This class encapsulates platform-specific file locking mechanisms, using `LockFileEx()` on Windows systems and `flock()` on POSIX/Linux systems. It supports both exclusive and shared locks, with both non-blocking (tryLock) and blocking (lock) modes.

## Files

- **Header**: `Source/Falcor/Core/Platform/LockFile.h` (112 lines)
- **Implementation**: `Source/Falcor/Core/Platform/LockFile.cpp` (148 lines)

## Class Definition

```cpp
class LockFile
{
public:
    enum class LockType
    {
        Exclusive,
        Shared,
    };

    LockFile() = default;

    /**
     * Construct and open lock file. This will create file if it doesn't exist yet.
     * @note Use isOpen() to check if file was successfully opened.
     * @param path File path.
     */
    LockFile(const std::filesystem::path& path);

    ~LockFile();

    /**
     * Open lock file. This will create file if it doesn't exist yet.
     * @param path File path.
     * @return True if successful.
     */
    bool open(const std::filesystem::path& path);

    /// Closes lock file.
    void close();

    /// Returns true if lock file is open.
    bool isOpen() const { return mIsOpen; }

    /**
     * Acquire lock in non-blocking mode.
     * @param lockType Lock type (Exclusive or Shared).
     * @return True if successful.
     */
    bool tryLock(LockType lockType = LockType::Exclusive);

    /**
     * Acquire lock in blocking mode.
     * @param lockType Lock type (Exclusive or Shared).
     * @return True if successful.
     */
    bool lock(LockType lockType = LockType::Exclusive);

    /**
     * Release lock.
     * @return True if successful.
     */
    bool unlock();

private:
    LockFile(const LockFile&) = delete;
    LockFile(LockFile&) = delete;
    LockFile& operator=(const LockFile&) = delete;
    LockFile& operator=(const LockFile&&) = delete;

#if FALCOR_WINDOWS
    using FileHandle = void*;
#elif FALCOR_LINUX
    using FileHandle = int;
#else
#error "Unknown OS"
#endif

    FileHandle mFileHandle;
    bool mIsOpen = false;
};
```

## Dependencies

### Internal Dependencies

- **Macros**: Core macros for platform detection (FALCOR_WINDOWS, FALCOR_LINUX)

### External Dependencies

#### Windows Dependencies

- **windows.h**: Windows API headers
- **CreateFileW**: Windows file creation API
- **CloseHandle**: Windows file closing API
- **LockFileEx**: Windows file locking API
- **UnlockFileEx**: Windows file unlocking API
- **CreateEvent**: Windows event creation API
- **GetOverlappedResult**: Windows overlapped result API
- **GetLastError**: Windows error retrieval API

#### Linux Dependencies

- **unistd.h**: POSIX API for file operations
- **sys/types.h**: POSIX system types
- **sys/file.h**: POSIX file locking API
- **open**: POSIX file opening API
- **close**: POSIX file closing API
- **flock**: POSIX file locking API

#### Common Dependencies

- **std::filesystem**: Standard library filesystem for file paths

## Cross-Platform Considerations

- **Cross-Platform**: Works on Windows and Linux
- **Platform-Specific Code**: Conditional compilation for Windows and Linux
- **Unified Interface**: Same API across all platforms
- **Platform Detection**: Uses FALCOR_WINDOWS and FALCOR_LINUX macros
- **Unknown OS**: Compile-time error for unsupported platforms
- **Filesystem**: Uses std::filesystem for cross-platform file paths
- **Lock Semantics**: Same lock semantics across platforms (exclusive/shared, blocking/non-blocking)

## Usage Patterns

### Creating and Opening a Lock File

```cpp
// Constructor opens the file
LockFile lockFile("/path/to/lockfile");

if (lockFile.isOpen())
{
    // File is open
}
```

### Opening a Lock File Explicitly

```cpp
LockFile lockFile;

if (lockFile.open("/path/to/lockfile"))
{
    // File is open
}
```

### Acquiring an Exclusive Lock (Non-Blocking)

```cpp
if (lockFile.tryLock(LockFile::LockType::Exclusive))
{
    // Lock acquired
    // ... do work ...
    lockFile.unlock();
}
else
{
    // Lock not acquired
}
```

### Acquiring a Shared Lock (Non-Blocking)

```cpp
if (lockFile.tryLock(LockFile::LockType::Shared))
{
    // Lock acquired
    // ... do work ...
    lockFile.unlock();
}
else
{
    // Lock not acquired
}
```

### Acquiring an Exclusive Lock (Blocking)

```cpp
if (lockFile.lock(LockFile::LockType::Exclusive))
{
    // Lock acquired (blocks until available)
    // ... do work ...
    lockFile.unlock();
}
else
{
    // Lock not acquired (file not open)
}
```

### Acquiring a Shared Lock (Blocking)

```cpp
if (lockFile.lock(LockFile::LockType::Shared))
{
    // Lock acquired (blocks until available)
    // ... do work ...
    lockFile.unlock();
}
else
{
    // Lock not acquired (file not open)
}
```

### RAII Pattern

```cpp
{
    LockFile lockFile("/path/to/lockfile");

    if (lockFile.isOpen() && lockFile.tryLock())
    {
        // Lock acquired
        // ... do work ...
        // Lock automatically released when lockFile goes out of scope
    }
} // Destructor automatically closes the file
```

### Checking if File is Open

```cpp
if (lockFile.isOpen())
{
    // File is open, can perform lock operations
}
```

### Closing a Lock File

```cpp
lockFile.close();
```

### Error Handling

```cpp
LockFile lockFile("/path/to/lockfile");

if (!lockFile.isOpen())
{
    // Handle open failure
    return false;
}

if (!lockFile.tryLock())
{
    // Handle lock acquisition failure
    return false;
}

// ... do work ...

if (!lockFile.unlock())
{
    // Handle unlock failure
    return false;
}

return true;
```

## Summary

**LockFile** is a cross-platform file locking abstraction that provides:

### LockFile (Windows)
- 16 bytes total size
- 8-byte natural alignment
- 7 bytes of padding
- Excellent cache locality (all members in single cache line)
- O(1) time complexity for all operations
- LockFileEx() API for locking
- OVERLAPPED structure for blocking locks
- void* file handle

### LockFile (Linux)
- 8 bytes total size
- 4-byte natural alignment
- 3 bytes of padding
- Excellent cache locality (all members in single cache line)
- O(1) time complexity for all operations
- flock() API for locking
- Simple blocking implementation
- int file descriptor

### Common Features
- RAII pattern for automatic resource management
- Non-copyable design (deleted copy/move constructors and assignment operators)
- Exclusive and shared lock types
- Non-blocking (tryLock) and blocking (lock) modes
- Platform-specific file handle types
- Unified API across platforms
- No exceptions (bool return values for error handling)
- Automatic file creation if not exists
- Thread-safe at OS level, not thread-safe for concurrent access to same instance

The class implements a simple and efficient cross-platform file locking mechanism with excellent cache locality, constant-time operations, RAII resource management, and support for both exclusive and shared locks with blocking and non-blocking modes, providing a solid foundation for file-based synchronization in the Falcor engine.
