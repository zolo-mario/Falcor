# OS

## Module Overview

The **OS** module provides comprehensive cross-platform operating system abstractions for Falcor engine. This module encapsulates platform-specific functionality including file I/O, directory operations, process management, threading, memory queries, display information, message boxes, file dialogs, and debugging utilities. It provides a unified interface across Windows and Linux platforms.

## Files

- **Header**: `Source/Falcor/Core/Platform/OS.h` (513 lines)
- **Implementation**: `Source/Falcor/Core/Platform/OS.cpp` (361 lines)

## Module Structure

### OSServices Class

```cpp
class OSServices
{
public:
    static void start();
    static void stop();
};
```

### Utility Functions

The OS module provides numerous utility functions organized by category:

#### Display Functions
- `setWindowIcon()` - Set window icon
- `getDisplayDpi()` - Get display DPI
- `getDisplayScaleFactor()` - Get display scale factor

#### Message Box Functions
- `msgBox()` - Display message box
- `MsgBoxIcon` enum - Message box icons (None, Info, Warning, Error)
- `MsgBoxType` enum - Message box types (Ok, OkCancel, RetryCancel, AbortRetryIgnore, YesNo)
- `MsgBoxButton` enum - Message box buttons (Ok, Retry, Cancel, Abort, Ignore, Yes, No)
- `MsgBoxCustomButton` struct - Custom message box button
- `msgBox()` (custom) - Display custom message box

#### Path Operations
- `isSamePath()` - Compare paths (case-insensitive on Windows)
- `findFileInDirectories()` - Find file in search directories
- `globFilesInDirectory()` - Find files matching regex pattern
- `globFilesInDirectories()` - Find files in data directories matching regex pattern

#### Shader Operations
- `findFileInShaderDirectories()` - Find shader file
- `getShaderDirectoriesList()` - Get shader directories

#### File Dialog Functions
- `FileDialogFilter` struct - File dialog filter
- `openFileDialog()` - Open file dialog
- `saveFileDialog()` - Save file dialog
- `chooseFolderDialog()` - Choose folder dialog

#### File Monitoring
- `monitorFileUpdates()` - Monitor file for changes
- `closeSharedFile()` - Close file monitoring

#### Temporary File Operations
- `getTempFilePath()` - Get unique temporary file path

#### Junction Operations
- `createJunction()` - Create junction (soft link)
- `deleteJunction()` - Delete junction

#### Process Operations
- `executeProcess()` - Execute process
- `isProcessRunning()` - Check if process is running
- `terminateProcess()` - Terminate process

#### Directory Operations
- `getProjectDirectory()` - Get project directory
- `getExecutablePath()` - Get executable path
- `getExecutableDirectory()` - Get executable directory
- `getExecutableName()` - Get executable name
- `getRuntimeDirectory()` - Get runtime directory
- `getAppDataDirectory()` - Get app data directory
- `getHomeDirectory()` - Get home directory

#### Environment Variables
- `getEnvironmentVariable()` - Get environment variable

#### Filename Generation
- `findAvailableFilename()` - Find available filename

#### Keyboard Interrupt
- `setKeyboardInterruptHandler()` - Set Ctrl-C handler

#### Debugging Utilities
- `isDebuggerPresent()` - Check if debugger is present
- `debugBreak()` - Break in debugger
- `printToDebugWindow()` - Print to debug window

#### Development Mode
- `isDevelopmentMode()` - Check if in development mode

#### File Extension Operations
- `hasExtension()` - Check if path has extension
- `getExtensionFromPath()` - Get extension from path

#### Thread Operations
- `getCurrentThread()` - Get current thread handle
- `setThreadAffinity()` - Set thread affinity mask
- `setThreadPriority()` - Set thread priority

#### File Operations
- `getFileModifiedTime()` - Get file modification time

#### Memory Operations
- `getTotalVirtualMemory()` - Get total virtual memory
- `getUsedVirtualMemory()` - Get used virtual memory
- `getProcessUsedVirtualMemory()` - Get process used virtual memory
- `getCurrentRSS()` - Get current resident set size
- `getPeakRSS()` - Get peak resident set size

#### Bit Operations
- `bitScanReverse()` - Find most significant set bit
- `bitScanForward()` - Find least significant set bit
- `popcount()` - Count set bits

#### File I/O
- `readFile()` - Read file contents
- `decompressFile()` - Read and decompress .gz file

#### Shared Library Operations
- `loadSharedLibrary()` - Load shared library
- `releaseSharedLibrary()` - Release shared library
- `getProcAddress()` - Get function pointer from library

#### Stack Trace
- `getStackTrace()` - Get stack trace as string

## Dependencies

### Internal Dependencies

- **Macros**: Core macros for platform detection (FALCOR_WINDOWS, FALCOR_LINUX)
- **PlatformHandles**: Platform-specific handle types
- **StringUtils**: String utilities for wstring_2_string conversion
- **StringFormatters**: String formatting utilities (fmt library)

### External Dependencies

#### Windows Dependencies

- **windows.h**: Windows API headers
- **Setup API**: Windows Setup API for device enumeration
- **Registry API**: Windows Registry API for device information
- **File System API**: Windows file system API

#### Linux Dependencies

- **GLFW**: Cross-platform window and input library
- **File System API**: POSIX file system API

#### Common Dependencies

- **std::filesystem**: Standard library filesystem
- **std::string**: Standard library string container
- **std::vector**: Standard library vector container
- **std::regex**: Standard library regex
- **std::thread**: Standard library thread
- **std::mutex**: Standard library mutex
- **std::function**: Standard library function wrapper
- **std::optional**: Standard library optional type
- **fstd::span**: Standard library span (C++20)
- **zlib**: Zlib compression library
- **backward**: Backward library for stack trace
- **fmt**: Format library for string formatting

## Cross-Platform Considerations

- **Cross-Platform**: Works on Windows and Linux
- **Platform-Specific Code**: Conditional compilation for Windows and Linux
- **Unified Interface**: Same API across all platforms
- **Platform Detection**: Uses FALCOR_WINDOWS and FALCOR_LINUX macros
- **Unknown OS**: Compile-time error for unsupported platforms
- **Different Implementations**: Different APIs for Windows and Linux
- **GLFW Dependency**: Linux implementation requires GLFW
- **Zlib Dependency**: Decompression requires zlib

## Usage Patterns

### Getting Display Information

```cpp
int dpi = OS::getDisplayDpi();
float scaleFactor = OS::getDisplayScaleFactor();
```

### Displaying Message Box

```cpp
// Simple message box
OS::MsgBoxButton result = OS::msgBox("Title", "Message");

// Message box with icon
OS::MsgBoxButton result = OS::msgBox("Warning", "Warning message", OS::MsgBoxType::Ok, OS::MsgBoxIcon::Warning);

// Message box with buttons
OS::MsgBoxButton result = OS::msgBox("Confirm", "Are you sure?", OS::MsgBoxType::YesNo, OS::MsgBoxIcon::Info);

// Custom message box
std::vector<OS::MsgBoxCustomButton> buttons = {
    {1, "Button 1"},
    {2, "Button 2"},
    {3, "Button 3"}
};
uint32_t result = OS::msgBox("Custom", "Choose an option", buttons, OS::MsgBoxIcon::Question, 1);
```

### File Dialogs

```cpp
// Open file dialog
std::vector<OS::FileDialogFilter> filters = {
    {"PNG Files", "png"},
    {"JPEG Files", "jpg"}
};
std::filesystem::path filePath;
if (OS::openFileDialog(filters, filePath))
{
    std::cout << "Selected: " << filePath << std::endl;
}

// Save file dialog
std::filesystem::path filePath;
if (OS::saveFileDialog(filters, filePath))
{
    std::cout << "Save to: " << filePath << std::endl;
}

// Choose folder dialog
std::filesystem::path folderPath;
if (OS::chooseFolderDialog(folderPath))
{
    std::cout << "Selected folder: " << folderPath << std::endl;
}
```

### Path Operations

```cpp
// Compare paths
bool same = OS::isSamePath(path1, path2);

// Find file in directories
std::vector<std::filesystem::path> searchDirs = {"/dir1", "/dir2", "/dir3"};
std::filesystem::path found = OS::findFileInDirectories("file.txt", fstd::span(searchDirs));

// Glob files in directory
std::regex pattern(R"(.*\.png)");
std::vector<std::filesystem::path> pngFiles = OS::globFilesInDirectory("/path/to/dir", pattern);

// Glob files in multiple directories
std::vector<std::filesystem::path> searchDirs = {"/dir1", "/dir2"};
std::vector<std::filesystem::path> pngFiles = OS::globFilesInDirectories("file.png", pattern, fstd::span(searchDirs));
```

### Shader Directory Operations

```cpp
// Find shader file
std::filesystem::path shaderPath;
if (OS::findFileInShaderDirectories("shader.slang", shaderPath))
{
    std::cout << "Found: " << shaderPath << std::endl;
}

// Get shader directories
const auto& shaderDirs = OS::getShaderDirectoriesList();
for (const auto& dir : shaderDirs)
{
    std::cout << "Shader dir: " << dir << std::endl;
}
```

### Temporary File Operations

```cpp
std::filesystem::path tempFile = OS::getTempFilePath();
std::ofstream ofs(tempFile);
ofs << "Temporary data";
ofs.close();
```

### Junction Operations

```cpp
// Create junction
if (OS::createJunction("link", "target"))
{
    std::cout << "Junction created" << std::endl;
}

// Delete junction
if (OS::deleteJunction("link"))
{
    std::cout << "Junction deleted" << std::endl;
}
```

### Process Operations

```cpp
// Execute process
size_t processID = OS::executeProcess("app.exe", "arg1 arg2");

// Check if process is running
if (OS::isProcessRunning(processID))
{
    std::cout << "Process is running" << std::endl;
}

// Terminate process
OS::terminateProcess(processID);
```

### Directory Queries

```cpp
// Get project directory
const auto& projectDir = OS::getProjectDirectory();

// Get executable path
const auto& exePath = OS::getExecutablePath();

// Get executable directory
const auto& exeDir = OS::getExecutableDirectory();

// Get executable name
const auto& exeName = OS::getExecutableName();

// Get runtime directory
const auto& runtimeDir = OS::getRuntimeDirectory();

// Get app data directory
const auto& appDataDir = OS::getAppDataDirectory();

// Get home directory
const auto& homeDir = OS::getHomeDirectory();
```

### Environment Variables

```cpp
// Get environment variable
auto value = OS::getEnvironmentVariable("PATH");
if (value)
{
    std::cout << "PATH: " << *value << std::endl;
}
```

### Filename Generation

```cpp
// Find available filename
std::filesystem::path filename = OS::findAvailableFilename("temp", "/path/to/dir", "txt");
```

### Debugging Utilities

```cpp
// Check if debugger is present
if (OS::isDebuggerPresent())
{
    std::cout << "Debugger is attached" << std::endl;
}

// Break in debugger
OS::debugBreak();

// Print to debug window
OS::printToDebugWindow("Debug message");

// Check development mode
if (OS::isDevelopmentMode())
{
    std::cout << "Development mode" << std::endl;
}
```

### File Extension Operations

```cpp
// Check if path has extension
bool hasPng = OS::hasExtension("image.png", "png");

// Get extension from path
std::string ext = OS::getExtensionFromPath("image.png"); // "png"
```

### Thread Operations

```cpp
// Get current thread
std::thread::native_handle_type thread = OS::getCurrentThread();

// Set thread affinity
OS::setThreadAffinity(thread, 0x1); // Core 0

// Set thread priority
OS::setThreadPriority(thread, OS::ThreadPriorityType::High);
```

### Memory Queries

```cpp
// Get memory information
uint64_t totalVM = OS::getTotalVirtualMemory();
uint64_t usedVM = OS::getUsedVirtualMemory();
uint64_t processVM = OS::getProcessUsedVirtualMemory();
uint64_t currentRSS = OS::getCurrentRSS();
uint64_t peakRSS = OS::getPeakRSS();
```

### Bit Operations

```cpp
// Bit operations
uint32_t value = 0b1010;
uint32_t msbIndex = OS::bitScanReverse(value); // Index of MSB
uint32_t lsbIndex = OS::bitScanForward(value); // Index of LSB
uint32_t bitCount = OS::popcount(value); // Count of set bits
```

### File I/O

```cpp
// Read file
std::string content = OS::readFile("/path/to/file.txt");

// Decompress file
std::string decompressed = OS::decompressFile("/path/to/file.gz");
```

### Shared Library Operations

```cpp
// Load shared library
OS::SharedLibraryHandle library = OS::loadSharedLibrary("/path/to/library.so");

// Get function pointer
void* funcPtr = OS::getProcAddress(library, "functionName");

// Release library
OS::releaseSharedLibrary(library);
```

### Stack Trace

```cpp
// Get stack trace
std::string stackTrace = OS::getStackTrace(0, 10); // Skip 0 frames, max 10 depth
std::cout << stackTrace << std::endl;
```

## Summary

**OS** is a comprehensive cross-platform operating system abstraction that provides:

### OSServices
- 0 bytes total size (no member variables)
- All methods are static
- Start/stop OS services
- Platform-specific implementations

### FileDialogFilter
- ~48 bytes total size (excluding std::string internal allocations)
- 8-byte natural alignment
- No padding
- Excellent cache locality (all members in single cache line)
- Description and extension strings

### MsgBoxCustomButton
- ~20 bytes total size (excluding std::string internal allocation)
- 8-byte natural alignment
- 4 bytes of padding
- Excellent cache locality (all members in single cache line)
- Button ID and title string

### Comprehensive Functionality
- Display management (DPI, scale factor, window icon)
- Message box system (icons, types, buttons, custom buttons)
- Path operations (comparison, search, globbing, canonicalization)
- Shader directory management (development/deployment modes)
- File dialogs (open, save, folder)
- File monitoring (watch thread, callbacks)
- Temporary file management (unique paths, thread-safe)
- Junction management (create, delete)
- Process management (execute, running check, terminate)
- Directory queries (project, executable, runtime, app data, home)
- Environment variables (query)
- Filename generation (unique with index)
- Debugging utilities (debugger check, break, debug output, development mode)
- File extension operations (has, get)
- Thread management (current thread, affinity, priority)
- Memory queries (total VM, used VM, process VM, RSS, peak RSS)
- Bit operations (scan reverse, scan forward, popcount)
- File I/O (read, decompress)
- Shared library management (load, release, get proc address)
- Stack trace (capture, format)

The module implements a comprehensive cross-platform OS abstraction with excellent cache locality for data structures, mixed thread safety (read-only queries are thread-safe), constant-time system queries, linear-time path and file operations, comprehensive functionality covering display, UI, filesystem, process, threading, memory, and debugging operations, providing a solid foundation for cross-platform development in Falcor engine.
