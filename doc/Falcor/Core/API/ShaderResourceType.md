# ShaderResourceType

## Overview
[`ShaderResourceType`](Source/Falcor/Core/API/ShaderResourceType.h) provides shader resource type enumeration for categorizing different types of GPU resources that can be bound to shaders. This is a header-only utility module.

## Source Files
- Header: [`Source/Falcor/Core/API/ShaderResourceType.h`](Source/Falcor/Core/API/ShaderResourceType.h) (50 lines)
- Implementation: Header-only (no separate .cpp file)

## Type Definitions

### ShaderResourceType Enum
```cpp
enum class ShaderResourceType
{
    TextureSrv,        ///< Texture shader resource view
    TextureUav,        ///< Texture unordered access view
    RawBufferSrv,       ///< Raw buffer shader resource view
    RawBufferUav,       ///< Raw buffer unordered access view
    TypedBufferSrv,     ///< Typed buffer shader resource view
    TypedBufferUav,     ///< Typed buffer unordered access view
    StructuredBufferSrv,  ///< Structured buffer shader resource view
    StructuredBufferUav,  ///< Structured buffer unordered access view
    Cbv,               ///< Constant buffer view
    AccelerationStructureSrv,  ///< Acceleration structure shader resource view
    AccelerationStructureUav,  ///< Acceleration structure unordered access view
    Dsv,               ///< Depth-stencil view
    Rtv,               ///< Render target view
    Sampler,            ///< Sampler
    Count               ///< Total number of resource types
};
```
**Purpose**: Enumeration of shader resource types

**Memory Layout**: 4 bytes (enum, likely 4 bytes)

**Cache Locativity**: **Excellent** - Fits in a single cache line

**Usage**: Categorizes different types of GPU resources that can be bound to shaders

**Total Types**: 13 resource types

## Type Descriptions

### TextureSrv
- **Type**: Texture shader resource view
- **Usage**: Read-only texture access in shaders
- **Access Pattern**: Sample from textures in pixel/vertex/geometry/hull/compute shaders

### TextureUav
- **Type**: Texture unordered access view
- **Usage**: Read-write texture access in shaders
- **Access Pattern**: Read/write to textures in pixel/vertex/geometry/hull/compute shaders
- **Atomic Operations**: Supports atomic operations on texture data

### RawBufferSrv
- **Type**: Raw buffer shader resource view
- **Usage**: Read-only raw buffer access in shaders
- **Access Pattern**: Read from untyped buffers in shaders
- **Data Type**: Raw bytes (no type information)

### RawBufferUav
- **Type**: Raw buffer unordered access view
- **Usage**: Read-write raw buffer access in shaders
- **Access Pattern**: Read/write to untyped buffers in shaders
- **Data Type**: Raw bytes (no type information)
- **Atomic Operations**: Supports atomic operations on buffer data

### TypedBufferSrv
- **Type**: Typed buffer shader resource view
- **Usage**: Read-only typed buffer access in shaders
- **Access Pattern**: Read from typed buffers in shaders
- **Data Type**: Typed data (with format information)

### TypedBufferUav
- **Type**: Typed buffer unordered access view
- **Usage**: Read-write typed buffer access in shaders
- **Access Pattern**: Read/write to typed buffers in shaders
- **Data Type**: Typed data (with format information)
- **Atomic Operations**: Supports atomic operations on buffer data

### StructuredBufferSrv
- **Type**: Structured buffer shader resource view
- **Usage**: Read-only structured buffer access in shaders
- **Access Pattern**: Read from structured buffers in shaders
- **Data Type**: Structured data (with stride information)

### StructuredBufferUav
- **Type**: Structured buffer unordered access view
- **Usage**: Read-write structured buffer access in shaders
- **Access Pattern**: Read/write to structured buffers in shaders
- **Data Type**: Structured data (with stride information)
- **Atomic Operations**: Supports atomic operations on buffer data

### Cbv
- **Type**: Constant buffer view
- **Usage**: Read-only constant buffer access in shaders
- **Access Pattern**: Read from constant buffers in shaders
- **Data Type**: Constant data (uniform buffers)
- **Update Frequency**: Typically updated per draw call

### AccelerationStructureSrv
- **Type**: Acceleration structure shader resource view
- **Usage**: Read-only acceleration structure access in shaders
- **Access Pattern**: Read from acceleration structures in raytracing shaders
- **Data Type**: Acceleration structure data (BVH, etc.)

### AccelerationStructureUav
- **Type**: Acceleration structure unordered access view
- **Usage**: Read-write acceleration structure access in shaders
- **Access Pattern**: Read/write to acceleration structures in raytracing shaders
- **Data Type**: Acceleration structure data (BVH, etc.)
- **Atomic Operations**: Supports atomic operations on acceleration structure data

### Dsv
- **Type**: Depth-stencil view
- **Usage**: Depth-stencil buffer access in shaders
- **Access Pattern**: Depth and stencil testing in shaders
- **Data Type**: Depth and stencil data

### Rtv
- **Type**: Render target view
- **Usage**: Render target output in shaders
- **Access Pattern**: Write to render targets in pixel/vertex/geometry shaders
- **Data Type**: Color/depth-stencil data

### Sampler
- **Type**: Sampler
- **Usage**: Texture sampling configuration in shaders
- **Access Pattern**: Sample from textures in shaders
- **Data Type**: Sampler state (filtering, addressing, etc.)

## Dependencies

### Direct Dependencies
- None (no dependencies)

### Indirect Dependencies
- None (no dependencies)

## Threading Model

**Thread Safety**: **Thread-Safe (No Mutable State)**

**Analysis**:
- No global or static mutable variables
- Enum is a pure type (no functions or mutable state)
- Safe for concurrent access from multiple threads
- No synchronization primitives needed

**Thread Safety Characteristics**:
1. **No Mutable State**: Enum is a pure type
2. **No Functions**: No functions that could have side effects
3. **Safe for Concurrent Access**: Multiple threads can safely use this type simultaneously
4. **No External Dependencies**: Only depends on standard C++ types

**Note**: This is a simple enum type with no functions or mutable state. It is inherently thread-safe for any use.

## Cache Locativity Analysis

### Access Patterns

#### Enum Usage
- **Type Definition**: Enum value definition
- **Single Cache Line**: Fits in a single cache line
- **Excellent for Repeated Access**: Excellent cache locality for repeated access

**Cache Locativity**: **Excellent**
- Single cache line access
- Excellent for repeated access
- No cache misses for typical usage

### Cache Line Analysis

#### ShaderResourceType Enum
- **Size**: 4 bytes (enum, likely 4 bytes)
- **Cache Line**: Fits in a single cache line (64 bytes)
- **Cache Locativity**: **Excellent**

## Algorithmic Complexity Analysis

### Enum Operations

#### Type Definition
**Time Complexity**: **O(1)**
- Enum value definition: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Enum value: 4 bytes

#### Type Comparison
**Time Complexity**: **O(1)**
- Enum comparison: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Comparison result: 1 byte (bool)

#### Type Assignment
**Time Complexity**: **O(1)**
- Enum assignment: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Assignment result: 4 bytes (enum)

## Key Technical Features

### Resource Type Categorization
- **Texture Resources**: TextureSrv, TextureUav
- **Buffer Resources**: RawBufferSrv, RawBufferUav, TypedBufferSrv, TypedBufferUav, StructuredBufferSrv, StructuredBufferUav
- **Constant Buffers**: Cbv
- **Acceleration Structures**: AccelerationStructureSrv, AccelerationStructureUav
- **Depth-Stencil**: Dsv
- **Render Targets**: Rtv
- **Samplers**: Sampler

### Access Pattern Classification
- **Read-Only**: TextureSrv, RawBufferSrv, TypedBufferSrv, StructuredBufferSrv, AccelerationStructureSrv, Cbv
- **Read-Write**: TextureUav, RawBufferUav, TypedBufferUav, StructuredBufferUav, AccelerationStructureUav
- **Output-Only**: Rtv (render target output)
- **Configuration**: Sampler (texture sampling configuration)

### Data Type Classification
- **Raw Data**: RawBufferSrv, RawBufferUav (no type information)
- **Typed Data**: TypedBufferSrv, TypedBufferUav, StructuredBufferSrv, StructuredBufferUav (with format/stride information)
- **Structured Data**: StructuredBufferSrv, StructuredBufferUav (with stride information)
- **Constant Data**: Cbv (uniform buffers)

### Shader Stage Support
- **Pixel Shaders**: TextureSrv, TextureUav, RawBufferSrv, RawBufferUav, TypedBufferSrv, TypedBufferUav, StructuredBufferSrv, StructuredBufferUav, Rtv, Sampler
- **Vertex Shaders**: RawBufferSrv, RawBufferUav, TypedBufferSrv, TypedBufferUav, StructuredBufferSrv, StructuredBufferUav, Cbv
- **Geometry Shaders**: RawBufferSrv, RawBufferUav, TypedBufferSrv, TypedBufferUav, StructuredBufferSrv, StructuredBufferUav
- **Hull Shaders**: RawBufferSrv, RawBufferUav, TypedBufferSrv, TypedBufferUav, StructuredBufferSrv, StructuredBufferUav
- **Domain Shaders**: RawBufferSrv, RawBufferUav, TypedBufferSrv, TypedBufferUav, StructuredBufferSrv, StructuredBufferUav, Cbv, TextureSrv, TextureUav, Sampler
- **Compute Shaders**: All resource types except Rtv (render target output only)
- **Raytracing Shaders**: AccelerationStructureSrv, AccelerationStructureUav

### Atomic Operations Support
- **TextureUav**: Supports atomic operations on texture data
- **RawBufferUav**: Supports atomic operations on buffer data
- **TypedBufferUav**: Supports atomic operations on buffer data
- **StructuredBufferUav**: Supports atomic operations on buffer data
- **AccelerationStructureUav**: Supports atomic operations on acceleration structure data

### Count Value
- **Count**: Total number of resource types (13)
- **Usage**: Can be used for array bounds checking
- **Iteration**: Can be used to iterate through all resource types

## Performance Characteristics

### Enum Usage Performance
- **Zero Runtime Overhead**: Enum value definition is compile-time only
- **Excellent Cache Locativity**: Single cache line access
- **No Memory Allocations**: Zero dynamic memory allocation
- **Thread-Safe**: Safe for concurrent access from multiple threads

### Type Comparison Performance
- **O(1) Comparison**: Constant time enum comparison
- **No Memory Allocations**: Zero dynamic memory allocation
- **Thread-Safe**: Safe for concurrent access from multiple threads

### Type Assignment Performance
- **O(1) Assignment**: Constant time enum assignment
- **No Memory Allocations**: Zero dynamic memory allocation
- **Thread-Safe**: Safe for concurrent access from multiple threads

### Cache Locativity
- **Excellent**: All operations fit in single cache line
- **No Cache Misses**: No cache misses for typical usage
- **Minimal Memory Footprint**: 4 bytes per enum value

## Dependencies

### Direct Dependencies
- None (no dependencies)

### Indirect Dependencies
- None (no dependencies)

## Known Issues and Limitations

1. **No Type Validation**: No validation of resource type compatibility with specific shader stages
2. **No Access Pattern Validation**: No validation of access pattern compatibility with specific resource types
3. **No Data Type Validation**: No validation of data type compatibility with specific resource types
4. **No Shader Stage Support Query**: No function to query which shader stages support which resource types
5. **No Atomic Operations Query**: No function to query atomic operations support for specific resource types
6. **No Format Support Query**: No function to query format support for specific resource types
7. **No Stride Support Query**: No function to query stride support for structured buffers
8. **No Resource Type Conversion**: No functions to convert between resource types
9. **No Resource Type Filtering**: No functions to filter resource types by criteria
10. **No Resource Type Counting**: No functions to count resource types by category

## Usage Example

```cpp
// Use resource type as function parameter
void setTextureSrv(ShaderResourceType type);
void setTextureUav(ShaderResourceType type);
void setRawBufferSrv(ShaderResourceType type);
void setRawBufferUav(ShaderResourceType type);
void setTypedBufferSrv(ShaderResourceType type);
void setTypedBufferUav(ShaderResourceType type);
void setStructuredBufferSrv(ShaderResourceType type);
void setStructuredBufferUav(ShaderResourceType type);
void setCbv(ShaderResourceType type);
void setAccelerationStructureSrv(ShaderResourceType type);
void setAccelerationStructureUav(ShaderResourceType type);
void setDsv(ShaderResourceType type);
void setRtv(ShaderResourceType type);
void setSampler(ShaderResourceType type);

// Check resource type
ShaderResourceType type = ShaderResourceType::TextureSrv;
if (type == ShaderResourceType::TextureSrv)
{
    // Type is texture SRV
}

// Iterate through all resource types
for (uint32_t i = 0; i < static_cast<uint32_t>(ShaderResourceType::Count); ++i)
{
    ShaderResourceType type = static_cast<ShaderResourceType>(i);
    // Process each resource type
}

// Use Count value
uint32_t totalTypes = static_cast<uint32_t>(ShaderResourceType::Count);
```

## Conclusion

ShaderResourceType provides a simple and efficient enum for categorizing different types of GPU resources that can be bound to shaders. The implementation is minimal with excellent cache locality and zero runtime overhead.

**Strengths**:
- Simple and minimal API
- Zero runtime overhead (enum is compile-time only)
- Excellent cache locality (fits in single cache line)
- Thread-safe (no mutable state)
- Comprehensive resource type categorization (13 types)
- Clear type names and descriptions
- Support for all shader stages (pixel, vertex, geometry, hull, domain, compute, raytracing)
- Read-only and read-write resource types
- Raw, typed, and structured buffer types
- Texture, buffer, constant buffer, and acceleration structure types
- Depth-stencil and render target types
- Sampler type
- Count value for array bounds checking

**Weaknesses**:
- No type validation for shader stage compatibility
- No access pattern validation for resource types
- No data type validation for resource types
- No shader stage support query functions
- No atomic operations query functions
- No format support query functions
- No stride support query for structured buffers
- No resource type conversion functions
- No resource type filtering functions
- No resource type counting functions by category

**Recommendations**:
1. Consider adding type validation functions for shader stage compatibility
2. Consider adding access pattern validation functions for resource types
3. Consider adding data type validation functions for resource types
4. Consider adding shader stage support query functions
5. Consider adding atomic operations query functions
6. Consider adding format support query functions
7. Consider adding stride support query functions for structured buffers
8. Consider adding resource type conversion functions
9. Consider adding resource type filtering functions
10. Consider adding resource type counting functions by category
