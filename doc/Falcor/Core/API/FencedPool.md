# FencedPool

## Overview
[`FencedPool`](Source/Falcor/Core/API/FencedPool.h:38) is a template class that manages a pool of objects with fence-based synchronization. It provides efficient object reuse by retiring objects when they are no longer in use (based on fence values) and reusing them when they become available.

## Source Files
- Header: [`Source/Falcor/Core/API/FencedPool.h`](Source/Falcor/Core/API/FencedPool.h) (110 lines)
- Implementation: Header-only (no separate .cpp file)

## Class Hierarchy
```
Object (base class)
  └── FencedPool<ObjectType> (template)
```

## Dependencies

### Direct Dependencies
- [`Fence.h`](Source/Falcor/Core/API/Fence.h) - Fence types
- [`Core/Macros.h`](Source/Falcor/Core/Macros.h) - Macros
- [`Core/Error.h`](Source/Falcor/Core/Error.h) - Error handling
- [`Core/Object.h`](Source/Falcor/Core/Object.h) - Base class

### Indirect Dependencies
- Object base class (from Core/Object.h)
- Fence class (from Fence.h)
- ref<> smart pointer (from Core/Object.h)

## Known Issues and Limitations

1. **Thread Safety**: Not thread-safe for concurrent access
2. **No Bounds Checking**: No bounds checking for queue operations
3. **No Validation**: No validation of object creation function
4. **No Pool Size Limit**: No limit on the number of objects in the pool
5. **No Leak Detection**: Does not detect memory leaks
6. **No Allocation Statistics**: Does not track allocation statistics
7. **No Custom Allocator**: Uses default allocator for queue
8. **No Object Destruction**: Does not destroy objects when clearing the queue
9. **Single Active Object**: Only maintains one active object at a time
10. **No Object Reuse Strategy**: Simple FIFO queue, no LRU or other strategies

## Usage Example

```cpp
// Define object creation function
MyObject* createMyObject(void* pUserData)
{
    return new MyObject();
}

// Create a fenced pool
ref<Fence> pFence = pDevice->createFence(FenceDesc{});
ref<FencedPool<MyObject>> pPool = FencedPool<MyObject>::create(pFence, createMyObject);

// Get a new object
MyObject* pObj = pPool->newObject();

// Use the object
// <use pObj>

// Get another object (retires the previous one)
MyObject* pObj2 = pPool->newObject();

// The previous object (pObj) is now retired and will be reused when the fence is signaled
```

## Conclusion

FencedPool provides an efficient fence-based object pooling mechanism. The implementation is straightforward with good cache locality but lacks thread safety.

**Strengths**:
- Efficient fence-based object pooling
- Good cache locality (members fit in a few cache lines)
- Lightweight construction and operations
- Template design for type safety
- Automatic object reuse to reduce allocation overhead
- FIFO queue for fair object reuse
- Automatic resource management using smart pointers

**Weaknesses**:
- Not thread-safe for concurrent access
- No bounds checking for queue operations
- No validation of object creation function
- No pool size limit
- No leak detection
- No allocation statistics
- Uses default allocator for queue
- Does not destroy objects when clearing the queue
- Only maintains one active object at a time
- Simple FIFO queue, no LRU or other strategies

**Recommendations**:
1. Add thread safety using mutex or lock-free data structures
2. Add bounds checking for queue operations
3. Add validation for object creation function
4. Add pool size limit to prevent unbounded growth
5. Add leak detection to track unreleased objects
6. Add allocation statistics for performance monitoring
7. Consider using a custom allocator for the queue
8. Consider destroying objects when clearing the queue
9. Consider supporting multiple active objects
10. Consider implementing LRU or other reuse strategies
11. Reorder members to improve cache locality
12. Consider using a lock-free queue for better performance in multi-threaded scenarios
