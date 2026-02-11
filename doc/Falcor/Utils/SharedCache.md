# SharedCache - Shared Cache Management

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- std::functional (std::function)
- std::map (std::map)
- std::memory (std::shared_ptr, std::weak_ptr)
- std::mutex (std::mutex, std::lock_guard)

### Dependent Modules

- Falcor/Core (Core resource management)
- Falcor/Rendering (Rendering resource management)
- Falcor/Scene (Scene resource management)

## Module Overview

SharedCache provides a helper class for managing a shared cache of resources. It's designed to replace global statics that don't work with multiple device support. The cache allows multiple instances to share a resource, with automatic cleanup when all instances are destroyed. It uses a key type to cache multiple versions of the same data, typically used to cache one set for every GPU device instance.

### Key Characteristics

- **Shared Resource Management**: Manages shared resources across instances
- **Automatic Cleanup**: Automatically releases resources when all instances are destroyed
- **Thread-Safe**: Thread-safe using mutex
- **Key-Based Caching**: Uses key type to cache multiple versions
- **Weak Pointer Storage**: Uses weak_ptr for cached items
- **Lazy Initialization**: Initializes resources on first access
- **Device-Specific Caching**: Typically used for GPU device-specific resources
- **Replacement for Globals**: Replaces global statics for multi-device support

### Architecture Principles

1. **Shared Ownership**: Shared ownership across instances
2. **Automatic Cleanup**: Automatic resource cleanup
3. **Thread Safety**: Thread-safe operations
4. **Key-Based Caching**: Key-based resource caching
5. **Lazy Initialization**: Initialize on first access

## Component Specifications

### SharedCache Struct

**File**: `Source/Falcor/Utils/SharedCache.h` (lines 51-73)

```cpp
template<typename T, typename Key>
struct SharedCache
{
    std::shared_ptr<T> acquire(Key key, const std::function<std::shared_ptr<T>()>& init);

    std::mutex mutex;
    std::map<Key, std::weak_ptr<T>> cache;
};
```

**Purpose**: Helper class for managing a shared cache

**Template Parameters**:
- **T**: Type of cached resource
- **Key**: Key type for caching multiple versions

**Members**:
- **mutex**: Mutex for thread-safe access
- **cache**: Map of weak pointers to cached resources

### acquire() Method

**File**: `Source/Falcor/Utils/SharedCache.h` (lines 54-69)

```cpp
std::shared_ptr<T> acquire(Key key, const std::function<std::shared_ptr<T>()>& init)
{
    std::lock_guard<std::mutex> lock(mutex);
    auto it = cache.find(key);
    if (it != cache.end())
    {
        if (auto data = it->second.lock())
            return data;
        else
            cache.erase(it);
    }

    std::shared_ptr<T> data = init();
    cache.emplace(key, data);
    return data;
}
```

**Purpose**: Acquire shared resource from cache

**Parameters**:
- key: Key for cached resource
- init: Initialization function that creates resource

**Returns**: std::shared_ptr<T> to shared resource

**Behavior**:
1. Lock mutex for thread-safe access
2. Search cache for key
3. If found:
   - Try to lock weak pointer
   - If lock succeeds, return shared pointer
   - If lock fails, erase entry
4. If not found:
   - Call init() to create resource
   - Insert into cache with weak pointer
5. Return shared pointer

**Algorithm**:
```
acquire(key, init):
    1. Lock mutex
    2. Find key in cache
    3. If found:
       a. Try to lock weak pointer
       b. If lock succeeds:
          - Return shared pointer
       c. If lock fails:
          - Erase entry
          - Continue to step 4
    4. If not found:
       a. Call init() to create resource
       b. Insert into cache with weak pointer
    5. Return shared pointer
    6. Unlock mutex (RAII)
```

## Technical Details

### Shared Resource Management

Uses shared_ptr for shared ownership:

```cpp
std::shared_ptr<T> data = init();
cache.emplace(key, data);
```

**Characteristics**:
- Shared ownership across instances
- Automatic reference counting
- Automatic cleanup when last reference is released
- Thread-safe reference counting

### Weak Pointer Storage

Uses weak_ptr for cache storage:

```cpp
std::map<Key, std::weak_ptr<T>> cache;
```

**Characteristics**:
- Weak pointer doesn't keep resource alive
- Allows automatic cleanup when all instances are destroyed
- Prevents memory leaks
- Thread-safe weak pointer operations

### Thread Safety

Uses mutex for thread-safe access:

```cpp
std::mutex mutex;
std::lock_guard<std::mutex> lock(mutex);
```

**Characteristics**:
- Mutex-protected cache access
- RAII-based locking (std::lock_guard)
- Thread-safe acquire() method
- Automatic unlock when lock goes out of scope

### Key-Based Caching

Uses key type to cache multiple versions:

```cpp
template<typename T, typename Key>
struct SharedCache
{
    // ...
    std::map<Key, std::weak_ptr<T>> cache;
};
```

**Characteristics**:
- Multiple versions of same data can be cached
- Typically used for GPU device-specific resources
- Key type can be any comparable type
- Map-based lookup (O(log n) average)

### Lazy Initialization

Initializes resources on first access:

```cpp
std::shared_ptr<T> data = init();
```

**Characteristics**:
- Resources created on first access
- Subsequent accesses reuse cached resource
- Initialization function provided by caller
- Flexible initialization logic

### Automatic Cleanup

Automatic cleanup when all instances are destroyed:

```cpp
if (auto data = it->second.lock())
    return data;
else
    cache.erase(it);
```

**Characteristics**:
- Weak pointer expires when all shared_ptrs are destroyed
- Expired entries are automatically removed from cache
- Prevents memory leaks
- Automatic resource cleanup

### Cache Lookup Algorithm

Cache lookup with automatic cleanup:

```cpp
auto it = cache.find(key);
if (it != cache.end())
{
    if (auto data = it->second.lock())
        return data;
    else
        cache.erase(it);
}
```

**Characteristics**:
- O(log n) average lookup
- Automatic cleanup of expired entries
- Thread-safe lookup
- Efficient cache management

## Integration Points

### STL Integration

- **std::functional**: std::function for initialization
- **std::map**: std::map for cache storage
- **std::memory**: std::shared_ptr, std::weak_ptr
- **std::mutex**: std::mutex for thread safety

### Internal Falcor Usage

- **Core**: Core resource management
- **Rendering**: Rendering resource management
- **Scene**: Scene resource management

## Architecture Patterns

### Shared Ownership Pattern

Uses shared_ptr for shared ownership:

```cpp
std::shared_ptr<T> data = init();
cache.emplace(key, data);
```

Benefits:
- Shared ownership across instances
- Automatic reference counting
- Automatic cleanup
- Thread-safe reference counting

### Weak Pointer Pattern

Uses weak_ptr for cache storage:

```cpp
std::map<Key, std::weak_ptr<T>> cache;
```

Benefits:
- Weak pointer doesn't keep resource alive
- Allows automatic cleanup
- Prevents memory leaks
- Thread-safe weak pointer operations

### Thread Safety Pattern

Uses mutex for thread-safe access:

```cpp
std::mutex mutex;
std::lock_guard<std::mutex> lock(mutex);
```

Benefits:
- Thread-safe cache access
- RAII-based locking
- Automatic unlock
- Exception safety

### Key-Based Caching Pattern

Uses key type to cache multiple versions:

```cpp
template<typename T, typename Key>
struct SharedCache
{
    std::map<Key, std::weak_ptr<T>> cache;
};
```

Benefits:
- Multiple versions of same data can be cached
- Device-specific caching
- Flexible key type
- Map-based lookup

### Lazy Initialization Pattern

Initializes resources on first access:

```cpp
std::shared_ptr<T> data = init();
```

Benefits:
- Resources created on first access
- Subsequent accesses reuse cached resource
- Flexible initialization logic
- No pre-initialization overhead

## Code Patterns

### Thread Safety Pattern

RAII-based locking:

```cpp
std::lock_guard<std::mutex> lock(mutex);
// ... access protected resources ...
// Automatic unlock when lock goes out of scope
```

Pattern:
- Lock mutex on construction
- Automatic unlock on destruction
- Exception safety
- No manual unlock needed

### Cache Lookup Pattern

Cache lookup with automatic cleanup:

```cpp
auto it = cache.find(key);
if (it != cache.end())
{
    if (auto data = it->second.lock())
        return data;
    else
        cache.erase(it);
}
```

Pattern:
- Find key in cache
- Try to lock weak pointer
- Return if lock succeeds
- Erase if lock fails
- Continue to initialization if not found

### Lazy Initialization Pattern

Initialize on first access:

```cpp
std::shared_ptr<T> data = init();
cache.emplace(key, data);
```

Pattern:
- Call initialization function
- Create shared resource
- Insert into cache with weak pointer
- Return shared pointer

### Weak Pointer Pattern

Use weak_ptr for cache storage:

```cpp
std::map<Key, std::weak_ptr<T>> cache;
```

Pattern:
- Store weak pointer in cache
- Try to lock when accessing
- Erase if lock fails
- Prevents memory leaks

## Use Cases

### Basic Usage

```cpp
struct Resource
{
    int data;
};

SharedCache<Resource, int> cache;

// Acquire resource
auto resource1 = cache.acquire(0, []() { return std::make_shared<Resource>(42); });
auto resource2 = cache.acquire(0, []() { return std::make_shared<Resource>(42); });

// resource1 and resource2 point to the same Resource instance
```

### Device-Specific Caching

```cpp
struct GPUResource
{
    void* deviceMemory;
};

SharedCache<GPUResource, int> cache;

// Acquire resource for device 0
auto resource0 = cache.acquire(0, []() { return std::make_shared<GPUResource>(); });

// Acquire resource for device 1
auto resource1 = cache.acquire(1, []() { return std::make_shared<GPUResource>(); });

// resource0 and resource1 are different instances
```

### Custom Initialization

```cpp
struct Resource
{
    int data;
};

SharedCache<Resource, int> cache;

// Custom initialization
auto resource = cache.acquire(0, []() {
    std::cout << "Initializing resource" << std::endl;
    return std::make_shared<Resource>(42);
});
```

### Multiple Instances

```cpp
struct Resource
{
    int data;
};

class MyClass
{
public:
    MyClass()
    {
        // All MyClass instances share the same resource for key 0
        mResource = mCache.acquire(0, []() { return std::make_shared<Resource>(42); });
    }

private:
    std::shared_ptr<Resource> mResource;
    static SharedCache<Resource, int> mCache;
};

MyClass obj1;
MyClass obj2;
// obj1.mResource and obj2.mResource point to the same Resource instance
```

### Automatic Cleanup

```cpp
struct Resource
{
    ~Resource() { std::cout << "Resource destroyed" << std::endl; }
    int data;
};

{
    SharedCache<Resource, int> cache;
    auto resource1 = cache.acquire(0, []() { return std::make_shared<Resource>(42); });
    auto resource2 = cache.acquire(0, []() { return std::make_shared<Resource>(42); });
    // resource1 and resource2 point to the same Resource instance
}
// Output: "Resource destroyed"
```

### Thread-Safe Usage

```cpp
struct Resource
{
    int data;
};

SharedCache<Resource, int> cache;

// Thread 1
std::thread thread1([&]() {
    auto resource = cache.acquire(0, []() { return std::make_shared<Resource>(42); });
    // Use resource
});

// Thread 2
std::thread thread2([&]() {
    auto resource = cache.acquire(0, []() { return std::make_shared<Resource>(42); });
    // Use resource
});

thread1.join();
thread2.join();
// Both threads get the same Resource instance
```

## Performance Considerations

### Memory Overhead

- **Map Overhead**: std::map overhead for cache storage
- **Weak Pointer Overhead**: std::weak_ptr overhead
- **Mutex Overhead**: std::mutex overhead
- **Shared Pointer Overhead**: std::shared_ptr overhead

### Acquisition Performance

- **Cache Lookup**: O(log n) average lookup time
- **Lock Overhead**: Mutex locking overhead
- **Weak Pointer Lock**: Weak pointer lock overhead
- **Initialization Overhead**: Initialization function overhead

### Memory Management

- **Shared Ownership**: Automatic reference counting
- **Weak Pointer Storage**: No reference counting overhead
- **Automatic Cleanup**: Automatic cleanup when all instances destroyed
- **Memory Efficiency**: No memory leaks

### Thread Safety Performance

- **Mutex Locking**: Mutex locking on every acquire()
- **Lock Contention**: Potential lock contention in multithreaded apps
- **RAII Overhead**: RAII-based locking overhead
- **Exception Safety**: Exception safety overhead

### Compiler Optimizations

- **Inline Expansion**: Methods can be inlined
- **Constant Propagation**: Constants can be propagated
- **Dead Code Elimination**: Unused code can be eliminated
- **Template Instantiation**: Template instantiation overhead

### Comparison with Alternatives

**SharedCache vs Global Statics**:
- SharedCache: Multi-device support, automatic cleanup, thread-safe
- Global Statics: Single device only, no automatic cleanup, not thread-safe
- Performance: SharedCache has minimal overhead

**SharedCache vs std::shared_ptr**:
- SharedCache: Key-based caching, automatic cleanup
- std::shared_ptr: No caching, no automatic cleanup
- Performance: SharedCache has lookup overhead

### Optimization Tips

1. **Reuse Resources**: Reuse resources when possible
2. **Minimize Lock Contention**: Minimize lock contention in multithreaded apps
3. **Use Appropriate Key Type**: Use appropriate key type for caching
4. **Enable Optimizations**: Enable compiler optimizations
5. **Profile**: Profile to identify bottlenecks

## Limitations

### Feature Limitations

- **No Cache Invalidation**: No method to invalidate cache entries
- **No Cache Clearing**: No method to clear cache
- **No Cache Size**: No method to get cache size
- **No Cache Iteration**: No method to iterate over cache
- **No Cache Statistics**: No cache statistics
- **No Cache Eviction**: No cache eviction policy (LRU, LFU, etc.)
- **No Cache Limits**: No cache size limits
- **No Cache Monitoring**: No cache monitoring

### API Limitations

- **No Erase Method**: No method to erase cache entries
- **No Clear Method**: No method to clear cache
- **No Size Method**: No method to get cache size
- **No Empty Method**: No method to check if cache is empty
- **No Contains Method**: No method to check if key exists
- **No Keys Method**: No method to get all keys
- **No Values Method**: No method to get all values

### Type Limitations

- **Comparable Key**: Key type must be comparable (for std::map)
- **Copyable Key**: Key type must be copyable
- **Copyable T**: T type must be copyable into std::shared_ptr
- **Movable T**: T type must be movable into std::shared_ptr

### Performance Limitations

- **Lock Contention**: Potential lock contention in multithreaded apps
- **Lookup Overhead**: O(log n) average lookup time
- **Weak Pointer Lock**: Weak pointer lock overhead
- **Initialization Overhead**: Initialization function overhead

### Platform Limitations

- **C++11 Only**: Requires C++11 or later
- **No C++20 Features**: No C++20 features (concepts, ranges, etc.)
- **Standard Library**: Uses standard library only

## Best Practices

### When to Use

**Appropriate Use Cases**:
- Replacing global statics for multi-device support
- Sharing resources across instances
- Device-specific resource caching
- Thread-safe resource management
- Automatic resource cleanup

**Inappropriate Use Cases**:
- High-performance caching (use specialized cache)
- Complex caching policies (use specialized cache)
- Cache eviction (use specialized cache)
- Cache statistics (use specialized cache)

### Usage Patterns

**Basic Usage**:
```cpp
SharedCache<Resource, int> cache;
auto resource = cache.acquire(0, []() { return std::make_shared<Resource>(42); });
```

**Device-Specific Caching**:
```cpp
SharedCache<Resource, int> cache;
auto resource0 = cache.acquire(0, []() { return std::make_shared<Resource>(); });
auto resource1 = cache.acquire(1, []() { return std::make_shared<Resource>(); });
```

**Custom Initialization**:
```cpp
auto resource = cache.acquire(0, []() {
    // Custom initialization logic
    return std::make_shared<Resource>(42);
});
```

### Thread Safety

- **Use Mutex**: Use mutex for thread-safe access
- **Use RAII**: Use RAII-based locking (std::lock_guard)
- **Minimize Lock Time**: Minimize lock time to reduce contention
- **Avoid Deadlocks**: Avoid deadlocks by consistent lock ordering

### Memory Management

- **Use Shared Pointer**: Use std::shared_ptr for shared ownership
- **Use Weak Pointer**: Use std::weak_ptr for cache storage
- **Automatic Cleanup**: Rely on automatic cleanup
- **Avoid Memory Leaks**: Weak pointers prevent memory leaks

### Performance Tips

1. **Reuse Resources**: Reuse resources when possible
2. **Minimize Lock Contention**: Minimize lock contention in multithreaded apps
3. **Use Appropriate Key Type**: Use appropriate key type for caching
4. **Enable Optimizations**: Enable compiler optimizations
5. **Profile**: Profile to identify bottlenecks

## Implementation Notes

### Shared Resource Management

Uses shared_ptr for shared ownership:

```cpp
std::shared_ptr<T> data = init();
cache.emplace(key, data);
```

**Characteristics**:
- Shared ownership across instances
- Automatic reference counting
- Automatic cleanup when last reference is released
- Thread-safe reference counting

### Weak Pointer Storage

Uses weak_ptr for cache storage:

```cpp
std::map<Key, std::weak_ptr<T>> cache;
```

**Characteristics**:
- Weak pointer doesn't keep resource alive
- Allows automatic cleanup when all instances are destroyed
- Prevents memory leaks
- Thread-safe weak pointer operations

### Thread Safety Implementation

Uses mutex for thread-safe access:

```cpp
std::mutex mutex;
std::lock_guard<std::mutex> lock(mutex);
```

**Characteristics**:
- Mutex-protected cache access
- RAII-based locking (std::lock_guard)
- Thread-safe acquire() method
- Automatic unlock when lock goes out of scope

### Key-Based Caching Implementation

Uses key type to cache multiple versions:

```cpp
template<typename T, typename Key>
struct SharedCache
{
    std::map<Key, std::weak_ptr<T>> cache;
};
```

**Characteristics**:
- Multiple versions of same data can be cached
- Typically used for GPU device-specific resources
- Key type can be any comparable type
- Map-based lookup (O(log n) average)

### Lazy Initialization Implementation

Initializes resources on first access:

```cpp
std::shared_ptr<T> data = init();
```

**Characteristics**:
- Resources created on first access
- Subsequent accesses reuse cached resource
- Initialization function provided by caller
- Flexible initialization logic

### Automatic Cleanup Implementation

Automatic cleanup when all instances are destroyed:

```cpp
if (auto data = it->second.lock())
    return data;
else
    cache.erase(it);
```

**Characteristics**:
- Weak pointer expires when all shared_ptrs are destroyed
- Expired entries are automatically removed from cache
- Prevents memory leaks
- Automatic resource cleanup

## Future Enhancements

### Potential Improvements

1. **Cache Invalidation**: Add method to invalidate cache entries
2. **Cache Clearing**: Add method to clear cache
3. **Cache Size**: Add method to get cache size
4. **Cache Iteration**: Add method to iterate over cache
5. **Cache Statistics**: Add cache statistics
6. **Cache Eviction**: Add cache eviction policy (LRU, LFU, etc.)
7. **Cache Limits**: Add cache size limits
8. **Cache Monitoring**: Add cache monitoring
9. **Cache Profiling**: Add cache profiling
10. **Cache Debugging**: Add cache debugging support

### API Extensions

1. **erase()**: Add method to erase cache entries
2. **clear()**: Add method to clear cache
3. **size()**: Add method to get cache size
4. **empty()**: Add method to check if cache is empty
5. **contains()**: Add method to check if key exists
6. **keys()**: Add method to get all keys
7. **values()**: Add method to get all values
8. **get()**: Add method to get cache entry without initialization
9. **try_acquire()**: Add method to try acquiring without initialization
10. **update()**: Add method to update cache entry

### Performance Enhancements

1. **Lock-Free**: Consider lock-free implementation
2. **Read-Write Locks**: Use read-write locks for better concurrency
3. **Cache Hints**: Add cache hints for optimization
4. **Thread-Local Cache**: Add thread-local cache for better performance
5. **Batch Operations**: Add batch operations for better performance

### Documentation Enhancements

1. **More Examples**: Add more usage examples
2. **Performance Guide**: Add performance optimization guide
3. **Best Practices**: Add best practices guide
4. **Migration Guide**: Add migration guide from global statics
5. **API Reference**: Add complete API reference

## References

### C++ Standard

- **std::shared_ptr**: C++11 shared pointer
- **std::weak_ptr**: C++11 weak pointer
- **std::mutex**: C++11 mutex
- **std::lock_guard**: C++11 RAII lock guard
- **std::map**: C++11 map container
- **std::function**: C++11 function wrapper

### Falcor Documentation

- **Falcor API**: Falcor API documentation
- **Falcor Utils**: Utils module documentation

### Related Technologies

- **Shared Ownership**: Shared ownership in C++
- **Weak Pointers**: Weak pointers in C++
- **Thread Safety**: Thread safety in C++
- **Caching**: Caching algorithms and patterns
