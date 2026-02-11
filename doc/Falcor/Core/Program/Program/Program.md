# Program

## Module Overview

The **Program** module provides shader program management for the Falcor engine. This module includes the [`TypeConformance`](Source/Falcor/Core/Program/Program.h:58) struct, [`TypeConformanceList`](Source/Falcor/Core/Program/Program.h:80) class, [`ProgramDesc`](Source/Falcor/Core/Program/Program.h:156) struct, and [`Program`](Source/Falcor/Core/Program/Program.h:573) class, which collectively handle shader compilation, reflection, variable binding, and program versioning.

## Files

- **Header**: `Source/Falcor/Core/Program/Program.h` (806 lines)
- **Implementation**: `Source/Falcor/Core/Program/Program.cpp` (455 lines)

## TypeConformance Struct

### Definition

```cpp
struct TypeConformance
{
    std::string typeName;
    std::string interfaceName;
    TypeConformance() = default;
    TypeConformance(const std::string& typeName_, const std::string& interfaceName_);
    bool operator<(const TypeConformance& other) const;
    bool operator==(const TypeConformance& other) const;
    struct HashFunction;
};
```

### Dependencies

### Internal Dependencies

- **Object**: Inherits from [`Object`](Source/Falcor/Core/Object.h) for reference counting
- **Device**: Uses [`Device`](Source/Falcor/Core/API/Device.h) for GPU device
- **ProgramManager**: Uses [`ProgramManager`](Source/Falcor/Core/Program/ProgramManager.h) for program management
- **ProgramVersion**: Uses [`ProgramVersion`](Source/Falcor/Core/Program/ProgramVersion.h) for program versioning
- **ProgramReflection**: Uses [`ProgramReflection`](Source/Falcor/Core/Program/ProgramReflection.h) for program reflection
- **RtStateObject**: Uses [`RtStateObject`](Source/Falcor/Core/API/RtStateObject.h) for raytracing state objects
- **StateGraph**: Uses [`StateGraph`](Source/Falcor/Core/State/StateGraph.h) for state management

### External Dependencies

- **Slang**: Uses Slang compiler for shader compilation
- **std::map**: Uses std::map for program version caching
- **std::unordered_map**: Uses std::unordered_map for file time tracking
- **std::vector**: Uses std::vector for container storage
- **std::string**: Uses std::string for string storage
- **std::filesystem**: Uses std::filesystem for file paths

## Usage Patterns

### Creating a Compute Program

```cpp
// Create a compute program
auto program = Program::createCompute(
    device,
    "shaders/compute.slang",
    "main",
    DefineList().add("USE_LIGHTING", "1"),
    SlangCompilerFlags::GenerateDebugInfo,
    ShaderModel::SM6_5
);
```

### Creating a Graphics Program

```cpp
// Create a graphics program
auto program = Program::createGraphics(
    device,
    "shaders/graphics.slang",
    "vsMain",
    "psMain",
    DefineList().add("USE_LIGHTING", "1"),
    SlangCompilerFlags::GenerateDebugInfo,
    ShaderModel::SM6_5
);
```

### Creating a Raytracing Program

```cpp
// Create a raytracing program
ProgramDesc desc;
desc.addShaderLibrary("shaders/raytracing.slang");
desc.addRayGen("rayGen");
desc.addMiss("miss");
desc.addHitGroup("closestHit", "anyHit");
desc.setMaxTraceRecursionDepth(2);
desc.setMaxPayloadSize(32);
desc.setMaxAttributeSize(8);

auto program = Program::create(device, desc, DefineList());
```

### Adding Defines

```cpp
// Add defines
program->addDefine("USE_LIGHTING", "1");
program->addDefine("MAX_LIGHTS", "16");

// Add multiple defines
program->addDefines(DefineList()
    .add("USE_SHADOWS", "1")
    .add("MAX_SHADOW_CASCADES", "4")
);
```

### Removing Defines

```cpp
// Remove defines
program->removeDefine("USE_LIGHTING");

// Remove multiple defines
program->removeDefines(DefineList()
    .add("USE_SHADOWS")
    .add("MAX_SHADOW_CASCADES")
);

// Remove defines by prefix
program->removeDefines(0, 3, "USE_");
```

### Setting Defines

```cpp
// Set defines
program->setDefines(DefineList()
    .add("USE_LIGHTING", "1")
    .add("MAX_LIGHTS", "16")
);
```

### Adding Type Conformances

```cpp
// Add type conformance
program->addTypeConformance("MyMaterial", "IMaterial", 0);

// Set type conformances
program->setTypeConformances(TypeConformanceList()
    .add("MyMaterial", "IMaterial", 0)
    .add("MyLight", "ILight", 1)
);
```

### Getting Active Version

```cpp
// Get active version
auto activeVersion = program->getActiveVersion();
auto reflector = program->getReflector();
```

## Summary

**Program** is a complex module that provides shader program management for the Falcor engine:

### TypeConformance Struct
- **Memory Layout**: ~48-64 bytes + string heap allocations
- **Threading Model**: Not thread-safe for concurrent writes
- **Cache Locality**: Poor (strings are heap-allocated)
- **Algorithmic Complexity**: O(K) for comparison, hashing, and copy (K = length of strings)

### TypeConformanceList Class
- **Memory Layout**: ~48-64 bytes (empty) + ~72-96 bytes per entry
- **Threading Model**: Not thread-safe for concurrent writes
- **Cache Locality**: Poor (red-black tree structure)
- **Algorithmic Complexity**: O(log N) for add/remove operations

### ProgramDesc Struct
- **Memory Layout**: ~144-160 bytes + heap allocations
- **Threading Model**: Not thread-safe for concurrent writes
- **Cache Locality**: Poor (vectors and TypeConformanceList are heap-allocated)
- **Algorithmic Complexity**: O(1) for add operations, O(N×M) for hasEntryPoint and finalize

### Program Class
- **Memory Layout**: ~768-1024 bytes + heap allocations
- **Threading Model**: Not thread-safe for concurrent writes
- **Cache Locality**: Poor (all containers are heap-allocated)
- **Algorithmic Complexity**: O(log N) for define/type conformance operations, O(compilation time) for link

### Module Characteristics
- Builder pattern for program creation
- Factory pattern for program creation
- Lazy compilation (programs are compiled on demand)
- Version caching (multiple program versions are cached)
- Hot reload (file time tracking)
- Raytracing support (raytracing-specific methods)
- Fluent interface (methods return *this for method chaining)
- High complexity due to multiple nested structures
- Poor cache locality (heap allocation for all containers)
- Not thread-safe for concurrent writes
- High memory overhead (heap allocation for all containers)

The module provides a comprehensive shader program management system with lazy compilation, version caching, and hot reload support, but suffers from poor cache locality and high memory overhead due to heap allocation for all containers, and is not thread-safe for concurrent writes.
