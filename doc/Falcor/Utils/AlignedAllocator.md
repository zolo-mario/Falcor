# AlignedAllocator - Aligned Memory Allocation Utility

## Module State Machine

**Status**: Complete

## Dependency Graph

### External Dependencies

- Falcor/Core/Error.h (Error handling)
- Falcor/Utils/Math/Common.h (Math utilities)
- STL (new, utility, vector)

## Module Overview

AlignedAllocator is a utility class for aligned memory allocations on GPU. It enforces various alignment requirements including minimum byte alignment and optional cache line size constraints to prevent allocated objects from spanning multiple cache lines. The allocator is designed to manage GPU allocations and assumes the base pointer starts at a cache line.

## Component Specifications

### AlignedAllocator Class

**Signature**: `class AlignedAllocator`

**Purpose**: GPU-aligned memory allocation with alignment and cache line constraints

**Public Methods**:

**Configuration Methods**:
- `setMinimumAlignment(int minAlignment)`: Sets minimum alignment for allocated objects (power of 2, 0 disables)
- `setCacheLineSize(int cacheLineSize)`: Sets cache line size constraint (power of 2, 0 disables)

**Allocation Methods**:
- `allocate<T, Args...>(Args&&... args)`: Allocates object of type T, executes constructor with args
- `allocateSized<T, Args...>(size_t size, Args&&... args)`: Allocates object with additional memory at end, executes constructor with args

**Buffer Management**:
- `reserve(size_t size)`: Reserves capacity in internal buffer
- `resize(size_t size)`: Resizes internal buffer with zero initialization
- `reset()`: Clears internal buffer

**Pointer Management**:
- `getStartPointer()`: Returns pointer to start of allocated buffer
- `getStartPointer() const`: Returns const pointer to start of allocated buffer
- `offsetOf(void* ptr) const`: Returns offset of given pointer inside allocation buffer

**Size Queries**:
- `getSize() const`: Returns current buffer size
- `getCapacity() const`: Returns current buffer capacity

**Private Methods**:
- `computeAndAllocatePadding(size_t size)`: Computes required padding and allocates
- `allocInternal(size_t size)`: Internal allocation using vector insertion

**Member Variables**:
- `size_t mMinAlignment`: Minimum alignment requirement (default 16)
- `size_t mCacheLineSize`: Cache line size constraint (default 128)
- `std::vector<uint8_t> mBuffer`: Internal allocation buffer

## Technical Details

### Alignment Constraints

**Minimum Alignment**:
- Must be power of 2 (asserted)
- Zero value disables minimum alignment enforcement
- Default value: 16 bytes
- Applied to all allocations

**Cache Line Size**:
- Must be power of 2 (asserted)
- Zero value disables cache line constraint
- Default value: 128 bytes
- Prevents allocations from spanning multiple cache lines

### Allocation Algorithm

**computeAndAllocatePadding()**:
```cpp
void computeAndAllocatePadding(size_t size)
{
    size_t currentOffset = mBuffer.size();

    // Apply minimum alignment
    if (mMinAlignment > 0 && (currentOffset % mMinAlignment) != 0)
    {
        // Not at minimum alignment; get aligned
        currentOffset += mMinAlignment - (currentOffset % mMinAlignment);
    }

    // Apply cache line constraint
    if (mCacheLineSize > 0)
    {
        const size_t cacheLineOffset = currentOffset % mCacheLineSize;
        if (size <= mCacheLineSize && cacheLineOffset + size > mCacheLineSize)
        {
            // Allocation is smaller than or equal to a cache line but
            // would span two cache lines; move to start of next cache line
            currentOffset += mCacheLineSize - cacheLineOffset;
        }
    }

    // Allocate padding if needed
    size_t pad = currentOffset - mBuffer.size();
    if (pad > 0)
    {
        allocInternal(pad);
    }

    // Verify alignment
    FALCOR_ASSERT(mMinAlignment == 0 || mBuffer.size() % mMinAlignment == 0);
}
```

**Padding Strategy**:
1. Compute current offset in buffer
2. Apply minimum alignment if enabled
3. Apply cache line constraint if enabled
4. Calculate required padding
5. Allocate padding bytes
6. Verify final alignment

### Object Allocation

**allocate() Template Method**:
```cpp
template<typename T, typename... Args>
T* allocate(Args&&... args)
{
    const size_t size = sizeof(T);
    computeAndAllocatePadding(size);
    void* ptr = allocInternal(size);
    return new (ptr) T(std::forward<Args>(args)...);
}
```

**allocateSized() Template Method**:
```cpp
template<typename T, typename... Args>
T* allocateSized(size_t size, Args&&... args)
{
    FALCOR_ASSERT(size >= sizeof(T));
    computeAndAllocatePadding(size);
    void* ptr = allocInternal(size);
    return new (ptr) T(std::forward<Args>(args)...);
}
```

**Perfect Forwarding**:
- Uses `std::forward<Args>(args)...` to forward arguments
- Preserves value category (lvalue/rvalue)
- Zero overhead for argument forwarding

### Internal Allocation

**allocInternal() Method**:
```cpp
void* allocInternal(size_t size)
{
    auto iter = mBuffer.insert(mBuffer.end(), size, {});
    return &*iter;
}
```

**Vector Insertion**:
- Uses `std::vector<uint8_t>::insert()` to allocate memory
- Returns iterator to inserted element
- Returns pointer to first byte of allocation

### Pointer Offset Calculation

**offsetOf() Method**:
```cpp
size_t offsetOf(void* ptr) const
{
    FALCOR_ASSERT(ptr >= mBuffer.data() && ptr < mBuffer.data() + mBuffer.size());
    return static_cast<uint8_t*>(ptr) - mBuffer.data();
}
```

**Validation**:
- Asserts pointer is within buffer bounds
- Computes byte offset from buffer start
- Returns offset as size_t

### Buffer Management

**reserve() Method**:
```cpp
void reserve(size_t size) { mBuffer.reserve(size); }
```

**resize() Method**:
```cpp
void resize(size_t size) { mBuffer.resize(size, 0); }
```

**reset() Method**:
```cpp
void reset() { mBuffer.clear(); }
```

**Characteristics**:
- `reserve()`: Pre-allocates capacity without initialization
- `resize()`: Resizes and zero-initializes
- `reset()`: Clears all allocations

## Integration Points

### Usage in Falcor Framework

The AlignedAllocator is used throughout the Falcor framework for GPU memory management:

1. **GPU Buffer Allocation**: Aligned allocations for GPU resources
2. **Shader Data Structures**: Aligned shader constant buffers
3. **Compute Shaders**: Aligned shared memory allocations
4. **Texture Data**: Aligned texture data buffers

### Integration Pattern

```cpp
// Create allocator
AlignedAllocator allocator;
allocator.setMinimumAlignment(16);
allocator.setCacheLineSize(128);

// Allocate aligned object
MyStruct* obj = allocator.allocate<MyStruct>(arg1, arg2);

// Get buffer pointer
void* bufferStart = allocator.getStartPointer();

// Calculate offset
size_t offset = allocator.offsetOf(obj);

// Get size
size_t size = allocator.getSize();
```

## Architecture Patterns

### Template Method Pattern

Generic allocation supporting any type:
- Template parameter T for object type
- Variadic template for constructor arguments
- Perfect forwarding of arguments
- Type-safe allocation

### Strategy Pattern

Multiple alignment strategies:
- Minimum alignment enforcement
- Cache line constraint enforcement
- Combined padding strategy
- Disabled modes for flexibility

### Allocator Pattern

Custom allocator using std::vector as backing:
- Vector provides contiguous memory
- Insert method for allocation
- Offset tracking for pointer management
- Automatic memory management

### RAII Pattern

Object placement new with automatic cleanup:
- `new (ptr) T(...)` placement new syntax
- Object constructed in allocated memory
- Cleanup handled by vector destruction

## Code Patterns

### Configuration Pattern

```cpp
void setMinimumAlignment(int minAlignment)
{
    FALCOR_ASSERT(minAlignment == 0 || isPowerOf2(minAlignment));
    mMinAlignment = minAlignment;
}

void setCacheLineSize(int cacheLineSize)
{
    FALCOR_ASSERT(cacheLineSize == 0 || isPowerOf2(cacheLineSize));
    mCacheLineSize = cacheLineSize;
}
```

### Allocation Pattern

```cpp
template<typename T, typename... Args>
T* allocate(Args&&... args)
{
    const size_t size = sizeof(T);
    computeAndAllocatePadding(size);
    void* ptr = allocInternal(size);
    return new (ptr) T(std::forward<Args>(args)...);
}
```

### Padding Computation Pattern

```cpp
void computeAndAllocatePadding(size_t size)
{
    size_t currentOffset = mBuffer.size();

    // Apply minimum alignment
    if (mMinAlignment > 0 && (currentOffset % mMinAlignment) != 0)
    {
        currentOffset += mMinAlignment - (currentOffset % mMinAlignment);
    }

    // Apply cache line constraint
    if (mCacheLineSize > 0)
    {
        const size_t cacheLineOffset = currentOffset % mCacheLineSize;
        if (size <= mCacheLineSize && cacheLineOffset + size > mCacheLineSize)
        {
            currentOffset += mCacheLineSize - cacheLineOffset;
        }
    }

    // Allocate padding
    size_t pad = currentOffset - mBuffer.size();
    if (pad > 0)
    {
        allocInternal(pad);
    }

    FALCOR_ASSERT(mMinAlignment == 0 || mBuffer.size() % mMinAlignment == 0);
}
```

## Use Cases

### GPU Buffer Allocation

Primary use case is allocating GPU-aligned memory:
- Vertex buffers
- Index buffers
- Constant buffers
- Shader data structures

### Cache Line Alignment

Preventing cache line splits for performance:
- Frequently accessed data structures
- Shader constant buffers
- Compute shader shared memory
- Texture data buffers

### Custom Memory Management

Custom allocation strategies:
- Specific alignment requirements
- Custom cache line sizes
- Debug memory allocation tracking
- Performance profiling

### Type-Safe Allocation

Template-based allocation for any type:
- POD types
- Complex classes with constructors
- Multiple argument forwarding
- Perfect forwarding support

## Performance Considerations

### Memory Efficiency

- **Minimal Padding**: Only allocates required padding bytes
- **Contiguous Memory**: Single vector for all allocations
- **No Fragmentation**: Linear allocation pattern
- **Automatic Cleanup**: Vector handles deallocation

### Cache Performance

- **Cache Line Alignment**: Prevents objects from spanning cache lines
- **Configurable**: Cache line size can be tuned for hardware
- **Optional**: Can be disabled if not needed
- **Performance Gain**: Reduces cache misses for aligned data

### Allocation Performance

- **O(1) Allocation**: Constant time allocation
- **O(1) Padding**: Constant time padding computation
- **Vector Growth**: Amortized O(1) for sequential allocations
- **No Reallocation**: Allocations are permanent within buffer

### Memory Overhead

- **Per-Allocation**: Minimal padding bytes
- **Total Overhead**: Depends on alignment requirements
- **Worst Case**: Up to (minAlignment + cacheLineSize - 1) bytes per allocation
- **Typical Case**: Much less than worst case

## Limitations

### Functional Limitations

- **No Deallocation**: Individual allocations cannot be freed
- **No Reallocation**: Allocations cannot be resized
- **No Pooling**: Cannot allocate from pool of pre-allocated memory
- **No Thread Safety**: Not thread-safe (requires external synchronization)

### Alignment Limitations

- **Power of 2 Only**: Alignment must be power of 2
- **Fixed at Runtime**: Cannot change alignment after allocations
- **No CPU Alignment**: No alignment guarantees for CPU side
- **Cache Line Assumption**: Assumes base pointer starts at cache line

### Memory Limitations

- **Single Buffer**: All allocations in one vector
- **No Growth Strategy**: Uses default vector growth
- **No Maximum Size**: No limit on total allocation size
- **No Memory Pressure**: No mechanism to handle memory pressure

### Type Limitations

- **POD Only**: Best suited for POD types
- **Constructor Requirements**: Type must be constructible
- **Move Semantics**: Requires move constructor for efficiency
- **Alignment**: Type must support placement new

## Best Practices

### When to Use AlignedAllocator

1. **GPU Allocations**: Ideal for GPU memory allocation patterns
2. **Cache Line Alignment**: Use for frequently accessed data structures
3. **Custom Alignment**: When specific alignment requirements exist
4. **Performance Critical**: When cache performance is critical
5. **Shader Data**: For shader constant buffers and data structures

### When to Avoid AlignedAllocator

1. **CPU Allocations**: Overkill for CPU-side allocations
2. **Small Objects**: Overhead may not be justified
3. **Frequent Reallocation**: Cannot handle frequent reallocations
4. **Threaded Allocation**: Not thread-safe
5. **Variable Alignment**: When alignment requirements vary per allocation

### Usage Guidelines

1. **Configuration**: Set alignment and cache line size before allocations
2. **Type Selection**: Use appropriate types for GPU data
3. **Buffer Management**: Use reserve() for known sizes
4. **Pointer Tracking**: Use offsetOf() for pointer management
5. **Reset Carefully**: Only reset when all allocations are no longer needed

### Optimization Tips

1. **Alignment Tuning**: Choose appropriate alignment for hardware (16, 32, 64)
2. **Cache Line Tuning**: Match cache line size to hardware (64, 128, 256)
3. **Pre-Reservation**: Use reserve() for known allocation sizes
4. **Batch Allocations**: Allocate related objects together
5. **Alignment Grouping**: Group objects with same alignment requirements

## Notes

- Designed for GPU memory allocation patterns
- Assumes base pointer starts at cache line
- No CPU-side alignment guarantees
- Minimum alignment must be power of 2 (asserted)
- Cache line size must be power of 2 (asserted)
- Zero values disable alignment and cache line constraints
- Default minimum alignment: 16 bytes
- Default cache line size: 128 bytes
- Uses placement new syntax for object construction
- Perfect forwarding of constructor arguments
- No individual deallocation support
- All allocations in single std::vector<uint8_t>
- Vector provides automatic memory management
- Offset tracking for pointer management
- Assertions for validation in debug builds
- Template-based for type safety
- Variadic templates for flexible constructor arguments
