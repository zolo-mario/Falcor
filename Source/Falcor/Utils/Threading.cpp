#include "Threading.h"
#include "Core/Error.h"

namespace Falcor
{
namespace
{
struct ThreadingData
{
    bool initialized = false;
    std::vector<std::thread> threads;
    uint32_t current;
} gData; // TODO: REMOVEGLOBAL
} // namespace

static std::mutex sThreadingInitMutex;
static uint32_t sThreadingInitCount = 0;

void Threading::start(uint32_t threadCount)
{
    std::lock_guard<std::mutex> lock(sThreadingInitMutex);
    if (sThreadingInitCount++ == 0)
    {
        gData.threads.resize(threadCount);
        gData.initialized = true;
    }
}

void Threading::shutdown()
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

Threading::Task Threading::dispatchTask(const std::function<void(void)>& func)
{
    FALCOR_ASSERT(gData.initialized);

    std::thread& t = gData.threads[gData.current];
    if (t.joinable())
        t.join();
    t = std::thread(func);
    gData.current = (gData.current + 1) % gData.threads.size();

    return Task();
}

void Threading::finish()
{
    for (auto& t : gData.threads)
    {
        if (t.joinable())
            t.join();
    }
}

Threading::Task::Task() {}

bool Threading::Task::isRunning()
{
    FALCOR_UNIMPLEMENTED();
}

void Threading::Task::finish()
{
    FALCOR_UNIMPLEMENTED();
}
} // namespace Falcor
