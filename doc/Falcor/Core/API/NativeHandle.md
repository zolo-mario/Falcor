# NativeHandle

## Overview
[`NativeHandle`](Source/Falcor/Core/API/NativeHandle.h) provides a type-safe abstraction for native graphics API handles (D3D12 and Vulkan). This module enables cross-platform handle management without exposing heavy D3D12/Vulkan headers everywhere.

## Source Files
- Header: [`Source/Falcor/Core/API/NativeHandle.h`](Source/Falcor/Core/API/NativeHandle.h) (101 lines)
- Implementation: Header-only (no separate .cpp file)

## Type Definitions

### NativeHandleType Enum
```cpp
enum class NativeHandleType
{
    Unknown,

    // D3D12 handles
    ID3D12Device,
    ID3D12Resource,
    ID3D12PipelineState,
    ID3D12Fence,
    ID3D12CommandQueue,
    ID3D12GraphicsCommandList,
    D3D12_CPU_DESCRIPTOR_HANDLE,

    // Vulkan handles
    VkInstance,
    VkPhysicalDevice,
    VkDevice,
    VkImage,
    VkImageView,
    VkBuffer,
    VkBufferView,
    VkPipeline,
    VkFence,
    VkQueue,
    VkCommandBuffer,
    VkSampler,
};
```
**Purpose**: Enumeration of native handle types

**Memory Layout**: 4 bytes (enum, likely 4 bytes)

**Cache Locativity**: **Excellent** - Fits in a single cache line

**Usage**: Identifies the type of native handle stored in NativeHandle

**Total Types**: 16 handle types (1 Unknown + 7 D3D12 + 8 Vulkan)

### NativeHandleTrait Template
```cpp
template<typename T>
struct NativeHandleTrait;
```
**Purpose**: Template for type traits of native handles

**Memory Layout**: N/A (template declaration)

**Cache Locativity**: N/A (template declaration)

**Usage**: Provides type information and conversion functions for native handles

**Implementation**: Must be specialized for each native handle type

**Note**: This is a forward declaration. Actual specializations are in NativeHandleTraits.h

### NativeHandle Class
```cpp
class NativeHandle
{
public:
    NativeHandle() = default;

    template<typename T>
    explicit NativeHandle(T native)
    {
        mType = NativeHandleTrait<T>::type;
        mValue = NativeHandleTrait<T>::pack(native);
    }

    NativeHandleType getType() const { return mType; }

    bool isValid() const { return mType != NativeHandleType::Unknown; }

    template<typename T>
    T as() const
    {
        FALCOR_ASSERT(mType == NativeHandleTrait<T>::type);
        return NativeHandleTrait<T>::unpack(mValue);
    }

private:
    NativeHandleType mType{NativeHandleType::Unknown};
    uint64_t mValue{0};
};
```
**Purpose**: Type-safe wrapper for native graphics API handles

**Memory Layout**: 12 bytes (4 bytes NativeHandleType + 8 bytes uint64_t)

**Member Layout**:
- `mType`: 4 bytes (offset 0)
- `mValue`: 8 bytes (offset 4)

**Cache Locativity**: **Excellent** - Fits in a single cache line (64 bytes)

**Alignment**: Natural alignment (4-byte alignment for NativeHandleType, 8-byte alignment for uint64_t)

**Usage**: Represents a native graphics API handle with type safety

**Key Features**:
- Type-safe construction from native handles
- Type information tracking
- Validity checking
- Type-safe conversion to native handles
- 64-bit handle storage (fits all D3D12 and Vulkan handles)

## Dependencies

### Direct Dependencies
- [`Core/Error.h`](Source/Falcor/Core/Error.h) - Error handling (FALCOR_ASSERT)
- [`cstdint`](Source/Falcor/Core/API/cstdint) - Integer types (uint64_t)
- [`NativeHandleTraits.h`](Source/Falcor/Core/API/NativeHandleTraits.h) - Native handle type traits (implied)

### Indirect Dependencies
- Error handling (from Core/Error.h)
- Native handle type traits (from NativeHandleTraits.h)
- Standard C++ integer types

## Threading Model

**Thread Safety**: **Thread-Safe for Concurrent Reads, Not Thread-Safe for Concurrent Writes**

**Analysis**:
- No global or static mutable variables
- Class has mutable state (mType, mValue)
- No synchronization primitives
- Safe for concurrent reads from multiple threads
- Not safe for concurrent writes to the same NativeHandle instance

**Thread Safety Characteristics**:
1. **Mutable State**: Class has mutable state (mType, mValue)
2. **No Synchronization**: No mutex or atomic operations
3. **Safe for Concurrent Reads**: Multiple threads can safely read the same NativeHandle instance
4. **Unsafe for Concurrent Writes**: Multiple threads writing to the same NativeHandle instance causes race conditions
5. **No External Dependencies**: Only depends on input parameters and NativeHandleTrait

**Note**: The class is designed for single-threaded use or concurrent reads. For multi-threaded scenarios with concurrent writes, external synchronization is required.

## Cache Locality Analysis

### Access Patterns

#### Default Constructor
- **Zero Initialization**: Initializes mType to Unknown and mValue to 0
- **Single Cache Line**: Fits in a single cache line (64 bytes)
- **Excellent for Repeated Construction**: Excellent cache locality for repeated construction

**Cache Locativity**: **Excellent**
- Single cache line access
- Excellent for repeated construction

#### Template Constructor
- **Type-Safe Construction**: Constructs from native handle with type checking
- **Single Cache Line**: Fits in a single cache line (64 bytes)
- **Excellent for Repeated Construction**: Excellent cache locality for repeated construction

**Cache Locativity**: **Excellent**
- Single cache line access
- Excellent for repeated construction

#### getType() Method
- **Type Query**: Returns the handle type
- **Single Cache Line**: Fits in a single cache line (64 bytes)
- **Excellent for Repeated Access**: Excellent cache locality for repeated access

**Cache Locativity**: **Excellent**
- Single cache line access
- Excellent for repeated access

#### isValid() Method
- **Validity Check**: Returns true if handle type is not Unknown
- **Single Cache Line**: Fits in a single cache line (64 bytes)
- **Excellent for Repeated Access**: Excellent cache locality for repeated access

**Cache Locativity**: **Excellent**
- Single cache line access
- Excellent for repeated access

#### as() Template Method
- **Type-Safe Conversion**: Converts to native handle with type checking
- **Single Cache Line**: Fits in a single cache line (64 bytes)
- **Excellent for Repeated Access**: Excellent cache locality for repeated access

**Cache Locativity**: **Excellent**
- Single cache line access
- Excellent for repeated access

### Cache Line Analysis

#### NativeHandle Class
- **Size**: 12 bytes (4 bytes NativeHandleType + 8 bytes uint64_t)
- **Cache Line**: Fits in a single cache line (64 bytes)
- **Padding**: 4 bytes padding after mType to align mValue to 8-byte boundary
- **Cache Locativity**: **Excellent**

## Algorithmic Complexity Analysis

### Constructor and Methods

#### Default Constructor
**Time Complexity**: **O(1)**
- Member initialization: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Member initialization: 12 bytes

#### Template Constructor
**Time Complexity**: **O(1)**
- Type trait lookup: O(1)
- Native handle packing: O(1)
- Member initialization: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Member initialization: 12 bytes

#### getType()
**Time Complexity**: **O(1)**
- Member access: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 4 bytes (enum)

#### isValid()
**Time Complexity**: **O(1)**
- Member access: O(1)
- Comparison: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 1 byte (bool)

#### as() Template Method
**Time Complexity**: **O(1)**
- Type trait lookup: O(1)
- Type assertion: O(1)
- Native handle unpacking: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 8 bytes (pointer or handle)

## Key Technical Features

### Type-Safe Handle Wrapper
- **Type Safety**: Compile-time type checking for native handles
- **Type Information**: Tracks handle type at runtime
- **Type Validation**: Validates handle type on conversion
- **64-bit Storage**: Stores all D3D12 and Vulkan handles in 64 bits

### Native Handle Types
- **16 Handle Types**: Support for D3D12 and Vulkan handles
- **D3D12 Handles**: 7 types (Device, Resource, PipelineState, Fence, CommandQueue, GraphicsCommandList, CPU_DESCRIPTOR_HANDLE)
- **Vulkan Handles**: 8 types (Instance, PhysicalDevice, Device, Image, ImageView, Buffer, BufferView, Pipeline, Fence, Queue, CommandBuffer, Sampler)

### Construction and Conversion
- **Default Constructor**: Creates invalid handle (Unknown type, 0 value)
- **Template Constructor**: Type-safe construction from native handles
- **Type-Safe Conversion**: Type-safe conversion to native handles with assertion
- **Type Trait Integration**: Uses NativeHandleTrait for type information and conversion

### Validity Checking
- **isValid()**: Returns true if handle type is not Unknown
- **Type-Based Validity**: Validity is based on handle type, not value
- **Runtime Check**: Runtime validity check for handles

### Cross-Platform Support
- **D3D12 Support**: Support for D3D12 handles
- **Vulkan Support**: Support for Vulkan handles
- **Unified Interface**: Unified interface for both platforms
- **Header Separation**: Separation from heavy D3D12/Vulkan headers

### Native Handle Traits
- **Type Trait Template**: NativeHandleTrait for type information
- **Pack/Unpack Functions**: NativeHandleTrait::pack() and NativeHandleTrait::unpack()
- **Type Information**: NativeHandleTrait<T>::type for handle type
- **Specialization**: Must be specialized for each native handle type

## Performance Characteristics

### Construction Overhead
- **Zero Runtime Overhead**: Default constructor and template constructor are inline
- **Compile-Time Type Checking**: Type safety is enforced at compile time
- **No Memory Allocations**: Zero dynamic memory allocation
- **Thread-Safe for Concurrent Reads**: Safe for concurrent reads from multiple threads

### Access Overhead
- **Zero Runtime Overhead**: All methods are inline
- **Direct Member Access**: Direct access to member variables
- **No Memory Allocations**: Zero dynamic memory allocation
- **Thread-Safe for Concurrent Reads**: Safe for concurrent reads from multiple threads

### Cache Locality
- **Excellent**: All members fit in single cache line
- **No Padding Issues**: Minimal padding (4 bytes for alignment)
- **Minimal Memory Footprint**: Minimal memory footprint (12 bytes per instance)
- **Excellent for Repeated Access**: Excellent cache locality for repeated access

### Type Safety Overhead
- **Compile-Time Only**: Type checking is done at compile time
- **Zero Runtime Overhead**: No runtime type checking overhead
- **Assertion-Based**: Runtime type validation via FALCOR_ASSERT
- **Debug-Only Overhead**: Assertions are only enabled in debug builds

## Known Issues and Limitations

1. **No Thread Safety for Writes**: Not thread-safe for concurrent writes to the same NativeHandle instance
2. **No Value Validation**: No validation of handle values (only type validation)
3. **No Handle Management**: No automatic handle management (creation, destruction)
4. **No Handle Comparison**: No comparison operators for NativeHandle
5. **No Handle Hashing**: No hash functions for NativeHandle
6. **No Handle Serialization**: No serialization functions for NativeHandle
7. **No Handle Deserialization**: No deserialization functions for NativeHandle
8. **No Handle Duplication**: No duplication functions for NativeHandle
9. **No Handle Invalidation**: No invalidation functions for NativeHandle
10. **No Handle Debugging**: No debugging functions for NativeHandle

## Usage Example

```cpp
// Create a NativeHandle from a D3D12 device
ID3D12Device* pD3D12Device = /* ... */;
NativeHandle handle(pD3D12Device);

// Check if handle is valid
if (handle.isValid())
{
    // Get handle type
    NativeHandleType type = handle.getType(); // NativeHandleType::ID3D12Device

    // Convert back to native handle
    ID3D12Device* pDevice = handle.as<ID3D12Device*>();
}

// Create a NativeHandle from a Vulkan image
VkImage vkImage = /* ... */;
NativeHandle handle(vkImage);

// Check if handle is valid
if (handle.isValid())
{
    // Get handle type
    NativeHandleType type = handle.getType(); // NativeHandleType::VkImage

    // Convert back to native handle
    VkImage image = handle.as<VkImage>();
}

// Create an invalid handle
NativeHandle invalidHandle;
if (!invalidHandle.isValid())
{
    // Handle is invalid (type is Unknown)
}
```

## Conclusion

NativeHandle provides a type-safe and efficient wrapper for native graphics API handles. The implementation is simple with excellent cache locality and zero runtime overhead for most operations.

**Strengths**:
- Type-safe wrapper for native handles
- Zero runtime overhead (all methods are inline)
- Excellent cache locality (all members fit in single cache line)
- Thread-safe for concurrent reads
- Cross-platform support (D3D12 and Vulkan)
- Type information tracking
- Type validation on conversion
- 64-bit handle storage (fits all handles)
- Header separation from heavy D3D12/Vulkan headers
- Minimal memory footprint (12 bytes per instance)

**Weaknesses**:
- No thread safety for concurrent writes
- No value validation (only type validation)
- No automatic handle management
- No comparison operators
- No hash functions
- No serialization/deserialization functions
- No duplication functions
- No invalidation functions
- No debugging functions
- No handle lifecycle management

**Recommendations**:
1. Consider adding thread safety for concurrent writes using atomic operations
2. Consider adding value validation functions
3. Consider adding automatic handle management
4. Consider adding comparison operators
5. Consider adding hash functions
6. Consider adding serialization/deserialization functions
7. Consider adding duplication functions
8. Consider adding invalidation functions
9. Consider adding debugging functions
10. Consider adding handle lifecycle management
