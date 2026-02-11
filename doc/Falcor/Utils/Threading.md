# Threading - Thread Pool and Barrier Utilities

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- Falcor/Core/Macros.h (FALCOR_API, FALCOR_ASSERT, FALCOR_THROW, FALCOR_UNIMPLEMENTED)
- std::condition_variable (std::condition_variable)
- std::functional (std::function)
- std::mutex (std::mutex, std::lock_guard, std::unique_lock)
- std::thread (std::thread)
- std::cstdint (uint32_t)
- std::vector (std::vector)

### Dependent Modules

- Falcor/Core (Core utilities)
- Falcor/Utils (Internal utilities)
- Falcor/Rendering (Rendering utilities)

## Module Overview

Threading provides thread pool and barrier utilities for Falcor framework. It includes a static thread pool class for managing worker threads and a simple thread barrier class for synchronizing multiple threads.

### Key Characteristics

- **Thread Pool**: Static thread pool for task execution
- **Barrier**: Simple thread barrier for synchronization
- **Global State**: Global threading state
- **Reference Counting**: Reference counting for thread pool initialization
- **Round-Robin**: Round-robin task dispatching
- **Thread Reuse**: Reuses threads for multiple tasks
- **Hardware Concurrency**: Returns hardware thread count

### Architecture Principles

1. **Static Class**: Static class for global thread pool
2. **Reference Counting**: Reference counting for initialization
3. **Round-Robin**: Round-robin task dispatching
4. **Barrier Pattern**: Thread barrier for synchronization
5. **Generation Counter**: Generation counter for multiple barrier uses

## Component Specifications

### Threading Class

**Files**: `Threading.h` (lines 38-88), `Threading.cpp` (lines 46-103)

```cpp
class FALCOR_API Threading
{
public:
    const static uint32_t kDefaultThreadCount = 16;

    class Task
    {
    public:
        bool isRunning();
        void finish();

    private:
        Task();
        friend class Threading;
    };

    static void start(uint32_t threadCount = kDefaultThreadCount);
    static void finish();
    static void shutdown();
    static uint32_t getLogicalThreadCount() { return std::thread::hardware_concurrency(); }
    static Task dispatchTask(const std::function<void(void)>& func);
};
```

**Purpose**: Manage global thread pool for task execution

**Type Definitions**:
```cpp
const static uint32_t kDefaultThreadCount = 16;
```

**kDefaultThreadCount**: Default thread count (16)

#### Task Class

**Task Class**:
```cpp
class Task
{
public:
    bool isRunning();
    void finish();

private:
    Task();
    friend class Threading;
};
```

**Purpose**: Handle to a dispatched task

**isRunning()**:
```cpp
bool isRunning()
{
    FALCOR_UNIMPLEMENTED();
}
```

**Purpose**: Check if task is still executing

**Returns**: true if task is running, false otherwise

**Behavior**: Throws FALCOR_UNIMPLEMENTED exception

**finish()**:
```cpp
void finish()
{
    FALCOR_UNIMPLEMENTED();
}
```

**Purpose**: Wait for task to finish executing

**Behavior**: Throws FALCOR_UNIMPLEMENTED exception

**Task() Constructor**:
```cpp
Task::Task() {}
```

**Purpose**: Private constructor for Task class

**Behavior**: Default constructor

#### Static Methods

**start()**:
```cpp
static void start(uint32_t threadCount = kDefaultThreadCount)
{
    std::lock_guard<std::mutex> lock(sThreadingInitMutex);
    if (sThreadingInitCount++ == 0)
    {
        gData.threads.resize(threadCount);
        gData.initialized = true;
    }
}
```

**Purpose**: Initializes global thread pool

**Parameters**:
- threadCount: Number of threads in pool (default: kDefaultThreadCount)

**Behavior**:
1. Lock initialization mutex
2. If first initialization:
   - Resize thread vector to thread count
   - Set initialized flag to true
3. Increment initialization counter
4. Unlock mutex (RAII)

**Characteristics**:
- Reference counting
- Thread-safe initialization
- Only initializes once
- Default thread count is 16

**finish()**:
```cpp
static void finish()
{
    for (auto& t : gData.threads)
    {
        if (t.joinable())
            t.join();
    }
}
```

**Purpose**: Waits for all currently executing threads to finish

**Behavior**:
1. Iterate through all threads
2. If thread is joinable, join it
3. Return after all threads joined

**Characteristics**:
- Waits for all threads
- Thread-safe operation
- No shutdown
- Can be called multiple times

**shutdown()**:
```cpp
static void shutdown()
{
    std::lock_guard<std::mutex> lock(sThreadingInitMutex);
    uint32_t count = sThreadingInitCount--;
    if (count == 1)
    {
        for (auto& t : gData.threads)
            if (t.joinable())
                t.join();
        gData.initialized = false;
    }
    else if (count == 0)
        FALCOR_THROW("Threading::stop() called more times than Threading::start().");
}
```

**Purpose**: Waits for all currently executing threads to finish and shuts down thread pool

**Behavior**:
1. Lock initialization mutex
2. Decrement initialization counter
3. If last reference:
   - Join all threads
   - Set initialized flag to false
4. If counter is 0, throw exception
5. Unlock mutex (RAII)

**Characteristics**:
- Reference counting
- Thread-safe shutdown
- Only shuts down once
- Throws if over-shutdown

**getLogicalThreadCount()**:
```cpp
static uint32_t getLogicalThreadCount() { return std::thread::hardware_concurrency(); }
```

**Purpose**: Returns maximum number of concurrent threads supported by hardware

**Returns**: Hardware concurrency

**Characteristics**:
- Inline function
- Returns hardware concurrency
- No side effects

**dispatchTask()**:
```cpp
static Task dispatchTask(const std::function<void(void)>& func)
{
    FALCOR_ASSERT(gData.initialized);

    std::thread& t = gData.threads[gData.current];
    if (t.joinable())
        t.join();
    t = std::thread(func);
    gData.current = (gData.current + 1) % gData.threads.size();

    return Task();
}
```

**Purpose**: Starts a task on an available thread

**Parameters**:
- func: Function to execute

**Returns**: Handle to task

**Behavior**:
1. Assert thread pool is initialized
2. Get current thread
3. If thread is joinable, join it
4. Create new thread with function
5. Update current thread index (round-robin)
6. Return task handle

**Characteristics**:
- Round-robin dispatching
- Thread reuse
- Thread-safe operation
- Returns task handle

### Barrier Class

**Files**: `Threading.h` (lines 95-132)

```cpp
class FALCOR_API Barrier
{
public:
    Barrier(size_t threadCount, std::function<void()> completionFunc = nullptr)
        : mThreadCount(threadCount), mWaitCount(threadCount), mCompletionFunc(completionFunc)
    {}

    Barrier(const Barrier& barrier) = delete;
    Barrier& operator=(const Barrier& barrier) = delete;

    void wait()
    {
        std::unique_lock<std::mutex> lock(mMutex);

        auto generation = mGeneration;

        if (--mWaitCount == 0)
        {
            if (mCompletionFunc)
                mCompletionFunc();
            ++mGeneration;
            mWaitCount = mThreadCount;
            mCondition.notify_all();
        }
        else
        {
            mCondition.wait(lock, [this, generation]() { return generation != mGeneration; });
        }
    }

private:
    size_t mThreadCount;
    size_t mWaitCount;
    size_t mGeneration = 0;
    std::function<void()> mCompletionFunc;
    std::mutex mMutex;
    std::condition_variable mCondition;
};
```

**Purpose**: Simple thread barrier class

**Constructor**:
```cpp
Barrier(size_t threadCount, std::function<void()> completionFunc = nullptr)
    : mThreadCount(threadCount), mWaitCount(threadCount), mCompletionFunc(completionFunc)
{}
```

**Purpose**: Construct barrier with thread count and optional completion function

**Parameters**:
- threadCount: Number of threads to synchronize
- completionFunc: Optional completion function (default: nullptr)

**Behavior**:
1. Initialize thread count
2. Initialize wait count to thread count
3. Store completion function

**Characteristics**:
- Optional completion function
- Thread-safe construction
- No copy construction
- No copy assignment

**wait()**:
```cpp
void wait()
{
    std::unique_lock<std::mutex> lock(mMutex);

    auto generation = mGeneration;

    if (--mWaitCount == 0)
    {
        if (mCompletionFunc)
            mCompletionFunc();
        ++mGeneration;
        mWaitCount = mThreadCount;
        mCondition.notify_all();
    }
    else
        {
        mCondition.wait(lock, [this, generation]() { return generation != mGeneration; });
    }
}
```

**Purpose**: Wait for all threads to reach barrier

**Behavior**:
1. Lock mutex
2. Capture current generation
3. Decrement wait count
4. If last thread:
   - Call completion function if set
   - Increment generation
   - Reset wait count
   - Notify all waiting threads
5. If not last thread:
   - Wait for generation to change
6. Unlock mutex (RAII)

**Characteristics**:
- Thread-safe
- Supports multiple uses
- Optional completion function
- Generation counter for multiple uses

### ThreadingData Struct

**File**: `Threading.cpp` (lines 35-40)

```cpp
struct ThreadingData
{
    bool initialized = false;
    std::vector<std::thread> threads;
    uint32_t current;
} gData; // TODO: REMOVEGLOBAL
```

**Purpose**: Global threading data

**Members**:
- **initialized**: Whether thread pool is initialized
- **threads**: Vector of threads
- **current**: Current thread index for round-robin dispatching

**Characteristics**:
- Global variable
- TODO comment to remove global
- Used by Threading class

### Global Variables

**sThreadingInitMutex**:
```cpp
static std::mutex sThreadingInitMutex;
```

**Purpose**: Mutex for thread pool initialization

**Characteristics**:
- Static mutex
- Protects initialization counter
- Thread-safe initialization

**sThreadingInitCount**:
```cpp
static uint32_t sThreadingInitCount = 0;
```

**Purpose**: Counter for thread pool initialization

**Characteristics**:
- Static counter
- Reference counting
- Thread-safe initialization

## Technical Details

### Thread Pool Pattern

Uses static thread pool for task execution:

```cpp
struct ThreadingData
{
    bool initialized = false;
    std::vector<std::thread> threads;
    uint32_t current;
} gData;

static void start(uint32_t threadCount = kDefaultThreadCount)
{
    std::lock_guard<std::mutex> lock(sThreadingInitMutex);
    if (sThreadingInitCount++ == 0)
    {
        gData.threads.resize(threadCount);
        gData.initialized = true;
    }
}
```

**Characteristics**:
- Static thread pool
- Reference counting
- Thread-safe initialization
- Global state

### Reference Counting

Reference counting for thread pool initialization:

```cpp
static std::mutex sThreadingInitMutex;
static uint32_t sThreadingInitCount = 0;

static void start(uint32_t threadCount = kDefaultThreadCount)
{
    std::lock_guard<std::mutex> lock(sThreadingInitMutex);
    if (sThreadingInitCount++ == 0)
    {
        gData.threads.resize(threadCount);
        gData.initialized = true;
    }
}

static void shutdown()
{
    std::lock_guard<std::mutex> lock(sThreadingInitMutex);
    uint32_t count = sThreadingInitCount--;
    if (count == 1)
    {
        for (auto& t : gData.threads)
            if (t.joinable())
                t.join();
        gData.initialized = false;
    }
    else if (count == 0)
        FALCOR_THROW("Threading::stop() called more times than Threading::start().");
}
```

**Characteristics**:
- Thread-safe reference counting
- Only initializes once
- Only shuts down once
- Throws if over-shutdown

### Round-Robin Dispatching

Round-robin task dispatching:

```cpp
static Task dispatchTask(const std::function<void(void)>& func)
{
    FALCOR_ASSERT(gData.initialized);

    std::thread& t = gData.threads[gData.current];
    if (t.joinable())
        t.join();
    t = std::thread(func);
    gData.current = (gData.current + 1) % gData.threads.size();

    return Task();
}
```

**Characteristics**:
- Round-robin dispatching
- Thread reuse
- Modulo arithmetic for circular buffer
- Thread-safe operation

### Thread Reuse

Reuses threads for multiple tasks:

```cpp
static Task dispatchTask(const std::function<void(void)>& func)
{
    FALCOR_ASSERT(gData.initialized);

    std::thread& t = gData.threads[gData.current];
    if (t.joinable())
        t.join();
    t = std::thread(func);
    gData.current = (gData.current + 1) % gData.threads.size();

    return Task();
}
```

**Characteristics**:
- Joins existing thread if joinable
- Creates new thread with function
- Reuses thread slot
- Efficient thread management

### Barrier Pattern

Simple thread barrier for synchronization:

```cpp
class Barrier
{
public:
    Barrier(size_t threadCount, std::function<void()> completionFunc = nullptr)
        : mThreadCount(threadCount), mWaitCount(threadCount), mCompletionFunc(completionFunc)
    {}

    void wait()
    {
        std::unique_lock<std::mutex> lock(mMutex);

        auto generation = mGeneration;

        if (--mWaitCount == 0)
        {
            if (mCompletionFunc)
                mCompletionFunc();
            ++mGeneration;
            mWaitCount = mThreadCount;
            mCondition.notify_all();
        }
        else
        {
            mCondition.wait(lock, [this, generation]() { return generation != mGeneration; });
        }
    }

private:
    size_t mThreadCount;
    size_t mWaitCount;
    size_t mGeneration = 0;
    std::function<void()> mCompletionFunc;
    std::mutex mMutex;
    std::condition_variable mCondition;
};
```

**Characteristics**:
- Thread-safe
- Supports multiple uses
- Optional completion function
- Generation counter for multiple uses

### Generation Counter

Generation counter for multiple barrier uses:

```cpp
size_t mGeneration = 0;

void wait()
{
    std::unique_lock<std::mutex> lock(mMutex);

    auto generation = mGeneration;

    if (--mWaitCount == 0)
    {
        if (mCompletionFunc)
            mCompletionFunc();
        ++mGeneration;
        mWaitCount = mThreadCount;
        mCondition.notify_all();
    }
    else
        {
        mCondition.wait(lock, [this, generation]() { return generation != mGeneration; });
    }
}
```

**Characteristics**:
- Incremented on each barrier completion
- Used to detect barrier completion
- Allows multiple barrier uses
- Thread-safe

### Completion Function

Optional completion function for barrier:

```cpp
std::function<void()> mCompletionFunc;

void wait()
{
    std::unique_lock<std::mutex> lock(mMutex);

    auto generation = mGeneration;

    if (--mWaitCount == 0)
    {
        if (mCompletionFunc)
            mCompletionFunc();
        ++mGeneration;
        mWaitCount = mThreadCount;
        mCondition.notify_all();
    }
    else
        {
        mCondition.wait(lock, [this, generation]() { return generation != mGeneration; });
    }
}
```

**Characteristics**:
- Optional completion function
- Called when all threads reach barrier
- Called by last thread
- Thread-safe

## Integration Points

### Falcor Core Integration

- **Falcor/Core/Macros.h**: FALCOR_API, FALCOR_ASSERT, FALCOR_THROW, FALCOR_UNIMPLEMENTED

### STL Integration

- **std::condition_variable**: Condition variable for synchronization
- **std::functional**: Function wrapper for tasks
- **std::mutex**: Mutex for synchronization
- **std::thread**: Thread for task execution
- **std::vector**: Vector for threads
- **std::cstdint**: uint32_t type

### Internal Falcor Usage

- **Falcor/Core**: Core utilities
- **Falcor/Utils**: Internal utilities
- **Falcor/Rendering**: Rendering utilities

## Architecture Patterns

### Static Class Pattern

Static class for global thread pool:

```cpp
class FALCOR_API Threading
{
public:
    static void start(uint32_t threadCount = kDefaultThreadCount);
    static void finish();
    static void shutdown();
    static uint32_t getLogicalThreadCount() { return std::thread::hardware_concurrency(); }
    static Task dispatchTask(const std::function<void(void)>& func);
};
```

Benefits:
- Global thread pool
- No instance management
- Easy to use
- Thread-safe

### Reference Counting Pattern

Reference counting for initialization:

```cpp
static std::mutex sThreadingInitMutex;
static uint32_t sThreadingInitCount = 0;

static void start(uint32_t threadCount = kDefaultThreadCount)
{
    std::lock_guard<std::mutex> lock(sThreadingInitMutex);
    if (sThreadingInitCount++ == 0)
    {
        gData.threads.resize(threadCount);
        gData.initialized = true;
    }
}

static void shutdown()
{
    std::lock_guard<std::mutex> lock(sThreadingInitMutex);
    uint32_t count = sThreadingInitCount--;
    if (count == 1)
    {
        for (auto& t : gData.threads)
            if (t.joinable())
                t.join();
        gData.initialized = false;
    }
    else if (count == 0)
        FALCOR_THROW("Threading::stop() called more times than Threading::start().");
}
```

Benefits:
- Thread-safe initialization
- Only initializes once
- Only shuts down once
- Throws if over-shutdown

### Round-Robin Pattern

Round-robin task dispatching:

```cpp
static Task dispatchTask(const std::function<void(void)>& func)
{
    FALCOR_ASSERT(gData.initialized);

    std::thread& t = gData.threads[gData.current];
    if (t.joinable())
        t.join();
    t = std::thread(func);
    gData.current = (gData.current + 1) % gData.threads.size();

    return Task();
}
```

Benefits:
- Balanced task distribution
- Thread reuse
- Efficient dispatching
- Simple implementation

### Barrier Pattern

Thread barrier for synchronization:

```cpp
class Barrier
{
public:
    Barrier(size_t threadCount, std::function<void()> completionFunc = nullptr)
        : mThreadCount(threadCount), mWaitCount(threadCount), mCompletionFunc(completionFunc)
    {}

    void wait()
    {
        std::unique_lock<std::mutex> lock(mMutex);

        auto generation = mGeneration;

        if (--mWaitCount == 0)
        {
            if (mCompletionFunc)
                mCompletionFunc();
            ++mGeneration;
            mWaitCount = mThreadCount;
            mCondition.notify_all();
        }
        else
        {
            mCondition.wait(lock, [this, generation]() { return generation != mGeneration; });
        }
    }

private:
    size_t mThreadCount;
    size_t mWaitCount;
    size_t mGeneration = 0;
    std::function<void()> mCompletionFunc;
    std::mutex mMutex;
    std::condition_variable mCondition;
};
```

Benefits:
- Thread synchronization
- Supports multiple uses
- Optional completion function
- Generation counter for multiple uses

### Generation Counter Pattern

Generation counter for multiple barrier uses:

```cpp
size_t mGeneration = 0;

void wait()
{
    std::unique_lock<std::mutex> lock(mMutex);

    auto generation = mGeneration;

    if (--mWaitCount == 0)
    {
        if (mCompletionFunc)
            mCompletionFunc();
        ++mGeneration;
        mWaitCount = mThreadCount;
        mCondition.notify_all();
    }
    else
        {
        mCondition.wait(lock, [this, generation]() { return generation != mGeneration; });
    }
}
```

Benefits:
- Supports multiple barrier uses
- Detects barrier completion
- Thread-safe
- Simple implementation

## Code Patterns

### Reference Counting Pattern

Reference counting for initialization:

```cpp
static std::mutex sThreadingInitMutex;
static uint32_t sThreadingInitCount = 0;

static void start(uint32_t threadCount = kDefaultThreadCount)
{
    std::lock_guard<std::mutex> lock(sThreadingInitMutex);
    if (sThreadingInitCount++ == 0)
    {
        gData.threads.resize(threadCount);
        gData.initialized = true;
    }
}

static void shutdown()
{
    std::lock_guard<std::mutex> lock(sThreadingInitMutex);
    uint32_t count = sThreadingInitCount--;
    if (count == 1)
    {
        for (auto& t : gData.threads)
            if (t.joinable())
                t.join();
        gData.initialized = false;
    }
    else if (count == 0)
        FALCOR_THROW("Threading::stop() called more times than Threading::start().");
}
```

Pattern:
- Lock mutex
- Check if first initialization
- Initialize if first
- Increment counter
- On shutdown, decrement counter
- If last reference, shutdown
- If counter is 0, throw exception

### Round-Robin Dispatching Pattern

Round-robin task dispatching:

```cpp
static Task dispatchTask(const std::function<void(void)>& func)
{
    FALCOR_ASSERT(gData.initialized);

    std::thread& t = gData.threads[gData.current];
    if (t.joinable())
        t.join();
    t = std::thread(func);
    gData.current = (gData.current + 1) % gData.threads.size();

    return Task();
}
```

Pattern:
- Get current thread
- Join if joinable
- Create new thread
- Update current index (round-robin)
- Return task handle

### Barrier Wait Pattern

Wait for all threads to reach barrier:

```cpp
void wait()
{
    std::unique_lock<std::mutex> lock(mMutex);

    auto generation = mGeneration;

    if (--mWaitCount == 0)
    {
        if (mCompletionFunc)
            mCompletionFunc();
        ++mGeneration;
        mWaitCount = mThreadCount;
        mCondition.notify_all();
    }
    else
        {
        mCondition.wait(lock, [this, generation]() { return generation != mGeneration; });
    }
}
```

Pattern:
- Lock mutex
- Capture current generation
- Decrement wait count
- If last thread:
  - Call completion function
  - Increment generation
  - Reset wait count
  - Notify all waiting threads
- If not last thread:
  - Wait for generation to change
- Unlock mutex (RAII)

### Generation Counter Pattern

Generation counter for multiple barrier uses:

```cpp
size_t mGeneration = 0;

void wait()
{
    std::unique_lock<std::mutex> lock(mMutex);

    auto generation = mGeneration;

    if (--mWaitCount == 0)
    {
        if (mCompletionFunc)
            mCompletionFunc();
        ++mGeneration;
        mWaitCount = mThreadCount;
        mCondition.notify_all();
    }
    else
        {
        mCondition.wait(lock, [this, generation]() { return generation != mGeneration; });
    }
}
```

Pattern:
- Capture current generation
- Increment generation on completion
- Wait for generation to change
- Supports multiple barrier uses

### RAII Locking Pattern

Use RAII for mutex locking:

```cpp
void start(uint32_t threadCount = kDefaultThreadCount)
{
    std::lock_guard<std::mutex> lock(sThreadingInitMutex);
    // ...
}
```

Pattern:
- Create lock guard
- Perform operations
- Lock guard unlocks automatically

## Use Cases

### Basic Thread Pool Usage

```cpp
#include "Utils/Threading.h"

// Start thread pool
Threading::start(8);

// Dispatch task
Threading::Task task = Threading::dispatchTask([]() {
    // Do some work
    std::cout << "Task executed" << std::endl;
});

// Finish all tasks
Threading::finish();

// Shutdown thread pool
Threading::shutdown();
```

### Round-Robin Task Dispatching

```cpp
#include "Utils/Threading.h"

// Start thread pool
Threading::start(4);

// Dispatch multiple tasks (round-robin)
for (int i = 0; i < 10; ++i) {
    Threading::dispatchTask([i]() {
        std::cout << "Task " << i << std::endl;
    });
}

// Finish all tasks
Threading::finish();

// Shutdown thread pool
Threading::shutdown();
```

### Barrier Synchronization

```cpp
#include "Utils/Threading.h"

// Create barrier for 4 threads
Barrier barrier(4, []() {
    std::cout << "All threads reached barrier" << std::endl;
});

// Start threads
std::vector<std::thread> threads;
for (int i = 0; i < 4; ++i) {
    threads.emplace_back([i, &barrier]() {
        // Do some work
        std::cout << "Thread " << i << " working" << std::endl;

        // Wait for all threads to reach barrier
        barrier.wait();

        // Continue work
        std::cout << "Thread " << i << " continuing" << std::endl;
    });
}

// Join all threads
for (auto& t : threads) {
    t.join();
}
```

### Multiple Barrier Uses

```cpp
#include "Utils/Threading.h"

// Create barrier for 4 threads
Barrier barrier(4);

// Start threads
std::vector<std::thread> threads;
for (int i = 0; i < 4; ++i) {
    threads.emplace_back([i, &barrier]() {
        // First phase
        std::cout << "Thread " << i << " phase 1" << std::endl;
        barrier.wait();

        // Second phase
        std::cout << "Thread " << i << " phase 2" << std::endl;
        barrier.wait();

        // Third phase
        std::cout << "Thread " << i << " phase 3" << std::endl;
    });
}

// Join all threads
for (auto& t : threads) {
    t.join();
}
```

### Hardware Concurrency

```cpp
#include "Utils/Threading.h"

// Get hardware concurrency
uint32_t threadCount = Threading::getLogicalThreadCount();
std::cout << "Hardware concurrency: " << threadCount << std::endl;

// Start thread pool with hardware concurrency
Threading::start(threadCount);
```

### Reference Counting

```cpp
#include "Utils/Threading.h"

// Start thread pool (first reference)
Threading::start(8);

// Start thread pool (second reference)
Threading::start(8);

// Dispatch tasks
Threading::dispatchTask([]() {
    std::cout << "Task 1" << std::endl;
});

Threading::dispatchTask([]() {
    std::cout << "Task 2" << std::endl;
});

// Finish all tasks
Threading::finish();

// Shutdown thread pool (second reference)
Threading::shutdown();

// Shutdown thread pool (first reference)
Threading::shutdown();
```

## Performance Considerations

### Thread Pool Overhead

- **Thread Creation**: Thread creation overhead
- **Thread Joining**: Thread joining overhead
- **Mutex Locking**: Mutex locking overhead
- **Context Switching**: Thread context switching overhead

### Barrier Overhead

- **Mutex Locking**: Mutex locking overhead
- **Condition Variable**: Condition variable waiting overhead
- **Generation Counter**: Generation counter overhead
- **Completion Function**: Completion function overhead

### Round-Robin Overhead

- **Modulo Operation**: Modulo arithmetic overhead
- **Thread Joining**: Thread joining overhead
- **Thread Creation**: Thread creation overhead

### Compiler Optimizations

- **Inline Expansion**: Methods can be inlined
- **Constant Propagation**: Constants can be propagated
- **Dead Code Elimination**: Unused code can be eliminated
- **Template Instantiation**: Template instantiation overhead

### Comparison with Alternatives

**Threading vs std::thread**:
- Threading: Thread pool, automatic management
- std::thread: Manual thread management

**Threading vs std::async**:
- Threading: More control, round-robin dispatching
- std::async: Less control, no round-robin

**Barrier vs std::barrier**:
- Barrier: Simple implementation, generation counter
- std::barrier: C++20 standard, more features

### Optimization Tips

1. **Use Hardware Concurrency**: Use hardware concurrency for thread count
2. **Minimize Locking**: Minimize mutex locking time
3. **Use Round-Robin**: Use round-robin for balanced dispatching
4. **Enable Optimizations**: Enable compiler optimizations
5. **Profile**: Profile to identify bottlenecks

## Limitations

### Feature Limitations

- **No Task Priority**: No task priority support
- **No Task Cancellation**: No task cancellation support
- **No Task Dependencies**: No explicit task dependencies
- **No Task Progress**: No task progress tracking
- **No Task Result**: No task result retrieval
- **No Task Timeout**: No task timeout support
- **No Task Retry**: No task retry support
- **No Task Scheduling**: No delayed task scheduling
- **Unimplemented Task Methods**: Task::isRunning() and Task::finish() are unimplemented

### API Limitations

- **No Task ID**: No task ID for tracking
- **No Task Status**: No task status query
- **No Task Result**: No task result retrieval
- **No Task Progress**: No task progress tracking
- **No Task Cancellation**: No task cancellation
- **No Task Priority**: No task priority
- **No Task Dependencies**: No task dependencies
- **No Task Scheduling**: No delayed scheduling
- **Unimplemented Task Methods**: Task::isRunning() and Task::finish() are unimplemented

### Performance Limitations

- **Round-Robin Dispatching**: Round-robin dispatching may not be optimal
- **Thread Reuse**: Thread reuse overhead
- **Mutex Locking**: Mutex locking overhead
- **Condition Variable**: Condition variable overhead

### Platform Limitations

- **C++11 Only**: Requires C++11 or later
- **Hardware Concurrency**: Limited by hardware
- **Global State**: Global state may cause issues

## Best Practices

### When to Use

**Appropriate Use Cases**:
- Parallel task execution
- Thread pool management
- Thread synchronization
- Barrier synchronization
- Round-robin task dispatching
- Hardware concurrency detection

**Inappropriate Use Cases**:
- Single task execution (use direct execution)
- Task priority (use priority queue)
- Task cancellation (use cancellation token)
- Task dependencies (use task graph)
- Task progress tracking (use progress callback)

### Usage Patterns

**Basic Thread Pool Usage**:
```cpp
Threading::start(8);

Threading::dispatchTask([]() {
    // Do some work
});

Threading::finish();

Threading::shutdown();
```

**Barrier Synchronization**:
```cpp
Barrier barrier(4);

std::vector<std::thread> threads;
for (int i = 0; i < 4; ++i) {
    threads.emplace_back([i, &barrier]() {
        // Do some work
        barrier.wait();
    });
}

for (auto& t : threads) {
    t.join();
}
```

### Performance Tips

1. **Use Hardware Concurrency**: Use hardware concurrency for thread count
2. **Minimize Locking**: Minimize mutex locking time
3. **Use Round-Robin**: Use round-robin for balanced dispatching
4. **Enable Optimizations**: Enable compiler optimizations
5. **Profile**: Profile to identify bottlenecks

### Thread Safety

- **Thread-Safe Operations**: All operations are thread-safe
- **Mutex Locking**: Mutexes protect shared data
- **Reference Counting**: Thread-safe reference counting
- **Barrier Synchronization**: Thread-safe barrier

### Error Handling

- **Unimplemented Methods**: Task::isRunning() and Task::finish() are unimplemented
- **Over-Shutdown**: Throws if shutdown called more than start
- **Assertion**: Asserts thread pool is initialized

## Implementation Notes

### Thread Pool Implementation

Uses static thread pool for task execution:

```cpp
struct ThreadingData
{
    bool initialized = false;
    std::vector<std::thread> threads;
    uint32_t current;
} gData;

static void start(uint32_t threadCount = kDefaultThreadCount)
{
    std::lock_guard<std::mutex> lock(sThreadingInitMutex);
    if (sThreadingInitCount++ == 0)
    {
        gData.threads.resize(threadCount);
        gData.initialized = true;
    }
}
```

**Characteristics**:
- Static thread pool
- Reference counting
- Thread-safe initialization
- Global state

### Reference Counting Implementation

Reference counting for thread pool initialization:

```cpp
static std::mutex sThreadingInitMutex;
static uint32_t sThreadingInitCount = 0;

static void start(uint32_t threadCount = kDefaultThreadCount)
{
    std::lock_guard<std::mutex> lock(sThreadingInitMutex);
    if (sThreadingInitCount++ == 0)
    {
        gData.threads.resize(threadCount);
        gData.initialized = true;
    }
}

static void shutdown()
{
    std::lock_guard<std::mutex> lock(sThreadingInitMutex);
    uint32_t count = sThreadingInitCount--;
    if (count == 1)
    {
        for (auto& t : gData.threads)
            if (t.joinable())
                t.join();
        gData.initialized = false;
    }
    else if (count == 0)
        FALCOR_THROW("Threading::stop() called more times than Threading::start().");
}
```

**Characteristics**:
- Thread-safe reference counting
- Only initializes once
- Only shuts down once
- Throws if over-shutdown

### Round-Robin Dispatching Implementation

Round-robin task dispatching:

```cpp
static Task dispatchTask(const std::function<void(void)>& func)
{
    FALCOR_ASSERT(gData.initialized);

    std::thread& t = gData.threads[gData.current];
    if (t.joinable())
        t.join();
    t = std::thread(func);
    gData.current = (gData.current + 1) % gData.threads.size();

    return Task();
}
```

**Characteristics**:
- Round-robin dispatching
- Thread reuse
- Modulo arithmetic for circular buffer
- Thread-safe operation

### Barrier Implementation

Simple thread barrier for synchronization:

```cpp
class Barrier
{
public:
    Barrier(size_t threadCount, std::function<void()> completionFunc = nullptr)
        : mThreadCount(threadCount), mWaitCount(threadCount), mCompletionFunc(completionFunc)
    {}

    void wait()
    {
        std::unique_lock<std::mutex> lock(mMutex);

        auto generation = mGeneration;

        if (--mWaitCount == 0)
        {
            if (mCompletionFunc)
                mCompletionFunc();
            ++mGeneration;
            mWaitCount = mThreadCount;
            mCondition.notify_all();
        }
        else
        {
            mCondition.wait(lock, [this, generation]() { return generation != mGeneration; });
        }
    }

private:
    size_t mThreadCount;
    size_t mWaitCount;
    size_t mGeneration = 0;
    std::function<void()> mCompletionFunc;
    std::mutex mMutex;
    std::condition_variable mCondition;
};
```

**Characteristics**:
- Thread-safe
- Supports multiple uses
- Optional completion function
- Generation counter for multiple uses

### Generation Counter Implementation

Generation counter for multiple barrier uses:

```cpp
size_t mGeneration = 0;

void wait()
{
    std::unique_lock<std::mutex> lock(mMutex);

    auto generation = mGeneration;

    if (--mWaitCount == 0)
    {
        if (mCompletionFunc)
            mCompletionFunc();
        ++mGeneration;
        mWaitCount = mThreadCount;
        mCondition.notify_all();
    }
    else
        {
            mCondition.wait(lock, [this, generation]() { return generation != mGeneration; });
        }
}
```

**Characteristics**:
- Incremented on each barrier completion
- Used to detect barrier completion
- Allows multiple barrier uses
- Thread-safe

## Future Enhancements

### Potential Improvements

1. **Task Priority**: Add task priority support
2. **Task Cancellation**: Add task cancellation support
3. **Task Dependencies**: Add explicit task dependencies
4. **Task Progress**: Add task progress tracking
5. **Task Result**: Add task result retrieval
6. **Task Timeout**: Add task timeout support
7. **Task Retry**: Add task retry support
8. **Task Scheduling**: Add delayed task scheduling
9. **Task ID**: Add task ID for tracking
10. **Task Status**: Add task status query
11. **Implement Task Methods**: Implement Task::isRunning() and Task::finish()
12. **Remove Global State**: Remove global state (TODO)
13. **std::barrier Migration**: Migrate to std::barrier when C++20 is available

### API Extensions

1. **Task Priority**: Add task priority support
2. **Task Cancellation**: Add task cancellation support
3. **Task Dependencies**: Add explicit task dependencies
4. **Task Progress**: Add task progress tracking
5. **Task Result**: Add task result retrieval
6. **Task Timeout**: Add task timeout support
7. **Task Retry**: Add task retry support
8. **Task Scheduling**: Add delayed task scheduling
9. **Task ID**: Add task ID for tracking
10. **Task Status**: Add task status query
11. **Implement Task Methods**: Implement Task::isRunning() and Task::finish()

### Performance Enhancements

1. **Work Stealing**: Add work stealing for better load balancing
2. **Lock-Free Queue**: Add lock-free task queue
3. **NUMA Awareness**: Add NUMA-aware task scheduling
4. **Thread Affinity**: Add thread affinity support
5. **Thread Pool Size**: Add dynamic thread pool size

### Documentation Enhancements

1. **More Examples**: Add more usage examples
2. **Performance Guide**: Add performance optimization guide
3. **Best Practices**: Add best practices guide
4. **Migration Guide**: Add migration guide from std::thread
5. **API Reference**: Add complete API reference

## References

### C++ Standard

- **std::thread**: C++ thread library
- **std::mutex**: C++ mutex
- **std::condition_variable**: C++ condition variable
- **std::function**: C++ function wrapper
- **std::vector**: C++ vector container

### C++20

- **std::barrier**: C++20 barrier (future migration target)

### Falcor Documentation

- **Falcor Utils**: Utils module documentation
- **Falcor Core**: Core module documentation

### Related Technologies

- **Thread Pool**: Thread pool techniques
- **Barrier**: Barrier synchronization techniques
- **Round-Robin**: Round-robin scheduling techniques
- **Reference Counting**: Reference counting techniques
- **Thread Synchronization**: Thread synchronization techniques
- **Parallel Programming**: Parallel programming techniques
