# RtBindingTable

## Module Overview

The **RtBindingTable** module provides raytracing binding table management for Falcor engine. This module includes [`RtBindingTable`](Source/Falcor/Core/Program/RtBindingTable.h:48) class, providing efficient mapping of raytracing shaders (raygen, miss, hit groups) to geometry IDs and ray types.

## Files

- **Header**: `Source/Falcor/Core/Program/RtBindingTable.h` (151 lines)
- **Implementation**: `Source/Falcor/Core/Program/RtBindingTable.cpp` (not provided in file list)

## Module Structure

### Classes

1. **RtBindingTable** - Raytracing binding table management

## RtBindingTable Class

### Definition

```cpp
class FALCOR_API RtBindingTable : public Object
{
    FALCOR_OBJECT(RtBindingTable)
public:
    using ShaderID = ProgramDesc::ShaderID;

    static ref<RtBindingTable> create(uint32_t missCount, uint32_t rayTypeCount, uint32_t geometryCount);

    void setRayGen(ShaderID shaderID);
    void setMiss(uint32_t missIndex, ShaderID shaderID);
    void setHitGroup(uint32_t rayType, uint32_t geometryID, ShaderID shaderID);
    void setHitGroup(uint32_t rayType, const std::vector<uint32_t>& geometryIDs, ShaderID shaderID);

    ShaderID getRayGen() const;
    ShaderID getMiss(uint32_t missIndex) const;
    ShaderID getHitGroup(uint32_t rayType, uint32_t geometryID) const;

    uint32_t getMissCount() const;
    uint32_t getRayTypeCount() const;
    uint32_t getGeometryCount() const;

private:
    RtBindingTable(uint32_t missCount, uint32_t rayTypeCount, uint32_t geometryCount);

    uint32_t getMissOffset(uint32_t missIndex) const;
    uint32_t getHitGroupOffset(uint32_t rayType, uint32_t geometryID) const;

    uint32_t mMissCount = 0;
    uint32_t mRayTypeCount = 0;
    uint32_t mGeometryCount = 0;

    std::vector<ShaderID> mShaderTable;
};
```

### Dependencies

### Internal Dependencies

- **Object**: Inherits from [`Object`](Source/Falcor/Core/Object.h) for reference counting
- **Program**: Uses [`Program`](Source/Falcor/Core/Program/Program.h:573) for shader program management
- **ProgramDesc**: Uses [`ProgramDesc::ShaderID`](Source/Falcor/Core/Program/Program.h:573) for shader ID
- **SceneIDs**: Uses [`GlobalGeometryID`](Source/Falcor/Scene/SceneIDs.h) for geometry ID

### External Dependencies

- **std::vector**: Uses std::vector for container storage
- **ref<T>**: Uses [`ref<T>`](Source/Falcor/Core/Object.h) for reference counting
- **uint32_t**: Uses uint32_t for index types

## Usage Patterns

### Creating RtBindingTable

```cpp
auto bindingTable = RtBindingTable::create(missCount, rayTypeCount, geometryCount);
```

### Setting Raygen Shader

```cpp
bindingTable->setRayGen(raygenShaderID);
```

### Setting Miss Shaders

```cpp
bindingTable->setMiss(0, missShaderID0);
bindingTable->setMiss(1, missShaderID1);
```

### Setting Hit Groups (Single Geometry)

```cpp
bindingTable->setHitGroup(rayType, geometryID, hitGroupShaderID);
```

### Setting Hit Groups (Multiple Geometries)

```cpp
std::vector<uint32_t> geometryIDs = {0, 1, 2};
bindingTable->setHitGroup(rayType, geometryIDs, hitGroupShaderID);
```

### Getting Raygen Shader

```cpp
ShaderID raygenShaderID = bindingTable->getRayGen();
```

### Getting Miss Shaders

```cpp
ShaderID missShaderID = bindingTable->getMiss(missIndex);
```

### Getting Hit Groups

```cpp
ShaderID hitGroupShaderID = bindingTable->getHitGroup(rayType, geometryID);
```

## Summary

**RtBindingTable** is a specialized module that provides raytracing binding table management for Falcor engine:

### RtBindingTable Class
- **Memory Layout**: ~32-40 bytes + heap allocations
- **Threading Model**: Not thread-safe
- **Cache Locality**: Excellent (inline members fit in cache line)
- **Algorithmic Complexity**: O(1) for most operations, O(N) for creation
- **Flat Array Layout**: Uses flat array for efficient lookup
- **Index-Based Access**: Uses index-based access for O(1) lookup
- **Raytracing-Specific**: Optimized for raytracing shader binding

### Module Characteristics
- Flat array layout (efficient memory layout)
- Index-based access (O(1) lookup)
- Factory pattern (static create method)
- Reference counting (ref<T> provides automatic resource management)
- Raytracing-specific (optimized for raytracing shader binding)
- Excellent cache locality (inline members fit in cache line)
- Not thread-safe (requires external synchronization)
- Moderate complexity (flat array layout)

The module provides a comprehensive raytracing binding table management system with flat array layout, index-based access, and excellent cache locality, optimized for raytracing shader binding with O(1) lookup operations.
