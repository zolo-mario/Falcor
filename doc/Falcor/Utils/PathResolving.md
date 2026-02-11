# PathResolving - Path Resolution Utilities

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- Falcor/Core/Macros (FALCOR_API, FALCOR_ASSERT)
- Falcor/Core/Error (FALCOR_ASSERT)
- Falcor/Core/Platform/OS (getEnvironmentVariable)
- Falcor/Utils/StringUtils (splitString)
- std::vector
- std::string
- std::filesystem

### Dependent Modules

- Falcor/Utils/Settings (Settings module)
- Falcor/Scene (Scene file loading)
- Falcor/Rendering (Rendering file loading)
- Falcor/Mogwai (Application file loading)

## Module Overview

PathResolving provides utilities for resolving file paths, environment variables, and search paths following Renderman semantics. It supports environment variable expansion, search path resolution with special characters (`@` for standard paths, `&` for current paths), and file path resolution using search paths and current working directory. The module is designed for flexible file path resolution in rendering applications.

## Component Specifications

### Type Definitions

**File**: `Source/Falcor/Utils/PathResolving.h` (lines 39-40)

```cpp
using EnvVarResolver = std::optional<std::string>(const std::string&);
using FileChecker = bool(const std::filesystem::path&);
```

**EnvVarResolver**: Function type for resolving environment variables

**Parameters**:
- const std::string&: Environment variable name

**Returns**: std::optional<std::string> (optional string value)

**FileChecker**: Function type for checking if a file exists

**Parameters**:
- const std::filesystem::path&: File path to check

**Returns**: bool (true if file exists, false otherwise)

### checkFileExists Function

**File**: `Source/Falcor/Utils/PathResolving.h` (lines 42-45)

```cpp
static bool checkFileExists(const std::filesystem::path& path)
{
    return path.has_filename() && std::filesystem::exists(path);
}
```

**Purpose**: Default file checker that checks if a file exists

**Parameters**:
- path: File path to check

**Returns**: true if file exists and has filename, false otherwise

**Behavior**:
- Checks if path has a filename
- Checks if path exists in filesystem
- Returns true only if both conditions are met

### resolveEnvVariables Function

**File**: `Source/Falcor/Utils/PathResolving.h` (lines 62-68)
**File**: `Source/Falcor/Utils/PathResolving.cpp` (lines 37-62)

```cpp
FALCOR_API
bool resolveEnvVariables(
    std::string& str,
    EnvVarResolver envResolver = getEnvironmentVariable,
    std::string_view beginToken = "${",
    std::string_view endToken = "}"
);
```

**Purpose**: Replace all occurrences of environment variable names between beginToken and endToken with environment variable value

**Parameters**:
- str: String in which env variables will be in-place resolved
- envResolver: Resolver used to resolve env variables (default: getEnvironmentVariable)
- beginToken: Token marking start of env variable (default: "${")
- endToken: Token marking end of env variable (default: "}")

**Returns**: false if begin/end tokens do not match, true otherwise

**Behavior**:
- If envResolver is not defined, standard OS env resolver is used
- If env variable is not found, it is replaced with an empty string
- If beginToken and endToken do not match in string, call fails returning false
- Not resolving an env variable does not result in false

**Implementation**:
```cpp
bool resolveEnvVariables(std::string& str, EnvVarResolver envResolver, std::string_view beginToken, std::string_view endToken)
{
    FALCOR_ASSERT(envResolver);

    std::string::size_type begin = str.find_first_of(beginToken);
    if (begin == std::string::npos)
        return true;

    std::string result = str.substr(0, begin);
    while (begin != std::string::npos)
    {
        std::string::size_type end = str.find_first_of(endToken, begin);
        // Didn't find a matching end
        if (end == std::string::npos)
            return false;
        std::string envVar = str.substr(begin + 2, end - begin - 2);
        std::string resolved = envResolver(envVar).value_or("");
        result += resolved;

        begin = str.find_first_of(beginToken, end);
        result += str.substr(end +1, begin - end -1);
    }

    str = std::move(result);
    return true;
}
```

**Algorithm**:
1. Assert that envResolver is valid
2. Find first beginToken in string
3. If not found, return true
4. Initialize result with string before first beginToken
5. While beginToken is found:
   - Find endToken after beginToken
   - If not found, return false
   - Extract environment variable name
   - Resolve environment variable (or empty string if not found)
   - Append resolved value to result
   - Find next beginToken
   - Append string between endToken and next beginToken to result
6. Move result to str
7. Return true

### ResolvedPaths Struct

**File**: `Source/Falcor/Utils/PathResolving.h` (lines 73-77)

```cpp
struct ResolvedPaths
{
    std::vector<std::filesystem::path> resolved; ///< Resolved paths.
    std::vector<std::string> invalid;            ///< Paths that could not be resolved (only for error reporting).
};
```

**Purpose**: Result of resolveSearchPaths

**Members**:
- **resolved**: Resolved paths (std::vector<std::filesystem::path>)
- **invalid**: Paths that could not be resolved (std::vector<std::string>)

**resolved**: Successfully resolved absolute paths

**invalid**: Paths that could not be resolved (for error reporting only)

### resolveSearchPaths Function

**File**: `Source/Falcor/Utils/PathResolving.h` (lines 97-103)
**File**: `Source/Falcor/Utils/PathResolving.cpp` (lines 64-105)

```cpp
FALCOR_API
ResolvedPaths resolveSearchPaths(
    const std::vector<std::filesystem::path>& current,
    const std::vector<std::string>& update,
    const std::vector<std::filesystem::path>& standard,
    EnvVarResolver envResolver = getEnvironmentVariable
);
```

**Purpose**: Given current search paths and update search paths as vectors, modifies current search paths in place to reflect new set of search paths

**Parameters**:
- current: Current search paths, used in place of `&` in update
- update: The update to the search paths
- standard: Standard search paths, used in place of `@` in update
- envResolver: Optionally provided custom env variable resolver (default: getEnvironmentVariable)

**Returns**: ResolvedPaths struct with resolved and invalid paths

**Behavior**:
- Follows Renderman semantic
- All search paths must be absolute paths
- `${ENV_VARIABLE}` get expanded using envResolver
- Each search path ends up as its own string
- The update or env variables can contain semi-colon separated list of paths
- Special character `@` will be replaced with `standard` search path
- Special character `&` will be replaced with `current` search path
- Non-absolute paths will be stripped away

**Implementation**:
```cpp
ResolvedPaths resolveSearchPaths(
    const std::vector<std::filesystem::path>& current,
    const std::vector<std::string>& update,
    const std::vector<std::filesystem::path>& standard,
    EnvVarResolver envResolver
)
{
    ResolvedPaths result;

    for (std::string it : update)
    {
        if (!resolveEnvVariables(it, envResolver))
        {
            result.invalid.push_back(std::move(it));
            continue;
        }

        std::vector<std::string> temp = splitString(it, ";");
        for (std::string& it2 : temp)
        {
            if (it2.empty())
                continue;
            if (it2 == "&")
            {
                result.resolved.insert(result.resolved.end(), current.begin(), current.end());
                continue;
            }
            if (it2 == "@")
            {
                result.resolved.insert(result.resolved.end(), standard.begin(), standard.end());
                continue;
            }
            std::filesystem::path path = std::filesystem::weakly_canonical(it2);
            if (path.is_absolute())
                result.resolved.push_back(std::move(path));
            else
                result.invalid.push_back(std::move(it2));
        }
    }

    return result;
}
```

**Algorithm**:
1. Initialize ResolvedPaths result
2. For each update string:
   - Resolve environment variables in update string
   - If resolution fails, add to invalid paths and continue
   - Split update string by semi-colon
   - For each path in split result:
     - Skip empty paths
     - If path is "&", insert all current paths
     - If path is "@", insert all standard paths
     - Otherwise, convert to weakly canonical path
     - If path is absolute, add to resolved paths
     - Otherwise, add to invalid paths
3. Return result

**Special Characters**:
- **&**: Replace with current search paths
- **@**: Replace with standard search paths
- **;**: Separator for multiple paths in single string

### resolvePath Function

**File**: `Source/Falcor/Utils/PathResolving.h` (lines 120-126)
**File**: `Source/Falcor/Utils/PathResolving.cpp` (lines 107-144)

```cpp
FALCOR_API
std::filesystem::path resolvePath(
    const std::vector<std::filesystem::path>& searchPaths,
    const std::filesystem::path& currentWorkingDirectory,
    const std::string& filePath,
    FileChecker fileChecker = checkFileExists
);
```

**Purpose**: Resolve a path into an absolute file path, using search paths and current work directory

**Parameters**:
- searchPaths: Search paths used to look for filePath when it is neither absolute nor relative
- currentWorkingDirectory: Current working directory, used when filePath is relative
- filePath: Requested file path
- fileChecker: Function that checks if file path is valid (default: checkFileExists)

**Returns**: Fully resolved absolute path, or empty path when it could not be resolved

**Behavior**:
- Follows Renderman semantic
- Paths starting with `/` or `<driveletter>:` are considered absolute paths and already resolved
- Paths starting with `.` are considered relative paths, resolved against currentWorkingDirectory only
- All other paths are resolved using searchPaths, in order in which they appear
- Returns first path that exists according to fileChecker

**Implementation**:
```cpp
std::filesystem::path resolvePath(
    const std::vector<std::filesystem::path>& searchPaths,
    const std::filesystem::path& currentWorkingDirectory,
    const std::string& filePath,
    FileChecker fileChecker
)
{
    FALCOR_ASSERT(currentWorkingDirectory.is_absolute());

    if (filePath.empty())
        return std::filesystem::path();

    std::filesystem::path path(filePath);
    if (path.is_absolute())
    {
        if (fileChecker(path))
            return std::filesystem::weakly_canonical(path);
        return std::filesystem::path();
    }

    // This is relative case
    if (filePath[0] == '.')
    {
        std::filesystem::path result = currentWorkingDirectory / path;
        if (fileChecker(result))
            return std::filesystem::weakly_canonical(result);
        return std::filesystem::path();
    }

    // This is searchpath case
    for (const auto& searchpath : searchPaths)
    {
        std::filesystem::path result = searchpath / path;
        if (fileChecker(result))
            return std::filesystem::weakly_canonical(result);
    }
    return std::filesystem::path();
}
```

**Algorithm**:
1. Assert that currentWorkingDirectory is absolute
2. If filePath is empty, return empty path
3. If filePath is absolute:
   - Check if file exists using fileChecker
   - If exists, return weakly canonical path
   - Otherwise, return empty path
4. If filePath starts with '.' (relative):
   - Combine with currentWorkingDirectory
   - Check if file exists using fileChecker
   - If exists, return weakly canonical path
   - Otherwise, return empty path
5. Otherwise (search path case):
   - For each search path:
     - Combine search path with filePath
     - Check if file exists using fileChecker
     - If exists, return weakly canonical path
   - Return empty path if not found

**Path Resolution Order**:
1. Absolute paths (checked directly)
2. Relative paths (checked against currentWorkingDirectory)
3. Search paths (checked in order)

## Technical Details

### Environment Variable Expansion

Environment variable expansion algorithm:

```cpp
bool resolveEnvVariables(std::string& str, EnvVarResolver envResolver, std::string_view beginToken, std::string_view endToken)
{
    // ... implementation ...
}
```

**Characteristics**:
- In-place string modification
- Multiple environment variables supported
- Default tokens: "${" and "}"
- Custom tokens supported
- Missing variables replaced with empty string
- Mismatched tokens return false

**Example**:
```cpp
std::string path = "${HOME}/data";
resolveEnvVariables(path);
// path = "/home/user/data" (if HOME=/home/user)
```

### Search Path Resolution

Search path resolution algorithm:

```cpp
ResolvedPaths resolveSearchPaths(
    const std::vector<std::filesystem::path>& current,
    const std::vector<std::string>& update,
    const std::vector<std::filesystem::path>& standard,
    EnvVarResolver envResolver
)
{
    // ... implementation ...
}
```

**Characteristics**:
- Follows Renderman semantics
- Environment variable expansion
- Special character support (@, &)
- Semi-colon separator for multiple paths
- Absolute path validation
- Error reporting for invalid paths

**Special Characters**:
- **&**: Replace with current search paths
- **@**: Replace with standard search paths
- **;**: Separator for multiple paths

**Example**:
```cpp
std::vector<std::filesystem::path> current = {"/path1", "/path2"};
std::vector<std::filesystem::path> standard = {"/std1", "/std2"};
std::vector<std::string> update = {"&", "@", "/path3"};

ResolvedPaths result = resolveSearchPaths(current, update, standard);
// result.resolved = {"/path1", "/path2", "/std1", "/std2", "/path3"}
```

### File Path Resolution

File path resolution algorithm:

```cpp
std::filesystem::path resolvePath(
    const std::vector<std::filesystem::path>& searchPaths,
    const std::filesystem::path& currentWorkingDirectory,
    const std::string& filePath,
    FileChecker fileChecker
)
{
    // ... implementation ...
}
```

**Characteristics**:
- Follows Renderman semantics
- Absolute path detection
- Relative path resolution
- Search path resolution
- Custom file checker support
- First match returned

**Path Resolution Order**:
1. Absolute paths (checked directly)
2. Relative paths (checked against currentWorkingDirectory)
3. Search paths (checked in order)

**Example**:
```cpp
std::vector<std::filesystem::path> searchPaths = {"/search1", "/search2"};
std::filesystem::path cwd = "/current";
std::string filePath = "file.txt";

std::filesystem::path result = resolvePath(searchPaths, cwd, filePath);
// Checks: /current/file.txt, /search1/file.txt, /search2/file.txt
```

### File Existence Checking

File existence checking algorithm:

```cpp
static bool checkFileExists(const std::filesystem::path& path)
{
    return path.has_filename() && std::filesystem::exists(path);
}
```

**Characteristics**:
- Checks for filename
- Checks for existence
- Default file checker
- Custom file checkers supported

### Custom Resolvers

Custom environment variable resolver:

```cpp
using EnvVarResolver = std::optional<std::string>(const std::string&);

std::optional<std::string> customResolver(const std::string& name)
{
    if (name == "TEST")
        return std::string("test_value");
    return std::nullopt;
}

std::string path = "${TEST}/data";
resolveEnvVariables(path, customResolver);
// path = "test_value/data"
```

Custom file checker:

```cpp
using FileChecker = bool(const std::filesystem::path&);

bool customChecker(const std::filesystem::path& path)
{
    // Custom file checking logic
    return true;
}

std::filesystem::path result = resolvePath(searchPaths, cwd, filePath, customChecker);
```

### Weakly Canonical Path

Uses std::filesystem::weakly_canonical:

```cpp
std::filesystem::path path = std::filesystem::weakly_canonical(filePath);
```

**Characteristics**:
- Resolves symlinks if they exist
- Does not require path to exist
- Returns canonical path if possible
- Returns original path if not possible

### Error Handling

Error handling for invalid paths:

```cpp
ResolvedPaths resolveSearchPaths(...)
{
    ResolvedPaths result;

    for (std::string it : update)
    {
        if (!resolveEnvVariables(it, envResolver))
        {
            result.invalid.push_back(std::move(it));
            continue;
        }

        // ... path resolution ...

        if (path.is_absolute())
            result.resolved.push_back(std::move(path));
        else
            result.invalid.push_back(std::move(it2));
    }

    return result;
}
```

**Characteristics**:
- Invalid paths reported in ResolvedPaths.invalid
- Environment variable resolution errors reported
- Non-absolute paths reported
- Error reporting for debugging

## Integration Points

### Falcor Core Integration

- **Core/Macros**: FALCOR_API, FALCOR_ASSERT
- **Core/Error**: FALCOR_ASSERT
- **Core/Platform/OS**: getEnvironmentVariable

### Falcor Utils Integration

- **Utils/StringUtils**: splitString

### STL Integration

- **std::filesystem**: File system operations
- **std::string**: String operations
- **std::vector**: Vector operations
- **std::optional**: Optional values

### Internal Falcor Usage

- **Utils/Settings**: Settings module
- **Scene**: Scene file loading
- **Rendering**: Rendering file loading
- **Mogwai**: Application file loading

## Architecture Patterns

### Renderman Semantics Pattern

Follows Renderman path resolution semantics:

```cpp
std::filesystem::path resolvePath(...)
{
    // Absolute paths
    if (path.is_absolute())
        return path;

    // Relative paths
    if (filePath[0] == '.')
        return currentWorkingDirectory / path;

    // Search paths
    for (const auto& searchpath : searchPaths)
    {
        std::filesystem::path result = searchpath / path;
        if (fileChecker(result))
            return result;
    }
}
```

Benefits:
- Industry-standard semantics
- Familiar to graphics programmers
- Well-defined behavior
- Predictable results

### Custom Resolver Pattern

Supports custom resolvers for testing:

```cpp
using EnvVarResolver = std::optional<std::string>(const std::string&);

bool resolveEnvVariables(
    std::string& str,
    EnvVarResolver envResolver = getEnvironmentVariable,
    ...
);
```

Benefits:
- Testability
- Flexibility
- Custom behavior
- Mocking support

### Error Reporting Pattern

Reports invalid paths for error handling:

```cpp
struct ResolvedPaths
{
    std::vector<std::filesystem::path> resolved;
    std::vector<std::string> invalid;
};
```

Benefits:
- Clear error reporting
- Debugging support
- Error recovery
- User feedback

### Special Character Pattern

Supports special characters for path expansion:

```cpp
if (it2 == "&")
{
    result.resolved.insert(result.resolved.end(), current.begin(), current.end());
    continue;
}
if (it2 == "@")
{
    result.resolved.insert(result.resolved.end(), standard.begin(), standard.end());
    continue;
}
```

Benefits:
- Concise syntax
- Flexible path expansion
- Renderman compatibility
- Powerful path manipulation

### File Checker Pattern

Supports custom file checkers:

```cpp
using FileChecker = bool(const std::filesystem::path&);

std::filesystem::path resolvePath(
    ...
    FileChecker fileChecker = checkFileExists
);
```

Benefits:
- Flexibility
- Custom validation
- Testing support
- Mocking support

## Code Patterns

### Environment Variable Expansion Pattern

Replace environment variables in strings:

```cpp
bool resolveEnvVariables(std::string& str, EnvVarResolver envResolver, std::string_view beginToken, std::string_view endToken)
{
    std::string::size_type begin = str.find_first_of(beginToken);
    if (begin == std::string::npos)
        return true;

    std::string result = str.substr(0, begin);
    while (begin != std::string::npos)
    {
        std::string::size_type end = str.find_first_of(endToken, begin);
        if (end == std::string::npos)
            return false;
        std::string envVar = str.substr(begin + 2, end - begin - 2);
        std::string resolved = envResolver(envVar).value_or("");
        result += resolved;

        begin = str.find_first_of(beginToken, end);
        result += str.substr(end +1, begin - end -1);
    }

    str = std::move(result);
    return true;
}
```

Pattern:
- Find begin token
- Find end token
- Extract variable name
- Resolve variable
- Build result string
- Move result to input

### Search Path Resolution Pattern

Resolve search paths with special characters:

```cpp
ResolvedPaths resolveSearchPaths(...)
{
    ResolvedPaths result;

    for (std::string it : update)
    {
        if (!resolveEnvVariables(it, envResolver))
        {
            result.invalid.push_back(std::move(it));
            continue;
        }

        std::vector<std::string> temp = splitString(it, ";");
        for (std::string& it2 : temp)
        {
            if (it2.empty())
                continue;
            if (it2 == "&")
            {
                result.resolved.insert(result.resolved.end(), current.begin(), current.end());
                continue;
            }
            if (it2 == "@")
            {
                result.resolved.insert(result.resolved.end(), standard.begin(), standard.end());
                continue;
            }
            std::filesystem::path path = std::filesystem::weakly_canonical(it2);
            if (path.is_absolute())
                result.resolved.push_back(std::move(path));
            else
                result.invalid.push_back(std::move(it2));
        }
    }

    return result;
}
```

Pattern:
- Iterate over update paths
- Resolve environment variables
- Split by separator
- Handle special characters
- Validate absolute paths
- Report invalid paths

### File Path Resolution Pattern

Resolve file path using search paths:

```cpp
std::filesystem::path resolvePath(...)
{
    if (filePath.empty())
        return std::filesystem::path();

    std::filesystem::path path(filePath);
    if (path.is_absolute())
    {
        if (fileChecker(path))
            return std::filesystem::weakly_canonical(path);
        return std::filesystem::path();
    }

    if (filePath[0] == '.')
    {
        std::filesystem::path result = currentWorkingDirectory / path;
        if (fileChecker(result))
            return std::filesystem::weakly_canonical(result);
        return std::filesystem::path();
    }

    for (const auto& searchpath : searchPaths)
    {
        std::filesystem::path result = searchpath / path;
        if (fileChecker(result))
            return std::filesystem::weakly_canonical(result);
    }
    return std::filesystem::path();
}
```

Pattern:
- Check for empty path
- Check for absolute path
- Check for relative path
- Check search paths
- Return first match

### Custom Resolver Pattern

Use custom resolver for testing:

```cpp
std::optional<std::string> customResolver(const std::string& name)
{
    if (name == "TEST")
        return std::string("test_value");
    return std::nullopt;
}

std::string path = "${TEST}/data";
resolveEnvVariables(path, customResolver);
```

Pattern:
- Define custom resolver
- Pass to function
- Use for testing
- Mock behavior

## Use Cases

### Environment Variable Expansion

```cpp
std::string path = "${HOME}/data";
resolveEnvVariables(path);
// path = "/home/user/data" (if HOME=/home/user)
```

### Search Path Resolution

```cpp
std::vector<std::filesystem::path> current = {"/path1", "/path2"};
std::vector<std::filesystem::path> standard = {"/std1", "/std2"};
std::vector<std::string> update = {"&", "@", "/path3"};

ResolvedPaths result = resolveSearchPaths(current, update, standard);
// result.resolved = {"/path1", "/path2", "/std1", "/std2", "/path3"}
```

### File Path Resolution

```cpp
std::vector<std::filesystem::path> searchPaths = {"/search1", "/search2"};
std::filesystem::path cwd = "/current";
std::string filePath = "file.txt";

std::filesystem::path result = resolvePath(searchPaths, cwd, filePath);
// Checks: /current/file.txt, /search1/file.txt, /search2/file.txt
```

### Absolute Path Resolution

```cpp
std::vector<std::filesystem::path> searchPaths = {"/search1"};
std::filesystem::path cwd = "/current";
std::string filePath = "/absolute/path/file.txt";

std::filesystem::path result = resolvePath(searchPaths, cwd, filePath);
// result = "/absolute/path/file.txt" (if exists)
```

### Relative Path Resolution

```cpp
std::vector<std::filesystem::path> searchPaths = {"/search1"};
std::filesystem::path cwd = "/current";
std::string filePath = "./relative/file.txt";

std::filesystem::path result = resolvePath(searchPaths, cwd, filePath);
// result = "/current/relative/file.txt" (if exists)
```

### Custom Environment Variable Resolver

```cpp
std::optional<std::string> customResolver(const std::string& name)
{
    if (name == "TEST")
        return std::string("test_value");
    return std::nullopt;
}

std::string path = "${TEST}/data";
resolveEnvVariables(path, customResolver);
// path = "test_value/data"
```

### Custom File Checker

```cpp
bool customChecker(const std::filesystem::path& path)
{
    // Custom file checking logic
    return std::filesystem::exists(path) && path.extension() == ".txt";
}

std::filesystem::path result = resolvePath(searchPaths, cwd, filePath, customChecker);
```

### Multiple Paths in Single String

```cpp
std::vector<std::string> update = {"/path1;/path2;/path3"};

ResolvedPaths result = resolveSearchPaths(current, update, standard);
// result.resolved = {"/path1", "/path2", "/path3"}
```

### Environment Variable in Search Paths

```cpp
std::vector<std::string> update = {"${HOME}/data"};

ResolvedPaths result = resolveSearchPaths(current, update, standard);
// result.resolved = {"/home/user/data"} (if HOME=/home/user)
```

### Error Handling

```cpp
std::vector<std::string> update = {"/valid", "invalid", "${MISSING}"};

ResolvedPaths result = resolveSearchPaths(current, update, standard);
// result.resolved = {"/valid"}
// result.invalid = {"invalid", "${MISSING}"}
```

## Performance Considerations

### String Operations

- **String Copying**: String copying for environment variable expansion
- **String Concatenation**: String concatenation for result building
- **String Searching**: String searching for tokens
- **String Substring**: String substring extraction

### File System Operations

- **File Existence Check**: std::filesystem::exists() for file checking
- **Path Resolution**: std::filesystem::weakly_canonical() for path resolution
- **Path Validation**: std::filesystem::path::is_absolute() for validation

### Vector Operations

- **Vector Insertion**: Vector insertion for path expansion
- **Vector Iteration**: Vector iteration for path checking
- **Vector Copying**: Vector copying for special characters

### Memory Overhead

- **String Allocation**: String allocation for result building
- **Vector Allocation**: Vector allocation for resolved paths
- **Path Allocation**: Path allocation for resolution
- **No Dynamic Allocation**: No dynamic allocation for small paths

### Compiler Optimizations

- **Inline Expansion**: Functions can be inlined
- **String Optimization**: String optimization for small strings (SSO)
- **Path Optimization**: Path optimization for small paths
- **Move Semantics**: Move semantics for efficiency

### Comparison with Alternatives

**PathResolving vs Manual Path Resolution**:
- PathResolving: Renderman semantics, error reporting, special characters
- Manual: More control, less features
- Performance: Similar (PathResolving has minimal overhead)

**PathResolving vs std::filesystem**:
- PathResolving: Renderman semantics, search paths, environment variables
- std::filesystem: Standard library, no search paths
- Performance: PathResolving has more features

### Optimization Tips

1. **Reuse Search Paths**: Reuse search paths for multiple resolutions
2. **Cache Resolved Paths**: Cache resolved paths when possible
3. **Use Weakly Canonical**: Use weakly_canonical for non-existent paths
4. **Minimize String Copies**: Minimize string copies in custom resolvers
5. **Enable Optimizations**: Enable compiler optimizations

## Limitations

### Feature Limitations

- **Renderman Semantics Only**: Only supports Renderman semantics
- **No Wildcard Support**: No wildcard support in paths
- **No Regular Expressions**: No regular expression support
- **No Path Normalization**: No path normalization (only weakly canonical)
- **No Case Sensitivity**: No case sensitivity control
- **No Path Validation**: No path validation beyond existence checking
- **No Path Caching**: No path caching
- **No Async Resolution**: No asynchronous path resolution

### API Limitations

- **No Path Comparison**: No path comparison functions
- **No Path Manipulation**: No path manipulation functions
- **No Path Iteration**: No path iteration functions
- **No Path Querying**: No path querying functions
- **No Error Codes**: No error codes (only success/failure)
- **No Error Messages**: No error messages (only invalid paths)

### Type Limitations

- **String Only**: Only works with strings and paths
- **No Custom Types**: No custom type support
- **No Type Erasure**: No type erasure

### Performance Limitations

- **String Copying**: String copying for environment variable expansion
- **File System Calls**: File system calls for existence checking
- **Vector Operations**: Vector operations for path expansion
- **No Caching**: No path caching

### Platform Limitations

- **C++17 Only**: Requires C++17 or later
- **std::filesystem**: Requires std::filesystem support
- **No C++20 Features**: No C++20 features (std::format, etc.)

## Best Practices

### When to Use

**Appropriate Use Cases**:
- File path resolution in rendering applications
- Environment variable expansion
- Search path management
- Renderman-style path resolution
- Scene file loading
- Resource file loading

**Inappropriate Use Cases**:
- Wildcard matching (use other methods)
- Regular expressions (use other methods)
- Path normalization (use other methods)
- Complex path manipulation (use other methods)

### Usage Patterns

**Basic Path Resolution**:
```cpp
std::vector<std::filesystem::path> searchPaths = {"/search1", "/search2"};
std::filesystem::path cwd = "/current";
std::string filePath = "file.txt";

std::filesystem::path result = resolvePath(searchPaths, cwd, filePath);
```

**Environment Variable Expansion**:
```cpp
std::string path = "${HOME}/data";
resolveEnvVariables(path);
```

**Search Path Resolution**:
```cpp
std::vector<std::filesystem::path> current = {"/path1", "/path2"};
std::vector<std::filesystem::path> standard = {"/std1", "/std2"};
std::vector<std::string> update = {"&", "@", "/path3"};

ResolvedPaths result = resolveSearchPaths(current, update, standard);
```

**Custom Resolvers**:
```cpp
std::optional<std::string> customResolver(const std::string& name)
{
    if (name == "TEST")
        return std::string("test_value");
    return std::nullopt;
}

resolveEnvVariables(path, customResolver);
```

### Error Handling

- **Check Invalid Paths**: Check ResolvedPaths.invalid for errors
- **Handle Missing Files**: Handle empty path returns
- **Validate Paths**: Validate paths before use
- **Report Errors**: Report errors to user

### Performance Tips

1. **Reuse Search Paths**: Reuse search paths for multiple resolutions
2. **Cache Resolved Paths**: Cache resolved paths when possible
3. **Use Weakly Canonical**: Use weakly_canonical for non-existent paths
4. **Minimize String Copies**: Minimize string copies in custom resolvers
5. **Enable Optimizations**: Enable compiler optimizations

### Testing

- **Use Custom Resolvers**: Use custom resolvers for testing
- **Use Custom Checkers**: Use custom checkers for testing
- **Mock File System**: Mock file system for testing
- **Test Edge Cases**: Test edge cases (empty paths, invalid paths, etc.)

### Path Validation

- **Check Absolute Paths**: Check if paths are absolute
- **Check Relative Paths**: Check if paths are relative
- **Check File Existence**: Check if files exist
- **Validate Paths**: Validate paths before use

## Implementation Notes

### Environment Variable Expansion Implementation

Environment variable expansion algorithm:

```cpp
bool resolveEnvVariables(std::string& str, EnvVarResolver envResolver, std::string_view beginToken, std::string_view endToken)
{
    FALCOR_ASSERT(envResolver);

    std::string::size_type begin = str.find_first_of(beginToken);
    if (begin == std::string::npos)
        return true;

    std::string result = str.substr(0, begin);
    while (begin != std::string::npos)
    {
        std::string::size_type end = str.find_first_of(endToken, begin);
        if (end == std::string::npos)
            return false;
        std::string envVar = str.substr(begin + 2, end - begin - 2);
        std::string resolved = envResolver(envVar).value_or("");
        result += resolved;

        begin = str.find_first_of(beginToken, end);
        result += str.substr(end +1, begin - end -1);
    }

    str = std::move(result);
    return true;
}
```

**Characteristics**:
- In-place string modification
- Multiple environment variables supported
- Default tokens: "${" and "}"
- Custom tokens supported
- Missing variables replaced with empty string
- Mismatched tokens return false

### Search Path Resolution Implementation

Search path resolution algorithm:

```cpp
ResolvedPaths resolveSearchPaths(...)
{
    ResolvedPaths result;

    for (std::string it : update)
    {
        if (!resolveEnvVariables(it, envResolver))
        {
            result.invalid.push_back(std::move(it));
            continue;
        }

        std::vector<std::string> temp = splitString(it, ";");
        for (std::string& it2 : temp)
        {
            if (it2.empty())
                continue;
            if (it2 == "&")
            {
                result.resolved.insert(result.resolved.end(), current.begin(), current.end());
                continue;
            }
            if (it2 == "@")
            {
                result.resolved.insert(result.resolved.end(), standard.begin(), standard.end());
                continue;
            }
            std::filesystem::path path = std::filesystem::weakly_canonical(it2);
            if (path.is_absolute())
                result.resolved.push_back(std::move(path));
            else
                result.invalid.push_back(std::move(it2));
        }
    }

    return result;
}
```

**Characteristics**:
- Follows Renderman semantics
- Environment variable expansion
- Special character support (@, &)
- Semi-colon separator for multiple paths
- Absolute path validation
- Error reporting for invalid paths

### File Path Resolution Implementation

File path resolution algorithm:

```cpp
std::filesystem::path resolvePath(...)
{
    FALCOR_ASSERT(currentWorkingDirectory.is_absolute());

    if (filePath.empty())
        return std::filesystem::path();

    std::filesystem::path path(filePath);
    if (path.is_absolute())
    {
        if (fileChecker(path))
            return std::filesystem::weakly_canonical(path);
        return std::filesystem::path();
    }

    if (filePath[0] == '.')
    {
        std::filesystem::path result = currentWorkingDirectory / path;
        if (fileChecker(result))
            return std::filesystem::weakly_canonical(result);
        return std::filesystem::path();
    }

    for (const auto& searchpath : searchPaths)
    {
        std::filesystem::path result = searchpath / path;
        if (fileChecker(result))
            return std::filesystem::weakly_canonical(result);
    }
    return std::filesystem::path();
}
```

**Characteristics**:
- Follows Renderman semantics
- Absolute path detection
- Relative path resolution
- Search path resolution
- Custom file checker support
- First match returned

### File Existence Checking Implementation

File existence checking algorithm:

```cpp
static bool checkFileExists(const std::filesystem::path& path)
{
    return path.has_filename() && std::filesystem::exists(path);
}
```

**Characteristics**:
- Checks for filename
- Checks for existence
- Default file checker
- Custom file checkers supported

### Weakly Canonical Path Implementation

Uses std::filesystem::weakly_canonical:

```cpp
std::filesystem::path path = std::filesystem::weakly_canonical(filePath);
```

**Characteristics**:
- Resolves symlinks if they exist
- Does not require path to exist
- Returns canonical path if possible
- Returns original path if not possible

## Future Enhancements

### Potential Improvements

1. **Wildcard Support**: Add wildcard support in paths
2. **Regular Expressions**: Add regular expression support
3. **Path Normalization**: Add path normalization
4. **Case Sensitivity**: Add case sensitivity control
5. **Path Validation**: Add path validation beyond existence
6. **Path Caching**: Add path caching
7. **Async Resolution**: Add asynchronous path resolution
8. **Path Comparison**: Add path comparison functions
9. **Path Manipulation**: Add path manipulation functions
10. **Path Iteration**: Add path iteration functions

### API Extensions

1. **Error Codes**: Add error codes for better error handling
2. **Error Messages**: Add error messages for better debugging
3. **Path Querying**: Add path querying functions
4. **Path Statistics**: Add path statistics functions
5. **Path Validation**: Add path validation functions
6. **Path Normalization**: Add path normalization functions
7. **Path Comparison**: Add path comparison functions
8. **Path Manipulation**: Add path manipulation functions

### Performance Enhancements

1. **Path Caching**: Add path caching
2. **Lazy Resolution**: Add lazy path resolution
3. **Batch Resolution**: Add batch path resolution
4. **Parallel Resolution**: Add parallel path resolution
5. **Memory Pool**: Add memory pool for paths

### Documentation Enhancements

1. **More Examples**: Add more usage examples
2. **Performance Guide**: Add performance optimization guide
3. **Best Practices**: Add best practices guide
4. **Migration Guide**: Add migration guide from manual resolution
5. **API Reference**: Add complete API reference

## References

### C++ Standard

- **std::filesystem**: C++17 file system library
- **std::string**: C++ string library
- **std::vector**: C++ vector library
- **std::optional**: C++17 optional library

### Falcor Documentation

- **Falcor API**: Falcor API documentation
- **Falcor Utils**: Utils module documentation

### Related Technologies

- **Renderman**: Renderman path resolution semantics
- **Path Resolution**: Path resolution algorithms
- **Environment Variables**: Environment variable expansion
- **File System**: File system operations
