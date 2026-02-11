# TaskManager - Task Management System

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- Falcor/Core/Macros.h (FALCOR_API)
- BS_thread_pool/BS_thread_pool.hpp (BS::thread_pool)
- std::functional (std::function)
- std::mutex (std::mutex, std::lock_guard, std::unique_lock)
- std::condition_variable (std::condition_variable)
- std::vector (std::vector)
- std::atomic (std::atomic_size_t)
- std::exception (std::exception_ptr, std::current_exception, std::rethrow_exception)

### Dependent Modules

- Falcor/Core (Core utilities)
- Falcor/Rendering (Rendering utilities)
- Falcor/Scene (Scene utilities)
- Falcor/Utils (Internal utilities)

## Module Overview

TaskManager provides a task management system for coordinating CPU and GPU tasks in the Falcor framework. It manages a thread pool for CPU tasks and a sequential execution queue for GPU tasks, with support for pausing, exception handling, and synchronization.

### Key Characteristics

- **Thread Pool**: Uses BS::thread_pool for CPU task execution
- **GPU Task Queue**: Sequential GPU task execution
- **Exception Handling**: Thread-safe exception storage and rethrowing
- **Synchronization**: Condition variable for task synchronization
- **Atomic Counters**: Atomic counters for task tracking
- **Pause Support**: Optional pause on construction
- **Thread-Safe**: Thread-safe operations throughout

### Architecture Principles

1. **Thread Pool Pattern**: Uses thread pool for CPU tasks
2. **Sequential Execution**: GPU tasks execute sequentially
3. **Exception Safety**: Thread-safe exception handling
4. **Synchronization**: Condition variable for synchronization
5. **Atomic Operations**: Atomic counters for task tracking

## Component Specifications

### TaskManager Class

**Files**: `TaskManager.h` (lines 44-82), `TaskManager.cpp` (lines 33-127)

```cpp
class FALCOR_API TaskManager
{
public:
    using CpuTask = std::function<void()>;
    using GpuTask = std::function<void(RenderContext* renderContext)>;

public:
    TaskManager(bool startPaused = false);

    void addTask(CpuTask&& task);
    void addTask(GpuTask&& task);
    void finish(RenderContext* renderContext);

private:
    void storeException();
    void rethrowException();
    void executeCpuTask(CpuTask&& task);

private:
    BS::thread_pool mThreadPool;
    std::atomic_size_t mCurrentlyRunning{0};
    std::atomic_size_t mCurrentlyScheduled{0};

    std::mutex mTaskMutex;
    std::condition_variable mGpuTaskCond;
    std::vector<GpuTask> mGpuTasks;

    std::mutex mExceptionMutex;
    std::exception_ptr mException;
};
```

**Purpose**: Manage CPU and GPU tasks with thread pool and sequential execution

**Type Definitions**:
```cpp
using CpuTask = std::function<void()>;
using GpuTask = std::function<void(RenderContext* renderContext)>;
```

**CpuTask**: CPU task function type (void return, no parameters)

**GpuTask**: GPU task function type (void return, RenderContext* parameter)

#### Constructor

**TaskManager()**:
```cpp
TaskManager(bool startPaused = false)
{
    if (startPaused)
        mThreadPool.pause();
}
```

**Purpose**: Construct TaskManager with optional pause

**Parameters**:
- startPaused: Whether to start thread pool paused (default: false)

**Behavior**:
1. If startPaused is true, pause thread pool
2. Otherwise, thread pool starts running

**Characteristics**:
- Optional pause on construction
- Thread pool starts running by default
- No other initialization needed

#### Public Methods

**addTask() - CPU Task**:
```cpp
void addTask(CpuTask&& task)
{
    std::lock_guard<std::mutex> l(mTaskMutex);
    ++mCurrentlyScheduled;
    mThreadPool.push_task(
        [task = std::move(task), this]() mutable
        {
            ++mCurrentlyRunning;
            --mCurrentlyScheduled;
            executeCpuTask(std::move(task));
            size_t running = --mCurrentlyRunning;
            // If nothing is running, lets wake up and try to exit.
            if (running == 0)
                mGpuTaskCond.notify_all();
        }
    );
}
```

**Purpose**: Add a CPU only task to manager

**Parameters**:
- task: CPU task to add

**Behavior**:
1. Lock task mutex
2. Increment scheduled counter
3. Push task to thread pool with wrapper:
   - Increment running counter
   - Decrement scheduled counter
   - Execute CPU task with exception handling
   - Decrement running counter
   - If no tasks running, notify condition variable
4. Unlock mutex (RAII)

**Characteristics**:
- Task starts right away if unpaused
- Thread-safe operation
- Exception handling
- Atomic counter updates
- Condition variable notification

**addTask() - GPU Task**:
```cpp
void addTask(GpuTask&& task)
{
    std::lock_guard<std::mutex> l(mTaskMutex);
    ++mCurrentlyScheduled;
    mGpuTasks.push_back(std::move(task));
    mGpuTaskCond.notify_all();
}
```

**Purpose**: Add a GPU task to manager

**Parameters**:
- task: GPU task to add

**Behavior**:
1. Lock task mutex
2. Increment scheduled counter
3. Add task to GPU task queue
4. Notify condition variable
5. Unlock mutex (RAII)

**Characteristics**:
- GPU tasks only start in finish call
- Sequential execution
- Thread-safe operation
- Condition variable notification

**finish()**:
```cpp
void finish(RenderContext* renderContext)
{
    mThreadPool.unpause();
    while (true)
    {
        while (true)
        {
            std::unique_lock<std::mutex> l(mTaskMutex);
            if (mGpuTasks.empty())
                break;
            auto task = std::move(mGpuTasks.back());
            mGpuTasks.pop_back();
            l.unlock();
            ++mCurrentlyRunning;
            --mCurrentlyScheduled;
            task(renderContext);
            --mCurrentlyRunning;
        }

        std::unique_lock<std::mutex> l(mTaskMutex);
        while (true)
        {
            // If there are GPU tasks, go do them
            if (!mGpuTasks.empty())
                break;
            // If there are absolutely no tasks, go finish
            if (mCurrentlyRunning == 0 && mCurrentlyScheduled == 0)
                break;
            // Otherwise wait for either a new GPU task, or last running to notify us to check
            mGpuTaskCond.wait(l);
        }

        if (mCurrentlyRunning == 0 && mCurrentlyScheduled == 0)
            break;
    }
    rethrowException();
}
```

**Purpose**: Unpause and wait for all tasks to finish

**Parameters**:
- renderContext: RenderContext for GPU tasks (might be needed even if no GPU tasks)

**Behavior**:
1. Unpause thread pool
2. While true:
   - While true:
     - Lock task mutex
     - If GPU tasks empty, break
     - Extract last GPU task
     - Pop task from queue
     - Unlock mutex
     - Increment running counter
     - Decrement scheduled counter
     - Execute GPU task
     - Decrement running counter
   - Lock task mutex
   - While true:
     - If GPU tasks available, break
     - If no tasks running or scheduled, break
     - Wait on condition variable
   - If no tasks running or scheduled, break
3. Rethrow stored exception

**Characteristics**:
- Unpauses thread pool
- Executes GPU tasks sequentially
- Waits for all tasks to finish
- Rethrows stored exception
- Thread-safe operation

#### Private Methods

**storeException()**:
```cpp
void storeException()
{
    std::lock_guard<std::mutex> l(mExceptionMutex);
    mException = std::current_exception();
}
```

**Purpose**: Thread-safe way to store an exception

**Behavior**:
1. Lock exception mutex
2. Store current exception
3. Unlock mutex (RAII)

**Characteristics**:
- Thread-safe operation
- Stores exception pointer
- RAII locking

**rethrowException()**:
```cpp
void rethrowException()
{
    std::lock_guard<std::mutex> l(mExceptionMutex);
    if (mException)
        std::rethrow_exception(mException);
}
```

**Purpose**: Thread-safe way to rethrow a stored exception

**Behavior**:
1. Lock exception mutex
2. If exception stored, rethrow it
3. Unlock mutex (RAII)

**Characteristics**:
- Thread-safe operation
- Rethrows stored exception
- RAII locking

**executeCpuTask()**:
```cpp
void executeCpuTask(CpuTask&& task)
{
    try
    {
        task();
    }
    catch (...)
    {
        storeException();
    }
}
```

**Purpose**: CPU task execution wrapped so it stores exception if task throws

**Parameters**:
- task: CPU task to execute

**Behavior**:
1. Try to execute task
2. If exception thrown, store exception
3. Otherwise, continue normally

**Characteristics**:
- Exception handling
- Thread-safe exception storage
- Catches all exceptions

#### Private Members

**mThreadPool**:
```cpp
BS::thread_pool mThreadPool;
```

**Purpose**: Thread pool for CPU task execution

**Characteristics**:
- BS::thread_pool instance
- Manages worker threads
- Executes CPU tasks concurrently

**mCurrentlyRunning**:
```cpp
std::atomic_size_t mCurrentlyRunning{0};
```

**Purpose**: Atomic counter for currently running tasks

**Characteristics**:
- Atomic size_t
- Initialized to 0
- Tracks running tasks

**mCurrentlyScheduled**:
```cpp
std::atomic_size_t mCurrentlyScheduled{0};
```

**Purpose**: Atomic counter for currently scheduled tasks

**Characteristics**:
- Atomic size_t
- Initialized to 0
- Tracks scheduled tasks

**mTaskMutex**:
```cpp
std::mutex mTaskMutex;
```

**Purpose**: Mutex for task queue synchronization

**Characteristics**:
- Protects GPU task queue
- Protects counter access
- Used with RAII locks

**mGpuTaskCond**:
```cpp
std::condition_variable mGpuTaskCond;
```

**Purpose**: Condition variable for task synchronization

**Characteristics**:
- Notifies when tasks complete
- Waits for new tasks
- Coordinates CPU and GPU tasks

**mGpuTasks**:
```cpp
std::vector<GpuTask> mGpuTasks;
```

**Purpose**: Queue for GPU tasks

**Characteristics**:
- Vector of GPU tasks
- Sequential execution
- Protected by mutex

**mExceptionMutex**:
```cpp
std::mutex mExceptionMutex;
```

**Purpose**: Mutex for exception storage synchronization

**Characteristics**:
- Protects exception pointer
- Used with RAII locks

**mException**:
```cpp
std::exception_ptr mException;
```

**Purpose**: Stored exception pointer

**Characteristics**:
- std::exception_ptr
- Stores first exception
- Protected by mutex

## Technical Details

### Thread Pool Pattern

Uses BS::thread_pool for CPU task execution:

```cpp
BS::thread_pool mThreadPool;

void addTask(CpuTask&& task)
{
    std::lock_guard<std::mutex> l(mTaskMutex);
    ++mCurrentlyScheduled;
    mThreadPool.push_task(
        [task = std::move(task), this]() mutable
        {
            ++mCurrentlyRunning;
            --mCurrentlyScheduled;
            executeCpuTask(std::move(task));
            size_t running = --mCurrentlyRunning;
            if (running == 0)
                mGpuTaskCond.notify_all();
        }
    );
}
```

**Characteristics**:
- Thread pool manages worker threads
- Tasks execute concurrently
- Automatic load balancing
- Efficient thread management

### Sequential GPU Execution

GPU tasks execute sequentially:

```cpp
std::vector<GpuTask> mGpuTasks;

void addTask(GpuTask&& task)
{
    std::lock_guard<std::mutex> l(mTaskMutex);
    ++mCurrentlyScheduled;
    mGpuTasks.push_back(std::move(task));
    mGpuTaskCond.notify_all();
}

void finish(RenderContext* renderContext)
{
    mThreadPool.unpause();
    while (true)
    {
        while (true)
        {
            std::unique_lock<std::mutex> l(mTaskMutex);
            if (mGpuTasks.empty())
                break;
            auto task = std::move(mGpuTasks.back());
            mGpuTasks.pop_back();
            l.unlock();
            ++mCurrentlyRunning;
            --mCurrentlyScheduled;
            task(renderContext);
            --mCurrentlyRunning;
        }
        // ...
    }
}
```

**Characteristics**:
- GPU tasks queued sequentially
- Execute one at a time
- Wait for all tasks to finish
- Thread-safe queue access

### Exception Handling

Thread-safe exception storage and rethrowing:

```cpp
std::mutex mExceptionMutex;
std::exception_ptr mException;

void storeException()
{
    std::lock_guard<std::mutex> l(mExceptionMutex);
    mException = std::current_exception();
}

void rethrowException()
{
    std::lock_guard<std::mutex> l(mExceptionMutex);
    if (mException)
        std::rethrow_exception(mException);
}

void executeCpuTask(CpuTask&& task)
{
    try
    {
        task();
    }
    catch (...)
    {
        storeException();
    }
}
```

**Characteristics**:
- Thread-safe exception storage
- Stores first exception
- Rethrows in finish()
- Catches all exceptions

### Atomic Counters

Atomic counters for task tracking:

```cpp
std::atomic_size_t mCurrentlyRunning{0};
std::atomic_size_t mCurrentlyScheduled{0};
```

**Characteristics**:
- Atomic operations
- Thread-safe increments/decrements
- Track running and scheduled tasks
- No locking needed

### Condition Variable Synchronization

Condition variable for task synchronization:

```cpp
std::condition_variable mGpuTaskCond;

void addTask(CpuTask&& task)
{
    // ...
    size_t running = --mCurrentlyRunning;
    if (running == 0)
        mGpuTaskCond.notify_all();
}

void finish(RenderContext* renderContext)
{
    // ...
    std::unique_lock<std::mutex> l(mTaskMutex);
    while (true)
    {
        if (!mGpuTasks.empty())
            break;
        if (mCurrentlyRunning == 0 && mCurrentlyScheduled == 0)
            break;
        mGpuTaskCond.wait(l);
    }
    // ...
}
```

**Characteristics**:
- Notifies when tasks complete
- Waits for new tasks
- Coordinates CPU and GPU tasks
- Efficient synchronization

### Pause Support

Optional pause on construction:

```cpp
TaskManager(bool startPaused = false)
{
    if (startPaused)
        mThreadPool.pause();
}

void finish(RenderContext* renderContext)
{
    mThreadPool.unpause();
    // ...
}
```

**Characteristics**:
- Optional pause on construction
- Unpause in finish()
- Allows task scheduling before execution
- Thread pool management

### Task Scheduling Algorithm

CPU task scheduling:

```cpp
void addTask(CpuTask&& task)
{
    std::lock_guard<std::mutex> l(mTaskMutex);
    ++mCurrentlyScheduled;
    mThreadPool.push_task(
        [task = std::move(task), this]() mutable
        {
            ++mCurrentlyRunning;
            --mCurrentlyScheduled;
            executeCpuTask(std::move(task));
            size_t running = --mCurrentlyRunning;
            if (running == 0)
                mGpuTaskCond.notify_all();
        }
    );
}
```

**Algorithm**:
1. Lock task mutex
2. Increment scheduled counter
3. Push task to thread pool with wrapper:
   - Increment running counter
   - Decrement scheduled counter
   - Execute CPU task with exception handling
   - Decrement running counter
   - If no tasks running, notify condition variable
4. Unlock mutex (RAII)

**Characteristics**:
- Thread-safe operation
- Atomic counter updates
- Exception handling
- Condition variable notification

### GPU Task Execution Algorithm

GPU task execution:

```cpp
void finish(RenderContext* renderContext)
{
    mThreadPool.unpause();
    while (true)
    {
        while (true)
        {
            std::unique_lock<std::mutex> l(mTaskMutex);
            if (mGpuTasks.empty())
                break;
            auto task = std::move(mGpuTasks.back());
            mGpuTasks.pop_back();
            l.unlock();
            ++mCurrentlyRunning;
            --mCurrentlyScheduled;
            task(renderContext);
            --mCurrentlyRunning;
        }

        std::unique_lock<std::mutex> l(mTaskMutex);
        while (true)
        {
            if (!mGpuTasks.empty())
                break;
            if (mCurrentlyRunning == 0 && mCurrentlyScheduled == 0)
                break;
            mGpuTaskCond.wait(l);
        }

        if (mCurrentlyRunning == 0 && mCurrentlyScheduled == 0)
            break;
    }
    rethrowException();
}
```

**Algorithm**:
1. Unpause thread pool
2. While true:
   - While true:
     - Lock task mutex
     - If GPU tasks empty, break
     - Extract last GPU task
     - Pop task from queue
     - Unlock mutex
     - Increment running counter
     - Decrement scheduled counter
     - Execute GPU task
     - Decrement running counter
   - Lock task mutex
   - While true:
     - If GPU tasks available, break
     - If no tasks running or scheduled, break
     - Wait on condition variable
   - If no tasks running or scheduled, break
3. Rethrow stored exception

**Characteristics**:
- Sequential GPU task execution
- Waits for all tasks to finish
- Thread-safe queue access
- Condition variable synchronization

## Integration Points

### Falcor Core Integration

- **Falcor/Core/Macros.h**: FALCOR_API
- **Falcor/Core/RenderContext**: RenderContext for GPU tasks

### BS_thread_pool Integration

- **BS::thread_pool**: Thread pool implementation

### STL Integration

- **std::function**: Function wrapper for tasks
- **std::mutex**: Mutex for synchronization
- **std::condition_variable**: Condition variable for synchronization
- **std::vector**: Vector for GPU task queue
- **std::atomic**: Atomic counters
- **std::exception**: Exception handling

### Internal Falcor Usage

- **Falcor/Core**: Core utilities
- **Falcor/Rendering**: Rendering utilities
- **Falcor/Scene**: Scene utilities
- **Falcor/Utils**: Internal utilities

## Architecture Patterns

### Thread Pool Pattern

Uses thread pool for CPU task execution:

```cpp
BS::thread_pool mThreadPool;

void addTask(CpuTask&& task)
{
    mThreadPool.push_task(
        [task = std::move(task), this]() mutable
        {
            // Execute task
        }
    );
}
```

Benefits:
- Concurrent task execution
- Automatic load balancing
- Efficient thread management
- No manual thread management

### Sequential Execution Pattern

GPU tasks execute sequentially:

```cpp
std::vector<GpuTask> mGpuTasks;

void finish(RenderContext* renderContext)
{
    while (true)
    {
        std::unique_lock<std::mutex> l(mTaskMutex);
        if (mGpuTasks.empty())
            break;
        auto task = std::move(mGpuTasks.back());
        mGpuTasks.pop_back();
        l.unlock();
        task(renderContext);
    }
}
```

Benefits:
- Sequential GPU task execution
- Thread-safe queue access
- Predictable execution order
- No GPU resource conflicts

### Exception Handling Pattern

Thread-safe exception storage and rethrowing:

```cpp
std::mutex mExceptionMutex;
std::exception_ptr mException;

void storeException()
{
    std::lock_guard<std::mutex> l(mExceptionMutex);
    mException = std::current_exception();
}

void rethrowException()
{
    std::lock_guard<std::mutex> l(mExceptionMutex);
    if (mException)
        std::rethrow_exception(mException);
}
```

Benefits:
- Thread-safe exception storage
- Stores first exception
- Rethrows in finish()
- Catches all exceptions

### Condition Variable Pattern

Condition variable for task synchronization:

```cpp
std::condition_variable mGpuTaskCond;

void addTask(CpuTask&& task)
{
    // ...
    size_t running = --mCurrentlyRunning;
    if (running == 0)
        mGpuTaskCond.notify_all();
}

void finish(RenderContext* renderContext)
{
    // ...
    std::unique_lock<std::mutex> l(mTaskMutex);
    while (true)
    {
        if (!mGpuTasks.empty())
            break;
        if (mCurrentlyRunning == 0 && mCurrentlyScheduled == 0)
            break;
        mGpuTaskCond.wait(l);
    }
}
```

Benefits:
- Efficient synchronization
- No busy waiting
- Coordinates CPU and GPU tasks
- Thread-safe notification

### RAII Pattern

RAII for mutex locking:

```cpp
void addTask(CpuTask&& task)
{
    std::lock_guard<std::mutex> l(mTaskMutex);
    // ...
}
```

Benefits:
- Automatic unlocking
- Exception safety
- Clean code
- No manual unlock

## Code Patterns

### Task Scheduling Pattern

Schedule task with wrapper:

```cpp
void addTask(CpuTask&& task)
{
    std::lock_guard<std::mutex> l(mTaskMutex);
    ++mCurrentlyScheduled;
    mThreadPool.push_task(
        [task = std::move(task), this]() mutable
        {
            ++mCurrentlyRunning;
            --mCurrentlyScheduled;
            executeCpuTask(std::move(task));
            size_t running = --mCurrentlyRunning;
            if (running == 0)
                mGpuTaskCond.notify_all();
        }
    );
}
```

Pattern:
- Lock mutex
- Increment scheduled counter
- Push task with wrapper
- Wrapper updates counters
- Wrapper executes task
- Wrapper notifies condition variable

### GPU Task Execution Pattern

Execute GPU tasks sequentially:

```cpp
void finish(RenderContext* renderContext)
{
    mThreadPool.unpause();
    while (true)
    {
        while (true)
        {
            std::unique_lock<std::mutex> l(mTaskMutex);
            if (mGpuTasks.empty())
                break;
            auto task = std::move(mGpuTasks.back());
            mGpuTasks.pop_back();
            l.unlock();
            ++mCurrentlyRunning;
            --mCurrentlyScheduled;
            task(renderContext);
            --mCurrentlyRunning;
        }

        std::unique_lock<std::mutex> l(mTaskMutex);
        while (true)
        {
            if (!mGpuTasks.empty())
                break;
            if (mCurrentlyRunning == 0 && mCurrentlyScheduled == 0)
                break;
            mGpuTaskCond.wait(l);
        }

        if (mCurrentlyRunning == 0 && mCurrentlyScheduled == 0)
            break;
    }
    rethrowException();
}
```

Pattern:
- Unpause thread pool
- Execute GPU tasks sequentially
- Wait for all tasks to finish
- Rethrow stored exception

### Exception Handling Pattern

Store and rethrow exceptions:

```cpp
void storeException()
{
    std::lock_guard<std::mutex> l(mExceptionMutex);
    mException = std::current_exception();
}

void rethrowException()
{
    std::lock_guard<std::mutex> l(mExceptionMutex);
    if (mException)
        std::rethrow_exception(mException);
}

void executeCpuTask(CpuTask&& task)
{
    try
    {
        task();
    }
    catch (...)
    {
        storeException();
    }
}
```

Pattern:
- Wrap task execution in try-catch
- Store exception if thrown
- Rethrow in finish()
- Thread-safe exception storage

### Condition Variable Pattern

Notify and wait on condition variable:

```cpp
void addTask(CpuTask&& task)
{
    // ...
    size_t running = --mCurrentlyRunning;
    if (running == 0)
        mGpuTaskCond.notify_all();
}

void finish(RenderContext* renderContext)
{
    // ...
    std::unique_lock<std::mutex> l(mTaskMutex);
    while (true)
    {
        if (!mGpuTasks.empty())
            break;
        if (mCurrentlyRunning == 0 && mCurrentlyScheduled == 0)
            break;
        mGpuTaskCond.wait(l);
    }
}
```

Pattern:
- Notify when tasks complete
- Wait for new tasks
- Check conditions before waiting
- Thread-safe notification

### RAII Locking Pattern

Use RAII for mutex locking:

```cpp
void addTask(CpuTask&& task)
{
    std::lock_guard<std::mutex> l(mTaskMutex);
    // ...
}
```

Pattern:
- Create lock guard
- Perform operations
- Lock guard unlocks automatically

## Use Cases

### Basic CPU Task Execution

```cpp
#include "Utils/TaskManager.h"

TaskManager taskManager;

// Add CPU task
taskManager.addTask([]() {
    // Do some CPU work
    std::cout << "CPU task executed" << std::endl;
});

// Wait for all tasks to finish
taskManager.finish(nullptr);
```

### Basic GPU Task Execution

```cpp
#include "Utils/TaskManager.h"

TaskManager taskManager;

// Add GPU task
taskManager.addTask([](RenderContext* renderContext) {
    // Do some GPU work
    std::cout << "GPU task executed" << std::endl;
});

// Wait for all tasks to finish
taskManager.finish(renderContext);
```

### Mixed CPU and GPU Tasks

```cpp
#include "Utils/TaskManager.h"

TaskManager taskManager;

// Add CPU tasks
taskManager.addTask([]() {
    std::cout << "CPU task 1" << std::endl;
});

taskManager.addTask([]() {
    std::cout << "CPU task 2" << std::endl;
});

// Add GPU tasks
taskManager.addTask([](RenderContext* renderContext) {
    std::cout << "GPU task 1" << std::endl;
});

taskManager.addTask([](RenderContext* renderContext) {
    std::cout << "GPU task 2" << std::endl;
});

// Wait for all tasks to finish
taskManager.finish(renderContext);
```

### Paused Task Manager

```cpp
#include "Utils/TaskManager.h"

// Create paused task manager
TaskManager taskManager(true);

// Add tasks (won't execute yet)
taskManager.addTask([]() {
    std::cout << "CPU task" << std::endl;
});

taskManager.addTask([](RenderContext* renderContext) {
    std::cout << "GPU task" << std::endl;
});

// Finish will unpause and execute tasks
taskManager.finish(renderContext);
```

### Exception Handling

```cpp
#include "Utils/TaskManager.h"

TaskManager taskManager;

// Add task that throws exception
taskManager.addTask([]() {
    throw std::runtime_error("Task failed");
});

// Finish will rethrow exception
try {
    taskManager.finish(nullptr);
} catch (const std::exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;
}
```

### Task Dependencies

```cpp
#include "Utils/TaskManager.h"

TaskManager taskManager;

// Add dependent tasks
taskManager.addTask([]() {
    std::cout << "Task 1" << std::endl;
});

taskManager.addTask([]() {
    std::cout << "Task 2" << std::endl;
});

// GPU tasks execute sequentially
taskManager.addTask([](RenderContext* renderContext) {
    std::cout << "GPU Task 1" << std::endl;
});

taskManager.addTask([](RenderContext* renderContext) {
    std::cout << "GPU Task 2" << std::endl;
});

// Wait for all tasks to finish
taskManager.finish(renderContext);
```

### Parallel CPU Tasks

```cpp
#include "Utils/TaskManager.h"

TaskManager taskManager;

// Add parallel CPU tasks
for (int i = 0; i < 10; ++i) {
    taskManager.addTask([i]() {
        std::cout << "CPU task " << i << std::endl;
    });
}

// Wait for all tasks to finish
taskManager.finish(nullptr);
```

## Performance Considerations

### Thread Pool Overhead

- **Thread Creation**: Thread pool creates threads on construction
- **Task Scheduling**: Task scheduling overhead
- **Thread Synchronization**: Mutex and condition variable overhead
- **Context Switching**: Thread context switching overhead

### Atomic Counter Overhead

- **Atomic Operations**: Atomic increment/decrement overhead
- **Memory Ordering**: Memory ordering overhead
- **Cache Coherence**: Cache coherence overhead

### GPU Task Overhead

- **Sequential Execution**: GPU tasks execute sequentially
- **Mutex Locking**: Mutex locking overhead
- **Condition Variable**: Condition variable waiting overhead

### Compiler Optimizations

- **Inline Expansion**: Methods can be inlined
- **Constant Propagation**: Constants can be propagated
- **Dead Code Elimination**: Unused code can be eliminated
- **Template Instantiation**: Template instantiation overhead

### Comparison with Alternatives

**TaskManager vs std::thread**:
- TaskManager: Thread pool, automatic management
- std::thread: Manual thread management

**TaskManager vs std::async**:
- TaskManager: More control, GPU task support
- std::async: Less control, no GPU task support

### Optimization Tips

1. **Batch Tasks**: Batch similar tasks together
2. **Minimize Locking**: Minimize mutex locking time
3. **Use Atomic Counters**: Use atomic counters for counters
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

### API Limitations

- **No Task ID**: No task ID for tracking
- **No Task Status**: No task status query
- **No Task Result**: No task result retrieval
- **No Task Progress**: No task progress tracking
- **No Task Cancellation**: No task cancellation
- **No Task Priority**: No task priority
- **No Task Dependencies**: No task dependencies
- **No Task Scheduling**: No delayed scheduling

### Performance Limitations

- **Sequential GPU Execution**: GPU tasks execute sequentially
- **Thread Pool Overhead**: Thread pool overhead
- **Atomic Counter Overhead**: Atomic counter overhead
- **Mutex Locking**: Mutex locking overhead
- **Condition Variable**: Condition variable overhead

### Platform Limitations

- **C++11 Only**: Requires C++11 or later
- **BS::thread_pool**: Requires BS::thread_pool library
- **Thread Count**: Limited by hardware threads

## Best Practices

### When to Use

**Appropriate Use Cases**:
- Parallel CPU task execution
- Sequential GPU task execution
- Mixed CPU and GPU tasks
- Exception handling in tasks
- Task synchronization
- Task coordination

**Inappropriate Use Cases**:
- Single task execution (use direct execution)
- Task priority (use priority queue)
- Task cancellation (use cancellation token)
- Task dependencies (use task graph)
- Task progress tracking (use progress callback)

### Usage Patterns

**Basic CPU Task Execution**:
```cpp
TaskManager taskManager;

taskManager.addTask([]() {
    // Do some CPU work
});

taskManager.finish(nullptr);
```

**Basic GPU Task Execution**:
```cpp
TaskManager taskManager;

taskManager.addTask([](RenderContext* renderContext) {
    // Do some GPU work
});

taskManager.finish(renderContext);
```

**Mixed CPU and GPU Tasks**:
```cpp
TaskManager taskManager;

taskManager.addTask([]() {
    // CPU work
});

taskManager.addTask([](RenderContext* renderContext) {
    // GPU work
});

taskManager.finish(renderContext);
```

### Performance Tips

1. **Batch Tasks**: Batch similar tasks together
2. **Minimize Locking**: Minimize mutex locking time
3. **Use Atomic Counters**: Use atomic counters for counters
4. **Enable Optimizations**: Enable compiler optimizations
5. **Profile**: Profile to identify bottlenecks

### Thread Safety

- **Thread-Safe Operations**: All operations are thread-safe
- **Mutex Locking**: Mutexes protect shared data
- **Atomic Counters**: Atomic counters for counters
- **Exception Handling**: Thread-safe exception handling

### Error Handling

- **Exception Storage**: Store first exception
- **Exception Rethrow**: Rethrow in finish()
- **All Exceptions**: Catch all exceptions
- **Thread-Safe**: Thread-safe exception handling

## Implementation Notes

### Thread Pool Implementation

Uses BS::thread_pool for CPU task execution:

```cpp
BS::thread_pool mThreadPool;

void addTask(CpuTask&& task)
{
    std::lock_guard<std::mutex> l(mTaskMutex);
    ++mCurrentlyScheduled;
    mThreadPool.push_task(
        [task = std::move(task), this]() mutable
        {
            ++mCurrentlyRunning;
            --mCurrentlyScheduled;
            executeCpuTask(std::move(task));
            size_t running = --mCurrentlyRunning;
            if (running == 0)
                mGpuTaskCond.notify_all();
        }
    );
}
```

**Characteristics**:
- Thread pool manages worker threads
- Tasks execute concurrently
- Automatic load balancing
- Efficient thread management

### Sequential GPU Execution Implementation

GPU tasks execute sequentially:

```cpp
std::vector<GpuTask> mGpuTasks;

void finish(RenderContext* renderContext)
{
    mThreadPool.unpause();
    while (true)
    {
        while (true)
        {
            std::unique_lock<std::mutex> l(mTaskMutex);
            if (mGpuTasks.empty())
                break;
            auto task = std::move(mGpuTasks.back());
            mGpuTasks.pop_back();
            l.unlock();
            ++mCurrentlyRunning;
            --mCurrentlyScheduled;
            task(renderContext);
            --mCurrentlyRunning;
        }
        // ...
    }
}
```

**Characteristics**:
- GPU tasks queued sequentially
- Execute one at a time
- Wait for all tasks to finish
- Thread-safe queue access

### Exception Handling Implementation

Thread-safe exception storage and rethrowing:

```cpp
std::mutex mExceptionMutex;
std::exception_ptr mException;

void storeException()
{
    std::lock_guard<std::mutex> l(mExceptionMutex);
    mException = std::current_exception();
}

void rethrowException()
{
    std::lock_guard<std::mutex> l(mExceptionMutex);
    if (mException)
        std::rethrow_exception(mException);
}

void executeCpuTask(CpuTask&& task)
{
    try
    {
        task();
    }
    catch (...)
    {
        storeException();
    }
}
```

**Characteristics**:
- Thread-safe exception storage
- Stores first exception
- Rethrows in finish()
- Catches all exceptions

### Condition Variable Implementation

Condition variable for task synchronization:

```cpp
std::condition_variable mGpuTaskCond;

void addTask(CpuTask&& task)
{
    // ...
    size_t running = --mCurrentlyRunning;
    if (running == 0)
        mGpuTaskCond.notify_all();
}

void finish(RenderContext* renderContext)
{
    // ...
    std::unique_lock<std::mutex> l(mTaskMutex);
    while (true)
    {
        if (!mGpuTasks.empty())
            break;
        if (mCurrentlyRunning == 0 && mCurrentlyScheduled == 0)
            break;
        mGpuTaskCond.wait(l);
    }
}
```

**Characteristics**:
- Notifies when tasks complete
- Waits for new tasks
- Coordinates CPU and GPU tasks
- Efficient synchronization

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

### Performance Enhancements

1. **Parallel GPU Execution**: Parallel GPU task execution
2. **Task Batching**: Task batching for better performance
3. **Lock-Free Queue**: Lock-free GPU task queue
4. **Work Stealing**: Work stealing for better load balancing
5. **NUMA Awareness**: NUMA-aware task scheduling

### Documentation Enhancements

1. **More Examples**: Add more usage examples
2. **Performance Guide**: Add performance optimization guide
3. **Best Practices**: Add best practices guide
4. **Migration Guide**: Add migration guide from std::thread
5. **API Reference**: Add complete API reference

## References

### C++ Standard

- **std::function**: C++ function wrapper
- **std::mutex**: C++ mutex
- **std::condition_variable**: C++ condition variable
- **std::vector**: C++ vector container
- **std::atomic**: C++ atomic operations
- **std::exception**: C++ exception handling

### BS::thread_pool

- **BS::thread_pool**: Thread pool implementation

### Falcor Documentation

- **Falcor Core**: Core module documentation
- **Falcor Rendering**: Rendering module documentation

### Related Technologies

- **Thread Pool**: Thread pool techniques
- **Task Scheduling**: Task scheduling techniques
- **Exception Handling**: Exception handling techniques
- **Synchronization**: Synchronization techniques
- **Parallel Programming**: Parallel programming techniques
