# Core Utilities - Fundamental Utility Components

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **AlignedAllocator** - GPU-aligned memory allocation
- [x] **Dictionary** - Key-value storage
- [x] **IndexedVector** - Vector with stable indices
- [x] **fast_vector** - Fast vector container
- [x] **BufferAllocator** - GPU buffer allocation and management
- [x] **TaskManager** - Task scheduling and execution

### External Dependencies

- **Core/Error** - Error handling (Exception)
- **Core/API** - Graphics API (Device, Buffer)
- **Utils/Math** - Mathematical utilities (Common, isPowerOf2)
- **BS_thread_pool** - Thread pool (BS::thread_pool_light)

## Module Overview

These core utility components provide fundamental infrastructure for the Falcor rendering framework. They handle memory allocation, data structures, task scheduling, and GPU resource management. These utilities are used throughout the framework for efficient memory management, parallel execution, and GPU-CPU synchronization.

## Component Specifications

### AlignedAllocator

**File**: [`AlignedAllocator.h`](Source/Falcor/Utils/AlignedAllocator.h:1)

**Purpose**: GPU-aligned memory allocation for managing GPU buffer allocations with alignment requirements.

**Key Features**:
- Custom alignment requirements (minimum byte alignment)
- Cache line size control to prevent objects from spanning cache lines
- Template-based allocation with arbitrary constructor arguments
- Padding support for alignment enforcement
- Buffer management with reserve and resize
- Start pointer access for CPU-side buffer operations

**Alignment Parameters**:
- **mMinAlignment** - Minimum alignment in bytes (default: 16)
- **mCacheLineSize** - Cache line size in bytes (default: 128)

**Core Methods**:
- [`setMinimumAlignment()`](Source/Falcor/Utils/AlignedAllocator.h:56) - Set minimum alignment (must be power of 2 or 0)
- [`setCacheLineSize()`](Source/Falcor/Utils/AlignedAllocator.h:68) - Set cache line size (must be power of 2 or 0)
- [`allocate()`](Source/Falcor/Utils/AlignedAllocator.h:80) - Allocate object with constructor arguments
- [`allocateSized()`](Source/Falcor/Utils/AlignedAllocator.h:96) - Allocate object with size and constructor
- [`reserve()`](Source/Falcor/Utils/AlignedAllocator.h:104) - Reserve capacity
- [`resize()`](Source/Falcor/Utils/AlignedAllocator.h:106) - Resize buffer (clears data)
- [`getStartPointer()`](Source/Falcor/Utils/AlignedAllocator.h:111) - Get mutable start pointer
- [`getStartPointer()`](Source/Falcor/Utils/AlignedAllocator.h:112) - Get const start pointer
- [`offsetOf()`](Source/Falcor/Utils/AlignedAllocator.h:117) - Get offset of pointer from start
- [`reset()`](Source/Falcor/Utils/AlignedAllocator.h:123) - Clear buffer
- [`getSize()`](Source/Falcor/Utils/AlignedAllocator.h:125) - Get current size
- [`getCapacity()`](Source/Falcor/Utils/AlignedAllocator.h:126) - Get current capacity

**Internal Methods**:
- [`computeAndAllocatePadding()`](Source/Falcor/Utils/AlignedAllocator.h:129) - Compute padding and alignment
- [`allocInternal()`](Source/Falcor/Utils/AlignedAllocator.h:158) - Internal allocation with iterator insertion

**Members**:
- **mMinAlignment** - Minimum alignment requirement
- **mCacheLineSize** - Cache line size for alignment
- **mBuffer** - Underlying vector for allocations

**Alignment Strategy**:
- Padding to minimum alignment if not at minimum
- Cache line alignment to prevent cross-line allocations
- Power-of-2 alignment requirement
- Zero alignment disables alignment enforcement

### Dictionary

**File**: [`Dictionary.h`](Source/Falcor/Utils/Dictionary.h:1)

**Purpose**: Type-safe key-value storage with std::any values.

**Key Features**:
- Type-safe value storage using std::any
- String-based key lookup
- Type-safe value retrieval with template methods
- Key existence checking
- Default value support for missing keys
- Exception throwing for missing keys

**Value Class** (from [`Value`](Source/Falcor/Utils/Dictionary.h:40)):
- **Value()** - Default constructor (empty any)
- **Value(std::any&)** - Constructor with value
- **operator=(T)** - Assignment operator for type T
- **operator T()** - Cast operator to type T

**Core Methods**:
- [`operator[]()`](Source/Falcor/Utils/Dictionary.h:67) - Get value by key (non-const, throws if missing)
- [`operator[]()`](Source/Falcor/Utils/Dictionary.h:68) - Get value by key (const, throws if missing)
- [`begin()`](Source/Falcor/Utils/Dictionary.h:70) - Iterator begin
- [`end()`](Source/Falcor/Utils/Dictionary.h:71) - Iterator end
- [`keyExists()`](Source/Falcor/Utils/Dictionary.h:79) - Check if key exists
- [`getValue()`](Source/Falcor/Utils/Dictionary.h:83) - Get value by key (throws if missing)
- [`getValue()`](Source/Falcor/Utils/Dictionary.h:92) - Get value by key with default
- [`size()`](Source/Falcor/Utils/Dictionary.h:76) - Get number of entries

**Members**:
- **mContainer** - std::unordered_map<std::string, Value> for storage

**Container Type**: std::unordered_map<std::string, Value>

### IndexedVector

**File**: [`IndexedVector.h`](Source/Falcor/Utils/IndexedVector.h:1)

**Purpose**: Convert vector of possibly duplicate items to a vector of unique data items with stable indices.

**Key Features**:
- Duplicate detection and elimination
- Stable index assignment
- Hash-based lookup for uniqueness
- Efficient append operations
- Index-based access to unique items
- Template-based design with hash function

**Core Methods**:
- [`append()`](Source/Falcor/Utils/IndexedVector.h:54) - Append data item (returns index)
- [`append()`](Source/Falcor/Utils/IndexedVector.h:67) - Append data item with index output
- [`getValues()`](Source/Falcor/Utils/IndexedVector.h:88) - Get span of unique values
- [`getIndices()`](Source/Falcor/Utils/IndexedVector.h:93) - Get span of indices

**Template Parameters**:
- **T** - Underlying data type
- **I** - Index value type
- **H** - Hash object for determining item equivalence
- **E** - Equality function (default: std::equal_to<T>)

**Members**:
- **mIndexMap** - std::unordered_map<T, I, H, E> for lookup
- **mValues** - std::vector<T> for unique values
- **mIndices** - std::vector<I> for indices

### fast_vector

**File**: [`fast_vector.h`](Source/Falcor/Utils/fast_vector.h:1)

**Purpose**: Drop-in replacement for STL vector that avoids global lock on every push_back.

**Key Features**:
- No global lock on push_back (unlike MSVC STL vector)
- Stack allocation for small vectors
- Fallback to heap for large sizes
- Move semantics support
- Iterator interface compatible with std::vector
- Debug mode performance improvement

**Core Methods**:
- [`reserve()`](Source/Falcor/Utils/fast_vector.h:73) - Reserve capacity
- [`resize()`](Source/Falcor/Utils/fast_vector.h:75) - Resize buffer (clears data)
- [`push_back()`](Source/Falcor/Utils/fast_vector.h:89) - Push element (with growth)
- [`assign()`](Source/Falcor/Utils/fast_vector.h:53) - Assign from other fast_vector
- [`assign()`](Source/Falcor/Utils/fast_vector.h:59) - Assign from other fast_vector
- [`operator=(fast_vector&&)`](Source/Falcor/Utils/fast_vector.h:49) - Move constructor
- [`operator=(const fast_vector&)`](Source/Falcor/Utils/fast_vector.h:53) - Copy assignment
- [`operator=(fast_vector&&)`](Source/Falcor/Utils/fast_vector.h:59) - Move assignment
- [`grow()`](Source/Falcor/Utils/fast_vector.h:123) - Grow buffer to required size
- [`resize()`](Source/Falcor/Utils/fast_vector.h:81) - Resize with capacity preservation
- [`operator std::vector<T>()`](Source/Falcor/Utils/fast_vector.h:104) - Convert to std::vector
- [`size()`](Source/Falcor/Utils/fast_vector.h:106) - Get size
- [`capacity()`](Source/Falcor/Utils/fast_vector.h:107) - Get capacity
- [`empty()`](Source/Falcor/Utils/fast_vector.h:108) - Check if empty
- [`clear()`](Source/Falcor/Utils/fast_vector.h:110) - Clear buffer
- [`begin()`](Source/Falcor/Utils/fast_vector.h:112) - Iterator begin
- [`end()`](Source/Falcor/Utils/fast_vector.h:113) - Iterator end
- [`data()`](Source/Falcor/Utils/fast_vector.h:117) - Get mutable data pointer
- [`data()`](Source/Falcor/Utils/fast_vector.h:118) - Get const data pointer
- [`operator[]()`](Source/Falcor/Utils/fast_vector.h:120) - Element access (mutable)
- [`operator[]()`](Source/Falcor/Utils/fast_vector.h:121) - Element access (const)

**Internal Methods**:
- [`grow()`](Source/Falcor/Utils/fast_vector.h:124) - Grow buffer with exponential capacity growth

**Members**:
- **m_capacity** - Current capacity
- **m_size** - Current size
- **m_data** - Unique pointer to data

**Growth Strategy**:
- Exponential capacity growth (multiply by 2)
- Stack allocation for small vectors
- Heap allocation fallback for large vectors

### BufferAllocator

**File**: [`BufferAllocator.h`](Source/Falcor/Utils/BufferAllocator.h:1), [`BufferAllocator.cpp`](Source/Falcor/Utils/BufferAllocator.cpp:1)

**Purpose**: Memory management for GPU buffers with CPU-side tracking and lazy GPU buffer updates.

**Key Features**:
- CPU-side buffer tracking with dirty range management
- Lazy GPU buffer creation
- Efficient dirty range tracking (single range instead of per-object)
- Structured buffer support
- Raw buffer support
- Alignment enforcement
- Cache line size control
- Resource bind flags support

**Alignment Parameters**:
- **mAlignment** - Minimum alignment for allocations (must be power of 2 or 0)
- **mElementSize** - Element size for structured buffers (0 for raw buffer)
- **mCacheLineSize** - Cache line size (default: 128)

**Core Methods**:
- [`allocate()`](Source/Falcor/Utils/BufferAllocator.h:75) - Allocate memory region
- [`allocate()`](Source/Falcor/Utils/BufferAllocator.h:83) - Allocate array of elements
- [`pushBack()`](Source/Falcor/Utils/BufferAllocator.h:94) - Allocate and copy object
- [`emplaceBack()`](Source/Falcor/Utils/BufferAllocator.h:111) - Allocate and construct object
- [`setBlob()`](Source/Falcor/Utils/BufferAllocator.h:128) - Copy data into buffer
- [`set()`](Source/Falcor/Utils/BufferAllocator.h:136) - Set object in buffer
- [`modified()`](Source/Falcor/Utils/BufferAllocator.h:156) - Mark memory region as modified
- [`getStartPointer()`](Source/Falcor/Utils/BufferAllocator.h:174) - Get CPU buffer start pointer
- [`getStartPointer()`](Source/Falcor/Utils/BufferAllocator.h:181) - Get CPU buffer start pointer (const)
- [`clear()`](Source/Falcor/Utils/BufferAllocator.h:192) - Clear all allocations
- [`getSize()`](Source/Falcor/Utils/BufferAllocator.h:187) - Get buffer size
- [`getGPUBuffer()`](Source/Falcor/Utils/BufferAllocator.h:198) - Get GPU buffer

**Internal Methods**:
- [`computeAndAllocatePadding()`](Source/Falcor/Utils/BufferAllocator.h:201) - Compute padding and alignment
- [`allocInternal()`](Source/Falcor/Utils/BufferAllocator.h:158) - Internal allocation
- [`markAsDirty()`](Source/Falcor/Utils/BufferAllocator.h:212) - Mark range as dirty
- [`getGPUBuffer()`](Source/Falcor/Utils/BufferAllocator.cpp:79) - Get or create GPU buffer

**Members**:
- **mAlignment** - Alignment requirement
- **mElementSize** - Element size
- **mCacheLineSize** - Cache line size
- **mBindFlags** - Resource bind flags
- **mBuffer** - CPU-side buffer
- **mpGpuBuffer** - GPU-side buffer
- **mDirty** - Dirty range tracking

**Dirty Range Management**:
- Single dirty range tracking (min/max)
- Inverted range for efficient updates
- Mark entire buffer as dirty initially
- Reset to inverted range after upload

**GPU Buffer Creation**:
- Lazy creation on first access
- Structured buffer support with element count
- Raw buffer support with zero element size
- UAV counter support (optional)

### TaskManager

**File**: [`TaskManager.h`](Source/Falcor/Utils/TaskManager.h:1), [`TaskManager.cpp`](Source/Falcor/Utils/TaskManager.cpp:1)

**Purpose**: Task scheduling and execution for CPU and GPU tasks with thread pool management.

**Key Features**:
- CPU and GPU task support
- Thread pool-based parallel execution
- Task dependency management
- Exception handling and storage
- Pause/resume support
- Automatic GPU task synchronization
- Work stealing for load balancing

**Task Types**:
- **CpuTask** - CPU task function: `void(RenderContext* renderContext)`
- **GpuTask** - GPU task function: `void(RenderContext* renderContext)`

**Core Methods**:
- [`addTask()`](Source/Falcor/Utils/TaskManager.h:54) - Add CPU task
- [`addTask()`](Source/Falcor/Utils/TaskManager.h:56) - Add GPU task
- [`finish()`](Source/Falcor/Utils/TaskManager.h:61) - Unpause and wait for all tasks
- [`executeCpuTask()`](Source/Falcor/Utils/TaskManager.cpp:69) - Execute CPU task with exception handling

**Members**:
- **mThreadPool** - BS::thread_pool_light for parallel execution
- **mCurrentlyRunning** - Currently running task count (atomic)
- **mCurrentlyScheduled** - Currently scheduled task count (atomic)
- **mTaskMutex** - Mutex for task queue
- **mGpuTaskCond** - Condition variable for GPU task notification
- **mGpuTasks** - Queue of GPU tasks
- **mExceptionMutex** - Mutex for exception storage
- **mException** - Stored exception for rethrowing

**Thread Pool**:
- BS::thread_pool_light for efficient parallel execution
- Work stealing for load balancing
- Task-based parallelism

**Task Execution Flow**:
- CPU tasks: Execute immediately, no synchronization needed
- GPU tasks: Added to queue, executed in finish()
- Finish call: Waits for all tasks, executes GPU tasks, waits for completion

**Exception Handling**:
- Store exceptions in CPU tasks for rethrowing
- Thread-safe exception storage
- Automatic exception propagation

## Technical Details

### Memory Management

**AlignedAllocator Strategy**:
- Padding-based alignment enforcement
- Cache line alignment to prevent cross-line allocations
- Power-of-2 alignment requirement
- Zero alignment disables alignment

**BufferAllocator Strategy**:
- CPU-side buffer tracking for dirty range management
- Lazy GPU buffer creation
- Single dirty range instead of per-object tracking
- Efficient GPU buffer updates with range-based dirty tracking

### Data Structures

**Dictionary**:
- std::unordered_map for O(1) lookup
- std::any for type-safe value storage
- Exception throwing for missing keys
- Default value support

**IndexedVector**:
- Hash-based duplicate detection
- Stable index assignment
- Efficient append with index return
- Template-based design with configurable hash function

**fast_vector**:
- Stack allocation for small vectors
- Exponential capacity growth
- No global lock on push_back
- Move semantics support

### Task Scheduling

**TaskManager Architecture**:
- Thread pool for parallel execution
- CPU task queue for immediate execution
- GPU task queue for deferred execution
- Atomic counters for tracking
- Condition variable for GPU task notification

**Work Stealing**:
- BS::thread_pool_light work stealing
- Load balancing across threads
- Efficient task distribution

### GPU-CPU Synchronization

**BufferAllocator Sync**:
- CPU buffer tracking
- GPU buffer lazy creation
- Dirty range tracking
- Automatic buffer updates on finish()

**TaskManager Sync**:
- CPU tasks execute immediately
- GPU tasks wait for finish() call
- Condition variable notification for GPU tasks
- Exception propagation from CPU to GPU tasks

## Integration Points

### Core Integration

**Memory Management**:
- Device integration for GPU buffer creation
- Resource bind flags support
- Memory type specification (DeviceLocal, DeviceLocal with UAV)

### Threading Integration

**BS_thread_pool**:
- Thread pool for parallel execution
- Work stealing scheduler
- Efficient task distribution

### Error Handling

**Exception System**:
- Falcor::Exception base class
- Type-safe exception storage
- Automatic exception propagation

## Architecture Patterns

**Allocator Pattern**:
- Alignment-aware allocation
- Padding for alignment enforcement
- Cache line management
- CPU-side tracking with GPU lazy updates

**Container Pattern**:
- Dictionary: Key-value storage with type safety
- IndexedVector: Unique items with stable indices
- fast_vector: Performance-optimized vector

**Task Scheduling Pattern**:
- Thread pool for parallel execution
- Separate CPU and GPU task queues
- Dependency management
- Exception handling and propagation

**Lazy Evaluation Pattern**:
- GPU buffer creation on first access
- Dirty range tracking for minimal updates
- Efficient resource management

## Progress Log

- **2026-01-07T20:53:00Z**: Core utilities analysis completed. Analyzed AlignedAllocator, Dictionary, IndexedVector, fast_vector, BufferAllocator, and TaskManager components. Documented memory management strategies, data structures, task scheduling, and GPU-CPU synchronization patterns. All core utilities are now documented.

## Next Steps

Core utilities analysis is complete. All fundamental utility components have been analyzed and documented. Ready to proceed with other modules or finalize the overall analysis.
