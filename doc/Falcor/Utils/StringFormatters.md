# StringFormatters - fmt Library Extensions

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- fmt/format.h (fmt::formatter, fmt::format_to)
- std::filesystem (std::filesystem::path)
- std::optional (std::optional<T>)
- std::string (std::string)

### Dependent Modules

- Falcor/Utils (Internal utilities)
- Falcor/Core (Core utilities)
- Falcor/Scene (Scene utilities)
- Falcor/Rendering (Rendering utilities)

## Module Overview

StringFormatters provides fmt::formatter specializations for standard library types that are not supported by the fmt library by default. It enables formatting of std::filesystem::path and std::optional<T> types using the fmt library's formatting syntax.

### Key Characteristics

- **fmt Integration**: Extends fmt library with custom formatters
- **Path Formatting**: Supports std::filesystem::path formatting
- **Optional Formatting**: Supports std::optional<T> formatting
- **Header-Only**: All functionality in header file
- **Template-Based**: Uses template specialization
- **Type-Safe**: Compile-time type checking
- **Minimal Overhead**: Zero runtime overhead for path formatting
- **Flexible**: Supports all fmt formatting options

### Architecture Principles

1. **Template Specialization**: Uses template specialization for custom formatters
2. **Inheritance**: Inherits from existing formatters
3. **Type Safety**: Compile-time type checking
4. **Zero Overhead**: Minimal runtime overhead
5. **Flexible Formatting**: Supports all fmt formatting options

## Component Specifications

### std::filesystem::path Formatter

**File**: `Source/Falcor/Utils/StringFormatters.h` (lines 34-42)

```cpp
template<>
struct fmt::formatter<std::filesystem::path> : formatter<std::string>
{
    template<typename FormatContext>
    auto format(const std::filesystem::path& p, FormatContext& ctx)
    {
        return formatter<std::string>::format(p.string(), ctx);
    }
};
```

**Purpose**: Enables formatting of std::filesystem::path objects using fmt library

**Template Parameters**:
- None (explicit specialization)

**Base Class**:
- formatter<std::string>

**format() Method**:
```cpp
template<typename FormatContext>
auto format(const std::filesystem::path& p, FormatContext& ctx)
{
    return formatter<std::string>::format(p.string(), ctx);
}
```

**Parameters**:
- p: const reference to std::filesystem::path to format
- ctx: FormatContext reference for formatting context

**Returns**: Output iterator

**Behavior**:
1. Converts path to string using p.string()
2. Formats string using base class formatter
3. Returns output iterator

**Characteristics**:
- Inherits all formatting options from std::string formatter
- Zero runtime overhead (direct string conversion)
- Supports all fmt format specifiers
- Type-safe (compile-time checking)

### std::optional<T> Formatter

**File**: `Source/Falcor/Utils/StringFormatters.h` (lines 44-57)

```cpp
template<typename T>
struct fmt::formatter<std::optional<T>> : formatter<T>
{
    template<typename FormatContext>
    auto format(const std::optional<T>& opt, FormatContext& ctx)
    {
        if (opt)
        {
            formatter<T>::format(*opt, ctx);
            return ctx.out();
        }
        return fmt::format_to(ctx.out(), "nullopt");
    }
};
```

**Purpose**: Enables formatting of std::optional<T> objects using fmt library

**Template Parameters**:
- T: Type of value in optional

**Base Class**:
- formatter<T>

**format() Method**:
```cpp
template<typename FormatContext>
auto format(const std::optional<T>& opt, FormatContext& ctx)
{
    if (opt)
    {
        formatter<T>::format(*opt, ctx);
        return ctx.out();
    }
    return fmt::format_to(ctx.out(), "nullopt");
}
```

**Parameters**:
- opt: const reference to std::optional<T> to format
- ctx: FormatContext reference for formatting context

**Returns**: Output iterator

**Behavior**:
1. Checks if optional has value
2. If has value:
   - Formats value using base class formatter
   - Returns output iterator
3. If no value:
   - Formats "nullopt" string
   - Returns output iterator

**Characteristics**:
- Inherits all formatting options from T's formatter
- Supports all fmt format specifiers
- Type-safe (compile-time checking)
- Handles empty optional gracefully

## Technical Details

### Template Specialization

Uses template specialization for custom formatters:

```cpp
template<>
struct fmt::formatter<std::filesystem::path> : formatter<std::string>
{
    // ...
};

template<typename T>
struct fmt::formatter<std::optional<T>> : formatter<T>
{
    // ...
};
```

**Characteristics**:
- Explicit specialization for std::filesystem::path
- Partial specialization for std::optional<T>
- Inherits from existing formatters
- Overrides format() method

### Inheritance Pattern

Inherits from existing formatters:

```cpp
struct fmt::formatter<std::filesystem::path> : formatter<std::string>
{
    // ...
};

struct fmt::formatter<std::optional<T>> : formatter<T>
{
    // ...
};
```

**Characteristics**:
- Inherits format specification parsing
- Inherits formatting options
- Inherits format specifiers
- Minimal code duplication

### Path Formatting Algorithm

Formats filesystem path:

```cpp
auto format(const std::filesystem::path& p, FormatContext& ctx)
{
    return formatter<std::string>::format(p.string(), ctx);
}
```

**Algorithm**:
1. Convert path to string using p.string()
2. Format string using base class formatter
3. Return output iterator

**Characteristics**:
- Direct string conversion
- Zero runtime overhead
- Supports all fmt format specifiers
- Type-safe

### Optional Formatting Algorithm

Formats optional value:

```cpp
auto format(const std::optional<T>& opt, FormatContext& ctx)
{
    if (opt)
    {
        formatter<T>::format(*opt, ctx);
        return ctx.out();
    }
    return fmt::format_to(ctx.out(), "nullopt");
}
```

**Algorithm**:
1. Check if optional has value
2. If has value:
   - Format value using base class formatter
   - Return output iterator
3. If no value:
   - Format "nullopt" string
   - Return output iterator

**Characteristics**:
- Handles empty optional gracefully
- Supports all fmt format specifiers
- Type-safe
- Minimal runtime overhead

### Type Safety

Compile-time type checking:

```cpp
template<typename T>
struct fmt::formatter<std::optional<T>> : formatter<T>
{
    // ...
};
```

**Characteristics**:
- Compile-time type checking
- No runtime type information
- Template instantiation error if T has no formatter
- Type-safe formatting

### Zero Overhead

Minimal runtime overhead:

```cpp
auto format(const std::filesystem::path& p, FormatContext& ctx)
{
    return formatter<std::string>::format(p.string(), ctx);
}
```

**Characteristics**:
- Direct string conversion for path
- Minimal conditional logic for optional
- No dynamic memory allocation
- Compiler optimizations

## Integration Points

### fmt Library Integration

- **fmt/format.h**: fmt::formatter, fmt::format_to
- **fmt::formatter**: Base class for custom formatters
- **fmt::format_to**: Function for formatting to output iterator

### Standard Library Integration

- **std::filesystem**: std::filesystem::path
- **std::optional**: std::optional<T>
- **std::string**: std::string

### Internal Falcor Usage

- **Falcor/Utils**: Internal utilities
- **Falcor/Core**: Core utilities
- **Falcor/Scene**: Scene utilities
- **Falcor/Rendering**: Rendering utilities

## Architecture Patterns

### Template Specialization Pattern

Uses template specialization for custom formatters:

```cpp
template<>
struct fmt::formatter<std::filesystem::path> : formatter<std::string>
{
    // ...
};

template<typename T>
struct fmt::formatter<std::optional<T>> : formatter<T>
{
    // ...
};
```

Benefits:
- Extends fmt library functionality
- Type-safe formatting
- Compile-time checking
- Minimal code duplication

### Inheritance Pattern

Inherits from existing formatters:

```cpp
struct fmt::formatter<std::filesystem::path> : formatter<std::string>
{
    // ...
};

struct fmt::formatter<std::optional<T>> : formatter<T>
{
    // ...
};
```

Benefits:
- Inherits format specification parsing
- Inherits formatting options
- Inherits format specifiers
- Minimal code duplication

### Zero Overhead Pattern

Minimal runtime overhead:

```cpp
auto format(const std::filesystem::path& p, FormatContext& ctx)
{
    return formatter<std::string>::format(p.string(), ctx);
}
```

Benefits:
- Direct string conversion
- Minimal conditional logic
- No dynamic memory allocation
- Compiler optimizations

### Type Safety Pattern

Compile-time type checking:

```cpp
template<typename T>
struct fmt::formatter<std::optional<T>> : formatter<T>
{
    // ...
};
```

Benefits:
- Compile-time type checking
- No runtime type information
- Template instantiation error if T has no formatter
- Type-safe formatting

## Code Patterns

### Template Specialization Pattern

Specialize fmt::formatter for custom types:

```cpp
template<>
struct fmt::formatter<std::filesystem::path> : formatter<std::string>
{
    template<typename FormatContext>
    auto format(const std::filesystem::path& p, FormatContext& ctx)
    {
        return formatter<std::string>::format(p.string(), ctx);
    }
};
```

Pattern:
- Explicit specialization for specific type
- Inherit from existing formatter
- Override format() method
- Convert to base type and format

### Partial Template Specialization Pattern

Specialize fmt::formatter for template type:

```cpp
template<typename T>
struct fmt::formatter<std::optional<T>> : formatter<T>
{
    template<typename FormatContext>
    auto format(const std::optional<T>& opt, FormatContext& ctx)
    {
        if (opt)
        {
            formatter<T>::format(*opt, ctx);
            return ctx.out();
        }
        return fmt::format_to(ctx.out(), "nullopt");
    }
};
```

Pattern:
- Partial specialization for template type
- Inherit from base type formatter
- Override format() method
- Handle empty optional case

### Inheritance Pattern

Inherit from existing formatter:

```cpp
struct fmt::formatter<std::filesystem::path> : formatter<std::string>
{
    // ...
};
```

Pattern:
- Inherit from base type formatter
- Override format() method
- Use base class formatting
- Minimal code duplication

### Conditional Formatting Pattern

Format based on condition:

```cpp
if (opt)
{
    formatter<T>::format(*opt, ctx);
    return ctx.out();
}
return fmt::format_to(ctx.out(), "nullopt");
```

Pattern:
- Check condition
- Format value if condition true
- Format default string if condition false
- Return output iterator

## Use Cases

### Path Formatting

```cpp
#include "Utils/StringFormatters.h"
#include <fmt/format.h>

std::filesystem::path path = "/path/to/file.txt";

// Format path
std::string formatted = fmt::format("Path: {}", path);
// Output: "Path: /path/to/file.txt"

// Format path with custom format
std::string formatted2 = fmt::format("Path: {:>20}", path);
// Output: "Path:     /path/to/file.txt"
```

### Optional Formatting

```cpp
#include "Utils/StringFormatters.h"
#include <fmt/format.h>
#include <optional>

std::optional<int> value = 42;

// Format optional with value
std::string formatted = fmt::format("Value: {}", value);
// Output: "Value: 42"

// Format optional without value
std::optional<int> empty;
std::string formatted2 = fmt::format("Value: {}", empty);
// Output: "Value: nullopt"

// Format optional with custom format
std::string formatted3 = fmt::format("Value: {:04}", value);
// Output: "Value: 0042"
```

### Mixed Formatting

```cpp
#include "Utils/StringFormatters.h"
#include <fmt/format.h>

std::filesystem::path path = "/path/to/file.txt";
std::optional<int> value = 42;

// Format multiple values
std::string formatted = fmt::format("Path: {}, Value: {}", path, value);
// Output: "Path: /path/to/file.txt, Value: 42"

// Format with custom formats
std::string formatted2 = fmt::format("Path: {:>20}, Value: {:04}", path, value);
// Output: "Path:     /path/to/file.txt, Value: 0042"
```

### Vector of Paths

```cpp
#include "Utils/StringFormatters.h"
#include <fmt/format.h>
#include <vector>

std::vector<std::filesystem::path> paths = {
    "/path/to/file1.txt",
    "/path/to/file2.txt",
    "/path/to/file3.txt"
};

// Format vector of paths
std::string formatted = fmt::format("Paths: {}", fmt::join(paths, ", "));
// Output: "Paths: /path/to/file1.txt, /path/to/file2.txt, /path/to/file3.txt"
```

### Vector of Optionals

```cpp
#include "Utils/StringFormatters.h"
#include <fmt/format.h>
#include <vector>

std::vector<std::optional<int>> values = {42, std::nullopt, 99};

// Format vector of optionals
std::string formatted = fmt::format("Values: {}", fmt::join(values, ", "));
// Output: "Values: 42, nullopt, 99"
```

### Logging with Paths

```cpp
#include "Utils/StringFormatters.h"
#include "Utils/Logger.h"

std::filesystem::path path = "/path/to/file.txt";

// Log path
logInfo("Loading file: {}", path);
// Output: [INFO] Loading file: /path/to/file.txt

// Log path with custom format
logInfo("Loading file: {:>20}", path);
// Output: [INFO] Loading file:     /path/to/file.txt
```

### Logging with Optionals

```cpp
#include "Utils/StringFormatters.h"
#include "Utils/Logger.h"
#include <optional>

std::optional<int> value = 42;

// Log optional with value
logInfo("Value: {}", value);
// Output: [INFO] Value: 42

// Log optional without value
std::optional<int> empty;
logInfo("Value: {}", empty);
// Output: [INFO] Value: nullopt
```

### Error Messages with Paths

```cpp
#include "Utils/StringFormatters.h"
#include "Core/Error.h"

std::filesystem::path path = "/path/to/file.txt";

// Throw error with path
FALCOR_THROW("File not found: {}", path);
// Output: [ERROR] File not found: /path/to/file.txt
```

### Error Messages with Optionals

```cpp
#include "Utils/StringFormatters.h"
#include "Core/Error.h"
#include <optional>

std::optional<int> value = 42;

// Throw error with optional
FALCOR_THROW("Invalid value: {}", value);
// Output: [ERROR] Invalid value: 42
```

## Performance Considerations

### Memory Overhead

- **Path Formatter**: Zero overhead (direct string conversion)
- **Optional Formatter**: Minimal overhead (conditional check)
- **Template Instantiation**: Minimal overhead (compile-time)
- **Inheritance**: Zero overhead (compile-time)

### Formatting Performance

- **Path Formatting**: O(n) for string conversion + O(n) for formatting
- **Optional Formatting**: O(1) for conditional + O(n) for formatting
- **Overall**: O(n) where n is string length

### Compiler Optimizations

- **Inline Expansion**: Methods can be inlined
- **Constant Propagation**: Constants can be propagated
- **Dead Code Elimination**: Unused code can be eliminated
- **Template Instantiation**: Template instantiation overhead

### Comparison with Alternatives

**StringFormatters vs Manual Formatting**:
- StringFormatters: Automatic formatting, type-safe, minimal overhead
- Manual Formatting: More control, more code, potential errors

**StringFormatters vs Custom Formatters**:
- StringFormatters: Standard library types, minimal code
- Custom Formatters: Custom types, more code

### Optimization Tips

1. **Use fmt::format_to**: For better performance with multiple values
2. **Use fmt::join**: For formatting vectors
3. **Enable Optimizations**: Enable compiler optimizations
4. **Profile**: Profile to identify bottlenecks

## Limitations

### Feature Limitations

- **No Custom Format Specifiers**: No custom format specifiers for path/optional
- **No Custom Formatting Options**: No custom formatting options
- **No Localization**: No localization support
- **No Unicode**: No explicit Unicode support (depends on std::string)
- **No Validation**: No validation of path/optional values

### API Limitations

- **No Parse Method**: No parse method for format specification
- **No Format Specification**: No custom format specification
- **No Format Options**: No custom format options
- **No Format Validation**: No format validation

### Type Limitations

- **std::filesystem::path**: Only std::filesystem::path supported
- **std::optional<T>**: Only std::optional<T> supported
- **T Requirements**: T must have fmt::formatter specialization

### Performance Limitations

- **String Conversion**: Path to string conversion overhead
- **Conditional Check**: Optional conditional check overhead
- **Template Instantiation**: Template instantiation overhead

### Platform Limitations

- **C++17 Only**: Requires C++17 or later
- **fmt Library**: Requires fmt library
- **Standard Library**: Requires standard library support

## Best Practices

### When to Use

**Appropriate Use Cases**:
- Formatting std::filesystem::path objects
- Formatting std::optional<T> objects
- Logging with paths/optionals
- Error messages with paths/optionals
- Debugging with paths/optionals

**Inappropriate Use Cases**:
- Formatting custom types (use custom formatter)
- Formatting with custom format specifiers (use custom formatter)
- Formatting with localization (use localization library)
- Formatting with Unicode (use Unicode library)

### Usage Patterns

**Basic Path Formatting**:
```cpp
std::filesystem::path path = "/path/to/file.txt";
std::string formatted = fmt::format("Path: {}", path);
```

**Basic Optional Formatting**:
```cpp
std::optional<int> value = 42;
std::string formatted = fmt::format("Value: {}", value);
```

**Custom Format Specifiers**:
```cpp
std::filesystem::path path = "/path/to/file.txt";
std::string formatted = fmt::format("Path: {:>20}", path);
```

**Multiple Values**:
```cpp
std::filesystem::path path = "/path/to/file.txt";
std::optional<int> value = 42;
std::string formatted = fmt::format("Path: {}, Value: {}", path, value);
```

### Performance Tips

1. **Use fmt::format_to**: For better performance with multiple values
2. **Use fmt::join**: For formatting vectors
3. **Enable Optimizations**: Enable compiler optimizations
4. **Profile**: Profile to identify bottlenecks

### Type Safety

- **Compile-Time Checking**: Compile-time type checking
- **Template Instantiation**: Template instantiation error if T has no formatter
- **Type-Safe Formatting**: Type-safe formatting

### Error Handling

- **Empty Optional**: Handles empty optional gracefully
- **Invalid Path**: No validation (relies on std::filesystem)
- **Invalid Value**: No validation (relies on T's formatter)

## Implementation Notes

### Template Specialization Implementation

Uses template specialization:

```cpp
template<>
struct fmt::formatter<std::filesystem::path> : formatter<std::string>
{
    // ...
};

template<typename T>
struct fmt::formatter<std::optional<T>> : formatter<T>
{
    // ...
};
```

**Characteristics**:
- Explicit specialization for std::filesystem::path
- Partial specialization for std::optional<T>
- Inherits from existing formatters
- Overrides format() method

### Inheritance Implementation

Inherits from existing formatters:

```cpp
struct fmt::formatter<std::filesystem::path> : formatter<std::string>
{
    // ...
};

struct fmt::formatter<std::optional<T>> : formatter<T>
{
    // ...
};
```

**Characteristics**:
- Inherits format specification parsing
- Inherits formatting options
- Inherits format specifiers
- Minimal code duplication

### Path Formatting Implementation

Formats filesystem path:

```cpp
auto format(const std::filesystem::path& p, FormatContext& ctx)
{
    return formatter<std::string>::format(p.string(), ctx);
}
```

**Characteristics**:
- Direct string conversion
- Zero runtime overhead
- Supports all fmt format specifiers
- Type-safe

### Optional Formatting Implementation

Formats optional value:

```cpp
auto format(const std::optional<T>& opt, FormatContext& ctx)
{
    if (opt)
    {
        formatter<T>::format(*opt, ctx);
        return ctx.out();
    }
    return fmt::format_to(ctx.out(), "nullopt");
}
```

**Characteristics**:
- Handles empty optional gracefully
- Supports all fmt format specifiers
- Type-safe
- Minimal runtime overhead

### Type Safety Implementation

Compile-time type checking:

```cpp
template<typename T>
struct fmt::formatter<std::optional<T>> : formatter<T>
{
    // ...
};
```

**Characteristics**:
- Compile-time type checking
- No runtime type information
- Template instantiation error if T has no formatter
- Type-safe formatting

### Zero Overhead Implementation

Minimal runtime overhead:

```cpp
auto format(const std::filesystem::path& p, FormatContext& ctx)
{
    return formatter<std::string>::format(p.string(), ctx);
}
```

**Characteristics**:
- Direct string conversion
- Minimal conditional logic
- No dynamic memory allocation
- Compiler optimizations

## Future Enhancements

### Potential Improvements

1. **Custom Format Specifiers**: Add custom format specifiers for path/optional
2. **Custom Formatting Options**: Add custom formatting options
3. **Localization**: Add localization support
4. **Unicode**: Add explicit Unicode support
5. **Validation**: Add validation of path/optional values
6. **Parse Method**: Add parse method for format specification
7. **Format Specification**: Add custom format specification
8. **Format Options**: Add custom format options
9. **Format Validation**: Add format validation
10. **More Types**: Add support for more standard library types

### API Extensions

1. **Custom Format Specifiers**: Add custom format specifiers
2. **Custom Formatting Options**: Add custom formatting options
3. **Parse Method**: Add parse method for format specification
4. **Format Specification**: Add custom format specification
5. **Format Options**: Add custom format options
6. **Format Validation**: Add format validation
7. **More Types**: Add support for more standard library types

### Performance Enhancements

1. **String Conversion Optimization**: Optimize path to string conversion
2. **Conditional Check Optimization**: Optimize optional conditional check
3. **Template Instantiation Optimization**: Optimize template instantiation
4. **Compiler Optimizations**: Better compiler optimizations

### Documentation Enhancements

1. **More Examples**: Add more usage examples
2. **Performance Guide**: Add performance optimization guide
3. **Best Practices**: Add best practices guide
4. **Migration Guide**: Add migration guide from manual formatting
5. **API Reference**: Add complete API reference

## References

### C++ Standard

- **std::filesystem**: C++17 filesystem library
- **std::optional**: C++17 optional library
- **std::string**: C++ string library

### fmt Library

- **fmt::formatter**: fmt formatter class
- **fmt::format**: fmt format function
- **fmt::format_to**: fmt format_to function
- **fmt::join**: fmt join function

### Falcor Documentation

- **Falcor Utils**: Utils module documentation
- **Falcor Logger**: Logger module documentation
- **Falcor Error**: Error handling documentation

### Related Technologies

- **String Formatting**: String formatting techniques
- **Template Specialization**: Template specialization techniques
- **Type Safety**: Type safety techniques
- **Zero Overhead**: Zero overhead techniques
