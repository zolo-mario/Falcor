# ProgramVars

## Module Overview

The **ProgramVars** module provides shader variable management for Falcor engine. This module includes [`ProgramVars`](Source/Falcor/Core/Program/ProgramVars.h:47) base class for graphics programs and [`RtProgramVars`](Source/Falcor/Core/Program/ProgramVars.h:82) derived class for raytracing programs, providing high-level abstraction for variable assignment, root signature management, and shader table preparation.

## Files

- **Header**: `Source/Falcor/Core/Program/ProgramVars.h` (128 lines)
- **Implementation**: `Source/Falcor/Core/Program/ProgramVars.cpp` (211 lines)

## Module Structure

### Classes

1. **ProgramVars** - Base class for graphics program variables
2. **RtProgramVars** - Derived class for raytracing program variables
3. **RtStateObject** - Raytracing state object

## ProgramVars Class

### Definition

```cpp
class FALCOR_API ProgramVars : public ParameterBlock
{
public:
    static ref<ProgramVars> create(ref<Device> pDevice, const ref<const ProgramReflection>& pReflector);
    static ref<ProgramVars> create(ref<Device> pDevice, const Program* pProg);

    const ref<const ProgramReflection>& getReflection() const { return mpReflector; }

    ref<Device> getDevice() const { return mpDevice; }

    bool prepareShaderTable(RenderContext* pCtx, RtStateObject* pRtso);

    ShaderTablePtr getShaderTable() const { return mpShaderTable; }

    uint32_t getRootDescriptorRangeCount() const;
    uint32_t getRootDescriptorRangeIndex(uint32_t index) const;

    uint32_t getParameterBlockSubObjectRangeCount() const;
    uint32_t getParameterBlockSubObjectRangeIndex(uint32_t index) const;

    void setConstantBuffer(const ConstantBuffer* pCB, size_t size);
    void setConstantBuffer(const ConstantBuffer* pCB, size_t offset, size_t size);

    void setTexture(Texture* pTexture, size_t mipLevel, size_t mostDetailedMip);
    void setTexture(Texture* pTexture, size_t mipLevel, size_t mostDetailedMip, const Texture* pTextureArray);
    void setTexture(Texture* pTexture, size_t mipLevel, size_t mostDetailedMip, size_t firstArraySlice, size_t arraySize);

    void setSampler(Sampler* pSampler);
    void setSampler(Sampler* pSampler, const Sampler* pSamplerArray);
    void setSampler(Sampler* pSampler, size_t firstArraySlice, size_t arraySize);

    void setBuffer(Buffer* pBuffer);
    void setBuffer(Buffer* pBuffer, uint64_t offset, uint64_t size);
    void setBuffer(Buffer* pBuffer, uint64_t offset, uint64_t size, const Buffer* pBufferArray);
    void setBuffer(Buffer* pBuffer, uint64_t offset, uint64_t size, size_t firstArraySlice, size_t arraySize);

    void setAccelerationStructure(AccelerationStructure* pAccelStruct);
    void setAccelerationStructure(AccelerationStructure* pAccelStruct, const AccelStructure* pAccelStructArray);
    void setAccelerationStructure(AccelerationStructure* pAccelStruct, size_t firstArraySlice, size_t arraySize);

    void setRootSignature(RootSignature* pRootSignature);

    void applyParameterBlocks();

protected:
    ProgramVars(ref<Device> pDevice, const ref<const ProgramReflection>& pReflector);
    ProgramVars(const ProgramVars&) = default;

    ref<Device> mpDevice;
    ref<const ProgramReflection> mpReflector;
    ShaderTablePtr mpShaderTable;
};
```

### Dependencies

### Internal Dependencies

- **ParameterBlock**: Inherits from [`ParameterBlock`](Source/Falcor/Core/Program/ProgramReflection.h:1391)
- **Program**: Uses [`Program`](Source/Falcor/Core/Program/Program.h:573) for program management
- **ProgramReflection**: Uses [`ProgramReflection`](Source/Falcor/Core/Program/ProgramReflection.h:1631) for reflection
- **RtBindingTable**: Uses [`RtBindingTable`](Source/Falcor/Core/Program/RtBindingTable.h) for raytracing binding table
- **ShaderTable**: Uses [`ShaderTable`](Source/Falcor/Core/API/ShaderTable.h) for shader table
- **RootSignature**: Uses [`RootSignature`](Source/Falcor/Core/API/RootSignature.h) for root signature

### External Dependencies

- **Device**: Uses [`Device`](Source/Falcor/Core/API/Device.h) for GPU device
- **RenderContext**: Uses [`RenderContext`](Source/Falcor/Core/API/RenderContext.h) for rendering context
- **ComputeContext**: Uses [`ComputeContext`](Source/Falcor/Core/API/ComputeContext.h) for compute context
- **Texture**: Uses [`Texture`](Source/Falcor/Core/API/Texture.h) for texture
- **Sampler**: Uses [`Sampler`](Source/Falcor/Core/API/Sampler.h) for sampler
- **Buffer**: Uses [`Buffer`](Source/Falcor/Core/API/Buffer.h) for buffer
- **AccelerationStructure**: Uses [`AccelerationStructure`](Source/Falcor/Core/API/AccelerationStructure.h) for acceleration structure
- **RootSignature**: Uses [`RootSignature`](Source/Falcor/Core/API/RootSignature.h) for root signature
- **ref<T>`**: Uses [`ref<T>`](Source/Falcor/Core/Object.h) for reference counting
- **std::vector**: Uses std::vector for container storage

## Usage Patterns

### Creating Graphics ProgramVars

```cpp
// Create graphics program vars
auto programVars = ProgramVars::create(device, reflector);
```

### Creating Raytracing ProgramVars

```cpp
// Create raytracing program vars
auto rtProgramVars = RtProgramVars::create(device, program, bindingTable);
```

### Setting Constant Buffer

```cpp
// Set constant buffer
programVars->setConstantBuffer(constantBuffer, size);
```

### Setting Texture

```cpp
// Set texture
programVars->setTexture(texture, mipLevel, mostDetailedMip);
```

### Setting Sampler

```cpp
// Set sampler
programVars->setSampler(sampler);
```

### Setting Buffer

```cpp
// Set buffer
programVars->setBuffer(buffer, offset, size);
```

### Setting Acceleration Structure

```cpp
// Set acceleration structure
programVars->setAccelerationStructure(accelStruct);
```

### Setting Root Signature

```cpp
// Set root signature
programVars->setRootSignature(rootSignature);
```

### Preparing Shader Table

```cpp
// Prepare shader table
programVars->prepareShaderTable(renderContext, rtStateObject);
```

## Summary

**ProgramVars** is a complex module that provides shader variable management for Falcor engine:

### ProgramVars Class
- **Memory Layout**: ~24-32 bytes + heap allocations (inherited from ParameterBlock)
- **Threading Model**: Not thread-safe for concurrent writes
- **Cache Locality**: Excellent (all members fit in cache line)
- **Algorithmic Complexity**: O(1) for most accessor methods
- **High-Level Abstraction**: Provides high-level abstraction for variable assignment

### RtProgramVars Class
- **Memory Layout**: ~128-160 bytes + heap allocations
- **Threading Model**: Not thread-safe for concurrent writes
- **Cache Locality**: Excellent (all members fit in cache line)
- **Algorithmic Complexity**: O(1) for most accessor methods
- **Raytracing Support**: Provides raytracing-specific variable management

### RtStateObject Class
- **Memory Layout**: ~40-48 bytes + heap allocations
- **Threading Model**: Thread-safe for concurrent reads (immutable after construction)
- **Cache Locality**: Excellent (all members fit in cache line)
- **Algorithmic Complexity**: O(1) for getGfxProgram()

### Module Characteristics
- Inheritance (RtProgramVars inherits from ProgramVars)
- Factory pattern (static create methods)
- RAII (ref<T> provides automatic resource management)
- Reference counting (ref<T> uses reference counting for shared ownership)
- Parameter block pattern (inherits from ParameterBlock)
- Shader table management (manages shader table for variable binding)
- Root signature management (manages root signature for descriptor binding)
- Variable assignment (high-level abstraction for variable assignment)
- Excellent cache locality (all members fit in cache line)
- Not thread-safe for concurrent writes
- High-level abstraction (provides high-level abstraction for variable assignment)

The module provides a comprehensive shader variable management system with high-level abstraction for variable assignment, shader table management, root signature management, and raytracing support, with excellent cache locality for all members, but is not thread-safe for concurrent writes.
