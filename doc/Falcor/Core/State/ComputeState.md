# ComputeState

## Module Overview

The **ComputeState** module provides compute state management for Falcor engine. This module includes [`ComputeState`](Source/Falcor/Core/State/ComputeState.h:45) class, providing compute state management with program binding, state object caching, and state graph management.

## Files

- **Header**: `Source/Falcor/Core/State/ComputeState.h` (93 lines)
- **Implementation**: `Source/Falcor/Core/State/ComputeState.cpp` (not provided in file list)

## Module Structure

### Classes

1. **ComputeState** - Compute state management

## ComputeState Class

### Definition

```cpp
class FALCOR_API ComputeState : public Object
{
    FALCOR_OBJECT(ComputeState)
public:
    ~ComputeState() = default;

    static ref<ComputeState> create(ref<Device> pDevice);

    ComputeState& setProgram(ref<Program> pProgram);
    ref<Program> getProgram() const;

    ref<ComputeStateObject> getCSO(const ProgramVars* pVars);

private:
    ComputeState(ref<Device> pDevice);

    ref<Device> mpDevice;
    ref<Program> mpProgram;
    ComputeStateObjectDesc mDesc;

    struct CachedData
    {
        const ProgramKernels* pProgramKernels = nullptr;
    };
    CachedData mCachedData;

    using ComputeStateGraph = StateGraph<ref<ComputeStateObject>, void*>;
    std::unique_ptr<ComputeStateGraph> mpCsoGraph;
};
```

### Dependencies

### Internal Dependencies

- **Object**: Inherits from [`Object`](Source/Falcor/Core/Object.h) for reference counting
- **Device**: Uses [`Device`](Source/Falcor/Core/API/Device.h) for device management
- **Program**: Uses [`Program`](Source/Falcor/Core/Program/Program.h:573) for program management
- **ProgramKernels**: Uses [`ProgramKernels`](Source/Falcor/Core/Program/ProgramVersion.h:171) for program kernels
- **ComputeStateObject**: Uses [`ComputeStateObject`](Source/Falcor/Core/API/ComputeStateObject.h) for compute state object
- **ComputeStateObjectDesc**: Uses [`ComputeStateObjectDesc`](Source/Falcor/Core/API/ComputeStateObject.h) for compute state object descriptor
- **StateGraph**: Uses [`StateGraph`](Source/Falcor/Core/State/StateGraph.h:36) for state graph management

### External Dependencies

- **ref<T>**: Uses [`ref<T>`](Source/Falcor/Core/Object.h) for reference counting
- **std::unique_ptr**: Uses std::unique_ptr for unique ownership
- **ProgramVars**: Uses [`ProgramVars`](Source/Falcor/Core/Program/ProgramVars.h) for program variables

## Usage Patterns

### Creating ComputeState

```cpp
auto computeState = ComputeState::create(pDevice);
```

### Setting Program

```cpp
computeState->setProgram(pProgram);
```

### Getting Program

```cpp
ref<Program> program = computeState->getProgram();
```

### Getting Compute State Object

```cpp
ref<ComputeStateObject> cso = computeState->getCSO(pVars);
```

## Summary

**ComputeState** is a specialized module that provides compute state management for Falcor engine:

### ComputeState Class
- **Memory Layout**: ~40-48 bytes + heap allocations
- **Threading Model**: Not thread-safe
- **Cache Locality**: Excellent (inline members fit in cache line)
- **Algorithmic Complexity**: O(1) for most operations, O(N) for scan for matching node
- **Mutable State**: Mutable state (not immutable after construction)
- **Factory Pattern**: Static create method for object creation
- **Reference Counting**: [`ref<T>`](Source/Falcor/Core/Object.h) provides automatic resource management
- **State Graph**: Uses state graph for state object caching
- **Fluent Interface**: Fluent interface for method chaining
- **Caching**: Caches compute state objects for performance

### Module Characteristics
- Mutable state (not immutable after construction)
- Reference counting (ref<T> provides automatic resource management)
- Factory pattern (static create method)
- State graph (uses state graph for state object caching)
- Fluent interface (fluent interface for method chaining)
- Caching (caches compute state objects for performance)
- Excellent cache locality (inline members fit in cache line)
- Not thread-safe (requires external synchronization)
- Moderate complexity (state graph)

The module provides a comprehensive compute state management system with mutable state, reference counting, state graph for state object caching, and fluent interface for method chaining, optimized for performance with caching of compute state objects.
