#pragma once
#include "Core/Macros.h"
#include "Core/API/fwd.h"
#include "Core/API/Resource.h"
#include "Core/API/Texture.h"
#include <condition_variable>
#include <filesystem>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <fstd/span.h>

namespace Falcor
{
class Barrier;

/**
 * Utility class to load textures asynchronously using multiple worker threads.
 */
class FALCOR_API AsyncTextureLoader
{
public:
    using LoadCallback = std::function<void(ref<Texture> pTexture)>;

    /**
     * Constructor.
     * @param[in] threadCount Number of worker threads.
     */
    AsyncTextureLoader(ref<Device> pDevice, size_t threadCount = std::thread::hardware_concurrency());

    /**
     * Destructor.
     * Blocks until all threads have terminated.
     */
    ~AsyncTextureLoader();

    /**
     * Request loading a texture with mips specified explicitly from individual files.
     * @param[in] path List of full paths of all mips, starting from mip0.
     * @param[in] loadAsSRGB Load the texture as sRGB format if supported, otherwise linear color.
     * @param[in] bindFlags The bind flags for the texture resource.
     * @param[in] importFlags Optional flags for the file import.
     * @param[in] callback Function called after the texture load has finished.
     * @return A future to a new texture, or nullptr if the texture failed to load.
     */
    std::future<ref<Texture>> loadMippedFromFiles(
        fstd::span<const std::filesystem::path> paths,
        bool loadAsSRGB,
        ResourceBindFlags bindFlags = ResourceBindFlags::ShaderResource,
        Bitmap::ImportFlags importFlags = Bitmap::ImportFlags::None,
        LoadCallback callback = {}
    );

    /**
     * Request loading a texture.
     * @param[in] path File path of the texture. This can be a full path or a relative path from a data directory.
     * @param[in] generateMipLevels Whether the full mip-chain should be generated.
     * @param[in] loadAsSRGB Load the texture as sRGB format if supported, otherwise linear color.
     * @param[in] bindFlags The bind flags for the texture resource.
     * @param[in] importFlags Optional flags for the file import.
     * @param[in] callback Function called after the texture load has finished.
     * @return A future to a new texture, or nullptr if the texture failed to load.
     */
    std::future<ref<Texture>> loadFromFile(
        const std::filesystem::path& path,
        bool generateMipLevels,
        bool loadAsSRGB,
        ResourceBindFlags bindFlags = ResourceBindFlags::ShaderResource,
        Bitmap::ImportFlags importFlags = Bitmap::ImportFlags::None,
        LoadCallback callback = {}
    );

private:
    void runWorkers(size_t threadCount);
    void runWorker();
    void terminateWorkers();

    struct LoadRequest
    {
        std::vector<std::filesystem::path> paths;
        bool generateMipLevels;
        bool loadAsSRGB;
        ResourceBindFlags bindFlags;
        Bitmap::ImportFlags importFlags;
        LoadCallback callback;
        std::promise<ref<Texture>> promise;
    };

    ref<Device> mpDevice;

    std::mutex mMutex;                      ///< Mutex for synchronizing access to shared resources.
    std::condition_variable mCondition;     ///< Condition variable for workers to wait on.
    std::shared_ptr<Barrier> mFlushBarrier; ///< Barrier for flushing the GPU to upload textures.
    std::vector<std::thread> mThreads;      ///< Worker threads.

    // Internal state. Do not access outside of critical section.
    std::queue<LoadRequest> mLoadRequestQueue; ///< Texture loading request queue.

    bool mTerminate = false;     ///< Flag to terminate worker threads.
    bool mFlushPending = false;  ///< Flag to indicate a GPU flush is pending.
    uint32_t mUploadCounter = 0; ///< Counter to issue a flush every few uploads.
};
} // namespace Falcor
