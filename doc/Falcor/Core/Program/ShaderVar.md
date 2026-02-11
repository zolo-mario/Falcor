# ShaderVar

## Module Overview

The **ShaderVar** module provides shader variable pointer management for Falcor engine. This module includes [`ShaderVar`](Source/Falcor/Core/Program/ShaderVar.h:77) struct, providing a lightweight pointer-like interface for accessing and modifying shader variables in parameter blocks.

## Files

- **Header**: `Source/Falcor/Core/Program/ShaderVar.h` (471 lines)
- **Implementation**: `Source/Falcor/Core/Program/ShaderVar.cpp` (not provided in file list)

## Module Structure

### Structs

1. **ShaderVar** - Shader variable pointer management

## ShaderVar Struct

### Definition

```cpp
struct FALCOR_API ShaderVar
{
    ShaderVar();
    ShaderVar(const ShaderVar& other);
    explicit ShaderVar(ParameterBlock* pObject, const TypedShaderVarOffset& offset);
    explicit ShaderVar(ParameterBlock* pObject);

    bool isValid() const;
    const ReflectionType* getType() const;
    TypedShaderVarOffset getOffset() const;
    size_t getByteOffset() const;

    ShaderVar operator[](std::string_view name) const;
    ShaderVar operator[](size_t index) const;
    ShaderVar operator[](const TypedShaderVarOffset& offset) const;
    ShaderVar operator[](const UniformShaderVarOffset& offset) const;

    ShaderVar findMember(std::string_view name) const;
    ShaderVar findMember(uint32_t index) const;
    bool hasMember(std::string_view name) const;
    bool hasMember(uint32_t index) const;

    template<typename T>
    void set(const T& val) const;

    template<typename T>
    void operator=(const T& val) const;

    void setBlob(void const* data, size_t size) const;
    template<typename T>
    void setBlob(const T& val) const;

    void setBuffer(const ref<Buffer>& pBuffer) const;
    ref<Buffer> getBuffer() const;
    operator ref<Buffer>() const;

    void setTexture(const ref<Texture>& pTexture) const;
    ref<Texture> getTexture() const;
    operator ref<Texture>() const;

    void setSrv(const ref<ShaderResourceView>& pSrv) const;
    ref<ShaderResourceView> getSrv() const;

    void setUav(const ref<UnorderedAccessView>& pUav) const;
    ref<UnorderedAccessView> getUav() const;

    void setAccelerationStructure(const ref<RtAccelerationStructure>& pAccl) const;
    ref<RtAccelerationStructure> getAccelerationStructure() const;

    void setSampler(const ref<Sampler>& pSampler) const;
    ref<Sampler> getSampler() const;
    operator ref<Sampler>() const;

    void setParameterBlock(const ref<ParameterBlock>& pBlock) const;
    ref<ParameterBlock> getParameterBlock() const;

    operator TypedShaderVarOffset() const;
    operator UniformShaderVarOffset() const;

    void const* getRawData() const;

private:
    ParameterBlock* mpBlock;
    TypedShaderVarOffset mOffset;

    void setImpl(const ref<Texture>& pTexture) const;
    void setImpl(const ref<Sampler>& pSampler) const;
    void setImpl(const ref<Buffer>& pBuffer) const;
    void setImpl(const ref<ParameterBlock>& pBlock) const;

    template<typename T>
    void setImpl(const T& val) const;
};
```

### Dependencies

### Internal Dependencies

- **ParameterBlock**: Uses [`ParameterBlock`](Source/Falcor/Core/API/ParameterBlock.h) for parameter block management
- **TypedShaderVarOffset**: Uses [`TypedShaderVarOffset`](Source/Falcor/Core/Program/ProgramReflection.h) for offset management
- **UniformShaderVarOffset**: Uses [`UniformShaderVarOffset`](Source/Falcor/Core/Program/ProgramReflection.h) for uniform offset management
- **ReflectionType**: Uses [`ReflectionType`](Source/Falcor/Core/Program/ProgramReflection.h) for type reflection

### External Dependencies

- **ref<T>**: Uses [`ref<T>`](Source/Falcor/Core/Object.h) for reference counting
- **Buffer**: Uses [`Buffer`](Source/Falcor/Core/API/Buffer.h) for buffer binding
- **Texture**: Uses [`Texture`](Source/Falcor/Core/API/Texture.h) for texture binding
- **Sampler**: Uses [`Sampler`](Source/Falcor/Core/API/Sampler.h) for sampler binding
- **ShaderResourceView**: Uses [`ShaderResourceView`](Source/Falcor/Core/API/ResourceViews.h) for SRV binding
- **UnorderedAccessView**: Uses [`UnorderedAccessView`](Source/Falcor/Core/API/ResourceViews.h) for UAV binding
- **RtAccelerationStructure**: Uses [`RtAccelerationStructure`](Source/Falcor/Core/API/RtAccelerationStructure.h) for acceleration structure binding
- **std::string_view**: Uses std::string_view for string arguments

## Usage Patterns

### Creating Null ShaderVar

```cpp
ShaderVar var; // Null/invalid shader variable pointer
```

### Creating ShaderVar from ParameterBlock

```cpp
ShaderVar var = ShaderVar(pObject); // Points to root of parameter block
```

### Creating ShaderVar from Offset

```cpp
ShaderVar var = ShaderVar(pObject, offset); // Points to specific offset
```

### Checking Validity

```cpp
if (var.isValid()) {
    // Use var
}
```

### Getting Type

```cpp
const ReflectionType* type = var.getType();
```

### Getting Offset

```cpp
TypedShaderVarOffset offset = var.getOffset();
size_t byteOffset = var.getByteOffset();
```

### Navigating Struct Fields

```cpp
ShaderVar field = var["fieldName"];
```

### Navigating Array Elements

```cpp
ShaderVar element = var[2];
```

### Finding Members (Safe)

```cpp
ShaderVar field = var.findMember("fieldName");
if (field.isValid()) {
    // Use field
}
```

### Setting Values

```cpp
var.set(float4(0));
var = float4(0); // Equivalent
```

### Setting Raw Binary Data

```cpp
var.setBlob(data, size);
var.setBlob(someStruct); // Equivalent
```

### Binding Buffers

```cpp
var.setBuffer(pBuffer);
ref<Buffer> buffer = var.getBuffer();
ref<Buffer> buffer = var; // Implicit conversion
```

### Binding Textures

```cpp
var.setTexture(pTexture);
ref<Texture> texture = var.getTexture();
ref<Texture> texture = var; // Implicit conversion
```

### Binding Samplers

```cpp
var.setSampler(pSampler);
ref<Sampler> sampler = var.getSampler();
ref<Sampler> sampler = var; // Implicit conversion
```

### Binding SRVs

```cpp
var.setSrv(pSrv);
ref<ShaderResourceView> srv = var.getSrv();
```

### Binding UAVs

```cpp
var.setUav(pUav);
ref<UnorderedAccessView> uav = var.getUav();
```

### Binding Acceleration Structures

```cpp
var.setAccelerationStructure(pAccl);
ref<RtAccelerationStructure> accl = var.getAccelerationStructure();
```

### Binding Parameter Blocks

```cpp
var.setParameterBlock(pBlock);
ref<ParameterBlock> block = var.getParameterBlock();
```

### Getting Raw Data (Unsafe)

```cpp
void const* data = var.getRawData();
```

### Using Offset Information

```cpp
TypedShaderVarOffset offset = pVars["myVar"];
pVars[offset] = someValue;
```

## Summary

**ShaderVar** is a lightweight module that provides shader variable pointer management for Falcor engine:

### ShaderVar Struct
- **Memory Layout**: ~16-24 bytes (no heap allocation)
- **Threading Model**: Not thread-safe
- **Cache Locality**: Excellent (all members fit in cache line)
- **Algorithmic Complexity**: O(1) for most operations, O(N) for string-based lookups
- **Pointer-Like Interface**: Provides pointer-like interface for accessing shader variables
- **Unowned Pointer**: Uses unowned pointer for performance (no reference counting)
- **Offset-Based Access**: Uses offset-based access for efficient navigation
- **Type-Safe**: Type-safe access to shader variables
- **Resource Binding**: Provides resource binding for textures, buffers, samplers, etc.
- **Implicit Conversion**: Provides implicit conversion to resource types

### Module Characteristics
- No heap allocation (lightweight)
- Unowned pointer (unsafe to hold for long periods)
- Pointer-like interface (provides pointer-like interface for accessing shader variables)
- Offset-based access (uses offset-based access for efficient navigation)
- Type-safe (type-safe access to shader variables)
- Resource binding (provides resource binding for textures, buffers, samplers, etc.)
- Implicit conversion (provides implicit conversion to resource types)
- Excellent cache locality (all members fit in cache line)
- Not thread-safe (requires external synchronization)
- Zero-copy access (zero-copy access to parameter block data)

The module provides a comprehensive shader variable pointer management system with pointer-like interface, offset-based access, and excellent cache locality, optimized for performance with zero-copy access to parameter block data and unowned pointer for performance.
