# GFXAPI

## Overview
[`GFXAPI`](Source/Falcor/Core/API/GFXAPI.h) provides error reporting macros and helper functions for cross-platform graphics API (GFX) and Direct3D (D3D12) error handling. This is a header-only utility module.

## Source Files
- Header: [`Source/Falcor/Core/API/GFXAPI.h`](Source/Falcor/Core/API/GFXAPI.h) (70 lines)
- Implementation: Header-only (no separate .cpp file)

## Type Definitions

### Macros

#### FALCOR_GFX_CALL
```cpp
#define FALCOR_GFX_CALL(call)                      \
    {                                              \
        gfx::Result result_ = call;                \
        if (SLANG_FAILED(result_))                 \
        {                                          \
            gfxReportError("GFX", #call, result_); \
        }                                          \
    }
```
**Purpose**: Macro for calling GFX API functions with automatic error checking and reporting

**Usage**: Wraps any GFX API call and automatically reports errors if the call fails

**Example**:
```cpp
FALCOR_GFX_CALL(pDevice->createCommandQueue(&queue));
```

**Memory Layout**: N/A (macro expansion at compile time)

**Cache Locality**: N/A (macro expansion at compile time)

#### FALCOR_D3D_CALL
```cpp
#define FALCOR_D3D_CALL(call)                      \
    {                                              \
        HRESULT result_ = call;                    \
        if (FAILED(result_))                       \
        {                                          \
            gfxReportError("D3D", #call, result_); \
        }                                          \
    }
```
**Purpose**: Macro for calling D3D12 API functions with automatic error checking and reporting

**Usage**: Wraps any D3D12 API call and automatically reports errors if the call fails

**Example**:
```cpp
FALCOR_D3D_CALL(pDevice->CreateCommandList(...));
```

**Memory Layout**: N/A (macro expansion at compile time)

**Cache Locality**: N/A (macro expansion at compile time)

**Platform**: Only compiled with FALCOR_HAS_D3D12

#### FALCOR_GET_COM_INTERFACE
```cpp
#define FALCOR_GET_COM_INTERFACE(_base, _type, _var) \
    FALCOR_MAKE_SMART_COM_PTR(_type);                \
    FALCOR_CONCAT_STRINGS(_type, Ptr) _var;          \
    FALCOR_D3D_CALL(_base->QueryInterface(IID_PPV_ARGS(&_var)));
```
**Purpose**: Macro for querying COM interfaces from D3D12 objects

**Usage**: Queries a COM interface from a base object and stores it in a smart pointer

**Example**:
```cpp
FALCOR_GET_COM_INTERFACE(pDevice, ID3D12Device5, pDevice5);
```
**Expansion**:
```cpp
_COM_SMARTPTR_TYPEDEF(ID3D12Device5, __uuidof(ID3D12Device5));
ID3D12Device5Ptr pDevice5;
FALCOR_D3D_CALL(pDevice->QueryInterface(IID_PPV_ARGS(&pDevice5)));
```

**Memory Layout**: N/A (macro expansion at compile time)

**Cache Locality**: N/A (macro expansion at compile time)

**Platform**: Only compiled with FALCOR_HAS_D3D12

#### FALCOR_MAKE_SMART_COM_PTR
```cpp
#define FALCOR_MAKE_SMART_COM_PTR(_a) _COM_SMARTPTR_TYPEDEF(_a, __uuidof(_a))
```
**Purpose**: Macro for creating smart COM pointer types

**Usage**: Creates a smart pointer type for COM interfaces

**Example**:
```cpp
FALCOR_MAKE_SMART_COM_PTR(ID3D12Device);
// Creates: ID3D12DevicePtr type
```

**Memory Layout**: N/A (macro expansion at compile time)

**Cache Locality**: N/A (macro expansion at compile time)

**Platform**: Only compiled with FALCOR_HAS_D3D12

### Functions

#### gfxReportError
```cpp
FALCOR_API void gfxReportError(const char* api, const char* call, gfx::Result result);
```
**Purpose**: Report a GFX or D3D error by throwing a RuntimeError exception

**Parameters**:
- `api`: API name ("GFX" or "D3D")
- `call`: Function call string (from macro stringification)
- `result`: Error result code (gfx::Result or HRESULT)

**Return**: void (throws RuntimeError on error)

**Memory Layout**: N/A (function call)

**Cache Locality**: N/A (function call)

**Usage**: Called by FALCOR_GFX_CALL and FALCOR_D3D_CALL macros

## Dependencies

### Direct Dependencies
- [`Handles.h`](Source/Falcor/Core/API/Handles.h) - Handle types for resources
- [`Core/Macros.h`](Source/Falcor/Core/Macros.h) - Macros (FALCOR_API, FALCOR_CONCAT_STRINGS)
- [`Utils/Logger.h`](Source/Falcor/Utils/Logger.h) - Logger for error reporting
- [`slang.h`](Source/Falcor/Core/API/slang.h) - Slang API
- [`slang-gfx.h`](Source/Falcor/Core/API/slang-gfx.h) - Slang GFX API
- [`slang-com-ptr.h`](Source/Falcor/Core/API/slang-com-ptr.h) - Slang COM smart pointers

### Indirect Dependencies
- Slang GFX types (from slang-gfx.h)
- Slang result codes (from slang.h)
- COM smart pointers (from slang-com-ptr.h)
- Logger (from Utils/Logger.h)

## Threading Model

**Thread Safety**: **Thread-Safe (No Mutable State)**

**Analysis**:
- No global or static mutable variables
- All macros expand to local variables
- `gfxReportError()` function has no mutable state
- Safe for concurrent access from multiple threads
- No synchronization primitives needed

**Thread Safety Characteristics**:
1. **No Mutable State**: All macros create local variables
2. **Pure Functions**: `gfxReportError()` has no side effects beyond throwing an exception
3. **Safe for Concurrent Access**: Multiple threads can safely use these macros simultaneously
4. **No External Dependencies**: Only depends on Slang GFX and D3D12 APIs

**Note**: The macros are designed for single-threaded error handling. If multiple threads call these macros simultaneously, each thread will have its own local variables and error handling.

## Cache Locality Analysis

### Access Patterns

#### Macro Expansion
- **Compile-Time**: Macros are expanded at compile time
- **No Runtime Overhead**: Macros generate inline code
- **Local Variables**: Each macro creates local variables on the stack

**Cache Locality**: **Excellent**
- All variables are local to the scope
- No global data access
- Excellent cache locality for local variables

#### Error Reporting
- **Exception Throwing**: `gfxReportError()` throws RuntimeError exception
- **String Operations**: String concatenation for error messages
- **Logging**: Logger writes error messages

**Cache Locality**: **Good**
- String operations may cause cache misses
- Logger may write to file or console (I/O bound)
- Error path is infrequent, so cache locality is not critical

### Cache Line Analysis

#### FALCOR_GFX_CALL Macro
- **Local Variables**: `result_` (4-8 bytes)
- **Stack Usage**: Minimal (4-8 bytes)
- **Cache Locality**: **Excellent** - Single cache line

#### FALCOR_D3D_CALL Macro
- **Local Variables**: `result_` (4 bytes)
- **Stack Usage**: Minimal (4 bytes)
- **Cache Locality**: **Excellent** - Single cache line

#### FALCOR_GET_COM_INTERFACE Macro
- **Local Variables**: Smart pointer (8-16 bytes)
- **Stack Usage**: Minimal (8-16 bytes)
- **Cache Locality**: **Excellent** - Single cache line

## Algorithmic Complexity Analysis

### Macro Expansion

#### FALCOR_GFX_CALL
**Time Complexity**: **O(1)**
- Function call: O(1)
- Result check: O(1)
- Error reporting (if failed): O(1) (string operations + exception throw)

**Space Complexity**: **O(1)**
- Local variable: 4-8 bytes

#### FALCOR_D3D_CALL
**Time Complexity**: **O(1)**
- Function call: O(1)
- Result check: O(1)
- Error reporting (if failed): O(1) (string operations + exception throw)

**Space Complexity**: **O(1)**
- Local variable: 4 bytes

#### FALCOR_GET_COM_INTERFACE
**Time Complexity**: **O(1)**
- COM interface query: O(1)
- Error checking: O(1)

**Space Complexity**: **O(1)**
- Smart pointer: 8-16 bytes

### Error Reporting

#### gfxReportError
**Time Complexity**: **O(1)**
- String operations: O(1)
- Exception throw: O(1)
- Logger write: O(1) (I/O bound)

**Space Complexity**: **O(1)**
- No dynamic memory allocation (except for exception object)

## Key Technical Features

### Cross-Platform Error Handling
- **GFX API Support**: Error reporting for Slang GFX API
- **D3D12 Support**: Error reporting for Direct3D 12 API
- **Unified Interface**: Consistent error reporting across platforms

### Automatic Error Checking
- **Macro-Based**: Automatic error checking with minimal code
- **Stringification**: Automatic stringification of function calls for error messages
- **Exception Throwing**: Automatic exception throwing on error

### COM Interface Helpers
- **Smart Pointers**: Automatic creation of smart COM pointer types
- **Interface Querying**: Automatic COM interface querying with error checking
- **Type Safety**: Type-safe COM interface handling

### Error Reporting
- **API Name**: Includes API name in error messages ("GFX" or "D3D")
- **Function Call**: Includes function call string in error messages
- **Result Code**: Includes result code in error messages
- **Exception Throwing**: Throws RuntimeError exception on error

### Platform-Specific Support
- **D3D12 Only**: FALCOR_D3D_CALL and COM helpers only compiled with FALCOR_HAS_D3D12
- **Conditional Compilation**: Platform-specific code is conditionally compiled

## Performance Characteristics

### Macro Overhead
- **Zero Runtime Overhead**: Macros are expanded at compile time
- **Inline Code**: Generated code is inline, no function call overhead
- **Local Variables**: Minimal stack usage (4-16 bytes per macro)

### Error Checking Overhead
- **Minimal**: Single result check per macro call
- **Branch Prediction**: Error path is infrequent, so branch prediction is effective
- **Exception Overhead**: Exception throwing is expensive but only on error path

### COM Interface Querying
- **Standard COM**: Uses standard COM QueryInterface
- **Smart Pointer**: Automatic reference counting
- **Error Checking**: Automatic error checking with HRESULT validation

### Cache Locality
- **Excellent**: All variables are local to the scope
- **No Global Data**: No global data access
- **Minimal Stack Usage**: Minimal stack usage (4-16 bytes per macro)

## Known Issues and Limitations

1. **Macro Limitations**: Macros cannot be debugged easily (expanded at compile time)
2. **No Error Recovery**: Macros throw exceptions, no error recovery mechanism
3. **No Error Context**: Limited error context (only API name, function call, result code)
4. **No Error Logging**: Errors are logged but may not be persisted
5. **No Error Filtering**: No error filtering or suppression mechanism
6. **No Error Aggregation**: No error aggregation or reporting mechanism
7. **Platform-Specific**: D3D12 macros only available on Windows
8. **No Custom Error Handlers**: No custom error handler support
9. **No Error Callbacks**: No error callback mechanism
10. **No Error Statistics**: No error statistics or metrics

## Usage Example

```cpp
// GFX API call with automatic error checking
FALCOR_GFX_CALL(pDevice->createCommandQueue(&queue));

// D3D12 API call with automatic error checking (Windows only)
FALCOR_D3D_CALL(pDevice->CreateCommandList(...));

// Query COM interface with automatic error checking (Windows only)
FALCOR_GET_COM_INTERFACE(pDevice, ID3D12Device5, pDevice5);

// Multiple error-checked calls
FALCOR_GFX_CALL(pDevice->createCommandQueue(&queue));
FALCOR_GFX_CALL(pDevice->createCommandBuffer(&commandBuffer));
FALCOR_GFX_CALL(pDevice->createSwapchain(&swapchain));
```

## Conclusion

GFXAPI provides a clean and efficient error handling mechanism for cross-platform graphics API and D3D12 operations. The implementation is simple with excellent cache locality and zero runtime overhead for macro expansion.

**Strengths**:
- Clean and simple API for error handling
- Automatic error checking with minimal code
- Cross-platform support (GFX and D3D12)
- Zero runtime overhead for macro expansion
- Excellent cache locality (all variables are local)
- COM interface helpers for D3D12
- Type-safe COM interface handling
- Automatic exception throwing on error
- Consistent error reporting across platforms

**Weaknesses**:
- Macros cannot be debugged easily
- No error recovery mechanism
- Limited error context
- No error filtering or suppression
- Platform-specific D3D12 support (Windows only)
- No custom error handler support
- No error callback mechanism
- No error aggregation or reporting
- No error statistics or metrics

**Recommendations**:
1. Consider adding error recovery mechanisms
2. Consider adding more error context (file, line number)
3. Consider adding error filtering or suppression
4. Consider adding custom error handler support
5. Consider adding error callback mechanism
6. Consider adding error aggregation or reporting
7. Consider adding error statistics or metrics
8. Consider adding cross-platform COM interface support for other platforms
9. Consider adding error logging to file
10. Consider adding error severity levels
