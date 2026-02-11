# MemoryMappedFile

## Module Overview

The **MemoryMappedFile** class provides a cross-platform memory-mapped file I/O utility for Falcor engine. This class encapsulates platform-specific memory mapping mechanisms, using `CreateFileMapping`/`MapViewOfFile` on Windows and `mmap64`/`munmap` on Linux. It supports access hints for optimizing memory access patterns, remapping of file regions, and automatic file size detection.

## Files

- **Header**: `Source/Falcor/Core/Platform/MemoryMappedFile.h` (130 lines)
- **Implementation**: `Source/Falcor/Core/Platform/MemoryMappedFile.cpp` (250 lines)

## Class Definition

```cpp
class MemoryMappedFile
{
public:
    enum class AccessHint
    {
        Normal,         ///< Good overall performance.
        SequentialScan, ///< Read file once with few seeks.
        RandomAccess    ///< Good for random access.
    };

    static constexpr size_t kWholeFile = std::numeric_limits<size_t>::max();

    /**
     * Default constructor. Use open() for opening a file.
     */
    MemoryMappedFile() = default;

    /**
     * Constructor opening a file. Use isOpen() to check if successful.
     * @param path Path to open.
     * @param mappedSize Number of bytes to map into memory (automatically clamped to file size).
     * @param accessHint Hint on how memory is accessed.
     */
    MemoryMappedFile(const std::filesystem::path& path, size_t mappedSize = kWholeFile, AccessHint accessHint = AccessHint::Normal);

    /// Destructor. Closes file.
    ~MemoryMappedFile();

    /**
     * Open a file.
     * @param path Path to open.
     * @param mappedSize Number of bytes to map into memory (automatically clamped to file size).
     * @param accessHint Hint on how memory is accessed.
     * @return True if file was successfully opened.
     */
    bool open(const std::filesystem::path& path, size_t mappedSize = kWholeFile, AccessHint accessHint = AccessHint::Normal);

    /// Close file.
    void close();

    /// True, if file successfully opened.
    bool isOpen() const { return mMappedData != nullptr; }

    /// Get file size in bytes.
    size_t getSize() const { return mSize; }

    /// Get mapped data.
    const void* getData() const { return mMappedData; };

    /// Get mapped memory size in bytes.
    size_t getMappedSize() const { return mMappedSize; };

    /// Get OS page size (for remap).
    static size_t getPageSize();

private:
    MemoryMappedFile(const MemoryMappedFile&) = delete;
    MemoryMappedFile(MemoryMappedFile&) = delete;
    MemoryMappedFile& operator=(const MemoryMappedFile&) = delete;
    MemoryMappedFile& operator=(const MemoryMappedFile&&) = delete;

    /**
     * Replace mapping by a new one of same file.
     * @param offset Offset from start of file in bytes (must be multiple of page size).
     * @param mappedSize Size of mapping in bytes.
     * @return True if successful.
     */
    bool remap(uint64_t offset, size_t mappedSize);

    std::filesystem::path mPath;
    AccessHint mAccessHint = AccessHint::Normal;
    size_t mSize = 0;

#if FALCOR_WINDOWS
    using FileHandle = void*;
    void* mMappedFile{nullptr};
#elif FALCOR_LINUX
    using FileHandle = int;
#else
#error "Unknown OS"
#endif

    FileHandle mFile = 0;
    void* mMappedData = 0;
    size_t mMappedSize = 0;
};
```

## Dependencies

### Internal Dependencies

- **Macros**: Core macros for platform detection (FALCOR_WINDOWS, FALCOR_LINUX)

### External Dependencies

#### Windows Dependencies

- **windows.h**: Windows API headers
- **CreateFile**: Windows file opening API
- **GetFileSizeEx**: Windows file size query API
- **CreateFileMapping**: Windows file mapping creation API
- **MapViewOfFile**: Windows memory mapping API
- **UnmapViewOfFile**: Windows memory unmapping API
- **CloseHandle**: Windows handle closing API
- **GetSystemInfo**: Windows system info query API
- **FILE_ATTRIBUTE_NORMAL**: Windows file attribute flag
- **FILE_FLAG_SEQUENTIAL_SCAN**: Windows access hint flag
- **FILE_FLAG_RANDOM_ACCESS**: Windows access hint flag
- **PAGE_READONLY**: Windows page protection flag
- **FILE_MAP_READ**: Windows file mapping access flag

#### Linux Dependencies

- **sys/stat.h**: POSIX file status API
- **sys/mman.h**: POSIX memory mapping API
- **fcntl.h**: POSIX file control API
- **errno.h**: POSIX error codes
- **unistd.h**: POSIX standard symbolic constants
- **open**: POSIX file opening API
- **fstat64**: POSIX file status query API
- **mmap64**: POSIX 64-bit memory mapping API
- **munmap**: POSIX memory unmapping API
- **close**: POSIX file closing API
- **madvise**: POSIX memory advice API
- **O_RDONLY**: POSIX read-only flag
- **O_LARGEFILE**: POSIX large file support flag
- **PROT_READ**: POSIX read protection flag
- **MAP_SHARED**: POSIX shared mapping flag
- **MADV_NORMAL**: POSIX normal access advice
- **MADV_SEQUENTIAL**: POSIX sequential access advice
- **MADV_RANDOM**: POSIX random access advice
- **sysconf**: POSIX system configuration query API

#### Common Dependencies

- **std::filesystem**: Standard library filesystem for file paths
- **cstdint**: Standard library fixed-width integers
- **limits**: Standard library numeric limits

## Cross-Platform Considerations

- **Cross-Platform**: Works on Windows and Linux
- **Platform-Specific Code**: Conditional compilation for Windows and Linux
- **Unified Interface**: Same API across all platforms
- **Platform Detection**: Uses FALCOR_WINDOWS and FALCOR_LINUX macros
- **Unknown OS**: Compile-time error for unsupported platforms
- **Filesystem**: Uses std::filesystem for cross-platform file paths
- **Memory Mapping**: Same memory mapping semantics across platforms
- **Access Hints**: Platform-specific implementation of access hints

## Usage Patterns

### Creating and Opening a Memory-Mapped File

```cpp
// Constructor opens and maps file
MemoryMappedFile mmf("/path/to/file.bin");

if (mmf.isOpen())
{
    // File is open and mapped
    const void* data = mmf.getData();
    size_t size = mmf.getMappedSize();
}
```

### Opening a Memory-Mapped File Explicitly

```cpp
MemoryMappedFile mmf;

if (mmf.open("/path/to/file.bin"))
{
    // File is open and mapped
    const void* data = mmf.getData();
    size_t size = mmf.getMappedSize();
}
```

### Mapping Entire File

```cpp
MemoryMappedFile mmf("/path/to/file.bin", MemoryMappedFile::kWholeFile);

if (mmf.isOpen())
{
    // Entire file is mapped
    const void* data = mmf.getData();
    size_t size = mmf.getMappedSize();
}
```

### Mapping Specific Size

```cpp
size_t mappedSize = 1024 * 1024; // 1 MB
MemoryMappedFile mmf("/path/to/file.bin", mappedSize);

if (mmf.isOpen())
{
    // First 1 MB is mapped (or less if file is smaller)
    const void* data = mmf.getData();
    size_t size = mmf.getMappedSize();
}
```

### Using Access Hints

```cpp
// Sequential scan access
MemoryMappedFile mmfSequential("/path/to/file.bin", MemoryMappedFile::kWholeFile, MemoryMappedFile::AccessHint::SequentialScan);

// Random access
MemoryMappedFile mmfRandom("/path/to/file.bin", MemoryMappedFile::kWholeFile, MemoryMappedFile::AccessHint::RandomAccess);

// Normal access (default)
MemoryMappedFile mmfNormal("/path/to/file.bin", MemoryMappedFile::kWholeFile, MemoryMappedFile::AccessHint::Normal);
```

### Remapping File Region

```cpp
MemoryMappedFile mmf("/path/to/file.bin");

if (mmf.isOpen())
{
    size_t pageSize = MemoryMappedFile::getPageSize();
    uint64_t offset = pageSize * 10; // Offset must be multiple of page size
    size_t mappedSize = 1024 * 1024; // 1 MB

    if (mmf.remap(offset, mappedSize))
    {
        // New region is mapped
        const void* data = mmf.getData();
        size_t size = mmf.getMappedSize();
    }
}
```

### Getting File Information

```cpp
MemoryMappedFile mmf("/path/to/file.bin");

if (mmf.isOpen())
{
    size_t fileSize = mmf.getSize();
    size_t mappedSize = mmf.getMappedSize();
    const void* data = mmf.getData();
}
```

### Getting Page Size

```cpp
size_t pageSize = MemoryMappedFile::getPageSize();
```

### RAII Pattern

```cpp
{
    MemoryMappedFile mmf("/path/to/file.bin");

    if (mmf.isOpen())
    {
        // File is open and mapped
        const void* data = mmf.getData();
        size_t size = mmf.getMappedSize();

        // ... use mapped data ...
    }
} // Destructor automatically unmaps and closes file
```

### Error Handling

```cpp
MemoryMappedFile mmf("/path/to/file.bin");

if (!mmf.isOpen())
{
    // Handle open failure
    return false;
}

// ... use mapped data ...

return true;
```

### Checking if File is Open

```cpp
if (mmf.isOpen())
{
    // File is open and mapped
    const void* data = mmf.getData();
}
```

### Closing a Memory-Mapped File

```cpp
mmf.close();
```

## Summary

**MemoryMappedFile** is a cross-platform memory-mapped file I/O utility that provides:

### MemoryMappedFile (Windows)
- ~88 bytes total size (excluding std::filesystem::path internal allocation)
- 8-byte natural alignment
- 4 bytes of padding
- Moderate cache locality (members span ~2 cache lines)
- O(1) time complexity for all operations (excluding OS-level allocation)
- CreateFileMapping()/MapViewOfFile() API for memory mapping
- UnmapViewOfFile() for memory unmapping
- void* file handle and mapping handle
- GetSystemInfo() for page size query

### MemoryMappedFile (Linux)
- ~80 bytes total size (excluding std::filesystem::path internal allocation)
- 8-byte natural alignment
- 8 bytes of padding
- Moderate cache locality (members span ~2 cache lines)
- O(1) time complexity for all operations (excluding OS-level allocation)
- mmap64() API for memory mapping
- munmap() for memory unmapping
- int file descriptor
- sysconf() for page size query

### Common Features
- RAII pattern for automatic resource management
- Non-copyable design (deleted copy/move constructors and assignment operators)
- Access hints (Normal, SequentialScan, RandomAccess)
- Remap functionality for replacing mapping with new one of same file
- Read-only memory mapping
- Automatic file size detection
- Automatic clamping of mapped size to file size
- Platform-specific file handle types
- Unified API across platforms
- No exceptions (bool return values for error handling)
- OS-managed memory allocation for mapped data
- Thread-safe data access at OS level (concurrent reads)
- Not thread-safe for concurrent write operations to same instance

The class implements a simple and efficient cross-platform memory-mapped file I/O utility with moderate cache locality, constant-time operations, RAII resource management, support for access hints, and remap functionality, providing a solid foundation for efficient file I/O in Falcor engine.
