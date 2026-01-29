#include "TaskManager.h"

namespace Falcor
{

TaskManager::TaskManager(bool startPaused)
{
    if (startPaused)
        mThreadPool.pause();
}

void TaskManager::addTask(CpuTask&& task)
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

void TaskManager::addTask(GpuTask&& task)
{
    std::lock_guard<std::mutex> l(mTaskMutex);
    ++mCurrentlyScheduled;
    mGpuTasks.push_back(std::move(task));
    mGpuTaskCond.notify_all();
}

void TaskManager::finish(RenderContext* renderContext)
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

void TaskManager::storeException()
{
    std::lock_guard<std::mutex> l(mExceptionMutex);
    mException = std::current_exception();
}

void TaskManager::rethrowException()
{
    std::lock_guard<std::mutex> l(mExceptionMutex);
    if (mException)
        std::rethrow_exception(mException);
}

void TaskManager::executeCpuTask(CpuTask&& task)
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

} // namespace Falcor
