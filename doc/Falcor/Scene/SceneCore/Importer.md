# Importer - Scene Importer Base Class

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **Importer** - Importer base class
  - [x] **Importer.h** - Importer header (92 lines)
  - [x] **Importer.cpp** - Importer implementation (58 lines)
- [x] **ImporterError** - Importer error exception
  - [x] **ImporterError.h** - Importer error header (72 lines)

### External Dependencies

- **Scene/SceneBuilder** - Scene builder interface
- **Core/Macros** - Macro definitions
- **Core/Error** - Error handling
- **Core/Plugin** - Plugin system
- **Core/Platform/OS** - Platform-specific code
- **Utils/Scripting/ScriptBindings** - Python scripting bindings

## Module Overview

Importer is the base class for all scene importers in Falcor. Importers are bound to a set of file extensions. This allows the right importer to be called when importing an asset file. The Importer class provides a plugin-based architecture for loading scene files from various formats.

## Component Specifications

### Importer (Base Class)

**File**: [`Importer.h`](Source/Falcor/Scene/Importer.h:1)

**Purpose**: Base class for importers. Importers are bound to a set of file extensions. This allows the right importer to be called when importing an asset file.

**Key Features**:
- Plugin-based architecture
- File extension binding
- Scene import from file
- Scene import from memory
- Material short name mapping
- Plugin manager integration
- Supported extensions enumeration

**Public Types**:
- [`PluginCreate`](Source/Falcor/Scene/Importer.h:50) - `std::function<std::unique_ptr<Importer>()>` - Plugin creation function type
- [`PluginInfo`](Source/Falcor/Scene/Importer.h:51) - Plugin information structure
  - [`desc`](Source/Falcor/Scene/Importer.h:53) - `std::string` - Importer description
  - [`extensions`](Source/Falcor/Scene/Importer.h:54) - `std::vector<std::string>` - List of handled file extensions

**Public Methods**:
- [`importScene(const std::filesystem::path& path, SceneBuilder& builder, const std::map<std::string, std::string>& materialToShortName = 0)`](Source/Falcor/Scene/Importer.h:67) - Import a scene from file
  - Throws an ImporterError if something went wrong
- [`importSceneFromMemory(const void* buffer, size_t byteSize, std::string_view extension, SceneBuilder& builder, const std::map<std::string, std::string>& materialToShortName)`](Source/Falcor/Scene/Importer.h:77) - Import a scene from memory
  - Throws an ImporterError if something went wrong
- [`create(std::string_view extension, const PluginManager& pm = PluginManager::instance())`](Source/Falcor/Scene/Importer.h:86) - Create an importer for a file of an asset with a given file extension
  - Returns an instance of the importer or nullptr if no compatible importer was found
- [`getSupportedExtensions(const PluginManager& pm = PluginManager::instance())`](Source/Falcor/Scene/Importer.h:90) - Return a list of supported file extensions by the current set of loaded importer plugins

**Plugin Macros**:
- [`FALCOR_PLUGIN_BASE_CLASS(Importer)`](Source/Falcor/Scene/Importer.h:57) - Plugin base class macro

**Virtual Destructor**:
- [`virtual ~Importer()`](Source/Falcor/Scene/Importer.h:59) - Virtual destructor

### Importer Implementation

**File**: [`Importer.cpp`](Source/Falcor/Scene/Importer.cpp:1)

**Purpose**: Importer implementation with plugin manager integration and Python bindings.

**Key Features**:
- Plugin manager integration for importer creation
- Extension-based importer lookup
- Supported extensions enumeration
- Python exception bindings
- Memory import not implemented

**Implementation Details**:

**Importer Creation**:
- [`create(std::string_view extension, const PluginManager& pm)`](Source/Falcor/Scene/Importer.cpp:33) - Creates importer for given extension
- Iterates through plugin manager infos: `for (const auto& [type, info] : pm.getInfos<Importer>())`
- Checks if extension is supported: `if (std::find(info.extensions.begin(), info.extensions.end(), extension) != info.extensions.end())`
- Creates importer instance: `return pm.createClass<Importer>(type)`
- Returns nullptr if no compatible importer found

**Supported Extensions**:
- [`getSupportedExtensions(const PluginManager& pm)`](Source/Falcor/Scene/Importer.cpp:41) - Returns list of supported extensions
- Iterates through all importer plugins
- Collects all extensions: `extensions.insert(extensions.end(), info.extensions.begin(), info.extensions.end())`
- Returns combined list of all supported extensions

**Memory Import**:
- [`importSceneFromMemory(const void* buffer, size_t byteSize, std::string_view extension, SceneBuilder& builder, const std::map<std::string, std::string>& materialToShortName)`](Source/Falcor/Scene/Importer.cpp:49) - Not implemented
- Throws exception: `FALCOR_THROW("Not implemented.")`

**Python Bindings**:
- [`FALCOR_SCRIPT_BINDING(Importer)`](Source/Falcor/Scene/Importer.cpp:54) - Script binding macro
- Registers ImporterError exception: `pybind11::register_exception<ImporterError>(m, "ImporterError")`

### ImporterError (Exception)

**File**: [`ImporterError.h`](Source/Falcor/Scene/ImporterError.h:1)

**Purpose**: Exception thrown during scene import. Holds the path of the imported asset and a description of the exception.

**Key Features**:
- Exception for import errors
- Path storage for error reporting
- Description storage
- Copy constructor support
- Template constructor support
- Path accessor

**Public Methods**:
- [`ImporterError() noexcept`](Source/Falcor/Scene/ImporterError.h:45) - Default constructor
- [`ImporterError(const std::filesystem::path& path, std::string_view what)`](Source/Falcor/Scene/ImporterError.h:48) - Constructor with path and description
  - Inherits from Exception with what parameter
  - Stores path: `mpPath(std::make_shared<std::filesystem::path>(path))`
- [`template<typename... Args> explicit ImporterError(const std::filesystem::path& path, fmt::format_string<Args...> format, Args&&... args)`](Source/Falcor/Scene/ImporterError.h:54) - Template constructor with formatted message
  - Uses fmt::format for message formatting
  - Forwards all arguments: `std::forward<Args>(args)...`
- [`virtual ~ImporterError() override`](Source/Falcor/Scene/ImporterError.h:58) - Virtual destructor override
- [`ImporterError(const ImporterError& other) noexcept`](Source/Falcor/Scene/ImporterError.h:61) - Copy constructor
  - Copies what and path from other exception
- [`const std::filesystem::path& path() const noexcept`](Source/Falcor/Scene/ImporterError.h:67) - Returns path accessor

**Private Members**:
- [`mpPath`](Source/Falcor/Scene/ImporterError.h:70) - `std::shared_ptr<std::filesystem::path>` - Shared pointer to path

## Technical Details

### Plugin Architecture

**Plugin Base Class**:
- Uses `FALCOR_PLUGIN_BASE_CLASS(Importer)` macro
- Integrates with Falcor plugin system
- Automatic registration with plugin manager

**Plugin Info Structure**:
- Description field for importer documentation
- Extensions field for file extension binding
- Used by plugin manager for importer discovery

### File Extension Binding

**Extension Lookup**:
- Plugin manager stores importer info with extensions
- `create()` method searches for matching extension
- Returns first matching importer
- Returns nullptr if no match found

**Supported Extensions**:
- Aggregates all extensions from all loaded importers
- Returns combined list for UI display
- Used for file dialog filters

### Scene Import Process

**File Import**:
- Takes file path and scene builder
- Optional material to short name mapping
- Throws ImporterError on failure
- Implemented by each importer plugin

**Memory Import**:
- Takes memory buffer and size
- Takes file extension for format detection
- Not implemented in base class
- Must be implemented by importer plugins

### Error Handling

**Exception Hierarchy**:
- Inherits from Exception base class
- Provides path context for error reporting
- Supports formatted error messages
- Supports exception chaining

**Error Information**:
- Path of imported asset
- Error description
- Optional additional context

## Integration Points

### Scene Builder Integration

**Scene Construction**:
- Importer receives SceneBuilder reference
- Adds geometry, materials, lights to scene
- Uses builder methods for scene construction

**Material System Integration**:
- Material to short name mapping
- Allows material name customization
- Supports material library integration

### Plugin System Integration

**Plugin Manager**:
- Uses PluginManager for importer discovery
- Automatic plugin registration
- Dynamic importer loading

**Python Integration**:
- ImporterError exception exposed to Python
- Allows Python scripts to catch import errors
- Provides path information for error reporting

### File System Integration

**Path Handling**:
- Uses std::filesystem for cross-platform path handling
- Stores path in shared_ptr for exception safety
- Supports path-based error reporting

## Architecture Patterns

### Plugin Pattern

- Importer as plugin base class
- Dynamic plugin loading
- Extension-based plugin discovery
- Plugin manager integration

### Factory Pattern

- `create()` method acts as factory
- Creates importer instances based on file extension
- Returns nullptr if no compatible importer found

### Template Method Pattern

- `importScene()` and `importSceneFromMemory()` are template methods
- Must be implemented by derived classes
- Base class provides interface

### Exception Pattern

- ImporterError extends Exception base class
- Provides context-specific error information
- Supports exception chaining

### Strategy Pattern

- Different import strategies for different file formats
- File-based import vs memory-based import
- Format-specific implementations

## Code Patterns

### Plugin Registration Pattern

```cpp
FALCOR_PLUGIN_BASE_CLASS(Importer)
```
- Registers class as plugin
- Provides metadata for plugin discovery
- Integrates with plugin manager

### Extension Lookup Pattern

```cpp
std::unique_ptr<Importer> Importer::create(std::string_view extension, const PluginManager& pm)
{
    for (const auto& [type, info] : pm.getInfos<Importer>())
        if (std::find(info.extensions.begin(), info.extensions.end(), extension) != info.extensions.end())
            return pm.createClass<Importer>(type);
    return nullptr;
}
```

### Exception Constructor Pattern

```cpp
template<typename... Args>
explicit ImporterError(const std::filesystem::path& path, fmt::format_string<Args...> format, Args&&... args)
    : ImporterError(path, fmt::format(format, std::forward<Args>(args)...))
{}
```

### Python Binding Pattern

```cpp
FALCOR_SCRIPT_BINDING(Importer)
{
    pybind11::register_exception<ImporterError>(m, "ImporterError");
}
```

## Use Cases

### Scene Loading

- Load scene from file on disk
- Support multiple file formats (Assimp, glTF, USD, etc.)
- Error reporting with file path context
- Material name customization

### Scene Import from Memory

- Load scene from memory buffer
- Support embedded scene data
- Format detection from extension
- Used for runtime scene generation

### Plugin Development

- Create new importers for custom formats
- Register with plugin system
- Automatic discovery and loading
- Extension-based routing

### Error Handling

- Catch import errors with specific exception type
- Get file path context for debugging
- Format error messages with templates
- Exception chaining for error propagation

### Python Scripting

- Catch ImporterError in Python scripts
- Access file path for error reporting
- Provide user-friendly error messages

## Performance Considerations

### Plugin Discovery

**Extension Lookup**:
- O(n) where n is number of importer plugins
- Linear search through plugin infos
- Negligible overhead for typical plugin count

**Supported Extensions**:
- O(n) where n is number of importer plugins
- Collects all extensions from all plugins
- Called infrequently (typically for UI display)

### Scene Import

**File I/O**:
- Depends on file format and scene size
- Typically O(n) where n is number of scene elements
- Memory allocation for scene data structures

**Memory Import**:
- Depends on file format and buffer size
- Typically O(n) where n is number of scene elements
- No file I/O overhead

### Error Handling

**Exception Overhead**:
- Exception creation is O(1)
- Path storage uses shared_ptr (reference counting)
- Negligible runtime overhead

## Limitations

### Feature Limitations

- **Memory Import Not Implemented**:
  - Base class throws "Not implemented" exception
  - Must be implemented by derived importer plugins
  - Limits flexibility for runtime scene generation

- **Extension Matching**:
  - Returns first matching importer
  - No priority or selection mechanism
  - May not handle multiple importers for same extension

- **Error Information**:
  - Limited to path and description
  - No error codes or severity levels
  - No additional context beyond path

### Performance Limitations

- **Plugin Discovery**:
  - Linear search through all plugins
  - No caching of plugin lookups
  - May be slow with many plugins

- **Extension Lookup**:
  - No priority or preference system
  - First match wins
  - No plugin version checking

### Integration Limitations

- **Scene Builder**:
  - Requires SceneBuilder interface
  - Tightly coupled to Falcor scene system
  - Not suitable for external scene systems

- **Plugin Manager**:
  - Requires PluginManager instance
  - Dependent on plugin system architecture
  - Not suitable for standalone use

## Best Practices

### Importer Development

- **Plugin Registration**:
  - Use `FALCOR_PLUGIN_BASE_CLASS(Importer)` macro
  - Provide clear description in PluginInfo
  - List all supported file extensions
  - Implement both `importScene()` and `importSceneFromMemory()`

- **Error Handling**:
  - Throw ImporterError for all import failures
  - Include file path in exception
  - Use descriptive error messages
  - Provide context for debugging

- **Material Handling**:
  - Use materialToShortName mapping
  - Support material name customization
  - Validate material names during import
  - Handle missing materials gracefully

### Performance Optimization

- **Plugin Discovery**:
  - Minimize number of importer plugins
  - Use specific extensions to avoid conflicts
  - Cache plugin lookups if needed
  - Profile plugin loading time

- **Scene Import**:
  - Use efficient data structures
  - Minimize memory allocations
  - Batch scene operations
  - Profile import performance

### Error Reporting

- **Exception Design**:
  - Use specific exception types
  - Include context information
  - Support exception chaining
  - Provide actionable error messages

- **Path Handling**:
  - Use std::filesystem for cross-platform support
  - Validate paths before import
  - Handle path errors gracefully
  - Provide clear error messages

### Integration

- **Scene Builder**:
  - Use SceneBuilder methods correctly
  - Follow scene construction patterns
  - Validate scene data
  - Handle builder errors

- **Plugin System**:
  - Follow plugin system conventions
  - Provide proper metadata
  - Test plugin loading
  - Handle plugin conflicts

### Development

- **Testing**:
  - Test import with various file formats
  - Test error handling
  - Test plugin discovery
  - Test memory import (if implemented)

- **Debugging**:
  - Log plugin discovery
  - Log import progress
  - Log errors with context
  - Use debugger for import failures

- **Documentation**:
  - Document supported file formats
  - Document import options
  - Document error conditions
  - Provide usage examples

## Progress Log

- **2026-01-07T23:51:00Z**: Importer analysis completed. Analyzed Importer.h (92 lines), Importer.cpp (58 lines), and ImporterError.h (72 lines). Documented plugin-based architecture, file extension binding, scene import from file and memory, plugin manager integration, supported extensions enumeration, exception handling with path context, Python bindings, and integration points. Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices. Marked Importer as Complete.

## Completion Status

The Importer module is complete. All source files have been analyzed and documented with comprehensive technical specifications covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
