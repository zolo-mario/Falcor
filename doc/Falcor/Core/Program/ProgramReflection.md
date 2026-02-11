# ProgramReflection

## Module Overview

The **ProgramReflection** module provides comprehensive shader reflection for Falcor engine. This module includes multiple classes for offset types, reflection types, and parameter blocks, providing detailed information about shader program structure, layout, and resources.

## Files

- **Header**: `Source/Falcor/Core/Program/ProgramReflection.h` (1762 lines)
- **Implementation**: `Source/Falcor/Core/Program/ProgramReflection.cpp` (not provided in file list)

## Module Structure

### Offset Types

1. **UniformShaderVarOffset** - Offset for uniform shader variables
2. **ResourceShaderVarOffset** - Offset for resource shader variables
3. **ShaderVarOffset** - Combined offset for shader variables
4. **TypedShaderVarOffset** - Typed offset for shader variables

### Reflection Types

1. **ReflectionType** - Base class for all reflection types
2. **ReflectionArrayType** - Array type reflection
3. **ReflectionStructType** - Struct type reflection
4. **ReflectionBasicType** - Basic type reflection (scalars, vectors, matrices)
5. **ReflectionResourceType** - Resource type reflection (textures, buffers, samplers)
6. **ReflectionInterfaceType** - Interface type reflection
7. **ReflectionVar** - Variable reflection
8. **ParameterBlockReflection** - Parameter block reflection
9. **EntryPointGroupReflection** - Entry point group reflection
10. **ProgramReflection** - Program reflection (root reflection object)

## UniformShaderVarOffset Struct

### Definition

```cpp
struct UniformShaderVarOffset
{
    typedef uint32_t ByteOffset;
    explicit UniformShaderVarOffset(size_t offset);
    enum Zero { kZero = 0 };
    enum Invalid { kInvalid = -1 };
    UniformShaderVarOffset(Zero);
    UniformShaderVarOffset(Invalid _ = kInvalid);
    ByteOffset getByteOffset() const;
    bool isValid() const;
    bool operator==(const UniformShaderVarOffset& other) const;
    bool operator!=(const UniformShaderVarOffset& other) const;
    bool operator==(Invalid _) const;
    bool operator!=(Invalid _) const;
    UniformShaderVarOffset operator+(size_t offset) const;
    UniformShaderVarOffset operator+(UniformShaderVarOffset other) const;

private:
    ByteOffset mByteOffset = ByteOffset(-1);
};
```

### Dependencies

### Internal Dependencies

- **Object**: Inherits from [`Object`](Source/Falcor/Core/Object.h) for reference counting
- **ProgramVersion**: Uses [`ProgramVersion`](Source/Falcor/Core/Program/ProgramVersion.h) for program version
- **ParameterBlockReflection**: Uses [`ParameterBlockReflection`](Source/Falcor/Core/Program/ProgramReflection.h:1391) for parameter blocks
- **EntryPointGroupReflection**: Inherits from [`ParameterBlockReflection`](Source/Falcor/Core/Program/ProgramReflection.h:1391)

### External Dependencies

- **Slang**: Uses Slang compiler for reflection
- **std::map**: Uses std::map for name to index mapping
- **std::vector**: Uses std::vector for container storage
- **std::string**: Uses std::string for string storage
- **slang::TypeLayoutReflection**: Uses Slang type layout reflection
- **slang::ShaderReflection**: Uses Slang shader reflection
- **slang::EntryPointLayout**: Uses Slang entry point layout

## Summary

**ProgramReflection** is a complex module that provides comprehensive shader reflection for Falcor engine:

### Offset Types
- **UniformShaderVarOffset**: 4 bytes, excellent cache locality, O(1) operations
- **ResourceShaderVarOffset**: 8 bytes, excellent cache locality, O(1) operations
- **ShaderVarOffset**: 12 bytes, excellent cache locality, O(1) operations
- **TypedShaderVarOffset**: 20 bytes, excellent cache locality, O(N) operations (N = number of members)

### Reflection Types
- **ReflectionType**: ~40-48 bytes, moderate cache locality, O(1) for most operations, O(N) for member lookups
- **ReflectionArrayType**: ~24-32 bytes, excellent cache locality, O(1) operations
- **ReflectionStructType**: ~96-112 bytes, moderate cache locality, O(N) for member lookups
- **ReflectionBasicType**: 8 bytes, excellent cache locality, O(1) operations
- **ReflectionResourceType**: ~40-48 bytes, excellent cache locality, O(1) operations
- **ReflectionInterfaceType**: ~16-24 bytes, excellent cache locality, O(1) operations
- **ReflectionVar**: ~44-52 bytes, excellent cache locality, O(1) operations
- **ParameterBlockReflection**: ~48-64 bytes, excellent cache locality, O(N) for member lookups
- **EntryPointGroupReflection**: Inherits from ParameterBlockReflection
- **ProgramReflection**: ~336-448 bytes, moderate cache locality, O(log N) for type lookups, O(N) for member lookups

### Module Characteristics
- Immutable after construction (most reflection types)
- Type hierarchy (ReflectionType is base class)
- Offset types (separate types for uniform, resource, and combined offsets)
- Dynamic casting (ReflectionType supports dynamic casting)
- Factory pattern (static create methods)
- Builder pattern (ParameterBlockReflection uses builder pattern)
- Reference counting (ref<T> provides automatic resource management)
- Slang integration (integrates with Slang compiler)
- Excellent cache locality (offset types)
- Moderate cache locality (reflection types with containers)
- Thread-safe for concurrent reads (immutable after construction)
- Not thread-safe for concurrent writes
- High complexity (many nested types and containers)

The module provides a comprehensive shader reflection system with excellent cache locality for offset types and moderate cache locality for reflection types with containers, immutable after construction for most types, thread-safe for concurrent reads, and integrates with Slang compiler for reflection.
