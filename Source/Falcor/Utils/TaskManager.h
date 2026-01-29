#pragma once

#include "Core/Macros.h"

#include <BS_thread_pool/BS_thread_pool.hpp>

#include <functional>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>
#include <exception>

namespace Falcor
{
class RenderContext;
class FALCOR_API TaskManager
{
public:
    using CpuTask = std::function<void()>;
    using GpuTask = std::function<void(RenderContext* renderContext)>;

public:
    TaskManager(bool startPaused = false);

    /// Adds a CPU only task to the manager, if unpaused, the task starts right away
    void addTask(CpuTask&& task);
    /// Adds a GPU task to the manager, GPU tasks only start in the finish call and are sequential
    void addTask(GpuTask&& task);

    /// Unpauses and waits for all tasks to finish.
    /// The renderContext might be needed even if the TaskManager contains no GPU tasks,
    /// as those could be spawned from the CPU tasks
    void finish(RenderContext* renderContext);

private:
    /// Thread safe way to store an exception
    void storeException();
    /// Thread safe way to retrow a stored exception
    void rethrowException();
    /// CPU task execution wrapped so it stores exception if the task throws
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

} // namespace Falcor
