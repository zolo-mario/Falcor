# QueryHeap

## Overview
[`QueryHeap`](Source/Falcor/Core/API/QueryHeap.h) provides a query heap management system for GPU queries (timestamp, occlusion, pipeline statistics). This module manages query allocation, deallocation, and synchronization.

## Source Files
- Header: [`Source/Falcor/Core/API/QueryHeap.h`](Source/Falcor/Core/API/QueryHeap.h) (99 lines)
- Implementation: [`Source/Falcor/Core/API/QueryHeap.cpp`](Source/Falcor/Core/API/QueryHeap.cpp) (not shown, likely exists)

## Class Hierarchy
```
Object (base class)
  └── QueryHeap
```

## Dependencies

### Direct Dependencies
- [`fwd.h`](Source/Falcor/Core/API/fwd.h) - Forward declarations
- [`Handles.h`](Source/Falcor/Core/API/Handles.h) - Handle types
- [`Core/Macros.h`](Source/Falcor/Core/Macros.h) - Macros (FALCOR_OBJECT, FALCOR_API, FALCOR_ASSERT)
- [`Core/Error.h`](Source/Falcor/Core/Error.h) - Error handling (FALCOR_ASSERT)
- [`Core/Object.h`](Source/Falcor/Core/Object.h) - Base class
- [`deque`](Source/Falcor/Core/API/deque) - Deque type (std::deque)

### Indirect Dependencies
- Device class (from fwd.h)
- Handle types (from Handles.h)
- Object base class (from Core/Object.h)
- Slang GFX types (from slang-gfx.h)
- Standard C++ types (std::deque)

## Known Issues and Limitations

1. **Thread Safety**: Not thread-safe for concurrent access
2. **No Query Validation**: No validation of query indices
3. **No Query State Tracking**: No tracking of query state (active, inactive, etc.)
4. **No Query Result Retrieval**: No methods to retrieve query results
5. **No Query Synchronization**: No methods to synchronize queries
6. **No Query Reset**: No methods to reset queries
7. **No Query Pool Reset**: No methods to reset query pool
8. **No Query Pool Flush**: No methods to flush query pool
9. **No Query Pool Invalidation**: No methods to invalidate query pool
10. **Poor Cache Locality**: Members scattered across multiple cache lines

## Usage Example

```cpp
// Create a timestamp query heap
ref<Device> pDevice = /* ... */;
ref<QueryHeap> pQueryHeap = QueryHeap::create(pDevice, QueryHeap::Type::Timestamp, 100);

// Allocate a query
uint32_t queryIndex = pQueryHeap->allocate();
if (queryIndex != QueryHeap::kInvalidIndex)
{
    // Query allocated successfully
}

// Release a query
pQueryHeap->release(queryIndex);

// Get query pool
gfx::IQueryPool* pQueryPool = pQueryHeap->getGfxQueryPool();

// Get query count
uint32_t queryCount = pQueryHeap->getQueryCount();

// Get query type
QueryHeap::Type type = pQueryHeap->getType();

// Break strong reference to device
pQueryHeap->breakStrongReferenceToDevice();
```

## Conclusion

QueryHeap provides a simple and efficient query heap management system for GPU queries. The implementation is simple with O(1) allocation and release operations, but suffers from poor cache locality due to scattered member layout.

**Strengths**:
- Simple and efficient query heap management
- O(1) allocation and release operations
- Support for multiple query types (Timestamp, Occlusion, PipelineStats)
- Cross-platform support (D3D12 and Vulkan via Slang GFX)
- BreakableReference pattern to avoid circular references
- Type-safe query type enum
- No dynamic memory allocation for allocation/release
- Efficient free list management using deque
- Constant time allocation from free list

**Weaknesses**:
- Not thread-safe for concurrent access
- No query validation
- No query state tracking
- No query result retrieval
- No query synchronization
- No query reset
- No query pool reset
- No query pool flush
- No query pool invalidation
- Poor cache locality (members scattered across multiple cache lines)
- Limited query types (only 3 types)

**Recommendations**:
1. Consider adding thread safety using mutex or atomic operations
2. Consider adding query validation
3. Consider adding query state tracking
4. Consider adding query result retrieval methods
5. Consider adding query synchronization methods
6. Consider adding query reset methods
7. Consider adding query pool reset methods
8. Consider adding query pool flush methods
9. Consider adding query pool invalidation methods
10. Consider grouping frequently accessed members to improve cache locality
