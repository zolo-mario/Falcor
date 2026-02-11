# GFXHelpers

## Overview
[`GFXHelpers`](Source/Falcor/Core/API/GFXHelpers.h) provides utility functions for converting between Falcor types and Slang GFX types. This module handles format conversion, resource state conversion, and resource bind flags conversion.

## Source Files
- Header: [`Source/Falcor/Core/API/GFXHelpers.h`](Source/Falcor/Core/API/GFXHelpers.h) (41 lines)
- Implementation: [`Source/Falcor/Core/API/GFXHelpers.cpp`](Source/Falcor/Core/API/GFXHelpers.cpp) (288 lines)

## Functions

### getGFXFormat
```cpp
gfx::Format getGFXFormat(ResourceFormat format);
```
**Purpose**: Convert Falcor ResourceFormat to Slang GFX Format

**Parameters**:
- `format`: Falcor ResourceFormat enum value

**Return**: Slang GFX Format enum value

**Memory Layout**: N/A (function call)

**Cache Locality**: **Excellent**
- Switch statement with direct lookup
- Single cache line access for most cases
- Excellent for repeated conversions

**Algorithmic Complexity**: **O(1)**
- Switch statement: O(1)

**Supported Formats**: 64 formats (all ResourceFormat values)

**Format Categories**:
- **Compressed Formats** (14): BC1-BC7 (DXT1-DXT5, RGTC, BPTC)
- **Unorm Formats** (13): R8Unorm, R16Unorm, RG8Unorm, RG16Unorm, RGB5A1Unorm, RGBA8Unorm, RGBA16Unorm, RGB10A2Unorm, BGRA4Unorm, BGRA8Unorm, BGRX8Unorm, R5G6B5Unorm
- **UnormSrgb Formats** (6): RGBA8UnormSrgb, BGRA8UnormSrgb, BGRX8UnormSrgb, BC1UnormSrgb, BC2UnormSrgb, BC3UnormSrgb, BC7UnormSrgb
- **Float Formats** (10): R16Float, RG16Float, RGBA16Float, R32Float, RG32Float, RGB32Float, RGBA32Float, R11G11B10Float, RGB9E5Float, BC6HS16, BC6HU16
- **Uint Formats** (12): R8Uint, R16Uint, R32Uint, RG8Uint, RG16Uint, RG32Uint, RGB32Uint, RGBA8Uint, RGBA16Uint, RGBA32Uint, RGB10A2Uint, BGRA4Unorm
- **Sint Formats** (11): R8Int, R16Int, R32Int, RG8Int, RG16Int, RG32Int, RGB32Int, RGBA8Int, RGBA16Int, RGBA32Int, RGBA8Snorm, RG16Snorm, BC4Snorm, BC5Snorm
- **BGRA Formats** (4): BGRA4Unorm, BGRA8Unorm, BGRA8UnormSrgb, BGRX8Unorm, BGRX8UnormSrgb
- **Depth-Stencil Formats** (3): D16Unorm, D32Float, D32FloatS8Uint
- **Unknown Format** (1): Unknown (returns gfx::Format::Unknown)

### getGFXResourceState (Resource::State)
```cpp
gfx::ResourceState getGFXResourceState(Resource::State state);
```
**Purpose**: Convert Falcor Resource::State to Slang GFX ResourceState

**Parameters**:
- `state`: Falcor Resource::State enum value

**Return**: Slang GFX ResourceState enum value

**Memory Layout**: N/A (function call)

**Cache Locality**: **Excellent**
- Switch statement with direct lookup
- Single cache line access for most cases
- Excellent for repeated conversions

**Algorithmic Complexity**: **O(1)**
- Switch statement: O(1)

**Supported States**: 20 states (all Resource::State values)

**State Mappings**:
- `Undefined` → `Undefined`
- `PreInitialized` → `PreInitialized`
- `Common` → `General`
- `VertexBuffer` → `VertexBuffer`
- `ConstantBuffer` → `ConstantBuffer`
- `IndexBuffer` → `IndexBuffer`
- `RenderTarget` → `RenderTarget`
- `UnorderedAccess` → `UnorderedAccess`
- `DepthStencil` → `DepthWrite`
- `ShaderResource` → `ShaderResource`
- `StreamOut` → `StreamOutput`
- `IndirectArg` → `IndirectArgument`
- `CopyDest` → `CopyDestination`
- `CopySource` → `CopySource`
- `ResolveDest` → `ResolveDestination`
- `ResolveSource` → `ResolveSource`
- `Present` → `Present`
- `GenericRead` → `General`
- `Predication` → `General`
- `PixelShader` → `PixelShaderResource`
- `NonPixelShader` → `NonPixelShaderResource`
- `AccelerationStructure` → `AccelerationStructure`

### getGFXResourceState (ResourceBindFlags)
```cpp
void getGFXResourceState(ResourceBindFlags flags, gfx::ResourceState& defaultState, gfx::ResourceStateSet& allowedStates);
```
**Purpose**: Convert Falcor ResourceBindFlags to Slang GFX resource states

**Parameters**:
- `flags`: Falcor ResourceBindFlags enum value (bitmask)
- `defaultState`: Output parameter for default resource state
- `allowedStates`: Output parameter for allowed resource states

**Return**: void (outputs via reference parameters)

**Memory Layout**: N/A (function call)

**Cache Locality**: **Good**
- Multiple if statements (up to 9 checks)
- Each check may cause a cache miss
- Good for repeated conversions with same flags

**Algorithmic Complexity**: **O(1)**
- Up to 9 if statements: O(1)

**Default State**: `gfx::ResourceState::General` (except for AccelerationStructure flag)

**Allowed States**: Always includes `CopyDestination` and `CopySource`

**Flag Mappings**:
- `UnorderedAccess` → adds `UnorderedAccess`
- `ShaderResource` → adds `ShaderResource`
- `RenderTarget` → adds `RenderTarget`
- `DepthStencil` → adds `DepthWrite`
- `Vertex` → adds `VertexBuffer`, `AccelerationStructureBuildInput`
- `Index` → adds `IndexBuffer`, `AccelerationStructureBuildInput`
- `IndirectArg` → adds `IndirectArgument`
- `Constant` → adds `ConstantBuffer`
- `AccelerationStructure` → adds `AccelerationStructure`, `ShaderResource`, `UnorderedAccess`, sets default state to `AccelerationStructure`

**Special Cases**:
- `AccelerationStructure` flag overrides default state to `AccelerationStructure`
- `CopyDestination` and `CopySource` are always added to allowed states

## Threading Model

**Thread Safety**: **Thread-Safe (No Mutable State)**

**Analysis**:
- No global or static mutable variables
- All functions are pure (no side effects)
- Safe for concurrent access from multiple threads
- No synchronization primitives needed

**Thread Safety Characteristics**:
1. **No Mutable State**: All functions are pure
2. **Pure Functions**: No side effects beyond output parameters
3. **Safe for Concurrent Access**: Multiple threads can safely call any function simultaneously
4. **No External Dependencies**: Only depends on input parameters and Slang GFX types

**Note**: The functions are designed for single-threaded or multi-threaded use. Multiple threads can safely call these functions simultaneously without any synchronization.

## Cache Locality Analysis

### Access Patterns

#### Format Conversion (getGFXFormat)
- **Switch Statement**: Direct lookup in switch statement
- **Single Cache Line**: Most cases access single cache line
- **Excellent for Repeated Conversions**: Excellent cache locality for repeated conversions to the same format

**Cache Locality**: **Excellent**
- Switch statement with direct lookup
- Single cache line access for most cases
- Excellent for repeated conversions

#### State Conversion (getGFXResourceState - Resource::State)
- **Switch Statement**: Direct lookup in switch statement
- **Single Cache Line**: Most cases access single cache line
- **Excellent for Repeated Conversions**: Excellent cache locality for repeated conversions to the same state

**Cache Locality**: **Excellent**
- Switch statement with direct lookup
- Single cache line access for most cases
- Excellent for repeated conversions

#### Bind Flags Conversion (getGFXResourceState - ResourceBindFlags)
- **Multiple If Statements**: Up to 9 if statements
- **Multiple Cache Lines**: Each if statement may cause a cache miss
- **Good for Repeated Conversions**: Good cache locality for repeated conversions with same flags

**Cache Locality**: **Good**
- Multiple if statements (up to 9 checks)
- Each check may cause a cache miss
- Good for repeated conversions with same flags

### Cache Line Analysis

#### Function Call Overhead
- **Stack Usage**: Minimal (parameters and return values)
- **Register Usage**: Minimal (parameters in registers)
- **Cache Locality**: **Excellent** - Minimal stack/register usage

#### Switch Statement
- **Jump Table**: Compiler generates jump table for switch statement
- **Direct Lookup**: O(1) direct lookup in jump table
- **Cache Locality**: **Excellent** - Jump table fits in cache

#### If Statement Chain
- **Sequential Checks**: Sequential if statement checks
- **Branch Prediction**: Branch prediction is effective for repeated conversions
- **Cache Locality**: **Good** - Sequential checks may cause cache misses

## Algorithmic Complexity Analysis

### Format Conversion

#### getGFXFormat
**Time Complexity**: **O(1)**
- Switch statement: O(1)
- Direct lookup in jump table: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 4 bytes (enum)

### State Conversion

#### getGFXResourceState (Resource::State)
**Time Complexity**: **O(1)**
- Switch statement: O(1)
- Direct lookup in jump table: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 4 bytes (enum)

#### getGFXResourceState (ResourceBindFlags)
**Time Complexity**: **O(1)**
- Up to 9 if statements: O(1)
- Each if statement: O(1)
- ResourceStateSet operations: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Output parameters: reference to existing objects

## Key Technical Features

### Format Conversion
- **64 Formats**: Complete support for all ResourceFormat values
- **Direct Mapping**: Direct switch statement mapping
- **Cross-Platform**: Works with Slang GFX API
- **Zero Allocation**: No dynamic memory allocation
- **Fast**: O(1) direct lookup

### Resource State Conversion
- **20 States**: Complete support for all Resource::State values
- **Direct Mapping**: Direct switch statement mapping
- **Cross-Platform**: Works with Slang GFX API
- **Zero Allocation**: No dynamic memory allocation
- **Fast**: O(1) direct lookup

### Resource Bind Flags Conversion
- **9 Flags**: Support for all ResourceBindFlags values
- **Default State**: Automatic default state calculation
- **Allowed States**: Automatic allowed states calculation
- **Cross-Platform**: Works with Slang GFX API
- **Zero Allocation**: No dynamic memory allocation
- **Fast**: O(1) if statement chain

### Special Cases
- **Acceleration Structure**: Special handling for acceleration structure bind flag (overrides default state)
- **Copy States**: Always includes copy destination and copy source states
- **Vertex/Index Buffers**: Includes acceleration structure build input state

## Performance Characteristics

### Format Conversion Performance
- **O(1) Direct Lookup**: Switch statement with jump table
- **Excellent Cache Locality**: Single cache line access for most cases
- **No Memory Allocations**: Zero dynamic memory allocation
- **Thread-Safe**: Safe for concurrent access from multiple threads

### State Conversion Performance
- **O(1) Direct Lookup**: Switch statement with jump table
- **Excellent Cache Locality**: Single cache line access for most cases
- **No Memory Allocations**: Zero dynamic memory allocation
- **Thread-Safe**: Safe for concurrent access from multiple threads

### Bind Flags Conversion Performance
- **O(1) If Statement Chain**: Up to 9 if statements
- **Good Cache Locality**: Sequential checks may cause cache misses
- **No Memory Allocations**: Zero dynamic memory allocation
- **Thread-Safe**: Safe for concurrent access from multiple threads

### Overall Performance
- **Fast**: All functions are O(1)
- **Efficient**: Minimal overhead (switch/if statements)
- **Cache-Friendly**: Excellent cache locality for format and state conversions
- **Thread-Safe**: Safe for concurrent access

## Dependencies

### Direct Dependencies
- [`Formats.h`](Source/Falcor/Core/API/Formats.h) - Format types (ResourceFormat, ResourceBindFlags)
- [`Resource.h`](Source/Falcor/Core/API/Resource.h) - Resource types (Resource::State)
- [`slang-gfx.h`](Source/Falcor/Core/API/slang-gfx.h) - Slang GFX types (gfx::Format, gfx::ResourceState, gfx::ResourceStateSet)
- [`Device.h`](Source/Falcor/Core/API/Device.h) - Device type (forward declaration)
- [`GFXAPI.h`](Source/Falcor/Core/API/GFXAPI.h) - GFX API macros

### Indirect Dependencies
- Slang GFX types (from slang-gfx.h)
- Resource types (from Resource.h)
- Format types (from Formats.h)
- Device types (from Device.h)

## Known Issues and Limitations

1. **No Reverse Conversion**: No functions to convert from GFX to Falcor types
2. **No Format Validation**: No validation of format compatibility with specific GPU features
3. **No State Validation**: No validation of state compatibility with specific resources
4. **No Bind Flags Validation**: No validation of bind flags compatibility with specific resources
5. **No Format Support Query**: No function to query which formats are supported by the current GPU
6. **No State Support Query**: No function to query which states are supported by the current GPU
7. **No Bind Flags Support Query**: No function to query which bind flags are supported by the current GPU
8. **No Format Conversion**: No pixel format conversion functions (e.g., RGB to BGR)
9. **No State Conversion**: No resource state transition functions
10. **No Bind Flags Conversion**: No bind flags combination functions

## Usage Example

```cpp
// Convert Falcor format to GFX format
ResourceFormat falcorFormat = ResourceFormat::RGBA8Unorm;
gfx::Format gfxFormat = getGFXFormat(falcorFormat); // gfx::Format::R8G8B8A8_UNORM

// Convert Falcor resource state to GFX resource state
Resource::State falcorState = Resource::State::ShaderResource;
gfx::ResourceState gfxState = getGFXResourceState(falcorState); // gfx::ResourceState::ShaderResource

// Convert Falcor resource bind flags to GFX resource states
ResourceBindFlags falcorFlags = ResourceBindFlags::ShaderResource | ResourceBindFlags::RenderTarget;
gfx::ResourceState defaultState;
gfx::ResourceStateSet allowedStates;
getGFXResourceState(falcorFlags, defaultState, allowedStates);
// defaultState: gfx::ResourceState::General
// allowedStates: {General, ShaderResource, RenderTarget, CopyDestination, CopySource}

// Convert acceleration structure bind flags
ResourceBindFlags asFlags = ResourceBindFlags::AccelerationStructure;
getGFXResourceState(asFlags, defaultState, allowedStates);
// defaultState: gfx::ResourceState::AccelerationStructure
// allowedStates: {AccelerationStructure, ShaderResource, UnorderedAccess, CopyDestination, CopySource}
```

## Conclusion

GFXHelpers provides a clean and efficient conversion utility between Falcor types and Slang GFX types. The implementation is simple with excellent cache locality and zero runtime overhead for format and state conversions.

**Strengths**:
- Clean and simple API for type conversion
- Complete support for all Falcor types (64 formats, 20 states, 9 bind flags)
- O(1) direct lookup for format and state conversions
- Excellent cache locality for format and state conversions
- Good cache locality for bind flags conversion
- Zero dynamic memory allocation
- Thread-safe (no mutable state)
- Cross-platform support (Slang GFX API)
- Special handling for acceleration structure bind flag
- Always includes copy states in allowed states

**Weaknesses**:
- No reverse conversion functions (GFX to Falcor)
- No format validation
- No state validation
- No bind flags validation
- No format support query
- No state support query
- No bind flags support query
- No format conversion (pixel format conversion)
- No state transition functions
- No bind flags combination functions

**Recommendations**:
1. Consider adding reverse conversion functions (GFX to Falcor)
2. Consider adding format validation functions
3. Consider adding state validation functions
4. Consider adding bind flags validation functions
5. Consider adding format support query functions
6. Consider adding state support query functions
7. Consider adding bind flags support query functions
8. Consider adding pixel format conversion functions
9. Consider adding resource state transition functions
10. Consider adding bind flags combination functions
