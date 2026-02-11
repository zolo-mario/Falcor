# Logger - Logging System

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- Falcor/Core/Macros (FALCOR_API, FALCOR_UNREACHABLE, FALCOR_PRINT)
- Falcor/Core/Error (FALCOR_THROW)
- Falcor/Core/Platform/OS (getExecutableName, getRuntimeDirectory)
- Falcor/Utils/Scripting/ScriptBindings (FALCOR_SCRIPT_BINDING)
- Falcor/Utils/StringFormatters
- fmt/core.h (fmt library)
- std::filesystem
- std::string_view
- std::mutex
- std::set

### Dependent Modules

- Falcor/Core (core logging)
- Falcor/Scene (scene logging)
- Falcor/Rendering (rendering logging)
- Falcor/Utils (internal utilities)
- Python scripts (Python bindings)

## Module Overview

Logger provides a comprehensive logging system for Falcor with support for multiple severity levels, output destinations, and message deduplication. The logger supports console output, file logging, and debug window output with configurable verbosity levels. It includes Python bindings for script access and thread-safe message deduplication for "Once" frequency messages.

## Component Specifications

### Logger Class

**File**: `Source/Falcor/Utils/Logger.h` (212 lines)
**File**: `Source/Falcor/Utils/Logger.cpp` (267 lines)

Static class providing comprehensive logging functionality.

#### Level Enum

```cpp
enum class Level
{
    Disabled, ///< Disable log messages.
    Fatal,    ///< Fatal messages.
    Error,    ///< Error messages.
    Warning,  ///< Warning messages.
    Info,     ///< Informative messages.
    Debug,    ///< Debugging messages.
    Count,    ///< Keep this last.
};
```

**Purpose**: Defines log message severity levels

**Values**:
- **Disabled**: No logging
- **Fatal**: Critical errors that terminate the application
- **Error**: Error messages that don't terminate
- **Warning**: Warning messages for potential issues
- **Info**: Informational messages
- **Debug**: Debugging messages for development
- **Count**: Counter messages (not used for logging)

#### Frequency Enum

```cpp
enum class Frequency
{
    Always, ///< Reports the message always
    Once,   ///< Reports the message only first time exact string appears
};
```

**Purpose**: Defines message frequency control

**Values**:
- **Always**: Report every message
- **Once**: Report message only once per unique string

#### OutputFlags Enum

```cpp
enum class OutputFlags
{
    None = 0x0,        ///< No output.
    Console = 0x2,     ///< Output to console (stdout/stderr).
    File = 0x1,        ///< Output to log file.
    DebugWindow = 0x4, ///< Output to debug window (if debugger is attached).
};
```

**Purpose**: Defines log output destinations

**Values**:
- **None**: No output
- **Console**: Output to stdout/stderr
- **File**: Output to log file
- **DebugWindow**: Output to debug window (if debugger attached)

**Bitwise Combinations**: Multiple outputs can be combined:
```cpp
Logger::OutputFlags::Console | Logger::OutputFlags::File  // Both console and file
```

#### Static Methods

**shutdown()**:
```cpp
static void shutdown();
```

**Purpose**: Shutdown logger and close log file

**Behavior**:
- Closes log file if open
- Sets sLogFile to nullptr
- Sets sInitialized to false
- Thread-safe via mutex lock

**setVerbosity()**:
```cpp
static void setVerbosity(Level level);
```

**Purpose**: Sets logger verbosity level

**Behavior**:
- Thread-safe via mutex lock
- Sets sVerbosity to specified level
- Messages below this level are not logged

**getVerbosity()**:
```cpp
static Level getVerbosity();
```

**Purpose**: Gets current logger verbosity level

**Returns**: Current verbosity level

**setOutputs()**:
```cpp
static void setOutputs(OutputFlags outputs);
```

**Purpose**: Sets logger output destinations

**Behavior**:
- Thread-safe via mutex lock
- Sets sOutputs to specified flags
- Controls where messages are sent

**getOutputs()**:
```cpp
static OutputFlags getOutputs();
```

**Purpose**: Gets current output destinations

**Returns**: Current output flags

**setLogFilePath()**:
```cpp
static void setLogFilePath(const std::filesystem::path& path);
```

**Purpose**: Sets path of log file

**Behavior**:
- Thread-safe via mutex lock
- Closes existing log file if open
- Sets sLogFilePath to specified path
- Sets sLogFile to nullptr (will open on next write)
- Sets sInitialized to false

**getLogFilePath()**:
```cpp
static std::filesystem::path getLogFilePath();
```

**Purpose**: Gets current log file path

**Returns**: Current log file path

**log()**:
```cpp
static void log(Level level, const std::string_view msg, Frequency frequency = Frequency::Always);
```

**Purpose**: Logs a message with specified level and frequency

**Parameters**:
- level: Log level
- msg: Log message
- frequency: Message frequency (default: Always)

**Behavior**:
- Thread-safe via mutex lock
- Checks if level <= sVerbosity
- Formats message with level prefix and newline
- Handles "Once" frequency with deduplication
- Writes to console, file, and/or debug window based on sOutputs
- Flushes file output

**Private Constructor**:
```cpp
Logger() = delete;
```

**Purpose**: Prevents instantiation

**Behavior**: Logger is static-only class

#### Logging Helper Functions

**logDebug(msg)**:
```cpp
inline void logDebug(const std::string_view msg)
{
    Logger::log(Logger::Level::Debug, msg);
}
```

**Purpose**: Logs debug message

**logDebug(format, args...)**:
```cpp
template<typename... Args>
inline void logDebug(fmt::format_string<Args...> format, Args&&... args)
{
    Logger::log(Logger::Level::Debug, fmt::format(format, std::forward<Args>(args)...));
}
```

**Purpose**: Logs formatted debug message

**logInfo(msg)**:
```cpp
inline void logInfo(const std::string_view msg)
{
    Logger::log(Logger::Level::Info, msg);
}
```

**Purpose**: Logs info message

**logInfo(format, args...)**:
```cpp
template<typename... Args>
inline void logInfo(fmt::format_string<Args...> format, Args&&... args)
{
    Logger::log(Logger::Level::Info, fmt::format(format, std::forward<Args>(args)...));
}
```

**Purpose**: Logs formatted info message

**logWarning(msg)**:
```cpp
inline void logWarning(const std::string_view msg)
{
    Logger::log(Logger::Level::Warning, msg);
}
```

**Purpose**: Logs warning message

**logWarning(format, args...)**:
```cpp
template<typename... Args>
inline void logWarning(fmt::format_string<Args...> format, Args&&... args)
{
    Logger::log(Logger::Level::Warning, fmt::format(format, std::forward<Args>(args)...));
}
```

**Purpose**: Logs formatted warning message

**logWarningOnce(msg)**:
```cpp
inline void logWarningOnce(const std::string_view msg)
{
    Logger::log(Logger::Level::Warning, msg, Logger::Frequency::Once);
}
```

**Purpose**: Logs warning message only once

**logWarningOnce(format, args...)**:
```cpp
template<typename... Args>
inline void logWarningOnce(fmt::format_string<Args...> format, Args&&... args)
{
    Logger::log(Logger::Level::Warning, fmt::format(format, std::forward<Args>(args)...), Logger::Frequency::Once);
}
```

**Purpose**: Logs formatted warning message only once

**logError(msg)**:
```cpp
inline void logError(const std::string_view msg)
{
    Logger::log(Logger::Level::Error, msg);
}
```

**Purpose**: Logs error message

**logError(format, args...)**:
```cpp
template<typename... Args>
inline void logError(fmt::format_string<Args...> format, Args&&... args)
{
    Logger::log(Logger::Level::Error, fmt::format(format, std::forward<Args>(args)...));
}
```

**Purpose**: Logs formatted error message

**logErrorOnce(msg)**:
```cpp
inline void logErrorOnce(const std::string_view msg)
{
    Logger::log(Logger::Level::Error, msg, Logger::Frequency::Once);
}
```

**Purpose**: Logs error message only once

**logErrorOnce(format, args...)**:
```cpp
template<typename... Args>
inline void logErrorOnce(fmt::format_string<Args...> format, Args&&... args)
{
    Logger::log(Logger::Level::Error, fmt::format(format, std::forward<Args>(args)...), Logger::Frequency::Once);
}
```

**Purpose**: Logs formatted error message only once

**logFatal(msg)**:
```cpp
inline void logFatal(const std::string_view msg)
{
    Logger::log(Logger::Level::Fatal, msg);
}
```

**Purpose**: Logs fatal message

**logFatal(format, args...)**:
```cpp
template<typename... Args>
inline void logFatal(fmt::format_string<Args...> format, Args&&... args)
{
    Logger::log(Logger::Level::Fatal, fmt::format(format, std::forward<Args>(args)...));
}
```

**Purpose**: Logs formatted fatal message

**FALCOR_PRINT Macro**:
```cpp
#define FALCOR_PRINT(x) \
    do \
    { \
        ::Falcor::logInfo("{} = {}", #x, x); \
    } while (0)
```

**Purpose**: Convenience macro for logging

**Behavior**:
- Logs info message with formatted output
- Uses do-while(0) pattern for single statement
- Example: FALCOR_PRINT("Hello {}", "world") -> "Hello world"

### MessageDeduplicator Class

**File**: `Source/Falcor/Utils/Logger.cpp` (lines 123-147)

Helper class for deduplicating messages.

#### Static Methods

**instance()**:
```cpp
static MessageDeduplicator& instance()
{
    static MessageDeduplicator sInstance;
    return sInstance;
}
```

**Purpose**: Returns singleton instance

**Returns**: Reference to singleton instance

**isDuplicate(msg)**:
```cpp
bool isDuplicate(std::string_view msg)
{
    std::lock_guard<std::mutex> lock(mMutex);
    auto it = mStrings.find(msg);
    if (it != mStrings.end())
        return true;
    mStrings.insert(std::string(msg));
    return false;
}
```

**Purpose**: Checks if message has been logged before

**Behavior**:
- Thread-safe via mutex lock
- Searches for message in set
- Returns true if found, false otherwise
- Inserts message if not found

#### Private Members

```cpp
std::mutex mMutex;
std::set<std::string> mStrings;
```

**mMutex**: Mutex for thread-safe access

**mStrings**: Set of logged messages for deduplication

### Logger Implementation

#### Static Members

```cpp
std::mutex sMutex;
Logger::Level sVerbosity = Logger::Level::Info;
Logger::OutputFlags sOutputs = Logger::OutputFlags::Console | Logger::OutputFlags::File | Logger::OutputFlags::DebugWindow;
std::filesystem::path sLogFilePath;
FILE* sLogFile = nullptr;
bool sInitialized = false;
```

**sMutex**: Mutex for thread-safe access to static members

**sVerbosity**: Current verbosity level (default: Info)

**sOutputs**: Current output flags (default: Console | File | DebugWindow)

**sLogFilePath**: Path to log file

**sLogFile**: File handle for log file

**sInitialized**: Whether logger has been initialized

#### Helper Functions

**generateLogFilePath()**:
```cpp
std::filesystem::path generateLogFilePath()
{
    std::string prefix = getExecutableName();
    std::filesystem::path directory = getRuntimeDirectory();
    return findAvailableFilename(prefix, directory, "log");
}
```

**Purpose**: Generates unique log file path

**Behavior**:
- Gets executable name as prefix
- Gets runtime directory
- Finds available filename with "log" extension
- Returns path to log file

**openLogFile()**:
```cpp
FILE* openLogFile()
{
    FILE* pFile = nullptr;

    if (sLogFilePath.empty())
    {
        sLogFilePath = generateLogFilePath();
    }

    pFile = std::fopen(sLogFilePath.string().c_str(), "w");
    if (pFile != nullptr)
    {
        return pFile;
    }

    FALCOR_UNREACHABLE();
    return pFile;
}
```

**Purpose**: Opens log file for writing

**Behavior**:
- Generates log file path if empty
- Opens file in write mode
- Throws if file cannot be opened

**printToLogFile()**:
```cpp
void printToLogFile(const std::string& s)
{
    if (!sInitialized)
    {
        sLogFile = openLogFile();
        sInitialized = true;
    }

    if (sLogFile)
    {
        std::fprintf(sLogFile, "%s", s.c_str());
        std::fflush(sLogFile);
    }
}
```

**Purpose**: Prints string to log file

**Behavior**:
- Opens log file if not initialized
- Writes string to file
- Flushes file to ensure data is written

**getLogLevelString()**:
```cpp
inline const char* getLogLevelString(Logger::Level level)
{
    switch (level)
    {
    case Logger::Level::Fatal:
        return "(Fatal)";
    case Logger::Level::Error:
        return "(Error)";
    case Logger::Level::Warning:
        return "(Warning)";
    case Logger::Level::Info:
        return "(Info)";
    case Logger::Level::Debug:
        return "(Debug)";
    default:
        FALCOR_UNREACHABLE();
        return nullptr;
    }
}
```

**Purpose**: Converts log level to string

**Returns**: String representation of log level

#### Logger::log() Implementation

```cpp
void Logger::log(Level level, const std::string_view msg, Frequency frequency)
{
    std::lock_guard<std::mutex> lock(sMutex);
    if (level <= sVerbosity)
    {
        std::string s = fmt::format("{} {}\n", getLogLevelString(level), msg);

        if (frequency == Frequency::Once && MessageDeduplicator::instance().isDuplicate(s))
            return;

        // Write to console.
        if (is_set(sOutputs, OutputFlags::Console))
        {
            auto& os = level > Logger::Level::Error ? std::cout : std::cerr;
            os << s;
            os.flush();
        }

        // Write to file.
        if (is_set(sOutputs, OutputFlags::File))
        {
            printToLogFile(s);
        }

        // Write to debug window if debugger is attached.
        if (is_set(sOutputs, OutputFlags::DebugWindow) && isDebuggerPresent())
        {
            printToDebugWindow(s);
        }
    }
}
```

**Purpose**: Core logging implementation

**Behavior**:
- Thread-safe via mutex lock
- Checks if level <= verbosity
- Formats message with level prefix
- Handles "Once" frequency with deduplication
- Writes to console (stdout/stderr based on level)
- Writes to log file
- Writes to debug window if debugger attached
- Flushes outputs

**Output Selection**:
- Fatal/Error → std::cerr
- Info/Warning/Debug → std::cout

**Message Format**:
```
(Level) Message\n
```

Example:
```
(Info) Hello World\n
(Error) Something went wrong\n
```

#### Configuration Methods

**setVerbosity()**:
```cpp
void Logger::setVerbosity(Level level)
{
    std::lock_guard<std::mutex> lock(sMutex);
    sVerbosity = level;
}
```

**getVerbosity()**:
```cpp
Logger::Level Logger::getVerbosity()
{
    std::lock_guard<std::mutex> lock(sMutex);
    return sVerbosity;
}
```

**setOutputs()**:
```cpp
void Logger::setOutputs(OutputFlags outputs)
{
    std::lock_guard<std::mutex> lock(sMutex);
    sOutputs = outputs;
}
```

**getOutputs()**:
```cpp
Logger::OutputFlags Logger::getOutputs()
{
    std::lock_guard<std::mutex> lock(sMutex);
    return sOutputs;
}
```

**setLogFilePath()**:
```cpp
void Logger::setLogFilePath(const std::filesystem::path& path)
{
    std::lock_guard<std::mutex> lock(sMutex);
    if (sLogFile)
    {
        fclose(sLogFile);
        sLogFile = nullptr;
        sInitialized = false;
    }
    sLogFilePath = path;
}
```

**getLogFilePath()**:
```cpp
std::filesystem::path Logger::getLogFilePath()
{
    std::lock_guard<std::mutex> lock(sMutex);
    return sLogFilePath;
}
```

**shutdown()**:
```cpp
void Logger::shutdown()
{
    if (sLogFile)
    {
        fclose(sLogFile);
        sLogFile = nullptr;
        sInitialized = false;
    }
}
```

**Purpose**: Closes log file and resets logger state

### Python Bindings

**File**: `Source/Falcor/Utils/Logger.cpp` (lines 223-265)

```cpp
FALCOR_SCRIPT_BINDING(Logger)
{
    using namespace pybind11::literals;

    pybind11::class_<Logger> logger(m, "Logger");

    pybind11::enum_<Logger::Level> level(logger, "Level");
    level.value("Disabled", Logger::Level::Disabled);
    level.value("Fatal", Logger::Level::Fatal);
    level.value("Error", Logger::Level::Error);
    level.value("Warning", Logger::Level::Warning);
    level.value("Info", Logger::Level::Info);
    level.value("Debug", Logger::Level::Debug);

    pybind11::enum_<Logger::OutputFlags> outputFlags(logger, "OutputFlags");
    outputFlags.value("None_", Logger::OutputFlags::None);
    outputFlags.value("Console", Logger::OutputFlags::Console);
    outputFlags.value("File", Logger::OutputFlags::File);
    outputFlags.value("DebugWindow", Logger::OutputFlags::DebugWindow);

    logger.def_property_static(
        "verbosity",
        [](pybind11::object) { return Logger::getVerbosity(); },
        [](pybind11::object, Logger::Level verbosity) { Logger::setVerbosity(verbosity); }
    );

    logger.def_property_static(
        "outputs",
        [](pybind11::object) { return Logger::getOutputs(); },
        [](pybind11::object, Logger::OutputFlags outputs) { Logger::setOutputs(outputs); }
    );

    logger.def_property_static(
        "log_file_path",
        [](pybind11::object) { return Logger::getLogFilePath(); },
        [](pybind11::object, std::filesystem::path path) { Logger::setLogFilePath(path); }
    );

    logger.def_static(
        "log",
        [](pybind11::object, Logger::Level level, const std::string_view msg) { Logger::log(level, msg, Logger::Frequency::Always); },
        "level"_a,
        "msg"_a
    );
}
```

**Purpose**: Python bindings for Logger class

**Bound Properties**:
- **verbosity**: Get/set verbosity level
- **outputs**: Get/set output flags
- **log_file_path**: Get/set log file path

**Bound Methods**:
- **log(level, msg)**: Log message with Always frequency

**Enum Values**:
- **Level**: Disabled, Fatal, Error, Warning, Info, Debug
- **OutputFlags**: None_, Console, File, DebugWindow

## Technical Details

### Verbosity Filtering

Messages are filtered based on verbosity level:

```
Message Level <= Verbosity Level → Log
Message Level > Verbosity Level → Don't Log
```

**Example**:
```
Verbosity = Info:
- Info messages → Log
- Debug messages → Don't Log

Verbosity = Debug:
- Info messages → Log
- Debug messages → Log
```

### Message Deduplication

The MessageDeduplicator class tracks duplicate messages:

```
isDuplicate(msg):
1. Lock mutex
2. Search for msg in mStrings
3. If found: return true
4. If not found: insert msg, return false
```

**Purpose**: Prevents spamming with "Once" frequency

**Thread Safety**: Uses std::mutex for concurrent access

### Output Routing

Messages are routed to configured outputs:

```
sOutputs & Console → Write to stdout/stderr
sOutputs & File → Write to log file
sOutputs & DebugWindow → Write to debug window
```

**Console Output**:
- Fatal/Error → std::cerr
- Info/Warning/Debug → std::cout

**File Output**:
- Writes to log file
- Flushes after each write

**Debug Window Output**:
- Only if debugger is attached
- Uses platform-specific debug output function

### Message Formatting

Messages are formatted with level prefix:

```
(Level) Message\n
```

**Level Strings**:
- Fatal → "(Fatal)"
- Error → "(Error)"
- Warning → "(Warning)"
- Info → "(Info)"
- Debug → "(Debug)"

**Example**:
```
(Info) Application started
(Error) Failed to load file
(Warning) Deprecated function used
(Debug) Variable value = 42
```

### Thread Safety

All static member access is protected by mutex:

```cpp
std::lock_guard<std::mutex> lock(sMutex);
// ... access static members ...
```

**Protected Operations**:
- Verbosity changes
- Output changes
- Log file path changes
- Log file operations
- Message deduplication

### Log File Management

**File Generation**:
```cpp
generateLogFilePath():
1. Get executable name as prefix
2. Get runtime directory
3. Find available filename (prefix + ".log")
4. Return path
```

**File Opening**:
```cpp
openLogFile():
1. Generate log file path if empty
2. Open file in write mode
3. Throw if cannot open
4. Return file handle
```

**File Writing**:
```cpp
printToLogFile():
1. Open log file if not initialized
2. Write string to file
3. Flush file
```

**File Closing**:
```cpp
shutdown():
1. Close log file if open
2. Reset file handle
3. Reset initialized flag
```

### Python Integration

Logger is exposed to Python via pybind11:

**Properties**:
- **verbosity**: Get/set verbosity level
- **outputs**: Get/set output flags
- **log_file_path**: Get/set log file path

**Methods**:
- **log(level, msg)**: Log message with Always frequency

**Usage Example**:
```python
import falcor

# Set verbosity to debug
falcor.Logger.verbosity = falcor.Logger.Level.Debug

# Set outputs to console and file
falcor.Logger.outputs = falcor.Logger.OutputFlags.Console | falcor.Logger.OutputFlags.File

# Log message
falcor.Logger.log(falcor.Logger.Level.Info, "Application started")
```

## Integration Points

### Falcor Core Integration

- **Core/Macros**: FALCOR_API, FALCOR_UNREACHABLE, FALCOR_PRINT
- **Core/Error**: FALCOR_THROW for error reporting
- **Core/Platform/OS**: getExecutableName, getRuntimeDirectory

### Falcor Utils Integration

- **Utils/Scripting/ScriptBindings**: FALCOR_SCRIPT_BINDING macro
- **Utils/StringFormatters**: String formatting utilities

### External Dependencies

- **fmt/core.h**: fmt library for formatting
- **std::filesystem**: File system operations
- **std::mutex**: Thread synchronization
- **std::set**: Deduplication storage
- **pybind11**: Python bindings

### Internal Falcor Usage

- **Core**: Core logging
- **Scene**: Scene logging
- **Rendering**: Rendering logging
- **Utils**: Internal utilities logging
- **Python Scripts**: Python script logging

## Architecture Patterns

### Static Class Pattern

Logger is static-only class:

```cpp
class FALCOR_API Logger
{
    Logger() = delete;  // Prevent instantiation
    static void log(...);  // Static methods only
};
```

Benefits:
- Single global instance
- No construction overhead
- Clear API

### Singleton Pattern

MessageDeduplicator uses singleton pattern:

```cpp
static MessageDeduplicator& instance()
{
    static MessageDeduplicator sInstance;
    return sInstance;
}
```

Benefits:
- Single instance for deduplication
- Thread-safe via mutex
- Global state management

### RAII Pattern

Uses lock_guard for automatic mutex unlocking:

```cpp
std::lock_guard<std::mutex> lock(sMutex);
// ... access protected resources ...
// Automatic unlock when lock goes out of scope
```

Benefits:
- Automatic mutex unlocking
- Exception safety
- No manual unlock needed

### Template Helper Pattern

Template functions for formatted logging:

```cpp
template<typename... Args>
inline void logInfo(fmt::format_string<Args...> format, Args&&... args)
{
    Logger::log(Logger::Level::Info, fmt::format(format, std::forward<Args>(args)...));
}
```

Benefits:
- Type-safe formatting
- Compile-time format string checking
- Perfect forwarding of arguments

### Macro Pattern

FALCOR_PRINT macro for convenience:

```cpp
#define FALCOR_PRINT(x) \
    do \
    { \
        ::Falcor::logInfo("{} = {}", #x, x); \
    } while (0)
```

Benefits:
- Single-statement logging
- Automatic formatting
- No semicolon needed

### Bitwise Pattern

OutputFlags uses bitwise operations:

```cpp
if (is_set(sOutputs, OutputFlags::Console))
{
    // Console output enabled
}
```

Benefits:
- Multiple outputs can be combined
- Efficient checking
- Clear intent

## Code Patterns

### Static Class Pattern

Prevent instantiation:

```cpp
Logger() = delete;
```

Pattern:
- Private deleted constructor
- Static methods only
- No instances possible

### Lock Guard Pattern

Automatic mutex unlocking:

```cpp
std::lock_guard<std::mutex> lock(sMutex);
// ... access protected resources ...
```

Pattern:
- RAII-based mutex locking
- Automatic unlock
- Exception safety

### Template Forwarding Pattern

Perfect forwarding of template arguments:

```cpp
template<typename... Args>
inline void logInfo(fmt::format_string<Args...> format, Args&&... args)
{
    Logger::log(Logger::Level::Info, fmt::format(format, std::forward<Args>(args)...));
}
```

Pattern:
- std::forward for perfect forwarding
- Preserves value category
- No extra copies

### Do-While Pattern

Single-statement macro:

```cpp
#define FALCOR_PRINT(x) \
    do \
    { \
        ::Falcor::logInfo("{} = {}", #x, x); \
    } while (0)
```

Pattern:
- do-while(0) for single statement
- Allows macro use with semicolon
- No statement block needed

### Switch-Default Pattern

Default case for unknown log levels:

```cpp
default:
    FALCOR_UNREACHABLE();
    return nullptr;
}
```

Pattern:
- Catch-all for unhandled cases
- Unreachable assertion
- Compile-time safety

### Singleton Pattern

Static instance for deduplication:

```cpp
static MessageDeduplicator& instance()
{
    static MessageDeduplicator sInstance;
    return sInstance;
}
```

Pattern:
- Static local instance
- Returns reference
- Thread-safe access

## Use Cases

### Basic Logging

```cpp
// Log info message
logInfo("Application started");

// Log warning message
logWarning("Deprecated function used");

// Log error message
logError("Failed to load file");

// Log fatal message
logFatal("Critical error, terminating");
```

### Formatted Logging

```cpp
// Formatted info message
logInfo("Processing {} items", count);

// Formatted warning message
logWarning("Value {} is out of range", value);

// Formatted error message
logError("Failed to open file: {}", filename);
```

### Deduplication

```cpp
// Log warning only once
logWarningOnce("This warning appears only once");

// Log error only once
logErrorOnce("This error appears only once");
```

### Verbosity Control

```cpp
// Set verbosity to debug
Logger::setVerbosity(Logger::Level::Debug);

// Debug messages will now be logged
logDebug("Debug information");

// Set verbosity to info
Logger::setVerbosity(Logger::Level::Info);

// Debug messages will not be logged
logDebug("This won't appear");
```

### Output Configuration

```cpp
// Enable console and file output
Logger::setOutputs(Logger::OutputFlags::Console | Logger::OutputFlags::File);

// Enable only file output
Logger::setOutputs(Logger::OutputFlags::File);

// Enable only console output
Logger::setOutputs(Logger::OutputFlags::Console);
```

### Log File Configuration

```cpp
// Set log file path
Logger::setLogFilePath("C:/Logs/myapp.log");

// Get log file path
auto path = Logger::getLogFilePath();

// Log messages will go to this file
logInfo("This will be logged to file");
```

### FALCOR_PRINT Macro

```cpp
// Print variable value
int value = 42;
FALCOR_PRINT("Value = {}", value);

// Print multiple values
FALCOR_PRINT("Width = {}, Height = {}", width, height);
```

### Python Usage

```python
import falcor

# Set verbosity to debug
falcor.Logger.verbosity = falcor.Logger.Level.Debug

# Set outputs
falcor.Logger.outputs = falcor.Logger.OutputFlags.Console | falcor.Logger.OutputFlags.File

# Log messages
falcor.Logger.log(falcor.Logger.Level.Info, "Application started")
falcor.Logger.log(falcor.Logger.Level.Warning, "Low memory")

# Set log file path
falcor.Logger.log_file_path = "C:/Logs/app.log"
```

## Performance Considerations

### Thread Safety

- **Mutex Overhead**: Mutex locking on every log call
- **Lock Contention**: Potential contention in multithreaded apps
- **Deduplication Overhead**: Set lookup for "Once" messages
- **File I/O**: File operations can be slow
- **Flush Overhead**: Flushing file after each write

### Memory Overhead

- **String Storage**: Deduplication stores all unique messages
- **Set Overhead**: std::set has overhead for insertions
- **Format String**: fmt::format creates temporary strings
- **Stack Usage**: Lock guard uses stack space

### I/O Performance

- **File Buffering**: File operations may be buffered by OS
- **Flush Cost**: Explicit flush after each write
- **Console I/O**: Console I/O can be slow in some environments

### Optimization Tips

1. **Reduce Verbosity**: Use appropriate verbosity level in production
2. **Disable Logging**: Disable logging for release builds
3. **File Logging**: Disable file logging for performance-critical code
4. **Batch Messages**: Combine multiple log messages when possible
5. **Avoid Format String**: Use raw string for simple messages

## Limitations

### Feature Limitations

- **No Log Rotation**: No automatic log file rotation
- **No Log Levels**: Only 6 fixed levels (no custom levels)
- **No Filtering**: No message filtering by content
- **No Async Logging**: All logging is synchronous
- **No Log Categories**: No categorization of messages
- **No Structured Logging**: No structured log data (JSON, etc.)
- **No Remote Logging**: No remote logging support

### API Limitations

- **Static Only**: Logger is static-only class
- **No Instance**: Cannot create Logger instances
- **No Inheritance**: Logger cannot be inherited
- **No Polymorphism**: No virtual functions
- **Limited Python API**: Only basic logging functions exposed

### Thread Safety Limitations

- **Global Mutex**: Single global mutex for all operations
- **Potential Contention**: High contention in multithreaded apps
- **No Fine-Grained Locking**: No per-logger locking
- **No Lock Hierarchy**: Single mutex for all operations

### Performance Limitations

- **File I/O**: File operations can be slow
- **Flush Overhead**: Explicit flush after each write
- **String Formatting**: fmt::format creates temporary strings
- **Deduplication**: Set lookup for "Once" messages
- **Mutex Overhead**: Mutex locking on every log call

### Platform Limitations

- **Debug Window**: Debug window output only works on Windows
- **File Paths**: Platform-specific file path handling
- **Console Output**: Console I/O may be slow on some platforms

## Best Practices

### When to Use

**Appropriate Use Cases**:
- Application logging
- Debugging information
- Error reporting
- Warning messages
- Performance tracking
- Development logging

**Inappropriate Use Cases**:
- High-frequency logging (use deduplication)
- Performance-critical paths (disable logging)
- Large data logging (use structured logging)
- Real-time systems (use async logging)

### Usage Patterns

**Verbosity Levels**:
```cpp
// Production: Info level
Logger::setVerbosity(Logger::Level::Info);

// Development: Debug level
Logger::setVerbosity(Logger::Level::Debug);

// Release: Disabled
Logger::setVerbosity(Logger::Level::Disabled);
```

**Output Configuration**:
```cpp
// Development: Console and file
Logger::setOutputs(Logger::OutputFlags::Console | Logger::OutputFlags::File | Logger::OutputFlags::DebugWindow);

// Production: File only
Logger::setOutputs(Logger::OutputFlags::File);

// Release: Console only
Logger::setOutputs(Logger::OutputFlags::Console);
```

**Deduplication**:
```cpp
// Use "Once" for warnings that should appear once
logWarningOnce("This warning appears only once");

// Use "Always" for regular messages
logInfo("Regular message");
```

**Formatted Logging**:
```cpp
// Use formatted logging for complex messages
logInfo("Processing {} items in {} ms", count, elapsedMs);

// Use raw strings for simple messages
logInfo("Application started");
```

### Thread Safety

- **Avoid Frequent Locking**: Minimize log calls in hot paths
- **Batch Messages**: Combine multiple log messages
- **Use Appropriate Verbosity**: Reduce logging in production
- **Consider Async Logging**: Use async logging for high-frequency logs

### Performance Tips

1. **Disable in Release**: Disable or reduce logging in release builds
2. **Use File Output**: File output is faster than console
3. **Avoid Frequent Formatting**: Minimize formatted logging
4. **Use Deduplication**: Use "Once" frequency for repeated messages
5. **Reduce Verbosity**: Use appropriate verbosity level

### Error Handling

- **Fatal Messages**: Use logFatal for critical errors
- **Error Messages**: Use logError for recoverable errors
- **Warning Messages**: Use logWarning for potential issues
- **Info Messages**: Use logInfo for informational messages
- **Debug Messages**: Use logDebug for debugging information

### Python Integration

- **Set Verbosity**: Use Logger.verbosity property
- **Set Outputs**: Use Logger.outputs property
- **Log Messages**: Use Logger.log() method
- **Set Log File**: Use Logger.log_file_path property

## Implementation Notes

### Static Class Design

Logger is designed as static-only class:

```cpp
class FALCOR_API Logger
{
    Logger() = delete;  // Prevent instantiation
    static void log(...);  // Static methods only
};
```

**Benefits**:
- Single global instance
- No construction overhead
- Clear API
- Prevents accidental instantiation

### Thread Safety Implementation

Uses std::mutex for thread-safe access:

```cpp
std::mutex sMutex;

void Logger::log(...) {
    std::lock_guard<std::mutex> lock(sMutex);
    // ... access static members ...
}
```

**Characteristics**:
- RAII-based locking
- Automatic unlocking
- Exception safety
- No manual unlock needed

### Message Deduplication

Uses std::set for tracking duplicate messages:

```cpp
std::set<std::string> mStrings;

bool isDuplicate(std::string_view msg) {
    std::lock_guard<std::mutex> lock(mMutex);
    auto it = mStrings.find(msg);
    if (it != mStrings.end())
        return true;
    mStrings.insert(std::string(msg));
    return false;
}
```

**Characteristics**:
- Thread-safe via mutex
- Stores all unique messages
- O(log n) lookup complexity

### File I/O

Uses C stdio for file operations:

```cpp
FILE* sLogFile = std::fopen(path, "w");
std::fprintf(sLogFile, "%s", s.c_str());
std::fflush(sLogFile);
fclose(sLogFile);
```

**Characteristics**:
- Standard C file I/O
- Manual file management
- No automatic rotation

### Python Bindings

Uses pybind11 for Python integration:

```cpp
pybind11::class_<Logger> logger(m, "Logger");
logger.def_property_static("verbosity", ...);
logger.def_property_static("outputs", ...);
logger.def_property_static("log_file_path", ...);
logger.def_static("log", ...);
```

**Characteristics**:
- Static properties for configuration
- Static method for logging
- Enum bindings for Level and OutputFlags
- Property setters for configuration

## Future Enhancements

### Potential Improvements

1. **Log Rotation**: Add automatic log file rotation
2. **Custom Levels**: Support custom log levels
3. **Message Filtering**: Add message content filtering
4. **Structured Logging**: Add JSON/structured logging
5. **Async Logging**: Add asynchronous logging support
6. **Log Categories**: Add categorization of messages
7. **Performance Metrics**: Add timing/performance metrics
8. **Stack Traces**: Add stack trace capture
9. **Multiple Loggers**: Support multiple independent loggers
10. **Log Levels**: Add more granular log levels

### API Extensions

1. **Log Methods**: Add more log methods (trace, verbose, etc.)
2. **Output Methods**: Add more output methods (syslog, event log, etc.)
3. **Configuration Methods**: Add more configuration options
4. **Query Methods**: Add query methods (get log file size, etc.)
5. **Flush Method**: Add explicit flush method
6. **Format Methods**: Add more format methods

### Performance Enhancements

1. **Async Logging**: Add asynchronous logging support
2. **Buffered I/O**: Add buffered file I/O
3. **Lazy Initialization**: Lazy initialization of log file
4. **Memory Pool**: Use memory pool for format strings
5. **Lock-Free**: Consider lock-free implementation
6. **Per-Logger Locking**: Use per-logger locking

### Thread Safety Enhancements

1. **Fine-Grained Locking**: Use fine-grained locking
2. **Lock Hierarchy**: Use lock hierarchy for different operations
3. **Read-Write Locks**: Use read-write locks
4. **Atomic Operations**: Use atomic operations where possible
5. **Lock-Free Algorithms**: Use lock-free algorithms

### Documentation Enhancements

1. **More Examples**: Add more usage examples
2. **Performance Guide**: Add performance optimization guide
3. **Best Practices**: Add best practices guide
4. **Troubleshooting**: Add troubleshooting guide
5. **API Reference**: Add complete API reference

## References

### C++ Standard

- **std::mutex**: C++11 mutex
- **std::lock_guard**: C++11 RAII lock guard
- **std::set**: C++11 unordered set
- **std::filesystem**: C++17 filesystem
- **std::string_view**: C++17 string view

### External Libraries

- **fmt**: fmt formatting library
- **pybind11**: Python bindings library

### Falcor Documentation

- **Falcor API**: Falcor API documentation
- **Falcor Utils**: Utils module documentation

### Related Technologies

- **Logging**: Logging systems and patterns
- **Structured Logging**: Structured logging formats
- **Async Logging**: Asynchronous logging patterns
- **Python Bindings**: Python integration patterns
