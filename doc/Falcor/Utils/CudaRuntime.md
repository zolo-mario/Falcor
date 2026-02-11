# CudaRuntime - CUDA Runtime Integration

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- CUDA Runtime API (<cuda_runtime.h>)

### Dependent Modules

- Falcor/Utils/CudaUtils
- Falcor/DiffRendering (PyTorch integration)
- Falcor/Rendering (CUDA-accelerated rendering)

## Module Overview

CudaRuntime provides a compatibility layer for integrating NVIDIA CUDA runtime with Falcor. The primary purpose is to resolve naming conflicts between CUDA's vector types (int1, int2, int3, int4, uint1, uint2, uint3, uint4, float1, float2, float3, float4) and Falcor's math vector types. This header should be included instead of <cuda_runtime.h> in all Falcor code to ensure proper type isolation.

## Component Specifications

### Type Renaming Macros

**File**: `Source/Falcor/Utils/CudaRuntime.h` (63 lines)

The header uses preprocessor macros to rename CUDA vector types before including the CUDA runtime header, then undefines them afterward.

#### Type Definitions

**Integer Types**:
```cpp
#define int1 cuda_int1
#define int2 cuda_int2
#define int3 cuda_int3
#define int4 cuda_int4
```

**Unsigned Integer Types**:
```cpp
#define uint1 cuda_uint1
#define uint2 cuda_uint2
#define uint3 cuda_uint3
#define uint4 cuda_uint4
```

**Floating-Point Types**:
```cpp
#define float1 cuda_float1
#define float2 cuda_float2
#define float3 cuda_float3
#define float4 cuda_float4
```

#### Macro Sequence

The header follows this sequence:

1. **Define Macros** (lines 37-48): Rename CUDA types to cuda_* variants
2. **Include CUDA** (line 50): Include <cuda_runtime.h> with renamed types
3. **Undefine Macros** (lines 52-63): Clean up macro definitions

This ensures:
- CUDA types are renamed before inclusion
- CUDA runtime sees the renamed types
- Macros are cleaned up after inclusion
- No macro pollution of global namespace

## Technical Details

### Naming Conflict Problem

CUDA runtime defines vector types in the global namespace:
- `int1`, `int2`, `int3`, `int4`
- `uint1`, `uint2`, `uint3`, `uint4`
- `float1`, `float2`, `float3`, `float4`

Falcor defines similar vector types in:
- `Falcor::math` namespace (e.g., `math::int2`, `math::float3`)
- `Falcor` namespace (e.g., `Falcor::int2`, `Falcor::float3`)

Without renaming, including <cuda_runtime.h> would cause:
- Compiler errors due to type redefinition
- Ambiguous type resolution
- Linker conflicts

### Macro-Based Renaming Strategy

The solution uses preprocessor macros to rename CUDA types:

```cpp
// Before including CUDA
#define int2 cuda_int2

// Include CUDA (types are now cuda_int2)
#include <cuda_runtime.h>

// Clean up macros
#undef int2
```

This approach:
- Renames types at preprocessing time
- Has zero runtime overhead
- Preserves CUDA functionality
- Maintains type safety

### Type Mapping

The following type mappings are applied:

| Original CUDA Type | Renamed Type | Falcor Equivalent |
|-------------------|--------------|-------------------|
| int1 | cuda_int1 | math::int1 |
| int2 | cuda_int2 | math::int2 |
| int3 | cuda_int3 | math::int3 |
| int4 | cuda_int4 | math::int4 |
| uint1 | cuda_uint1 | math::uint1 |
| uint2 | cuda_uint2 | math::uint2 |
| uint3 | cuda_uint3 | math::uint3 |
| uint4 | cuda_uint4 | math::uint4 |
| float1 | cuda_float1 | math::float1 |
| float2 | cuda_float2 | math::float2 |
| float3 | cuda_float3 | math::float3 |
| float4 | cuda_float4 | math::float4 |

### Macro Cleanup

After including <cuda_runtime.h>, all macros are undefined:

```cpp
#undef int1
#undef int2
#undef int3
#undef int4
#undef uint1
#undef uint2
#undef uint3
#undef uint4
#undef float1
#undef float2
#undef float3
#undef float4
```

This ensures:
- No macro pollution of global namespace
- Clean compilation environment
- No interference with other headers
- Predictable behavior

## Integration Points

### CUDA Runtime Integration

- **<cuda_runtime.h>**: Core CUDA runtime API
- **CUDA Types**: Vector types renamed to cuda_* namespace
- **CUDA Functions**: All CUDA runtime functions available

### Falcor Math Integration

- **Falcor::math**: Math namespace with vector types
- **Falcor::int2**: Falcor's 2D integer vector type
- **Falcor::float3**: Falcor's 3D float vector type
- **Type Isolation**: Prevents conflicts with CUDA types

### Internal Falcor Usage

- **CudaUtils**: Higher-level CUDA utilities
- **DiffRendering**: PyTorch/CUDA integration
- **Rendering**: CUDA-accelerated rendering passes

## Architecture Patterns

### Preprocessor Wrapper Pattern

The header implements the preprocessor wrapper pattern:

```cpp
// Define renaming macros
#define TYPE renamed_TYPE

// Include external header
#include <external.h>

// Clean up macros
#undef TYPE
```

This pattern:
- Isolates external header effects
- Prevents namespace pollution
- Maintains clean compilation environment

### Include Guard Pattern

Although not explicitly shown in the file, the header should use include guards:

```cpp
#pragma once  // Modern include guard
```

This prevents:
- Multiple inclusion
- Redefinition errors
- Circular dependencies

### Header-Only Pattern

The header is purely header-only:
- No .cpp implementation file
- No compilation required
- No linking needed
- Zero runtime overhead

## Code Patterns

### Macro Definition Pattern

Consistent macro naming:

```cpp
#define int1 cuda_int1
#define int2 cuda_int2
#define int3 cuda_int3
#define int4 cuda_int4
```

Pattern:
- Original type name as macro name
- cuda_ prefix for renamed type
- Consistent naming across all types

### Macro Cleanup Pattern

Systematic macro cleanup:

```cpp
#undef int1
#undef int2
#undef int3
#undef int4
```

Pattern:
- Same order as definition
- All macros undefined
- No leftover macros

### Comment Documentation Pattern

Clear documentation of purpose:

```cpp
/**
 * CUDA runtime defines vector types in the global namespace. Some of these
 * types clash with the vector types in Falcor, which live in the Falcor::math
 * and Falcor namespace. To avoid this clash, we rename the CUDA types here.
 * Falcor code should includle this header instead of <cuda_runtime.h>.
 */
```

Pattern:
- Explains problem (CUDA types in global namespace)
- Explains solution (rename CUDA types)
- Provides usage guidance (include this header)

## Use Cases

### Including CUDA Runtime

**Correct Usage**:
```cpp
#include "Utils/CudaRuntime.h"  // Include this header

// Now use CUDA runtime functions
cudaMalloc(&ptr, size);
cudaMemcpy(dst, src, size, cudaMemcpyHostToDevice);
```

**Incorrect Usage**:
```cpp
#include <cuda_runtime.h>  // Don't include directly - causes conflicts!
```

### Using CUDA Types

**CUDA Types** (renamed):
```cpp
cuda_int2 a;     // CUDA's int2 type
cuda_float3 b;   // CUDA's float3 type
```

**Falcor Types** (original):
```cpp
math::int2 a;    // Falcor's int2 type
math::float3 b;  // Falcor's float3 type
```

### Mixed CUDA/Falcor Code

```cpp
#include "Utils/CudaRuntime.h"
#include "Utils/Math/Vector.h"

void process() {
    // Falcor types for CPU processing
    math::float3 cpuPos = {1.0f, 2.0f, 3.0f};
    
    // CUDA types for GPU processing
    cuda_float3* gpuPos;
    cudaMalloc(&gpuPos, sizeof(cuda_float3));
    
    // Copy between CPU and GPU
    cudaMemcpy(gpuPos, &cpuPos, sizeof(cuda_float3), cudaMemcpyHostToDevice);
}
```

### CUDA Kernel Development

```cpp
#include "Utils/CudaRuntime.h"

__global__ void kernel(cuda_float3* positions, int count) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < count) {
        cuda_float3 pos = positions[idx];
        // Process position
    }
}
```

## Performance Considerations

### Zero Runtime Overhead

- **Preprocessor Only**: All work done at preprocessing time
- **No Runtime Cost**: No function calls or overhead
- **No Memory Allocation**: No dynamic memory usage
- **No Compilation Overhead**: Minimal preprocessing time

### Compilation Impact

- **Header Size**: Small header (63 lines)
- **Fast Compilation**: Minimal preprocessing required
- **No Dependencies**: Only depends on CUDA runtime
- **Cache Friendly**: Compiler can cache preprocessed output

### Linking Impact

- **No Object Files**: Header-only, no compilation
- **No Linking Required**: Direct inclusion
- **No Symbol Conflicts**: Proper namespace isolation
- **Clean Binary**: No extra symbols

### Memory Impact

- **No Runtime Memory**: Zero runtime memory usage
- **No Static Data**: No global variables
- **No Code Generation**: No code generated
- **Minimal Binary Size**: No impact on binary size

## Limitations

### Type Coverage

- **Limited Types**: Only renames 12 vector types
- **No Other Types**: Doesn't handle other CUDA types
- **No Structs**: Doesn't handle CUDA structs
- **No Enums**: Doesn't handle CUDA enums

### Macro Limitations

- **Global Scope**: Macros affect entire translation unit
- **Order Dependency**: Must be included before other CUDA headers
- **No Type Safety**: Macros don't provide type checking
- **Debug Difficulty**: Macros can make debugging harder

### Namespace Limitations

- **cuda_* Prefix**: Requires using cuda_* prefix for CUDA types
- **No Namespace**: CUDA types still in global namespace (just renamed)
- **Potential Conflicts**: cuda_* prefix could still conflict
- **No Isolation**: Doesn't provide true namespace isolation

### Platform Limitations

- **CUDA Required**: Only works with CUDA installed
- **NVIDIA Only**: Only works on NVIDIA GPUs
- **Windows/Linux**: May have platform-specific issues
- **Compiler Dependent**: May have compiler-specific behavior

### API Limitations

- **No Wrappers**: Doesn't provide CUDA function wrappers
- **No Error Handling**: Doesn't add error handling
- **No RAII**: Doesn't provide RAII wrappers
- **No High-Level Abstractions**: Doesn't provide high-level abstractions

## Best Practices

### When to Use

**Always Use CudaRuntime.h**:
- When including CUDA runtime in Falcor code
- When mixing CUDA and Falcor types
- When writing CUDA kernels
- When using CUDA APIs

**Never Include cuda_runtime.h Directly**:
- In Falcor source files
- In Falcor headers
- In code that uses Falcor types

### Usage Patterns

**Include Order**:
```cpp
// Include CudaRuntime.h first
#include "Utils/CudaRuntime.h"

// Then include other headers
#include "Utils/Math/Vector.h"
#include "Core/API/Device.h"
```

**Type Usage**:
```cpp
// Use cuda_* types for CUDA code
cuda_float3 gpuPos;

// Use math::* types for Falcor code
math::float3 cpuPos;
```

**Error Handling**:
```cpp
#include "Utils/CudaRuntime.h"

void checkCudaError(cudaError_t err) {
    if (err != cudaSuccess) {
        // Handle error
    }
}

cudaError_t err = cudaMalloc(&ptr, size);
checkCudaError(err);
```

### Integration Guidelines

- **Include First**: Always include CudaRuntime.h before other CUDA headers
- **Use cuda_* Prefix**: Always use cuda_* prefix for CUDA types
- **Avoid Mixing**: Don't mix CUDA and Falcor types in same expression
- **Document Usage**: Document when CUDA is used in Falcor code

### Error Handling

- **CUDA Errors**: Check CUDA API return values
- **Type Errors**: Ensure correct type usage (cuda_* vs math::*)
- **Include Errors**: Ensure correct include order
- **Compilation Errors**: Check for CUDA installation

### Performance Tips

- **Zero Overhead**: No performance impact from using CudaRuntime.h
- **Direct CUDA**: Use CUDA APIs directly for best performance
- **Minimize Transfers**: Minimize CPU-GPU data transfers
- **Use Shared Memory**: Use CUDA shared memory for performance

### Debugging Tips

- **Check Includes**: Verify CudaRuntime.h is included
- **Check Types**: Verify correct type usage (cuda_* vs math::*)
- **Check CUDA Installation**: Verify CUDA is properly installed
- **Check Compilation**: Verify CUDA compilation flags

## Implementation Notes

### Preprocessor Behavior

The header relies on preprocessor behavior:

1. **Macro Expansion**: Macros are expanded before compilation
2. **Text Replacement**: Macros perform text replacement
3. **Scope**: Macros affect entire translation unit
4. **Order**: Macros must be defined before use

### Include Order Importance

Include order is critical:

```cpp
// Correct order
#include "Utils/CudaRuntime.h"  // Rename CUDA types
#include <cuda_runtime.h>      // Include CUDA runtime

// Incorrect order
#include <cuda_runtime.h>      // Include CUDA runtime (causes conflicts!)
#include "Utils/CudaRuntime.h"  // Too late to rename
```

### Macro Scope

Macros have global scope within translation unit:

- Affects all code after definition
- Affects included headers
- Requires careful ordering
- Requires cleanup (#undef)

### Type Compatibility

Renamed types maintain compatibility:

```cpp
cuda_float3 a;  // CUDA type
math::float3 b; // Falcor type

// Cannot directly assign (different types)
// a = b;  // Error!

// Must convert explicitly
a.x = b.x;
a.y = b.y;
a.z = b.z;
```

### Header Guard

The header uses `#pragma once`:

```cpp
#pragma once
```

This provides:
- Modern include guard
- Faster compilation
- Cleaner code
- Portability (supported by most compilers)

## Future Enhancements

### Potential Improvements

1. **More Types**: Add renaming for more CUDA types
2. **Namespace Wrapper**: Wrap CUDA in a namespace
3. **Type Conversions**: Add conversion functions
4. **Error Handling**: Add CUDA error handling wrappers
5. **RAII Wrappers**: Add RAII wrappers for CUDA resources
6. **High-Level Abstractions**: Add high-level CUDA abstractions

### API Extensions

1. **Type Aliases**: Add type aliases for convenience
2. **Conversion Functions**: Add CUDA/Falcor type conversion
3. **Smart Pointers**: Add smart pointers for CUDA memory
4. **Stream Wrappers**: Add CUDA stream wrappers
5. **Event Wrappers**: Add CUDA event wrappers

### Platform Enhancements

1. **Cross-Platform**: Add support for other GPU platforms
2. **CPU Fallback**: Add CPU fallback for non-CUDA systems
3. **Dynamic Loading**: Add dynamic CUDA runtime loading
4. **Version Detection**: Add CUDA version detection

### Documentation Enhancements

1. **Examples**: Add more usage examples
2. **Best Practices**: Add best practices guide
3. **Troubleshooting**: Add troubleshooting guide
4. **Performance Guide**: Add performance optimization guide

## References

### CUDA Documentation

- **CUDA Runtime API**: Official CUDA runtime documentation
- **CUDA Programming Guide**: CUDA programming guide
- **CUDA Toolkit**: CUDA toolkit documentation

### Falcor Documentation

- **Falcor Math**: Falcor math library documentation
- **Falcor API**: Falcor API documentation
- **Falcor Architecture**: Falcor architecture documentation

### Related Technologies

- **CUDA**: NVIDIA's parallel computing platform
- **OpenCL**: Open standard for parallel programming
- **Vulkan**: Modern graphics and compute API
- **DirectX 12**: Microsoft's graphics and compute API

## Security Considerations

### Type Safety

- **Macro Limitations**: Macros don't provide type safety
- **Type Confusion**: Potential for type confusion between cuda_* and math::*
- **Explicit Conversion**: Require explicit type conversions
- **Static Analysis**: Use static analysis to catch type errors

### Memory Safety

- **CUDA Memory**: CUDA memory is separate from CPU memory
- **Manual Management**: CUDA memory requires manual management
- **Error Checking**: Always check CUDA API return values
- **Memory Leaks**: Be careful to avoid CUDA memory leaks

### Platform Security

- **Driver Access**: CUDA requires GPU driver access
- **Privilege Escalation**: Potential for privilege escalation
- **Code Execution**: CUDA kernels execute on GPU
- **Sandboxing**: Consider sandboxing CUDA execution

## Troubleshooting

### Common Issues

**Compilation Errors**:
- **Problem**: Type redefinition errors
- **Solution**: Ensure CudaRuntime.h is included before other CUDA headers

**Linking Errors**:
- **Problem**: Undefined symbols
- **Solution**: Ensure CUDA libraries are linked correctly

**Runtime Errors**:
- **Problem**: CUDA initialization failures
- **Solution**: Check CUDA installation and GPU availability

**Type Errors**:
- **Problem**: Type mismatch between cuda_* and math::*
- **Solution**: Use explicit type conversions

### Debugging Tips

1. **Check Includes**: Verify CudaRuntime.h is included
2. **Check Types**: Verify correct type usage
3. **Check CUDA**: Verify CUDA is installed and working
4. **Check Compilation**: Verify CUDA compilation flags

### Getting Help

- **CUDA Forums**: NVIDIA CUDA forums
- **Falcor Issues**: Falcor GitHub issues
- **Stack Overflow**: Stack Overflow CUDA/Falcor tags
- **Documentation**: CUDA and Falcor documentation
