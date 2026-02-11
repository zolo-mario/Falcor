# D3D12Handles

## Module Overview

The **D3D12Handles** module provides D3D12-specific smart COM pointer type definitions for automatic reference counting and resource management. It defines smart pointer wrappers for common D3D12 COM interfaces, simplifying resource management and preventing memory leaks.

## Files

- **Header**: `Source/Falcor/Core/API/Shared/D3D12Handles.h` (45 lines)
- **Implementation**: Header-only (no separate .cpp file)

## Macro Definitions

### FALCOR_MAKE_SMART_COM_PTR

```cpp
#define FALCOR_MAKE_SMART_COM_PTR(_a) _COM_SMARTPTR_TYPEDEF(_a, __uuidof(_a))
```

**Purpose**: Creates a smart COM pointer type definition for a specified COM interface.

**Parameters**:
- `_a`: The COM interface type to create a smart pointer for

**Expansion**: Expands to `_COM_SMARTPTR_TYPEDEF(_a, __uuidof(_a))`, which creates a typedef for a smart pointer type.

## Type Definitions

### Smart COM Pointer Types

```cpp
#if FALCOR_HAS_D3D12
FALCOR_MAKE_SMART_COM_PTR(ID3DBlob);
FALCOR_MAKE_SMART_COM_PTR(ID3D12DescriptorHeap);
FALCOR_MAKE_SMART_COM_PTR(ID3D12RootSignature);
#endif
```

**Defined Types**:
1. `ID3DBlobPtr` - Smart pointer for ID3DBlob interface
2. `ID3D12DescriptorHeapPtr` - Smart pointer for ID3D12DescriptorHeap interface
3. `ID3D12RootSignaturePtr` - Smart pointer for ID3D12RootSignature interface

## Dependencies

### Internal Dependencies

- None

### External Dependencies

- **D3D12**: Requires Direct3D 12 API (conditionally compiled)
- **comdef.h**: COM definitions (conditionally compiled)
- **_COM_SMARTPTR_TYPEDEF**: COM smart pointer typedef macro
- **__uuidof**: Compiler intrinsic for UUID generation

## Cross-Platform Considerations

- **Windows Only**: D3D12 is only available on Windows
- **Conditional Compilation**: Only compiles when `FALCOR_HAS_D3D12` is defined
- **No Vulkan Equivalent**: Vulkan uses different handle management
- **Platform-Specific**: COM interfaces are Windows-specific

## Usage Patterns

### Using Smart COM Pointers

```cpp
ID3D12DescriptorHeapPtr pHeap;
// Create D3D12 descriptor heap
pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pHeap));
// Smart pointer automatically manages reference counting
// No need to call pHeap->Release()
```

### Copying Smart COM Pointers

```cpp
ID3D12DescriptorHeapPtr pHeap1 = ...;
ID3D12DescriptorHeapPtr pHeap2 = pHeap1; // Atomic reference count increment
// Both pointers share ownership
```

### Moving Smart COM Pointers

```cpp
ID3D12DescriptorHeapPtr pHeap1 = ...;
ID3D12DescriptorHeapPtr pHeap2 = std::move(pHeap1); // Efficient move
// pHeap2 now owns the interface, pHeap1 is null
```

### Accessing COM Interface

```cpp
ID3D12DescriptorHeapPtr pHeap = ...;
// Dereference operator
D3D12_DESCRIPTOR_HEAP_DESC desc = pHeap->GetDesc();
// Arrow operator
D3D12_CPU_DESCRIPTOR_HANDLE handle = pHeap->GetCPUDescriptorHandleForHeapStart();
```

## Summary

**D3D12Handles** is a lightweight header-only module that provides:

### Smart COM Pointer Types
- Excellent cache locality (all members in single cache line)
- O(1) time complexity for all operations
- Thread-safe atomic reference counting
- Automatic resource cleanup via RAII
- Exception-safe resource management
- Efficient move semantics
- Three defined types: ID3DBlobPtr, ID3D12DescriptorHeapPtr, ID3D12RootSignaturePtr

### Macro-Based Type Definition
- FALCOR_MAKE_SMART_COM_PTR macro for type generation
- Expands to _COM_SMARTPTR_TYPEDEF
- Uses __uuidof for UUID generation
- Conditional compilation for D3D12 support

The module provides a simple, efficient way to manage D3D12 COM interfaces with automatic reference counting and resource cleanup, ensuring thread-safe operations and preventing memory leaks through RAII principles.
