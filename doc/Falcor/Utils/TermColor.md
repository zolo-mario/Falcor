# TermColor - Terminal Color Utilities

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- Falcor/Core/Macros.h (FALCOR_WINDOWS, FALCOR_LINUX)
- std::iostream (std::cout, std::cerr, std::ostream)
- std::string (std::string)
- std::unordered_map (std::unordered_map)

### Platform-Specific Dependencies

- **Windows**: windows.h, io.h, GetStdHandle, GetConsoleMode, SetConsoleMode, ENABLE_VIRTUAL_TERMINAL_PROCESSING, INVALID_HANDLE_VALUE, STD_OUTPUT_HANDLE, STD_ERROR_HANDLE, _isatty, _fileno
- **Linux**: unistd.h, isatty, fileno

### Dependent Modules

- Falcor/Utils (Internal utilities)
- Falcor/Core (Core utilities)
- Falcor/Logging (Logging utilities)

## Module Overview

TermColor provides terminal color utilities for adding ANSI color codes to strings when outputting to a terminal. It includes platform-specific support for enabling virtual terminal processing on Windows and detecting TTY (teletypewriter) streams.

### Key Characteristics

- **ANSI Color Codes**: Uses ANSI escape sequences for colors
- **Platform-Specific**: Windows and Linux support
- **TTY Detection**: Detects if stream is a terminal
- **Virtual Terminal**: Enables virtual terminal processing on Windows
- **Static Initialization**: Static initialization for Windows console mode
- **Color Mapping**: Maps colors to ANSI escape codes
- **No Color on Non-TTY**: Returns original string if not a terminal

### Architecture Principles

1. **Platform Abstraction**: Platform-specific code for Windows and Linux
2. **Static Initialization**: Static initialization for Windows console mode
3. **Color Mapping**: Map colors to ANSI escape codes
4. **TTY Detection**: Detect if stream is a terminal
5. **Conditional Coloring**: Only color if stream is a terminal

## Component Specifications

### TermColor Enum

**File**: `TermColor.h` (lines 34-42)

```cpp
enum class TermColor
{
    Gray,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta
};
```

**Purpose**: Enumeration of supported terminal colors

**Values**:
- **Gray**: Gray color
- **Red**: Red color
- **Green**: Green color
- **Yellow**: Yellow color
- **Blue**: Blue color
- **Magenta**: Magenta color

**Characteristics**:
- Enum class (scoped enum)
- Limited color set
- ANSI-compatible colors
- No default value

### colored() Function

**Files**: `TermColor.h` (line 51), `TermColor.cpp` (lines 100-103)

```cpp
std::string colored(const std::string& str, TermColor color, const std::ostream& stream = std::cout);
```

**Purpose**: Colorize a string for writing to a terminal

**Parameters**:
- str: String to colorize
- color: Color
- stream: Output stream (default: std::cout)

**Returns**: String wrapped in color codes if stream is a terminal, original string otherwise

**Behavior**:
1. Check if stream is a TTY
2. If TTY:
   - Get begin tag for color
   - Append string
   - Append end tag
   - Return colored string
3. If not TTY:
   - Return original string

**Characteristics**:
- Conditional coloring
- ANSI escape codes
- TTY detection
- Default stream is std::cout

### EnableVirtualTerminal Struct (Windows Only)

**File**: `TermColor.cpp` (lines 56-73)

```cpp
struct EnableVirtualTerminal
{
    EnableVirtualTerminal()
    {
        auto enableVirtualTerminal = [](DWORD handle)
        {
            HANDLE console = GetStdHandle(handle);
            if (console == INVALID_HANDLE_VALUE)
                return;
            DWORD mode;
            GetConsoleMode(console, &mode);
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(console, mode);
        };
        enableVirtualTerminal(STD_OUTPUT_HANDLE);
        enableVirtualTerminal(STD_ERROR_HANDLE);
    }
};
```

**Purpose**: Enable virtual terminal processing on Windows console

**Behavior**:
1. Define lambda to enable virtual terminal processing
2. Get console handle
3. If handle is valid:
   - Get current console mode
   - Enable virtual terminal processing
   - Set console mode
4. Enable for both stdout and stderr

**Characteristics**:
- Static initialization
- Windows only
- Enables ANSI escape code support
- Handles both stdout and stderr

### kBeginTag Map

**File**: `TermColor.cpp` (lines 78-87)

```cpp
static const std::unordered_map<TermColor, std::string> kBeginTag = {
    { TermColor::Gray,    "\33[90m" },
    { TermColor::Red,     "\33[91m" },
    { TermColor::Green,   "\33[92m" },
    { TermColor::Yellow,  "\33[93m" },
    { TermColor::Blue,    "\33[94m" },
    { TermColor::Magenta, "\33[95m" },
};
```

**Purpose**: Map colors to ANSI escape code begin tags

**Values**:
- **Gray**: "\33[90m" (ANSI bright black)
- **Red**: "\33[91m" (ANSI bright red)
- **Green**: "\33[92m" (ANSI bright green)
- **Yellow**: "\33[93m" (ANSI bright yellow)
- **Blue**: "\33[94m" (ANSI bright blue)
- **Magenta**: "\33[95m" (ANSI bright magenta)

**Characteristics**:
- Static constant map
- ANSI escape codes
- Bright colors
- Used for color begin tags

### kEndTag Constant

**File**: `TermColor.cpp` (line 89)

```cpp
static const std::string kEndTag = "\033[0m";
```

**Purpose**: ANSI escape code end tag for resetting color

**Value**: "\033[0m" (ANSI reset code)

**Characteristics**:
- Static constant string
- ANSI reset code
- Resets all attributes
- Used for color end tag

### isTTY() Function

**File**: `TermColor.cpp` (lines 91-98)

```cpp
inline bool isTTY(const std::ostream& stream)
{
    if (&stream == &std::cout && ISATTY(FILENO(stdout)))
        return true;
    if (&stream == &std::cerr && ISATTY(FILENO(stderr)))
        return true;
    return false;
}
```

**Purpose**: Check if stream is a TTY (teletypewriter)

**Parameters**:
- stream: Output stream to check

**Returns**: true if stream is a TTY, false otherwise

**Behavior**:
1. If stream is std::cout and stdout is a TTY, return true
2. If stream is std::cerr and stderr is a TTY, return true
3. Otherwise, return false

**Characteristics**:
- Platform-specific macros
- Checks stdout and stderr
- Inline function for performance
- Returns false for non-TTY streams

## Technical Details

### ANSI Escape Codes

Uses ANSI escape sequences for colors:

```cpp
static const std::unordered_map<TermColor, std::string> kBeginTag = {
    { TermColor::Gray,    "\33[90m" },
    { TermColor::Red,     "\33[91m" },
    { TermColor::Green,   "\33[92m" },
    { TermColor::Yellow,  "\33[93m" },
    { TermColor::Blue,    "\33[94m" },
    { TermColor::Magenta, "\33[95m" },
};

static const std::string kEndTag = "\033[0m";
```

**Characteristics**:
- "\33[" is ESC character
- "90m" is bright black (gray)
- "91m" is bright red
- "92m" is bright green
- "93m" is bright yellow
- "94m" is bright blue
- "95m" is bright magenta
- "0m" resets all attributes

### Platform-Specific Code

Windows and Linux support:

```cpp
#if FALCOR_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <io.h>
#define ISATTY _isatty
#define FILENO _fileno
#elif FALCOR_LINUX
#include <unistd.h>
#define ISATTY isatty
#define FILENO fileno
#endif
```

**Characteristics**:
- Windows: windows.h, io.h, _isatty, _fileno
- Linux: unistd.h, isatty, fileno
- Platform-specific macros
- Conditional compilation

### Windows Virtual Terminal

Enables virtual terminal processing on Windows:

```cpp
struct EnableVirtualTerminal
{
    EnableVirtualTerminal()
    {
        auto enableVirtualTerminal = [](DWORD handle)
        {
            HANDLE console = GetStdHandle(handle);
            if (console == INVALID_HANDLE_VALUE)
                return;
            DWORD mode;
            GetConsoleMode(console, &mode);
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(console, mode);
        };
        enableVirtualTerminal(STD_OUTPUT_HANDLE);
        enableVirtualTerminal(STD_ERROR_HANDLE);
    }
};

static EnableVirtualTerminal sEnableVirtualTerminal;
```

**Characteristics**:
- Static initialization
- Enables ANSI escape code support
- Handles both stdout and stderr
- Windows only

### TTY Detection

Detects if stream is a terminal:

```cpp
inline bool isTTY(const std::ostream& stream)
{
    if (&stream == &std::cout && ISATTY(FILENO(stdout)))
        return true;
    if (&stream == &std::cerr && ISATTY(FILENO(stderr)))
        return true;
    return false;
}
```

**Characteristics**:
- Platform-specific macros
- Checks stdout and stderr
- Inline function for performance
- Returns false for non-TTY streams

### Color Mapping Algorithm

Maps colors to ANSI escape codes:

```cpp
static const std::unordered_map<TermColor, std::string> kBeginTag = {
    { TermColor::Gray,    "\33[90m" },
    { TermColor::Red,     "\33[91m" },
    { TermColor::Green,   "\33[92m" },
    { TermColor::Yellow,  "\33[93m" },
    { TermColor::Blue,    "\33[94m" },
    { TermColor::Magenta, "\33[95m" },
};
```

**Algorithm**:
1. Define map of colors to ANSI escape codes
2. Use bright colors (90-95)
3. Use ESC character ("\33[")
4. Use color code (90-95)
5. Use "m" suffix

**Characteristics**:
- Static constant map
- ANSI escape codes
- Bright colors
- Used for color begin tags

### Colored String Algorithm

Colorizes string for terminal output:

```cpp
std::string colored(const std::string& str, TermColor color, const std::ostream& stream)
{
    return isTTY(stream) ? (kBeginTag.at(color) + str + kEndTag) : str;
}
```

**Algorithm**:
1. Check if stream is a TTY
2. If TTY:
   - Get begin tag for color
   - Append string
   - Append end tag
   - Return colored string
3. If not TTY:
   - Return original string

**Characteristics**:
- Conditional coloring
- ANSI escape codes
- TTY detection
- Default stream is std::cout

## Integration Points

### Falcor Core Integration

- **Falcor/Core/Macros.h**: FALCOR_WINDOWS, FALCOR_LINUX

### STL Integration

- **std::iostream**: std::cout, std::cerr, std::ostream
- **std::string**: std::string
- **std::unordered_map**: std::unordered_map

### Platform-Specific Integration

- **Windows**: windows.h, io.h, GetStdHandle, GetConsoleMode, SetConsoleMode
- **Linux**: unistd.h, isatty, fileno

### Internal Falcor Usage

- **Falcor/Utils**: Internal utilities
- **Falcor/Core**: Core utilities
- **Falcor/Logging**: Logging utilities

## Architecture Patterns

### Platform Abstraction Pattern

Platform-specific code for Windows and Linux:

```cpp
#if FALCOR_WINDOWS
#include <windows.h>
#define ISATTY _isatty
#define FILENO _fileno
#elif FALCOR_LINUX
#include <unistd.h>
#define ISATTY isatty
#define FILENO fileno
#endif
```

Benefits:
- Platform-specific code
- Conditional compilation
- Unified API
- Easy to maintain

### Static Initialization Pattern

Static initialization for Windows console mode:

```cpp
struct EnableVirtualTerminal
{
    EnableVirtualTerminal()
    {
        // Enable virtual terminal processing
    }
};

static EnableVirtualTerminal sEnableVirtualTerminal;
```

Benefits:
- Automatic initialization
- No manual setup
- Thread-safe
- Runs before main()

### Color Mapping Pattern

Map colors to ANSI escape codes:

```cpp
static const std::unordered_map<TermColor, std::string> kBeginTag = {
    { TermColor::Gray,    "\33[90m" },
    { TermColor::Red,     "\33[91m" },
    { TermColor::Green,   "\33[92m" },
    { TermColor::Yellow,  "\33[93m" },
    { TermColor::Blue,    "\33[94m" },
    { TermColor::Magenta, "\33[95m" },
};
```

Benefits:
- Easy color lookup
- ANSI escape codes
- Extensible
- Type-safe

### Conditional Coloring Pattern

Only color if stream is a terminal:

```cpp
std::string colored(const std::string& str, TermColor color, const std::ostream& stream)
{
    return isTTY(stream) ? (kBeginTag.at(color) + str + kEndTag) : str;
}
```

Benefits:
- No color on non-TTY
- Clean output on non-TTY
- Automatic detection
- No manual configuration

## Code Patterns

### Platform-Specific Code Pattern

Platform-specific code with macros:

```cpp
#if FALCOR_WINDOWS
#include <windows.h>
#define ISATTY _isatty
#define FILENO _fileno
#elif FALCOR_LINUX
#include <unistd.h>
#define ISATTY isatty
#define FILENO fileno
#endif
```

Pattern:
- Check platform macro
- Include platform-specific headers
- Define platform-specific macros
- Use macros in code

### Static Initialization Pattern

Static initialization with struct:

```cpp
struct EnableVirtualTerminal
{
    EnableVirtualTerminal()
    {
        // Enable virtual terminal processing
    }
};

static EnableVirtualTerminal sEnableVirtualTerminal;
```

Pattern:
- Define struct with constructor
- Perform initialization in constructor
- Create static instance
- Initialization runs before main()

### Color Mapping Pattern

Map colors to ANSI escape codes:

```cpp
static const std::unordered_map<TermColor, std::string> kBeginTag = {
    { TermColor::Gray,    "\33[90m" },
    { TermColor::Red,     "\33[91m" },
    { TermColor::Green,   "\33[92m" },
    { TermColor::Yellow,  "\33[93m" },
    { TermColor::Blue,    "\33[94m" },
    { TermColor::Magenta, "\33[95m" },
};
```

Pattern:
- Define static constant map
- Map colors to ANSI escape codes
- Use bright colors
- Use ESC character

### Conditional Coloring Pattern

Only color if stream is a terminal:

```cpp
std::string colored(const std::string& str, TermColor color, const std::ostream& stream)
{
    return isTTY(stream) ? (kBeginTag.at(color) + str + kEndTag) : str;
}
```

Pattern:
- Check if stream is a TTY
- If TTY, color the string
- If not TTY, return original
- Use ternary operator

### TTY Detection Pattern

Detect if stream is a terminal:

```cpp
inline bool isTTY(const std::ostream& stream)
{
    if (&stream == &std::cout && ISATTY(FILENO(stdout)))
        return true;
    if (&stream == &std::cerr && ISATTY(FILENO(stderr)))
        return true;
    return false;
}
```

Pattern:
- Check if stream is std::cout or std::cerr
- Check if file descriptor is a TTY
- Return true if TTY, false otherwise
- Inline function for performance

## Use Cases

### Basic Colored Output

```cpp
#include "Utils/TermColor.h"
#include <iostream>

// Colorize string
std::string colored = colored("Hello World", TermColor::Red);
std::cout << colored << std::endl;
```

### Colored Error Messages

```cpp
#include "Utils/TermColor.h"
#include <iostream>

// Colorize error message
std::string error = colored("Error: Something went wrong", TermColor::Red);
std::cerr << error << std::endl;
```

### Colored Warning Messages

```cpp
#include "Utils/TermColor.h"
#include <iostream>

// Colorize warning message
std::string warning = colored("Warning: Something might be wrong", TermColor::Yellow);
std::cout << warning << std::endl;
```

### Colored Success Messages

```cpp
#include "Utils/TermColor.h"
#include <iostream>

// Colorize success message
std::string success = colored("Success: Operation completed", TermColor::Green);
std::cout << success << std::endl;
```

### Colored Info Messages

```cpp
#include "Utils/TermColor.h"
#include <iostream>

// Colorize info message
std::string info = colored("Info: Operation in progress", TermColor::Blue);
std::cout << info << std::endl;
```

### Colored Debug Messages

```cpp
#include "Utils/TermColor.h"
#include <iostream>

// Colorize debug message
std::string debug = colored("Debug: Variable value", TermColor::Gray);
std::cout << debug << std::endl;
```

### Multiple Colors

```cpp
#include "Utils/TermColor.h"
#include <iostream>

// Colorize multiple strings
std::string red = colored("Red", TermColor::Red);
std::string green = colored("Green", TermColor::Green);
std::string blue = colored("Blue", TermColor::Blue);
std::cout << red << " " << green << " " << blue << std::endl;
```

### Custom Stream

```cpp
#include "Utils/TermColor.h"
#include <iostream>
#include <fstream>

// Colorize string for custom stream
std::ofstream file("output.txt");
std::string colored = colored("Hello World", TermColor::Red, file);
file << colored << std::endl;
```

## Performance Considerations

### Memory Overhead

- **String Copying**: colored() creates new string
- **Map Lookup**: ANSI escape code lookup
- **TTY Detection**: TTY detection overhead

### Algorithmic Complexity

- **colored()**: O(1) (map lookup + string concatenation)
- **isTTY()**: O(1) (pointer comparison + system call)

### Compiler Optimizations

- **Inline Expansion**: isTTY() is inline
- **Constant Propagation**: Constants can be propagated
- **Dead Code Elimination**: Unused code can be eliminated
- **String Concatenation**: String concatenation optimization

### Comparison with Alternatives

**TermColor vs Manual ANSI Codes**:
- TermColor: Easy to use, cross-platform
- Manual ANSI Codes: More control, more code

**TermColor vs Other Libraries**:
- TermColor: Simple, lightweight
- Other Libraries: More features, more dependencies

### Optimization Tips

1. **Use Inline Functions**: Use inline functions for performance
2. **Cache Colored Strings**: Cache colored strings if used repeatedly
3. **Enable Optimizations**: Enable compiler optimizations
4. **Profile**: Profile to identify bottlenecks

## Limitations

### Feature Limitations

- **Limited Colors**: Only 6 colors supported
- **No Background Colors**: No background color support
- **No Text Attributes**: No bold, underline, etc.
- **No Custom Colors**: No custom color support
- **No Color Intensity**: No color intensity control
- **No Color Mixing**: No color mixing

### API Limitations

- **No Color Intensity**: No color intensity control
- **No Background Colors**: No background color support
- **No Text Attributes**: No bold, underline, etc.
- **No Custom Colors**: No custom color support
- **No Color Mixing**: No color mixing
- **No Color Palette**: No color palette support

### Platform Limitations

- **Windows**: Requires virtual terminal processing
- **Linux**: Requires TTY support
- **No macOS**: No macOS support (may work)

### Performance Limitations

- **String Copying**: colored() creates new string
- **Map Lookup**: ANSI escape code lookup
- **TTY Detection**: TTY detection overhead

## Best Practices

### When to Use

**Appropriate Use Cases**:
- Terminal output coloring
- Error message highlighting
- Warning message highlighting
- Success message highlighting
- Info message highlighting
- Debug message highlighting

**Inappropriate Use Cases**:
- File output (no color needed)
- Non-terminal output (no color needed)
- GUI applications (no terminal)
- Complex formatting (use formatting library)

### Usage Patterns

**Basic Colored Output**:
```cpp
std::string colored = colored("Hello World", TermColor::Red);
std::cout << colored << std::endl;
```

**Colored Error Messages**:
```cpp
std::string error = colored("Error: Something went wrong", TermColor::Red);
std::cerr << error << std::endl;
```

**Multiple Colors**:
```cpp
std::string red = colored("Red", TermColor::Red);
std::string green = colored("Green", TermColor::Green);
std::cout << red << " " << green << std::endl;
```

### Performance Tips

1. **Use Inline Functions**: Use inline functions for performance
2. **Cache Colored Strings**: Cache colored strings if used repeatedly
3. **Enable Optimizations**: Enable compiler optimizations
4. **Profile**: Profile to identify bottlenecks

### Platform Considerations

- **Windows**: Virtual terminal processing enabled automatically
- **Linux**: TTY detection works automatically
- **macOS**: May work, not tested

### Color Selection

- **Red**: Errors, failures
- **Yellow**: Warnings, cautions
- **Green**: Success, completion
- **Blue**: Info, progress
- **Gray**: Debug, verbose
- **Magenta**: Special, highlight

## Implementation Notes

### ANSI Escape Codes

Uses ANSI escape sequences for colors:

```cpp
static const std::unordered_map<TermColor, std::string> kBeginTag = {
    { TermColor::Gray,    "\33[90m" },
    { TermColor::Red,     "\33[91m" },
    { TermColor::Green,   "\33[92m" },
    { TermColor::Yellow,  "\33[93m" },
    { TermColor::Blue,    "\33[94m" },
    { TermColor::Magenta, "\33[95m" },
};
```

**Characteristics**:
- "\33[" is ESC character
- "90m" is bright black (gray)
- "91m" is bright red
- "92m" is bright green
- "93m" is bright yellow
- "94m" is bright blue
- "95m" is bright magenta

### Platform-Specific Implementation

Windows and Linux support:

```cpp
#if FALCOR_WINDOWS
#include <windows.h>
#include <io.h>
#define ISATTY _isatty
#define FILENO _fileno
#elif FALCOR_LINUX
#include <unistd.h>
#define ISATTY isatty
#define FILENO fileno
#endif
```

**Characteristics**:
- Windows: windows.h, io.h, _isatty, _fileno
- Linux: unistd.h, isatty, fileno
- Platform-specific macros
- Conditional compilation

### Windows Virtual Terminal Implementation

Enables virtual terminal processing on Windows:

```cpp
struct EnableVirtualTerminal
{
    EnableVirtualTerminal()
    {
        auto enableVirtualTerminal = [](DWORD handle)
        {
            HANDLE console = GetStdHandle(handle);
            if (console == INVALID_HANDLE_VALUE)
                return;
            DWORD mode;
            GetConsoleMode(console, &mode);
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(console, mode);
        };
        enableVirtualTerminal(STD_OUTPUT_HANDLE);
        enableVirtualTerminal(STD_ERROR_HANDLE);
    }
};

static EnableVirtualTerminal sEnableVirtualTerminal;
```

**Characteristics**:
- Static initialization
- Enables ANSI escape code support
- Handles both stdout and stderr
- Windows only

### TTY Detection Implementation

Detects if stream is a terminal:

```cpp
inline bool isTTY(const std::ostream& stream)
{
    if (&stream == &std::cout && ISATTY(FILENO(stdout)))
        return true;
    if (&stream == &std::cerr && ISATTY(FILENO(stderr)))
        return true;
    return false;
}
```

**Characteristics**:
- Platform-specific macros
- Checks stdout and stderr
- Inline function for performance
- Returns false for non-TTY streams

### Colored String Implementation

Colorizes string for terminal output:

```cpp
std::string colored(const std::string& str, TermColor color, const std::ostream& stream)
{
    return isTTY(stream) ? (kBeginTag.at(color) + str + kEndTag) : str;
}
```

**Characteristics**:
- Conditional coloring
- ANSI escape codes
- TTY detection
- Default stream is std::cout

## Future Enhancements

### Potential Improvements

1. **More Colors**: Add more colors (cyan, white, etc.)
2. **Background Colors**: Add background color support
3. **Text Attributes**: Add bold, underline, etc.
4. **Custom Colors**: Add custom color support
5. **Color Intensity**: Add color intensity control
6. **Color Mixing**: Add color mixing
7. **Color Palette**: Add color palette support
8. **RGB Colors**: Add RGB color support
9. **Color Themes**: Add color theme support
10. **macOS Support**: Add macOS support

### API Extensions

1. **More Colors**: Add more colors
2. **Background Colors**: Add background color support
3. **Text Attributes**: Add bold, underline, etc.
4. **Custom Colors**: Add custom color support
5. **Color Intensity**: Add color intensity control
6. **Color Mixing**: Add color mixing
7. **Color Palette**: Add color palette support
8. **RGB Colors**: Add RGB color support
9. **Color Themes**: Add color theme support
10. **macOS Support**: Add macOS support

### Performance Enhancements

1. **String View**: Use string view to avoid copies
2. **Cache Colored Strings**: Cache colored strings
3. **Lazy Initialization**: Lazy initialization for Windows console
4. **Compiler Optimizations**: Better compiler optimizations

### Documentation Enhancements

1. **More Examples**: Add more usage examples
2. **Performance Guide**: Add performance optimization guide
3. **Best Practices**: Add best practices guide
4. **Migration Guide**: Add migration guide from manual ANSI codes
5. **API Reference**: Add complete API reference

## References

### C++ Standard

- **std::iostream**: C++ iostream library
- **std::string**: C++ string library
- **std::unordered_map**: C++ unordered_map container

### ANSI Escape Codes

- **ANSI Escape Codes**: ANSI escape code documentation

### Platform-Specific Documentation

- **Windows Console**: Windows console documentation
- **Linux TTY**: Linux TTY documentation

### Falcor Documentation

- **Falcor Utils**: Utils module documentation
- **Falcor Core**: Core module documentation

### Related Technologies

- **ANSI Escape Codes**: ANSI escape code techniques
- **Terminal Color**: Terminal color techniques
- **TTY Detection**: TTY detection techniques
- **Platform-Specific Code**: Platform-specific code techniques
