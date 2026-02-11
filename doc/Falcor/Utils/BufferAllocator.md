# BufferAllocator - GPU Buffer Memory Management

## Module State Machine

**Status**: Complete

## Dependency Graph

### External Dependencies

- Falcor/Core/Macros.h (FALCOR_API macro)
- Falcor/Core/API/Buffer.h (Buffer class for GPU resources)
- STL (vector)

## Module Overview

BufferAllocator is a utility class for memory management of a GPU buffer. It maintains a dynamically sized backing buffer on CPU in which memory can be allocated and updated. The GPU buffer is lazily created and updated upon access, with dirty tracking to minimize GPU updates. The allocator enforces alignment requirements and optional cache line constraints to prevent allocated objects from spanning multiple cache lines.

## Component Specifications

### BufferAllocator Class

**Signature**: `class FALCOR_API BufferAllocator`

**Purpose**: GPU buffer memory management with lazy GPU creation and dirty tracking

**Constructor**:
- `BufferAllocator(size_t alignment, size_t elementSize, size_t cacheLineSize = 128, ResourceBindFlags bindFlags = ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess)`: Create buffer allocator

**Allocation Methods**:
- `allocate(size_t byteSize)`: Allocates a memory region, returns offset
- `allocate<T>(size_t count = 1)`: Allocates memory to hold an array of type T
- `pushBack<T>(const T& obj)`: Allocates and copies an object into buffer

**Data Setting Methods**:
- `setBlob(const void* pData, size_t byteOffset, size_t byteSize)`: Sets data into memory region
- `set<T>(size_t byteOffset, const T& obj)`: Sets an object of type T

**Modification Tracking**:
- `modified(const Range& range)`: Marks memory region as modified
- `modified<T>(size_t byteOffset)`: Marks an object as modified

**Pointer Access**:
- `getStartPointer()`: Returns pointer to start of CPU buffer (read/write)
- `getStartPointer() const`: Returns const pointer to start of CPU buffer (read only)

**GPU Buffer Management**:
- `getGPUBuffer(ref<Device> pDevice)`: Creates/returns GPU buffer
- `clear()`: Clears all allocations

**Private Methods**:
- `computeAndAllocatePadding(size_t byteSize)`: Computes required padding and allocates
- `allocInternal(size_t byteSize)`: Internal allocation using vector insertion
- `markAsDirty(const Range& range)`: Marks range as dirty
- `markAsDirty(size_t byteOffset, size_t byteSize)`: Marks byte range as dirty

**Member Variables**:
- `size_t mAlignment`: Minimum alignment for allocations (default from constructor)
- `size_t mElementSize`: Element size for structured buffers (default from constructor)
- `size_t mCacheLineSize`: Cache line size constraint (default 128)
- `ResourceBindFlags mBindFlags`: Resource bind flags for GPU buffer
- `Range mDirty`: Dirty range tracking GPU updates
- `std::vector<uint8_t> mBuffer`: CPU backing buffer
- `ref<Buffer> mpGpuBuffer`: GPU buffer (lazy creation)

## Technical Details

### Constructor

```cpp
BufferAllocator(
    size_t alignment,
    size_t elementSize,
    size_t cacheLineSize = 128,
    ResourceBindFlags bindFlags = ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess
)
```

**Parameters**:
- `alignment`: Minimum alignment in bytes for allocations
- `elementSize`: Element size for structured buffers (0 = raw buffer)
- `cacheLineSize`: Cache line size constraint (default 128)
- `bindFlags`: Resource bind flags for GPU buffer

**Default Behavior**:
- Alignment and elementSize stored for validation
- Cache line size defaults to 128 bytes
- Bind flags default to ShaderResource | UnorderedAccess

### Memory Allocation Algorithm

**computeAndAllocatePadding()**:
```cpp
void computeAndAllocatePadding(size_t byteSize)
{
    struct Range
    {
        size_t start = 0;
        size_t end = 0;
        Range(size_t s, size_t e) : start(s), end(e) {}
    };

    size_t currentOffset = mBuffer.size();

    // Find overlapping dirty ranges
    for (const auto& range : mDirty)
    {
        if (range.start < currentOffset && range.end > currentOffset)
        {
            currentOffset = std::max(currentOffset, range.end);
        }
    }

    // Compute required padding
    size_t pad = currentOffset - mBuffer.size();
    if (pad > 0)
    {
        allocInternal(pad);
    }

    // Validate alignment
    if (mAlignment > 0)
    {
        FALCOR_ASSERT((currentOffset + byteSize) % mAlignment == 0);
    }
}
```

**Padding Strategy**:
1. Find current offset in buffer
2. Find overlapping dirty ranges
3. Compute required padding to satisfy alignment
4. Allocate padding bytes
5. Validate alignment of final offset

**Dirty Range Handling**:
- Iterates through all dirty ranges
- Finds maximum overlapping end position
- Uses maximum to ensure all dirty ranges are covered
- Single dirty range tracked for efficiency

### Internal Allocation

**allocInternal()**:
```cpp
size_t allocInternal(size_t byteSize)
{
    auto iter = mBuffer.insert(mBuffer.end(), byteSize, {});
    return &*iter;
}
```

**Characteristics**:
- Uses `std::vector<uint8_t>::insert()` to allocate memory
- Returns iterator to inserted element
- Returns pointer to first byte of allocation

### Data Setting

**setBlob()**:
```cpp
void setBlob(const void* pData, size_t byteOffset, size_t byteSize)
{
    FALCOR_ASSERT(byteOffset + byteSize <= mBuffer.size());
    std::memcpy(mBuffer.data() + byteOffset, pData, byteSize);
    modified(byteOffset, byteSize);
}
```

**set<T>() Template Method**:
```cpp
template<typename T>
void set(size_t byteOffset, const T& obj)
{
    setBlob(&obj, byteOffset, sizeof(T));
}
```

**Characteristics**:
- Validates offset and size don't exceed buffer size
- Uses `std::memcpy()` for efficient data copying
- Marks region as modified after setting

### Object Allocation

**pushBack() Template Method**:
```cpp
template<typename T>
size_t pushBack(const T& obj)
{
    return allocate(sizeof(T));
}
```

**Characteristics**:
- Allocates memory for single object
- Returns byte offset of allocation
- Uses `allocate()` method internally
- Marks memory as modified

### Modification Tracking

**modified() Method**:
```cpp
void modified(const Range& range)
{
    mDirty = range;
}
```

**modified<T>() Template Method**:
```cpp
template<typename T>
void modified(size_t byteOffset)
{
    modified(byteOffset, sizeof(T));
}
```

**Range Structure**:
```cpp
struct Range
{
    size_t start;
    size_t end;
    Range(size_t s, size_t e) : start(s), end(e) {}
};
```

**Characteristics**:
- Tracks start and end of dirty range
- Single range for all modifications
- Used to minimize GPU buffer updates

### GPU Buffer Management

**getGPUBuffer() Method**:
```cpp
ref<Buffer> getGPUBuffer(ref<Device> pDevice)
{
    if (!mpGpuBuffer)
    {
        mpGpuBuffer = pDevice->createStructuredBuffer(
            mElementSize,
            uint32_t(mBuffer.size() / mElementSize),
            mBindFlags,
            MemoryType::DeviceLocal,
            mBuffer.data()
        );
    }

    return mpGpuBuffer;
}
```

**Lazy Creation**:
- GPU buffer created on first access
- Created with CPU buffer data
- Uses Falcor's Buffer class
- DeviceLocal memory type

**clear() Method**:
```cpp
void clear()
{
    mBuffer.clear();
    mDirty = Range(0, 0);
    mpGpuBuffer = nullptr;
}
```

**Characteristics**:
- Clears CPU buffer
- Resets dirty range
- Releases GPU buffer reference

### Pointer Access

**getStartPointer()**:
```cpp
uint8_t* getStartPointer() { return mBuffer.data(); }
```

**Characteristics**:
- Returns non-const pointer for read/write access
- Pointer is transient (valid until next allocation)
- Caller must not hold pointer across allocations

**getStartPointer() const**:
```cpp
const uint8_t* getStartPointer() const { return mBuffer.data(); }
```

**Characteristics**:
- Returns const pointer for read-only access
- Pointer is transient (valid until next allocation)
- Safe for read-only operations

## Integration Points

### Usage in Falcor Framework

The BufferAllocator is used throughout Falcor framework for GPU buffer management:

1. **Structured Buffers**: Aligned allocations for shader data
2. **Dynamic Data**: Runtime data updates with dirty tracking
3. **Resource Management**: Efficient GPU buffer updates
4. **Memory Alignment**: Enforces alignment requirements

### Integration Pattern

```cpp
// Create allocator
BufferAllocator allocator(16, sizeof(MyStruct));

// Allocate object
size_t offset = allocator.pushBack(myObject);

// Get GPU buffer
ref<Buffer> gpuBuffer = allocator.getGPUBuffer(device);

// Update data
allocator.set(offset, myNewObject);

// Get CPU pointer
uint8_t* cpuPtr = allocator.getStartPointer();
```

## Architecture Patterns

### Lazy Evaluation Pattern

GPU buffer created only when needed:
- First access triggers GPU buffer creation
- Subsequent accesses reuse existing buffer
- Minimizes GPU resource allocation

### Dirty Tracking Pattern

Single dirty range for all modifications:
- All modifications tracked in one range
- Minimizes GPU buffer updates
- Simple range merging logic

### Template Pattern

Generic allocation supporting any type:
- Template parameter T for object type
- Type-safe operations
- Perfect forwarding of arguments

### RAII Pattern

Automatic resource management:
- GPU buffer reference counted
- CPU buffer managed by std::vector
- Clear method releases resources

## Code Patterns

### Allocation Pattern

```cpp
template<typename T>
size_t pushBack(const T& obj)
{
    return allocate(sizeof(T));
}
```

### Data Setting Pattern

```cpp
template<typename T>
void set(size_t byteOffset, const T& obj)
{
    setBlob(&obj, byteOffset, sizeof(T));
}
```

### Modification Tracking Pattern

```cpp
template<typename T>
void modified(size_t byteOffset)
{
    modified(byteOffset, sizeof(T));
}
```

### Padding Computation Pattern

```cpp
void computeAndAllocatePadding(size_t byteSize)
{
    // Find current offset
    size_t currentOffset = mBuffer.size();

    // Find overlapping dirty ranges
    for (const auto& range : mDirty)
    {
        if (range.start < currentOffset && range.end > currentOffset)
        {
            currentOffset = std::max(currentOffset, range.end);
        }
    }

    // Compute padding
    size_t pad = currentOffset - mBuffer.size();
    if (pad > 0)
    {
        allocInternal(pad);
    }

    // Validate alignment
    if (mAlignment > 0)
    {
        FALCOR_ASSERT((currentOffset + byteSize) % mAlignment == 0);
    }
}
```

## Use Cases

### Structured Buffer Allocation

Primary use case is allocating aligned memory for GPU buffers:
- Shader constant buffers
- Vertex buffers
- Index buffers
- Compute shader data structures

### Dynamic Data Updates

Supporting runtime data modifications:
- Scene data updates
- Animation state changes
- Material property updates
- Volume data modifications

### Memory Alignment

Enforcing alignment requirements:
- 16-byte alignment for most GPU resources
- Custom alignment for specific hardware requirements
- Cache line alignment for performance

### Cache Line Optimization

Preventing cache line splits:
- Frequently accessed data structures
- Shader constant buffers
- Texture data buffers
- Compute shader shared memory

## Performance Considerations

### Memory Efficiency

- **Lazy GPU Creation**: GPU buffer created only when needed
- **Dirty Tracking**: Single range minimizes GPU updates
- **Padding Optimization**: Only allocates required padding bytes
- **Vector Backing**: Efficient memory allocation using std::vector

### GPU Performance

- **Minimal Updates**: Dirty tracking reduces GPU buffer updates
- **Batch Updates**: Multiple modifications before GPU update
- **Alignment**: Proper alignment for optimal GPU access
- **Cache-Friendly**: Cache line constraints for better performance

### CPU Performance

- **memcpy for Bulk Data**: Efficient data copying
- **Vector Growth**: Amortized O(1) for sequential allocations
- **No Reallocation**: Allocations are permanent within buffer
- **Pointer Arithmetic**: Simple offset calculations

### Memory Overhead

- **Per-Allocation**: Minimal padding bytes
- **Total Overhead**: Depends on alignment requirements
- **Dirty Range**: Single Range structure (2 size_t values)
- **Vector Overhead**: std::vector internal overhead

## Limitations

### Functional Limitations

- **No Deallocation**: Individual allocations cannot be freed
- **No Reallocation**: Allocations cannot be resized
- **No Pooling**: Cannot allocate from pool of pre-allocated memory
- **Transient Pointers**: CPU pointers invalid after next allocation
- **Single Dirty Range**: All modifications tracked as one range

### Alignment Limitations

- **Fixed Alignment**: Alignment cannot be changed after construction
- **No CPU Alignment**: No alignment guarantees for CPU side
- **Power of 2 Only**: Alignment must be power of 2
- **Cache Line Assumption**: Assumes base pointer starts at cache line

### Memory Limitations

- **Single Buffer**: All allocations in one vector
- **No Growth Strategy**: Uses default vector growth
- **No Maximum Size**: No limit on total allocation size
- **No Memory Pressure**: No mechanism to handle memory pressure

### GPU Limitations

- **Lazy Creation**: GPU buffer not available until first access
- **No Async Upload**: All uploads are synchronous
- **No Streaming**: Cannot stream large data in chunks
- **Bind Flags Fixed**: Cannot be changed after construction

## Best Practices

### When to Use BufferAllocator

1. **Structured Buffers**: Ideal for shader constant buffers
2. **Dynamic Data**: When data changes frequently at runtime
3. **Memory Alignment**: When specific alignment requirements exist
4. **Cache Line Alignment**: When cache performance is critical
5. **Performance Critical**: When GPU buffer updates are expensive

### When to Avoid BufferAllocator

1. **Static Data**: Overkill for constant data
2. **Small Objects**: Overhead may not be justified
3. **Frequent Reallocation**: Cannot handle frequent reallocations
4. **Threaded Allocation**: Not thread-safe (requires external synchronization)
5. **Variable Alignment**: When alignment requirements vary per allocation

### Usage Guidelines

1. **Alignment Selection**: Choose appropriate alignment for hardware (16, 32, 64)
2. **Element Size**: Set element size for structured buffers (0 for raw)
3. **Cache Line Size**: Match cache line size to hardware (64, 128, 256)
4. **Bind Flags**: Choose appropriate flags for usage (ShaderResource, UnorderedAccess)
5. **Pointer Management**: Use pointers carefully, they're transient

### Optimization Tips

1. **Batch Modifications**: Group related modifications before GPU access
2. **Pre-Allocation**: Allocate space for known future allocations
3. **Alignment Tuning**: Match alignment to GPU requirements
4. **Cache Line Tuning**: Match cache line size to hardware
5. **Dirty Tracking**: Leverage single dirty range for efficiency

## Notes

- Designed for GPU buffer memory management
- Lazy GPU buffer creation on first access
- Single dirty range tracks all modifications
- Enforces minimum alignment (power of 2)
- Optional cache line size constraint (default 128)
- CPU backing buffer uses std::vector<uint8_t>
- GPU buffer uses Falcor's Buffer class
- Transient CPU pointers (invalid after next allocation)
- Element size for structured buffers (0 = raw buffer)
- ResourceBindFlags for GPU buffer (default: ShaderResource | UnorderedAccess)
- Template-based allocation for any type
- Perfect forwarding of constructor arguments
- Uses std::memcpy() for efficient data copying
- Validates alignment of final offset
- Range structure for dirty tracking (start, end)
- clear() releases all resources
- getStartPointer() returns non-const pointer
- getStartPointer() const returns const pointer
- FALCOR_API macro for export
- FALCOR_ASSERT for validation
