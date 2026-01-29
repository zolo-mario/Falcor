#include "AsyncTextureLoader.h"
#include "Core/API/Device.h"
#include "Utils/Threading.h"

namespace Falcor
{
namespace
{
constexpr size_t kUploadsPerFlush = 16; ///< Number of texture uploads before issuing a flush (to keep upload heap from growing).
}

AsyncTextureLoader::AsyncTextureLoader(ref<Device> pDevice, size_t threadCount) : mpDevice(pDevice)
{
    runWorkers(threadCount);
}

AsyncTextureLoader::~AsyncTextureLoader()
{
    terminateWorkers();

    mpDevice->wait();
}

std::future<ref<Texture>> AsyncTextureLoader::loadMippedFromFiles(
    fstd::span<const std::filesystem::path> paths,
    bool loadAsSrgb,
    ResourceBindFlags bindFlags,
    Bitmap::ImportFlags importFlags,
    LoadCallback callback
)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mLoadRequestQueue.push(LoadRequest{{paths.begin(), paths.end()}, false, loadAsSrgb, bindFlags, importFlags, callback});
    mCondition.notify_one();
    return mLoadRequestQueue.back().promise.get_future();
}

std::future<ref<Texture>> AsyncTextureLoader::loadFromFile(
    const std::filesystem::path& path,
    bool generateMipLevels,
    bool loadAsSrgb,
    ResourceBindFlags bindFlags,
    Bitmap::ImportFlags importFlags,
    LoadCallback callback
)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mLoadRequestQueue.push(LoadRequest{{path}, generateMipLevels, loadAsSrgb, bindFlags, importFlags, callback});
    mCondition.notify_one();
    return mLoadRequestQueue.back().promise.get_future();
}

void AsyncTextureLoader::runWorkers(size_t threadCount)
{
    // Create a barrier to synchronize worker threads before issuing a global flush.
    mFlushBarrier = std::make_shared<Barrier>(
        threadCount,
        [&]()
        {
            mpDevice->wait();
            mFlushPending = false;
            mUploadCounter = 0;
        }
    );

    for (size_t i = 0; i < threadCount; ++i)
    {
        mThreads.emplace_back(&AsyncTextureLoader::runWorker, this);
    }
}

void AsyncTextureLoader::runWorker()
{
    // This function is the entry point for worker threads.
    // The workers wait on the load request queue and load a texture when woken up.
    // To avoid the upload heap growing too large, we synchronize the threads and
    // issue a global GPU flush at regular intervals.

    while (true)
    {
        // Wait on condition until more work is ready.
        std::unique_lock<std::mutex> lock(mMutex);
        mCondition.wait(lock, [&]() { return mTerminate || !mLoadRequestQueue.empty() || mFlushPending; });

        // Sync thread if a flush is pending.
        if (mFlushPending)
        {
            lock.unlock();
            mFlushBarrier->wait();
            mCondition.notify_one();
            continue;
        }

        // Terminate thread unless there is more work to do.
        if (mTerminate && mLoadRequestQueue.empty() && !mFlushPending)
            break;

        // Go back waiting if queue is currently empty.
        if (mLoadRequestQueue.empty())
            continue;

        // Pop next load request from queue.
        auto request = std::move(mLoadRequestQueue.front());
        mLoadRequestQueue.pop();

        lock.unlock();

        // Load the textures (this part is running in parallel).
        ref<Texture> pTexture;
        if (request.paths.size() == 1)
        {
            pTexture = Texture::createFromFile(
                mpDevice, request.paths[0], request.generateMipLevels, request.loadAsSRGB, request.bindFlags, request.importFlags
            );
        }
        else
        {
            pTexture = Texture::createMippedFromFiles(mpDevice, request.paths, request.loadAsSRGB, request.bindFlags, request.importFlags);
        }

        request.promise.set_value(pTexture);

        if (request.callback)
        {
            request.callback(pTexture);
        }

        lock.lock();

        // Issue a global flush if necessary.
        // TODO: It would be better to check the size of the upload heap instead.
        if (!mTerminate && pTexture != nullptr && ++mUploadCounter >= kUploadsPerFlush)
        {
            mFlushPending = true;
            mCondition.notify_all();
        }

        mCondition.notify_one();
    }
}

void AsyncTextureLoader::terminateWorkers()
{
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mTerminate = true;
    }

    mCondition.notify_all();

    for (auto& thread : mThreads)
        thread.join();
}
} // namespace Falcor
