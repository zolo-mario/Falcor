# Resource - Base GPU Resource Class

## File Location
- Header: [`Source/Falcor/Core/API/Resource.h`](Source/Falcor/Core/API/Resource.h:1)
- Implementation: [`Source/Falcor/Core/API/Resource.cpp`](Source/Falcor/Core/API/Resource.cpp:1)

## State Machine Status
**Analysis Phase**: Complete
**Last Updated**: 2026-01-07T15:33:43.962Z
**Parent Node**: API
**Current Node**: Resource

## Class Overview

The [`Resource`](Source/Falcor/Core/API/Resource.h:47) class is the abstract base class for all GPU resources in Falcor, including [`Buffer`](Source/Falcor/Core/API/Buffer.h:1) and [`Texture`](Source/Falcor/Core/API/Texture.h:1). It provides resource state tracking, view management, and serves as the foundation for all GPU memory allocations.

### Inheritance Hierarchy
```
Object (ref-counted base)
  └── Resource (abstract)
      ├── Buffer
      └── Texture
```

### Derived Classes
- [`Buffer`](Source/Falcor/Core/API/Buffer.h:1) - GPU buffer resources
- [`Texture`](Source/Falcor/Core/API/Texture.h:1) - GPU texture resources (1D, 2D, 3D, Cube, 2DMS)

## Memory Layout and Alignment

### Class Member Layout

```cpp
class Resource : public Object {
protected:
    BreakableReference<Device> mpDevice;    // 8 bytes (ptr)
    Type mType;                           // 4 bytes (enum)
    ResourceBindFlags mBindFlags;           // 4 bytes (enum)
    struct {
        bool isGlobal = true;               // 1 byte
        State global = State::Undefined;     // 4 bytes (enum)
        std::vector<State> perSubresource;   // 24 bytes (vector overhead)
    } mutable mState;                      // ~32 bytes total
    size_t mSize = 0;                     // 8 bytes
    std::string mName;                      // 24 bytes (string overhead)
    mutable SharedResourceApiHandle mSharedApiHandle = 0;  // 8 bytes
    mutable std::unordered_map<ResourceViewInfo, ref<ShaderResourceView>, ViewInfoHashFunc> mSrvs;  // ~56 bytes
    mutable std::unordered_map<ResourceViewInfo, ref<RenderTargetView>, ViewInfoHashFunc> mRtvs;  // ~56 bytes
    mutable std::unordered_map<ResourceViewInfo, ref<DepthStencilView>, ViewInfoHashFunc> mDsvs;  // ~56 bytes
    mutable std::unordered_map<ResourceViewInfo, ref<UnorderedAccessView>, ViewInfoHashFunc> mUavs;  // ~56 bytes
};
```

### Memory Alignment Analysis

**Total Estimated Size**: ~360-400 bytes (excluding virtual table and base class)

**Alignment Characteristics**:
- **Natural Alignment**: Most members are naturally aligned (8-byte pointers, 4-byte enums)
- **Cache Line Alignment**: NOT cache-line aligned (64 bytes), potential for false sharing
- **Padding Issues**:
  - [`mType`](Source/Falcor/Core/API/Resource.h:195) (4 bytes) + [`mBindFlags`](Source/Falcor/Core/API/Resource.h:196) (4 bytes) = 8 bytes (no padding)
  - [`mState`](Source/Falcor/Core/API/Resource.h:197) struct: [`isGlobal`](Source/Falcor/Core/API/Resource.h:199) (1 byte) + [`global`](Source/Falcor/Core/API/Resource.h:200) (4 bytes) + 3 bytes padding = 8 bytes
  - [`mSize`](Source/Falcor/Core/API/Resource.h:207) (8 bytes) naturally aligned
  - [`mName`](Source/Falcor/Core/API/Resource.h:208) (24 bytes) + [`mSharedApiHandle`](Source/Falcor/Core/API/Resource.h:209) (8 bytes) = 32 bytes (no padding)
  - Four unordered_maps: ~224 bytes total, each ~56 bytes

**Hot Path Members**:
- [`mpDevice`](Source/Falcor/Core/API/Resource.h:194): Frequently accessed for device operations
- [`mType`](Source/Falcor/Core/API/Resource.h:195): Accessed for type checking
- [`mState`](Source/Falcor/Core/API/Resource.h:197): Frequently accessed for state tracking
- View maps ([`mSrvs`](Source/Falcor/Core/API/Resource.h:211), [`mUavs`](Source/Falcor/Core/API/Resource.h:214), etc.): Accessed during view creation

### Cache Locality Assessment

**Poor Cache Locality**:
- Hot path members ([`mpDevice`](Source/Falcor/Core/API/Resource.h:194), [`mType`](Source/Falcor/Core/API/Resource.h:195), [`mState`](Source/Falcor/Core/API/Resource.h:197)) are in first cache line (good)
- View maps are at the end, spanning multiple cache lines
- [`mName`](Source/Falcor/Core/API/Resource.h:208) and [`mSharedApiHandle`](Source/Falcor/Core/API/Resource.h:209) are infrequently accessed but occupy cache lines

**Optimization Opportunities**:
1. Move view maps to separate allocation (not in hot path)
2. Group infrequently accessed members ([`mName`](Source/Falcor/Core/API/Resource.h:208), [`mSharedApiHandle`](Source/Falcor/Core/API/Resource.h:209)) together
3. Align state tracking to cache line boundary for multi-threaded access

## Threading Model

### Thread Safety Analysis

**NOT Thread-Safe**: The [`Resource`](Source/Falcor/Core/API/Resource.h:47) class is not thread-safe.

### Concurrent Access Patterns

**Single-Threaded Design**:
- Resource creation and modification assume single-threaded access
- State tracking ([`setGlobalState()`](Source/Falcor/Core/API/Resource.cpp:176), [`setSubresourceState()`](Source/Falcor/Core/API/Resource.cpp:182)) is not protected
- View creation and caching is not thread-safe

### Mutable State

**Mutable Members**:
```cpp
mutable struct {
    bool isGlobal = true;
    State global = State::Undefined;
    std::vector<State> perSubresource;
} mState;

mutable std::unordered_map<ResourceViewInfo, ref<ShaderResourceView>, ViewInfoHashFunc> mSrvs;
mutable std::unordered_map<ResourceViewInfo, ref<RenderTargetView>, ViewInfoHashFunc> mRtvs;
mutable std::unordered_map<ResourceViewInfo, ref<DepthStencilView>, ViewInfoHashFunc> mDsvs;
mutable std::unordered_map<ResourceViewInfo, ref<UnorderedAccessView>, ViewInfoHashFunc> mUavs;

mutable SharedResourceApiHandle mSharedApiHandle = 0;
```

**Thread Safety Implications**:
- [`mState`](Source/Falcor/Core/API/Resource.h:197) is mutable and can be modified from const methods
- View maps are mutable and can be modified from const methods
- No synchronization primitives protect these mutable members
- **Race Conditions**: Multiple threads calling [`getSRV()`](Source/Falcor/Core/API/Resource.h:178), [`getUAV()`](Source/Falcor/Core/API/Resource.h:179), or state methods will cause data races

### State Tracking

**Global vs Per-Subresource State**:
```cpp
struct {
    bool isGlobal = true;
    State global = State::Undefined;
    std::vector<State> perSubresource;
} mutable mState;
```

**State Transition Logic**:
- [`setGlobalState()`](Source/Falcor/Core/API/Resource.cpp:176): Sets global state, clears per-subresource tracking
- [`setSubresourceState()`](Source/Falcor/Core/API/Resource.cpp:182): Transitions to per-subresource tracking, initializes vector from global state
- [`getGlobalState()`](Source/Falcor/Core/API/Resource.cpp:144): Returns global state if valid, warns otherwise
- [`getSubresourceState()`](Source/Falcor/Core/API/Resource.cpp:157): Returns subresource state if tracking per-subresource

**Thread Safety Issues**:
- No mutex protects state transitions
- Multiple threads can corrupt [`isGlobal`](Source/Falcor/Core/API/Resource.h:199) flag
- Vector initialization in [`setSubresourceState()`](Source/Falcor/Core/API/Resource.cpp:182) can race with other state changes

### View Caching

**View Map Structure**:
```cpp
mutable std::unordered_map<ResourceViewInfo, ref<ShaderResourceView>, ViewInfoHashFunc> mSrvs;
mutable std::unordered_map<ResourceViewInfo, ref<RenderTargetView>, ViewInfoHashFunc> mRtvs;
mutable std::unordered_map<ResourceViewInfo, ref<DepthStencilView>, ViewInfoHashFunc> mDsvs;
mutable std::unordered_map<ResourceViewInfo, ref<UnorderedAccessView>, ViewInfoHashFunc> mUavs;
```

**View Creation Pattern**:
1. Check if view exists in cache
2. If not, create new view
3. Store in cache for future use

**Thread Safety Issues**:
- No synchronization on view map access
- Multiple threads can create duplicate views
- Race conditions on map insertion and lookup

### Reference Counting

**Breakable Reference to Device**:
```cpp
BreakableReference<Device> mpDevice;
```

**Cycle Prevention**:
- [`breakStrongReferenceToDevice()`](Source/Falcor/Core/API/Resource.cpp:227) breaks strong reference to device
- Prevents reference cycles between device and resources
- Called during device initialization

**Thread Safety**: Safe due to immutable reference after construction

### Threading Model Summary

| Operation | Thread Safety | Synchronization |
|-----------|--------------|----------------|
| Resource Creation | Single-threaded | None |
| State Query | Single-threaded | None |
| State Modification | Single-threaded | None |
| View Creation | Single-threaded | None |
| View Invalidation | Single-threaded | None |
| Name Setting | Single-threaded | None |
| Type Conversion | Single-threaded | None |

## Algorithmic Complexity

### State Tracking

**Global State Query**: [`getGlobalState()`](Source/Falcor/Core/API/Resource.cpp:144)
```cpp
State Resource::getGlobalState() const {
    if (mState.isGlobal == false) {
        logWarning("Resource::getGlobalState() - resource doesn't have a global state...");
        return State::Undefined;
    }
    return mState.global;
}
```
- **Time Complexity**: O(1) - Simple conditional check
- **Space Complexity**: O(1) - No additional allocation

**Subresource State Query**: [`getSubresourceState()`](Source/Falcor/Core/API/Resource.cpp:157)
```cpp
State Resource::getSubresourceState(uint32_t arraySlice, uint32_t mipLevel) const {
    const Texture* pTexture = dynamic_cast<const Texture*>(this);
    if (pTexture) {
        uint32_t subResource = pTexture->getSubresourceIndex(arraySlice, mipLevel);
        return (mState.isGlobal) ? mState.global : mState.perSubresource[subResource];
    }
    // ... warning and fallback
}
```
- **Time Complexity**: O(1) - Dynamic cast + array access
- **Space Complexity**: O(1) - No additional allocation
- **Dynamic Cast**: O(1) for single inheritance, but has runtime overhead

**Global State Set**: [`setGlobalState()`](Source/Falcor/Core/API/Resource.cpp:176)
```cpp
void Resource::setGlobalState(State newState) const {
    mState.isGlobal = true;
    mState.global = newState;
}
```
- **Time Complexity**: O(1) - Simple assignment
- **Space Complexity**: O(1) - No allocation
- **Side Effect**: Clears per-subresource tracking (vector not deallocated, just ignored)

**Subresource State Set**: [`setSubresourceState()`](Source/Falcor/Core/API/Resource.cpp:182)
```cpp
void Resource::setSubresourceState(uint32_t arraySlice, uint32_t mipLevel, State newState) const {
    const Texture* pTexture = dynamic_cast<const Texture*>(this);
    if (pTexture == nullptr) {
        logWarning("Calling Resource::setSubresourceState() on an object that is not a texture...");
        return;
    }

    // If we are transitioning from a global to local state, initialize the subresource array
    if (mState.isGlobal) {
        std::fill(mState.perSubresource.begin(), mState.perSubresource.end(), mState.global);
    }
    mState.isGlobal = false;
    mState.perSubresource[pTexture->getSubresourceIndex(arraySlice, mipLevel)] = newState;
}
```
- **Time Complexity**: O(n) where n = number of subresources (first transition only)
- **Space Complexity**: O(n) - Vector already allocated, filled on first transition
- **Initialization**: `std::fill()` is O(n) but only happens once per resource
- **Subsequent Calls**: O(1) after initialization

### View Caching

**View Hash Function**: [`ViewInfoHashFunc`](Source/Falcor/Core/API/Resource.h:144)
```cpp
struct ViewInfoHashFunc {
    std::size_t operator()(const ResourceViewInfo& v) const {
        return ((std::hash<uint32_t>()(v.firstArraySlice) ^ (std::hash<uint32_t>()(v.arraySize) << 1)) >> 1) ^
               (std::hash<uint32_t>()(v.mipCount) << 1) ^ (std::hash<uint32_t>()(v.mostDetailedMip) << 3) ^
               (std::hash<uint32_t>()(v.offset) << 5) ^ (std::hash<uint32_t>()(v.size) << 7);
    }
};
```
- **Time Complexity**: O(1) - Fixed number of hash operations
- **Collision Probability**: Low due to bit-shifting and XOR combination
- **Quality**: Good hash distribution for typical view parameters

**View Lookup**: O(1) average case (unordered_map)
**View Insertion**: O(1) average case (unordered_map)
**View Invalidation**: O(n) where n = number of views in cache

**View Invalidation**: [`invalidateViews()`](Source/Falcor/Core/API/Resource.cpp:109)
```cpp
void Resource::invalidateViews() const {
    auto invalidateAll = [](auto& vec) {
        for (const auto& item : vec)
            item.second->invalidate();
        vec.clear();
    };

    invalidateAll(mSrvs);
    invalidateAll(mUavs);
    invalidateAll(mRtvs);
    invalidateAll(mDsvs);
}
```
- **Time Complexity**: O(n) where n = total views across all maps
- **Space Complexity**: O(1) - Clears maps, doesn't deallocate memory
- **View Invalidation**: Each view's [`invalidate()`](Source/Falcor/Core/API/ResourceViews.h:1) method called (typically O(1))

### Type Conversion

**Dynamic Cast to Texture**: [`asTexture()`](Source/Falcor/Core/API/Resource.cpp:130)
```cpp
ref<Texture> Resource::asTexture() {
    FALCOR_ASSERT(this);
    return ref<Texture>(dynamic_cast<Texture*>(this));
}
```
- **Time Complexity**: O(1) - Single dynamic cast
- **Overhead**: RTTI lookup (typically 1-2 pointer dereferences)
- **Null Check**: Removed (was undefined behavior, now asserts)

**Dynamic Cast to Buffer**: [`asBuffer()`](Source/Falcor/Core/API/Resource.cpp:137)
```cpp
ref<Buffer> Resource::asBuffer() {
    FALCOR_ASSERT(this);
    return ref<Buffer>(dynamic_cast<Buffer*>(this));
}
```
- **Time Complexity**: O(1) - Single dynamic cast
- **Overhead**: RTTI lookup (typically 1-2 pointer dereferences)

### Native Handle Retrieval

**Native Handle**: [`getNativeHandle()`](Source/Falcor/Core/API/Resource.cpp:207)
```cpp
NativeHandle Resource::getNativeHandle() const {
    gfx::InteropHandle gfxNativeHandle = {};
    FALCOR_GFX_CALL(getGfxResource()->getNativeResourceHandle(&gfxNativeHandle));
#if FALCOR_HAS_D3D12
    if (mpDevice->getType() == Device::Type::D3D12)
        return NativeHandle(reinterpret_cast<ID3D12Resource*>(gfxNativeHandle.handleValue));
#endif
#if FALCOR_HAS_VULKAN
    if (mpDevice->getType() == Device::Type::Vulkan) {
        if (mType == Type::Buffer)
            return NativeHandle(reinterpret_cast<VkBuffer>(gfxNativeHandle.handleValue));
        else
            return NativeHandle(reinterpret_cast<VkImage>(gfxNativeHandle.handleValue));
    }
#endif
    return {};
}
```
- **Time Complexity**: O(1) - Single GFX call + conditional
- **Space Complexity**: O(1) - Fixed-size handle
- **Platform-Specific**: Different handle types for D3D12 vs Vulkan

**Shared API Handle**: [`getSharedApiHandle()`](Source/Falcor/Core/API/Resource.cpp:200)
```cpp
SharedResourceApiHandle Resource::getSharedApiHandle() const {
    gfx::InteropHandle handle = {};
    FALCOR_GFX_CALL(getGfxResource()->getSharedHandle(&handle));
    return (SharedResourceApiHandle)handle.handleValue;
}
```
- **Time Complexity**: O(1) - Single GFX call
- **Caching**: [`mSharedApiHandle`](Source/Falcor/Core/API/Resource.h:209) mutable but not used for caching
- **On-Demand**: Created on first call (not cached in current implementation)

### String Conversion

**Type to String**: [`to_string(Resource::Type)`](Source/Falcor/Core/API/Resource.cpp:49)
```cpp
const std::string to_string(Resource::Type type) {
    switch (type) {
        type_2_string(Buffer);
        type_2_string(Texture1D);
        // ...
    default:
        FALCOR_UNREACHABLE();
        return "";
    }
}
```
- **Time Complexity**: O(1) - Switch statement
- **Space Complexity**: O(1) - Returns string literal (no allocation)

**State to String**: [`to_string(Resource::State)`](Source/Falcor/Core/API/Resource.cpp:69)
```cpp
const std::string to_string(Resource::State state) {
    if (state == Resource::State::Common) {
        return "Common";
    }
    std::string s;
    // ... series of if statements
    return s;
}
```
- **Time Complexity**: O(n) where n = number of state enum values (up to 20)
- **Space Complexity**: O(1) - Returns string literal
- **Optimization**: Could use switch statement for O(1) lookup

## Performance Characteristics

### Memory Allocation Patterns

**State Vector Allocation**:
- Allocated on demand (when first subresource state is set)
- Size: O(n) where n = arraySize × mipLevels
- Typical sizes:
  - 2D texture (512×512, 1 array, 9 mips): 9 elements
  - 2D texture array (512×512, 6 array, 9 mips): 54 elements
  - Cube texture (512×512, 6 faces, 9 mips): 54 elements

**View Map Allocation**:
- Allocated on first view creation
- Size: O(m) where m = number of unique views
- Typical sizes: 1-10 views per resource
- Hash map overhead: ~56 bytes per map + ~24 bytes per entry

**String Allocation**:
- [`mName`](Source/Falcor/Core/API/Resource.h:208): Small string optimization (typically 24 bytes on stack)
- String literals in [`to_string()`](Source/Falcor/Core/API/Resource.cpp:49): No allocation

### Hot Path Analysis

**State Query Operations**:
- [`getGlobalState()`](Source/Falcor/Core/API/Resource.cpp:144): O(1), 1 cache line access
- [`getSubresourceState()`](Source/Falcor/Core/API/Resource.cpp:157): O(1), 1-2 cache line accesses (dynamic cast + array access)
- **Cache Misses**: Low if state vector is in L1 cache

**State Modification Operations**:
- [`setGlobalState()`](Source/Falcor/Core/API/Resource.cpp:176): O(1), 1 cache line access
- [`setSubresourceState()`](Source/Falcor/Core/API/Resource.cpp:182): O(n) first call, O(1) subsequent
- **Cache Misses**: High on first call (vector fill), low thereafter

**View Operations**:
- View lookup: O(1) average, 1-2 cache line accesses (hash map)
- View creation: O(1) average, 3-4 cache line accesses (hash map + view object)
- View invalidation: O(n), n cache line accesses (iterate all views)

### Memory Bandwidth

**State Vector Fill**: [`std::fill()`](Source/Falcor/Core/API/Resource.cpp:194)
- Bandwidth: O(n) where n = number of subresources
- Typical: 9 × 4 bytes = 36 bytes for 512×512 texture
- Cache-friendly: Sequential memory access

**View Hash Computation**: [`ViewInfoHashFunc`](Source/Falcor/Core/API/Resource.h:144)
- Bandwidth: O(1) - 5 hash operations
- CPU-bound: Hash computation is CPU-intensive

## Critical Path Analysis

### Hot Paths

1. **State Query**: [`getSubresourceState()`](Source/Falcor/Core/API/Resource.cpp:157)
   - Called frequently during resource barrier transitions
   - Dynamic cast overhead (1-2 pointer dereferences)
   - Array access (1 cache line)
   - **Optimization**: Cache texture pointer to avoid dynamic cast

2. **View Lookup**: [`getSRV()`](Source/Falcor/Core/API/Resource.h:178), [`getUAV()`](Source/Falcor/Core/API/Resource.h:179)
   - Called frequently during shader binding
   - Hash map lookup (1-2 cache lines)
   - View creation if not cached (3-4 cache lines)
   - **Optimization**: Pre-allocate views for frequently used resources

3. **State Transition**: [`setSubresourceState()`](Source/Falcor/Core/API/Resource.cpp:182)
   - Called during resource barrier transitions
   - Vector fill on first call (O(n))
   - Single assignment thereafter (O(1))
   - **Optimization**: Initialize state vector during resource creation

### Bottlenecks

1. **Dynamic Cast Overhead**: [`dynamic_cast<Texture*>`](Source/Falcor/Core/API/Resource.cpp:159)
   - RTTI lookup on every subresource state query
   - **Mitigation**: Cache texture pointer or use virtual methods

2. **View Hash Computation**: [`ViewInfoHashFunc`](Source/Falcor/Core/API/Resource.h:144)
   - 5 hash operations per view lookup
   - **Mitigation**: Use simpler hash or pre-compute hash

3. **State Vector Initialization**: [`std::fill()`](Source/Falcor/Core/API/Resource.cpp:194)
   - O(n) on first subresource state transition
   - **Mitigation**: Initialize during resource creation

## Memory Management

### Reference Counting

**Smart Pointer Usage**:
- All resources use `ref<T>` smart pointers
- Automatic reference counting via [`Object`](Source/Falcor/Core/Object.h:1) base class
- Breakable reference to device prevents cycles

**Cycle Prevention**:
```cpp
void Resource::breakStrongReferenceToDevice() {
    mpDevice.breakStrongReference();
}
```
- Breaks strong reference to device
- Called during device initialization
- Prevents memory leaks

### View Lifecycle

**View Caching**:
- Views are cached in unordered_maps
- Created on-demand and reused
- Invalidated when resource state changes

**View Invalidation**:
- [`invalidateViews()`](Source/Falcor/Core/API/Resource.cpp:109) clears all view caches
- Called when resource state changes significantly
- Views are not destroyed, just invalidated

### Shared Handles

**On-Demand Creation**:
- [`getSharedApiHandle()`](Source/Falcor/Core/API/Resource.cpp:200) creates handle on first call
- Not cached in current implementation
- Could be cached for performance

## Platform-Specific Considerations

### DirectX 12

**Native Handle**: `ID3D12Resource*`
- Single handle type for all resources
- No distinction between buffers and textures

**Shared Handle**: NT HANDLE
- Created via `getSharedHandle()`
- Can be shared across processes

### Vulkan

**Native Handle**: `VkBuffer` or `VkImage`
- Different handle types for buffers and textures
- Type check required in [`getNativeHandle()`](Source/Falcor/Core/API/Resource.cpp:207)

**Shared Handle**: Not supported in current implementation
- Returns empty handle

## Summary

### Strengths

1. **Clean Abstraction**: Unified interface for all GPU resources
2. **Efficient State Tracking**: Global vs per-subresource state with lazy initialization
3. **View Caching**: Reduces view creation overhead
4. **Reference Counting**: Automatic memory management with cycle prevention
5. **Type Safety**: Dynamic casts for type conversion with assertions

### Weaknesses

1. **Not Thread-Safe**: No synchronization on mutable members
2. **Poor Cache Locality**: View maps scattered across cache lines
3. **Dynamic Cast Overhead**: RTTI lookup on every subresource state query
4. **State Vector Initialization**: O(n) on first subresource state transition
5. **Hash Computation Overhead**: 5 hash operations per view lookup

### Optimization Recommendations

1. **Add Thread Safety**: Protect mutable members with mutexes or atomic operations
2. **Cache Texture Pointer**: Avoid dynamic cast in [`getSubresourceState()`](Source/Falcor/Core/API/Resource.cpp:157)
3. **Initialize State Vector Early**: Allocate and fill during resource creation
4. **Optimize Hash Function**: Reduce number of hash operations
5. **Separate View Maps**: Move view maps to separate allocation to improve cache locality
6. **Cache Shared Handle**: Store [`mSharedApiHandle`](Source/Falcor/Core/API/Resource.h:209) after first creation

---

*This technical specification is derived solely from static analysis of provided source code files.*
