# BinaryFileStream - Binary File I/O Utility

## Module State Machine

**Status**: Complete

## Dependency Graph

### External Dependencies

- STL (filesystem, fstream)

## Module Overview

BinaryFileStream is a helper class for managing file I/O with binary files. It provides a convenient interface for reading and writing binary data with support for different access modes (Read, Write, ReadWrite), error checking, and file management operations.

## Component Specifications

### BinaryFileStream Class

**Signature**: `class BinaryFileStream`

**Purpose**: Helper class for managing binary file I/O operations

**Enums**:
- `Mode`: File access mode (Read, Write, ReadWrite)

**Constructors**:
- `BinaryFileStream()`: Default constructor (no file opened)
- `BinaryFileStream(const std::filesystem::path& path, Mode mode = Mode::ReadWrite)`: Opens file with specified mode

**Destructor**:
- `~BinaryFileStream()`: Closes file stream automatically

**File Management Methods**:
- `open(const std::filesystem::path& path, Mode mode = Mode::ReadWrite)`: Opens file stream (fails if already open)
- `close()`: Closes file stream
- `remove()`: Deletes managed file (closes if open)

**Stream Control Methods**:
- `skip(uint32_t count)`: Skips data in input stream without reading
- `getRemainingStreamSize()`: Calculates amount of remaining data in file

**Error Checking Methods**:
- `isGood()`: Checks for validity (no errors, end not reached)
- `isBad()`: Checks for stream errors
- `isFail()`: Checks for stream failure
- `isEof()`: Checks if end of file has been reached

**I/O Methods**:
- `read(void* pData, size_t count)`: Reads data from file stream
- `write(const void* pData, size_t count)`: Writes data to file stream

**Operator Overloads**:
- `operator>>(T& val)`: Extracts single value from stream
- `operator<<(const T& val)`: Writes single value to stream

**Private Members**:
- `std::fstream mStream`: File stream object
- `std::filesystem::path mPath`: File path for management

## Technical Details

### File Mode Enum

```cpp
enum class Mode
{
    Read = 0x1,     ///< Open file for reading
    Write = 0x2,    ///< Open file for writing
    ReadWrite = 0x3 ///< Open file for both reading and writing
};
```

**Mode Characteristics**:
- `Read`: Input-only access (std::ios::in)
- `Write`: Output-only access (std::ios::out)
- `ReadWrite`: Both input and output access (std::ios::in | std::ios::out)

### File Opening Algorithm

```cpp
void open(const std::filesystem::path& path, Mode mode = Mode::ReadWrite)
{
    std::ios::openmode iosMode = std::ios::binary;
    iosMode |= ((mode == Mode::Read) || (mode == Mode::ReadWrite)) ? std::ios::in : (std::ios::openmode)0;
    iosMode |= ((mode == Mode::Write) || (mode == Mode::ReadWrite)) ? std::ios::out : (std::ios::openmode)0;
    mStream.open(path, iosMode);
    mPath = path;
}
```

**Mode Mapping**:
- Read mode: Sets std::ios::in flag
- Write mode: Sets std::ios::out flag
- ReadWrite mode: Sets both std::ios::in and std::ios::out flags
- Always sets std::ios::binary flag

**Failure Condition**:
- Fails silently if file is already open
- Does not throw exceptions
- Caller must check stream state after opening

### File Closing Algorithm

```cpp
~BinaryFileStream()
{
    close();
}

void close()
{
    mStream.close();
}
```

**RAII Pattern**:
- Destructor automatically calls close()
- Ensures file is closed when object goes out of scope
- Prevents resource leaks
- Safe even in exception scenarios

### File Deletion Algorithm

```cpp
void remove()
{
    if (mStream.is_open())
    {
        close();
    }
    std::filesystem::remove(mPath);
}
```

**Safety Checks**:
- Closes file if it's open before deletion
- Prevents deleting open files on some platforms
- Uses std::filesystem::remove() for cross-platform deletion

### Stream Skipping Algorithm

```cpp
void skip(uint32_t count)
{
    mStream.ignore(count);
}
```

**Characteristics**:
- Advances file position without reading data
- Uses std::istream::ignore() method
- Efficient for skipping large blocks of data
- Does not return skipped data

### Remaining Size Calculation

```cpp
uint32_t getRemainingStreamSize()
{
    std::streamoff currentPos = mStream.tellg();
    mStream.seekg(0, mStream.end);
    std::streamoff length = mStream.tellg();
    mStream.seekg(currentPos);
    return (uint32_t)(length - currentPos);
}
```

**Algorithm**:
1. Save current position
2. Seek to end of file
3. Get total file length
4. Restore original position
5. Calculate remaining bytes

**Use Case**: Determining how much data is left to read

### Error Checking Methods

**isGood()**:
```cpp
bool isGood() { return mStream.good(); }
```
- Returns true if no errors and end not reached
- Equivalent to `!isBad() && !isFail() && !isEof()`

**isBad()**:
```cpp
bool isBad() { return mStream.bad(); }
```
- Returns true if an error has occurred
- Indicates stream corruption or invalid operation

**isFail()**:
```cpp
bool isFail() { return mStream.fail(); }
```
- Returns true if any error has occurred
- Includes bad bit and other failure conditions

**isEof()**:
```cpp
bool isEof() { return mStream.eof(); }
```
- Returns true if end of file has been reached
- Does not indicate error condition

### Data Reading Algorithm

```cpp
BinaryFileStream& read(void* pData, size_t count)
{
    mStream.read((char*)pData, count);
    return *this;
}
```

**Characteristics**:
- Reads count bytes from file into buffer
- Uses std::istream::read() method
- Returns reference to this for chaining
- Type-unsafe: Caller must ensure buffer is large enough

### Data Writing Algorithm

```cpp
BinaryFileStream& write(const void* pData, size_t count)
{
    mStream.write((char*)pData, count);
    return *this;
}
```

**Characteristics**:
- Writes count bytes from buffer to file
- Uses std::ostream::write() mode
- Returns reference to this for chaining
- Type-unsafe: Caller must ensure buffer contains count bytes

### Template Extraction Operator

```cpp
template<typename T>
BinaryFileStream& operator>>(T& val)
{
    return read(&val, sizeof(T));
}
```

**Characteristics**:
- Extracts single value of type T from stream
- Uses read() method with sizeof(T)
- Returns reference to this for chaining
- Type-safe: Automatically handles size based on type

**Usage Example**:
```cpp
BinaryFileStream file("data.bin", BinaryFileStream::Mode::Read);
int value;
file >> value;  // Reads sizeof(int) bytes
```

### Template Insertion Operator

```cpp
template<typename T>
BinaryFileStream& operator<<(const T& val)
{
    return write(&val, sizeof(T));
}
```

**Characteristics**:
- Writes single value of type T to stream
- Uses write() method with sizeof(T)
- Returns reference to this for chaining
- Type-safe: Automatically handles size based on type

**Usage Example**:
```cpp
BinaryFileStream file("data.bin", BinaryFileStream::Mode::Write);
int value = 42;
file << value;  // Writes sizeof(int) bytes
```

## Integration Points

### Usage in Falcor Framework

The BinaryFileStream class is used throughout Falcor framework for binary file I/O:

1. **Scene Loading**: Loading scene files in binary format
2. **Asset Importing**: Importing binary asset files
3. **Configuration Files**: Reading/writing configuration data
4. **Cache Files**: Managing binary cache files
5. **Serialization**: Saving/loading serialized data structures

### Integration Pattern

```cpp
// Open file for reading
BinaryFileStream file("data.bin", BinaryFileStream::Mode::Read);
if (file.isGood())
{
    // Read data
    int value;
    file >> value;
    
    // Check for errors
    if (file.isBad())
    {
        // Handle error
    }
}
```

## Architecture Patterns

### RAII Pattern

Automatic resource management:
- Destructor closes file automatically
- Prevents resource leaks
- Exception-safe

### Template Pattern

Generic type support:
- Template operators for any type
- Type-safe operations
- Compile-time size calculation

### Chaining Pattern

Fluent API design:
- Operators return reference to this
- Enables chaining: `file >> a >> b << c`
- Improves code readability

### State Pattern

Multiple error checking methods:
- isGood(), isBad(), isFail(), isEof()
- Different error conditions for different scenarios
- Comprehensive error state tracking

## Code Patterns

### File Opening Pattern

```cpp
BinaryFileStream(const std::filesystem::path& path, Mode mode = Mode::ReadWrite)
{
    open(path, mode);
}
```

### Safe Deletion Pattern

```cpp
void remove()
{
    if (mStream.is_open())
    {
        close();
    }
    std::filesystem::remove(mPath);
}
```

### Chaining Read Pattern

```cpp
int a, b, c;
file >> a >> b >> c;
```

### Chaining Write Pattern

```cpp
int a = 1, b = 2, c = 3;
file << a << b << c;
```

### Error Checking Pattern

```cpp
if (file.isGood())
{
    // Process data
}
else if (file.isBad())
{
    // Handle error
}
```

## Use Cases

### Binary File Reading

Primary use case is reading binary data from files:
- Scene files in binary format
- Asset files (meshes, textures, etc.)
- Configuration files
- Cache files

### Binary File Writing

Primary use case is writing binary data to files:
- Saving scene data
- Exporting assets
- Writing configuration files
- Creating cache files

### File Management

File lifecycle management:
- Opening files with specific access modes
- Closing files automatically
- Deleting files safely
- Checking file state

### Error Handling

Comprehensive error checking:
- Detecting stream errors
- Checking end-of-file condition
- Validating stream state
- Handling I/O failures

## Performance Considerations

### I/O Performance

- **Binary Mode**: No text conversion overhead
- **Buffered I/O**: Uses std::fstream internal buffering
- **Direct Access**: Direct memory access for read/write operations
- **Efficient Skipping**: skip() doesn't allocate memory

### Memory Efficiency

- **No Copying**: read() and write() directly access buffer
- **Minimal Overhead**: Lightweight wrapper around std::fstream
- **Stack Allocation**: File path stored in std::filesystem::path
- **No Dynamic Allocation**: No heap allocation in common operations

### File System Performance

- **Platform Optimized**: Uses std::filesystem for path operations
- **Cross-Platform**: Works on Windows and Linux
- **Efficient Deletion**: std::filesystem::remove() is optimized
- **Path Caching**: Stores path for repeated operations

### Template Performance

- **Compile-Time Size**: sizeof(T) computed at compile time
- **No Virtual Calls**: Template operators are inlined
- **Type Safety**: Compile-time type checking
- **Zero Overhead**: Template instantiation has no runtime cost

## Limitations

### Functional Limitations

- **No Exception Handling**: Does not throw exceptions on errors
- **Silent Failures**: open() fails silently if file is already open
- **No Buffer Validation**: read() and write() don't validate buffer size
- **No Type Checking**: Template operators don't validate type compatibility

### Error Handling Limitations

- **Limited Error Information**: Cannot determine specific error type
- **No Recovery**: Cannot recover from errors automatically
- **No Callbacks**: No mechanism to register error handlers
- **No Logging**: Errors must be logged by caller

### File System Limitations

- **No Atomic Operations**: File operations are not atomic
- **No Locking**: No built-in file locking mechanism
- **No Transaction Support**: Cannot rollback operations
- **Platform Dependent**: std::filesystem behavior varies by platform

### Performance Limitations

- **Buffer Size**: Uses std::fstream default buffer size
- **No Async I/O**: All operations are synchronous
- **No Memory Mapping**: Cannot use memory-mapped I/O
- **No Direct I/O**: Always goes through OS file system

## Best Practices

### When to Use BinaryFileStream

1. **Binary Data**: Ideal for reading/writing binary file formats
2. **Structured Data**: Perfect for structured binary data (headers, records)
3. **Type-Safe Operations**: Use template operators for type safety
4. **Error Checking**: Always check stream state after operations
5. **RAII**: Rely on automatic file closing in destructor

### When to Avoid BinaryFileStream

1. **Text Files**: Use std::ifstream/std::ofstream for text files
2. **Small Files**: Overhead may not be justified for tiny files
3. **Simple Operations**: Use std::fstream directly for simple cases
4. **Performance Critical**: In performance-critical code, consider lower-level APIs

### Usage Guidelines

1. **Mode Selection**: Choose appropriate mode (Read, Write, ReadWrite)
2. **Error Checking**: Check isGood() before using data
3. **Buffer Sizing**: Ensure buffer is large enough for operations
4. **Path Management**: Use absolute paths or ensure working directory is correct
5. **Cleanup**: Let destructor handle file closing when possible

### Optimization Tips

1. **Batch Operations**: Read/write multiple values in single operation
2. **Use Chaining**: Leverage chaining for cleaner code
3. **Check Errors Early**: Check stream state before processing data
4. **Skip Efficiently**: Use skip() instead of reading and discarding
5. **Buffer Reuse**: Reuse buffers instead of allocating for each operation

## Notes

- Wrapper around std::fstream for binary file I/O
- Always opens files in binary mode
- Supports three access modes: Read, Write, ReadWrite
- Comprehensive error checking: isGood(), isBad(), isFail(), isEof()
- Template operators for type-safe reading/writing: >> and <<
- Automatic file closing in destructor (RAII pattern)
- File path tracking for management operations
- Skip method for efficient data skipping
- getRemainingStreamSize() for calculating remaining bytes
- remove() method for safe file deletion
- Chaining support for fluent API
- No exception handling on errors
- open() fails silently if file is already open
- read() and write() don't validate buffer size
- Uses std::filesystem for cross-platform file operations
- Platform-dependent behavior for std::filesystem operations
