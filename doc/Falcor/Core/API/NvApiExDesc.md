# NvApiExDesc

## Overview
[`NvApiExDesc`](Source/Falcor/Core/API/NvApiExDesc.h) provides NVIDIA API extension descriptor structures and helper functions for D3D12 pipeline state extensions. This module enables NVIDIA-specific shader extensions and UAV slot configuration.

## Source Files
- Header: [`Source/Falcor/Core/API/NvApiExDesc.h`](Source/Falcor/Core/API/NvApiExDesc.h) (124 lines)
- Implementation: Header-only (no separate .cpp file)

## Type Definitions

### NvApiPsoExDesc Struct
```cpp
#if FALCOR_HAS_NVAPI
struct NvApiPsoExDesc
{
    NV_PSO_EXTENSION psoExtension;
    NVAPI_D3D12_PSO_VERTEX_SHADER_DESC mVsExDesc;
    NVAPI_D3D12_PSO_HULL_SHADER_DESC mHsExDesc;
    NVAPI_D3D12_PSO_DOMAIN_SHADER_DESC mDsExDesc;
    NVAPI_D3D12_PSO_GEOMETRY_SHADER_DESC mGsExDesc;
    NVAPI_D3D12_PSO_SET_SHADER_EXTENSION_SLOT_DESC mExtSlotDesc;
    std::vector<NV_CUSTOM_SEMANTIC> mCustomSemantics;
};
#else
using NvApiPsoExDesc = uint32_t;
#endif
```
**Purpose**: NVIDIA API pipeline state extension descriptor

**Memory Layout**: ~48-72 bytes (excluding vector allocation)

**Member Layout**:
- `psoExtension`: 4 bytes (offset 0)
- `mVsExDesc`: ~8-12 bytes (offset 4)
- `mHsExDesc`: ~8-12 bytes (offset 12-16)
- `mDsExDesc`: ~8-12 bytes (offset 20-28)
- `mGsExDesc`: ~8-12 bytes (offset 28-40)
- `mExtSlotDesc`: ~8-12 bytes (offset 40-52)
- `mCustomSemantics`: ~8-16 bytes (offset 52-64, excluding vector allocation)

**Cache Locativity**: **Poor** - Members scattered across multiple cache lines

**Alignment**: Natural alignment (4-byte alignment for most members)

**Usage**: Describes NVIDIA-specific pipeline state extensions

**Platform**: Only available with FALCOR_HAS_NVAPI (Windows with NVIDIA GPU)

**Dynamic Memory**: 
- `mCustomSemantics`: Vector of NV_CUSTOM_SEMANTIC (dynamic allocation)

## Functions

### createNvApiVsExDesc
```cpp
#if FALCOR_HAS_NVAPI
inline void createNvApiVsExDesc(NvApiPsoExDesc& ret)
{
    ret.psoExtension = NV_PSO_VERTEX_SHADER_EXTENSION;

    auto& desc = ret.mVsExDesc;
    std::memset(&desc, 0, sizeof(desc));

    desc.psoExtension = NV_PSO_VERTEX_SHADER_EXTENSION;
    desc.version = NV_VERTEX_SHADER_PSO_EXTENSION_DESC_VER;
    desc.baseVersion = NV_PSO_EXTENSION_DESC_VER;
    desc.NumCustomSemantics = 2;

    ret.mCustomSemantics.resize(2);

    // desc.pCustomSemantics = (NV_CUSTOM_SEMANTIC *)malloc(2 * sizeof(NV_CUSTOM_SEMANTIC));
    // memset(ret.mCustomSemantics.data(), 0, (2 * sizeof(NV_CUSTOM_SEMANTIC)));

    ret.mCustomSemantics[0].version = NV_CUSTOM_SEMANTIC_VERSION;
    ret.mCustomSemantics[0].NVCustomSemanticType = NV_X_RIGHT_SEMANTIC;
    strcpy_s(&(ret.mCustomSemantics[0].NVCustomSemanticNameString[0]), NVAPI_LONG_STRING_MAX, "NV_X_RIGHT");

    ret.mCustomSemantics[1].version = NV_CUSTOM_SEMANTIC_VERSION;
    ret.mCustomSemantics[1].NVCustomSemanticType = NV_VIEWPORT_MASK_SEMANTIC;
    strcpy_s(&(ret.mCustomSemantics[1].NVCustomSemanticNameString[0]), NVAPI_LONG_STRING_MAX, "NV_VIEWPORT_MASK");

    desc.pCustomSemantics = ret.mCustomSemantics.data();
}
#else
inline void createNvApiVsExDesc(NvApiPsoExDesc& ret)
{
    FALCOR_UNREACHABLE();
}
#endif
```
**Purpose**: Create NVIDIA API vertex shader extension descriptor

**Parameters**:
- `ret`: Reference to NvApiPsoExDesc to initialize

**Return**: void (outputs via reference parameter)

**Memory Layout**: N/A (function call)

**Cache Locativity**: **Good**
- Member access: O(1)
- String copy: O(1) (fixed length)
- Vector resize: O(1) (fixed size)

**Algorithmic Complexity**: **O(1)**
- Member access: O(1)
- Memory initialization: O(1)
- String copy: O(1) (fixed length)
- Vector resize: O(1) (fixed size)

**Custom Semantics**:
- `NV_X_RIGHT_SEMANTIC`: Custom semantic for X-right viewport
- `NV_VIEWPORT_MASK_SEMANTIC`: Custom semantic for viewport mask

**Platform**: Only available with FALCOR_HAS_NVAPI

### createNvApiUavSlotExDesc
```cpp
#if FALCOR_HAS_NVAPI
inline void createNvApiUavSlotExDesc(NvApiPsoExDesc& ret, uint32_t uavSlot)
{
    ret.psoExtension = NV_PSO_SET_SHADER_EXTNENSION_SLOT_AND_SPACE;

    auto& desc = ret.mExtSlotDesc;
    std::memset(&desc, 0, sizeof(desc));

    desc.psoExtension = NV_PSO_SET_SHADER_EXTNENSION_SLOT_AND_SPACE;
    desc.version = NV_SET_SHADER_EXTENSION_SLOT_DESC_VER;
    desc.baseVersion = NV_PSO_EXTENSION_DESC_VER;
    desc.uavSlot = uavSlot;
    desc.registerSpace = 0; // SM5.1+: If "space" keyword is omitted, default space index of 0 is implicitly assigned
}
#else
inline void createNvApiUavSlotExDesc(NvApiPsoExDesc& ret, uint32_t uavSlot)
{
    FALCOR_UNREACHABLE();
}
#endif
```
**Purpose**: Create NVIDIA API UAV slot extension descriptor

**Parameters**:
- `ret`: Reference to NvApiPsoExDesc to initialize
- `uavSlot`: UAV slot index

**Return**: void (outputs via reference parameter)

**Memory Layout**: N/A (function call)

**Cache Locativity**: **Excellent**
- Member access: O(1)
- Memory initialization: O(1)

**Algorithmic Complexity**: **O(1)**
- Member access: O(1)
- Memory initialization: O(1)

**Register Space**: Default space index of 0 (SM5.1+)

**Platform**: Only available with FALCOR_HAS_NVAPI

### findNvApiShaderRegister
```cpp
#if FALCOR_HAS_NVAPI
inline std::optional<uint32_t> findNvApiShaderRegister(const ref<const ProgramKernels>& pKernels)
{
    auto pBlock = pKernels->getReflector()->getDefaultParameterBlock();
    auto pVar = pBlock->getResource("g_NvidiaExt");
    if (!pVar)
        return std::optional<uint32_t>();

    auto rangeInfo = pBlock->getResourceRangeBindingInfo(pVar->getBindLocation().getResourceRangeIndex());
    return std::optional<uint32_t>(rangeInfo.regIndex);
}
#else
inline std::optional<uint32_t> findNvApiShaderRegister(const ref<const ProgramKernels>& pKernels, uint32_t& outRegisterIndex)
{
    return std::optional<uint32_t>();
}
#endif
```
**Purpose**: Find NVIDIA API shader register index from program kernels

**Parameters**:
- `pKernels`: Reference to ProgramKernels

**Return**: std::optional<uint32_t> - Shader register index if found, empty otherwise

**Memory Layout**: N/A (function call)

**Cache Locativity**: **Poor**
- Multiple pointer dereferences: O(1)
- Resource lookup: O(1)
- String comparison: O(1)

**Algorithmic Complexity**: **O(1)**
- Get reflector: O(1)
- Get default parameter block: O(1)
- Get resource: O(1)
- Get resource range binding info: O(1)

**Resource Name**: `g_NvidiaExt` - NVIDIA extension global resource

**Platform**: Only available with FALCOR_HAS_NVAPI

## Dependencies

### Direct Dependencies
- [`Core/Macros.h`](Source/Falcor/Core/Macros.h) - Macros (FALCOR_HAS_NVAPI, FALCOR_UNREACHABLE)
- [`Core/Error.h`](Source/Falcor/Core/Error.h) - Error handling (FALCOR_ASSERT)
- [`Core/Program/ProgramVersion.h`](Source/Falcor/Core/Program/ProgramVersion.h) - Program version types
- [`d3d12.h`](Source/Falcor/Core/API/d3d12.h) - D3D12 types (conditional)
- [`dxgi1_4.h`](Source/Falcor/Core/API/dxgi1_4.h) - DXGI types (conditional)
- [`nvapi.h`](Source/Falcor/Core/API/nvapi.h) - NVIDIA API types (conditional)
- [`optional`](Source/Falcor/Core/API/optional) - Optional type (std::optional)
- [`string`](Source/Falcor/Core/API/string) - String type (std::string)
- [`vector`](Source/Falcor/Core/API/vector) - Vector type (std::vector)

### Indirect Dependencies
- NVIDIA API types (from nvapi.h)
- D3D12 types (from d3d12.h)
- DXGI types (from dxgi1_4.h)
- Program types (from Program/ProgramVersion.h)
- Standard C++ types (std::optional, std::string, std::vector)

## Threading Model

**Thread Safety**: **Thread-Safe for Concurrent Reads, Not Thread-Safe for Concurrent Writes**

**Analysis**:
- No global or static mutable variables
- Functions modify NvApiPsoExDesc (mutable state)
- No synchronization primitives
- Safe for concurrent reads from multiple threads
- Not safe for concurrent writes to the same NvApiPsoExDesc instance

**Thread Safety Characteristics**:
1. **Mutable State**: Functions modify NvApiPsoExDesc (mutable state)
2. **No Synchronization**: No mutex or atomic operations
3. **Safe for Concurrent Reads**: Multiple threads can safely read from same NvApiPsoExDesc instance
4. **Unsafe for Concurrent Writes**: Multiple threads writing to same NvApiPsoExDesc instance causes race conditions
5. **No External Dependencies**: Only depends on input parameters and NVIDIA API types

**Note**: The functions are designed for single-threaded use. For multi-threaded scenarios with concurrent writes, external synchronization is required.

## Cache Locality Analysis

### Access Patterns

#### createNvApiVsExDesc
- **Member Access**: Access to NvApiPsoExDesc members
- **Multiple Cache Lines**: Members scattered across multiple cache lines
- **Poor Cache Locality**: Poor cache locality for repeated access

**Cache Locativity**: **Poor**
- Members scattered across multiple cache lines
- Multiple cache line accesses
- Poor for repeated access

#### createNvApiUavSlotExDesc
- **Member Access**: Access to NvApiPsoExDesc members
- **Multiple Cache Lines**: Members scattered across multiple cache lines
- **Poor Cache Locality**: Poor cache locality for repeated access

**Cache Locativity**: **Poor**
- Members scattered across multiple cache lines
- Multiple cache line accesses
- Poor for repeated access

#### findNvApiShaderRegister
- **Pointer Dereferences**: Multiple pointer dereferences
- **String Comparison**: String comparison for resource name
- **Poor Cache Locality**: Poor cache locality for repeated access

**Cache Locativity**: **Poor**
- Multiple pointer dereferences
- String comparison
- Poor for repeated access

### Cache Line Analysis

#### NvApiPsoExDesc Struct
- **Size**: ~48-72 bytes (excluding vector allocation)
- **Cache Lines**: ~1-2 cache lines (64 bytes each)
- **Access Pattern**: Member access scattered across cache lines
- **Cache Locativity**: **Poor**

## Algorithmic Complexity Analysis

### Initialization Functions

#### createNvApiVsExDesc
**Time Complexity**: **O(1)**
- Member access: O(1)
- Memory initialization: O(1)
- String copy: O(1) (fixed length)
- Vector resize: O(1) (fixed size)

**Space Complexity**: **O(1)**
- Dynamic memory allocation: 2 × NV_CUSTOM_SEMANTIC (fixed size)

#### createNvApiUavSlotExDesc
**Time Complexity**: **O(1)**
- Member access: O(1)
- Memory initialization: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation

### Query Function

#### findNvApiShaderRegister
**Time Complexity**: **O(1)**
- Get reflector: O(1)
- Get default parameter block: O(1)
- Get resource: O(1)
- Get resource range binding info: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 4 bytes (optional<uint32_t>)

## Key Technical Features

### NVIDIA Pipeline State Extensions
- **NvApiPsoExDesc**: NVIDIA API pipeline state extension descriptor
- **Vertex Shader Extension**: NV_PSO_VERTEX_SHADER_EXTENSION
- **Hull Shader Extension**: NVAPI_D3D12_PSO_HULL_SHADER_DESC
- **Domain Shader Extension**: NVAPI_D3D12_PSO_DOMAIN_SHADER_DESC
- **Geometry Shader Extension**: NVAPI_D3D12_PSO_GEOMETRY_SHADER_DESC
- **Set Shader Extension**: NV_PSO_SET_SHADER_EXTNENSION_SLOT_AND_SPACE
- **Custom Semantics**: Support for custom shader semantics

### Custom Semantics
- **NV_X_RIGHT_SEMANTIC**: Custom semantic for X-right viewport
- **NV_VIEWPORT_MASK_SEMANTIC**: Custom semantic for viewport mask
- **Two Semantics**: Fixed set of 2 custom semantics

### UAV Slot Configuration
- **UAV Slot**: Configurable UAV slot index
- **Register Space**: Default space index of 0 (SM5.1+)
- **Space Keyword**: If "space" keyword is omitted, default space index of 0 is implicitly assigned

### Shader Register Query
- **Resource Name**: `g_NvidiaExt` - NVIDIA extension global resource
- **Register Index**: Returns shader register index for NVIDIA extension
- **Optional Return**: Returns empty optional if resource not found

### Conditional Compilation
- **FALCOR_HAS_NVAPI**: Only compiled with NVIDIA API support
- **Platform-Specific**: Only available on Windows with NVIDIA GPU
- **Fallback Type**: uint32_t when FALCOR_HAS_NVAPI is not defined

### Memory Management
- **Dynamic Allocation**: Vector allocation for custom semantics
- **Fixed Size**: Custom semantics vector is fixed size (2)
- **Memory Initialization**: Zero initialization of descriptors

## Performance Characteristics

### Initialization Performance
- **O(1) Operations**: All initialization functions are O(1)
- **Memory Initialization**: Zero initialization of descriptors
- **String Copy**: Fixed-length string copy (O(1))
- **Vector Resize**: Fixed-size vector resize (O(1))
- **No Memory Allocations**: Minimal dynamic memory allocation (fixed-size vector)

### Query Performance
- **O(1) Operations**: Query function is O(1)
- **Pointer Dereferences**: Multiple pointer dereferences
- **String Comparison**: String comparison for resource name
- **No Memory Allocations**: Zero dynamic memory allocation

### Cache Locality
- **Poor**: NvApiPsoExDesc members scattered across multiple cache lines
- **Multiple Cache Lines**: Member access may cause multiple cache misses
- **Poor for Repeated Access**: Poor cache locality for repeated access

### Platform-Specific Performance
- **NVIDIA GPU Only**: Only available on Windows with NVIDIA GPU
- **Conditional Compilation**: Platform-specific code is conditionally compiled
- **No Overhead**: Zero overhead when FALCOR_HAS_NVAPI is not defined

## Known Issues and Limitations

1. **No Thread Safety for Writes**: Not thread-safe for concurrent writes to the same NvApiPsoExDesc instance
2. **No Value Validation**: No validation of UAV slot values
3. **No Resource Validation**: No validation of resource names
4. **No Error Handling**: No error handling for NVIDIA API failures
5. **No Hs/Ds/Gs Functions**: TODO comment indicates missing functions for hull, domain, and geometry shaders
6. **No Dynamic Semantics**: Custom semantics are fixed (NV_X_RIGHT_SEMANTIC, NV_VIEWPORT_MASK_SEMANTIC)
7. **No Semantic Validation**: No validation of custom semantic names
8. **No Semantic Type Validation**: No validation of custom semantic types
9. **No Register Space Validation**: No validation of register space values
10. **No Platform Abstraction**: No abstraction for non-NVIDIA GPUs

## Usage Example

```cpp
#if FALCOR_HAS_NVAPI
// Create NVIDIA API vertex shader extension descriptor
NvApiPsoExDesc nvApiDesc;
createNvApiVsExDesc(nvApiDesc);
// nvApiDesc now contains:
// - psoExtension: NV_PSO_VERTEX_SHADER_EXTENSION
// - mVsExDesc: Initialized vertex shader extension descriptor
// - mCustomSemantics: Two custom semantics (NV_X_RIGHT_SEMANTIC, NV_VIEWPORT_MASK_SEMANTIC)

// Create NVIDIA API UAV slot extension descriptor
NvApiPsoExDesc nvApiDesc;
uint32_t uavSlot = 0;
createNvApiUavSlotExDesc(nvApiDesc, uavSlot);
// nvApiDesc now contains:
// - psoExtension: NV_PSO_SET_SHADER_EXTNENSION_SLOT_AND_SPACE
// - mExtSlotDesc: Initialized UAV slot extension descriptor
// - uavSlot: 0
// - registerSpace: 0

// Find NVIDIA API shader register
ref<const ProgramKernels> pKernels = /* ... */;
auto registerIndex = findNvApiShaderRegister(pKernels);
if (registerIndex.has_value())
{
    // Register index found
    uint32_t regIndex = registerIndex.value();
}
else
{
    // Register index not found
}
#endif
```

## Conclusion

NvApiExDesc provides NVIDIA-specific pipeline state extension support for D3D12. The implementation is simple with good cache locality for initialization functions, but the struct has poor cache locality due to scattered member layout.

**Strengths**:
- Clean and simple API for NVIDIA extensions
- O(1) initialization functions
- O(1) query function
- Support for custom shader semantics
- UAV slot configuration
- Shader register query
- Conditional compilation for platform support
- Fixed-size vector allocation (no dynamic resizing)
- Zero memory initialization
- Type-safe API

**Weaknesses**:
- No thread safety for concurrent writes
- No value validation
- No resource validation
- No error handling for NVIDIA API failures
- Missing functions for Hs/Ds/Gs shaders (TODO comment)
- No dynamic custom semantics (fixed set of 2)
- No semantic validation
- No semantic type validation
- No register space validation
- No platform abstraction for non-NVIDIA GPUs
- Poor cache locality for NvApiPsoExDesc struct
- Platform-specific (only available on Windows with NVIDIA GPU)

**Recommendations**:
1. Consider adding thread safety for concurrent writes using atomic operations
2. Consider adding value validation functions
3. Consider adding resource validation functions
4. Consider adding error handling for NVIDIA API failures
5. Consider implementing missing Hs/Ds/Gs functions (TODO comment)
6. Consider supporting dynamic custom semantics
7. Consider adding semantic validation
8. Consider adding semantic type validation
9. Consider adding register space validation
10. Consider adding platform abstraction for non-NVIDIA GPUs
