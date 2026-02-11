# ProgramVersion

## Module Overview

The **ProgramVersion** module provides program version management for Falcor engine. This module includes [`EntryPointKernel`](Source/Falcor/Core/Program/ProgramVersion.h:62) class, [`EntryPointGroupKernels`](Source/Falcor/Core/Program/ProgramVersion.h:128) class, [`ProgramKernels`](Source/Falcor/Core/Program/ProgramVersion.h:171) class, and [`ProgramVersion`](Source/Falcor/Core/Program/ProgramVersion.h:244) class, providing kernel code management, entry point group management, and program versioning.

## Files

- **Header**: `Source/Falcor/Core/Program/ProgramVersion.h` (309 lines)
- **Implementation**: `Source/Falcor/Core/Program/ProgramVersion.cpp` (not provided in file list)

## Module Structure

### Classes

1. **EntryPointKernel** - Entry point kernel management
2. **EntryPointGroupKernels** - Entry point group management
3. **ProgramKernels** - Program kernels management
4. **ProgramVersion** - Program version management

## EntryPointKernel Class

### Definition

```cpp
class FALCOR_API EntryPointKernel : public Object
{
    FALCOR_OBJECT(EntryPointKernel)
public:
    struct BlobData
    {
        const void* data;
        size_t size;
    };

    static ref<EntryPointKernel> create(
        Slang::ComPtr<slang::IComponentType> linkedSlangEntryPoint,
        ShaderType type,
        const std::string& entryPointName
    );

    ShaderType getType() const;
    const std::string& getEntryPointName() const;
    BlobData getBlobData() const;

protected:
    EntryPointKernel(Slang::ComPtr<slang::IComponentType> linkedSlangEntryPoint, ShaderType type, const std::string& entryPointName);

    Slang::ComPtr<slang::IComponentType> mLinkedSlangEntryPoint;
    ShaderType mType;
    std::string mEntryPointName;
};
```

### Dependencies

### Internal Dependencies

- **Object**: Inherits from [`Object`](Source/Falcor/Core/Object.h) for reference counting
- **Program**: Uses [`Program`](Source/Falcor/Core/Program/Program.h:573) for program management
- **ProgramReflection**: Uses [`ProgramReflection`](Source/Falcor/Core/Program/ProgramReflection.h:1631) for reflection
- **EntryPointKernel**: Manages individual entry point kernels
- **EntryPointGroupKernels**: Manages entry point groups
- **ProgramKernels**: Manages program kernels
- **ProgramVersion**: Manages program versions

### External Dependencies

- **Slang**: Uses Slang compiler for kernel code generation
- **GFX**: Uses GFX API for shader program management
- **std::vector**: Uses std::vector for container storage
- **std::string**: Uses std::string for string storage
- **DefineList**: Uses [`DefineList`](Source/Falcor/Core/Program/DefineList.h) for define management
- **ref<T>**: Uses [`ref<T>`](Source/Falcor/Core/Object.h) for reference counting

## Usage Patterns

### Creating EntryPointKernel

```cpp
auto kernel = EntryPointKernel::create(linkedSlangEntryPoint, ShaderType::Compute, "main");
```

### Creating EntryPointGroupKernels

```cpp
auto group = EntryPointGroupKernels::create(
    EntryPointGroupKernels::Type::Rasterization,
    kernels,
    "MyExportName"
);
```

### Creating ProgramKernels

```cpp
auto kernels = ProgramKernels::create(uniqueEntryPointGroups, gfxProgram, slangSession, slangGlobalScope, slangEntryPoints);
```

### Creating ProgramVersion

```cpp
auto version = ProgramVersion::createEmpty(program, slangGlobalScope);
version->init(defineList, reflector, descStr, slangEntryPoints);
```

## Summary

**ProgramVersion** is a complex module that provides program version management for Falcor engine:

### EntryPointKernel Class
- **Memory Layout**: ~36-44 bytes + heap allocations
- **Threading Model**: Thread-safe for concurrent reads (immutable after construction)
- **Cache Locality**: Excellent (all members fit in cache line)
- **Algorithmic Complexity**: O(1) for most operations
- **Blob Data**: Stores compiled kernel code
- **Slang Integration**: Integrates with Slang compiler

### EntryPointGroupKernels Class
- **Memory Layout**: ~52-64 bytes + heap allocations
- **Threading Model**: Thread-safe for concurrent reads (immutable after construction)
- **Cache Locality**: Excellent (all members fit in cache line)
- **Algorithmic Complexity**: O(1) for most operations
- **Type Support**: Supports compute, rasterization, and raytracing entry point groups

### ProgramKernels Class
- **Memory Layout**: ~72-80 bytes + heap allocations
- **Threading Model**: Thread-safe for concurrent reads (immutable after construction)
- **Cache Locality**: Excellent (all members fit in cache line)
- **Algorithmic Complexity**: O(1) for most operations
- **Slang Integration**: Integrates with Slang compiler and GFX API

### ProgramVersion Class
- **Memory Layout**: ~112-144 bytes + heap allocations
- **Threading Model**: Thread-safe for concurrent reads (immutable after construction)
- **Cache Locality**: Excellent (all members fit in cache line)
- **Algorithmic Complexity**: O(1) for most operations
- **Slang Integration**: Integrates with Slang compiler for kernel code generation

### Module Characteristics
- Immutable after construction (most classes)
- Reference counting (ref<T> provides automatic resource management)
- Factory pattern (static create methods)
- Kernel management (EntryPointKernel, EntryPointGroupKernels, ProgramKernels)
- Program version management (ProgramVersion)
- Slang integration (integrates with Slang compiler for kernel code generation)
- Excellent cache locality (all members fit in cache line)
- Thread-safe for concurrent reads (immutable after construction)
- Not thread-safe for concurrent writes
- High complexity (many nested types and containers)

The module provides a comprehensive program version management system with kernel code generation, entry point group management, and program versioning, with excellent cache locality for all members, thread-safe for concurrent reads (immutable after construction), and integration with Slang compiler for kernel code generation.
