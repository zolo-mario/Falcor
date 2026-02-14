#include "Plugin.h"
#include "Utils/Logger.h"
#include "Utils/Timing/CpuTimer.h"

#include <nlohmann/json.hpp>

#include <fstream>

namespace Falcor
{

PluginManager& PluginManager::instance()
{
    static PluginManager sInstance;
    return sInstance;
}

bool PluginManager::loadPluginByName(std::string_view name)
{
    auto path = getRuntimeDirectory() / "plugins" / std::string(name);
#if FALCOR_WINDOWS
    path.replace_extension(".dll");
#elif FALCOR_LINUX
    path.replace_extension(".so");
#endif
    return loadPlugin(path);
}

bool PluginManager::loadPlugin(const std::filesystem::path& path)
{
    // Early exit if plugin is already loaded.
    {
        std::lock_guard<std::mutex> lock(mLibrariesMutex);
        if (mLibraries.find(path) != mLibraries.end())
            return false;
    }

    if (!std::filesystem::exists(path))
        FALCOR_THROW("Failed to load plugin library from {}. File not found.", path);

    SharedLibraryHandle library = loadSharedLibrary(path);
    if (library == nullptr)
        FALCOR_THROW("Failed to load plugin library from {}. Cannot load shared library.", path);

    using RegisterPluginProc = void (*)(PluginRegistry&);

    auto registerPluginProc = (RegisterPluginProc)getProcAddress(library, "registerPlugin");
    if (registerPluginProc == nullptr)
        FALCOR_THROW("Failed to load plugin library from {}. Symbol 'registerPlugin' not found.", path);

    // Register plugin library.
    {
        std::lock_guard<std::mutex> lock(mLibrariesMutex);
        mLibraries[path] = library;
    }

    // Call plugin library to register plugin classes.
    {
        PluginRegistry registry(*this, library);
        registerPluginProc(registry);
    }

    return true;
}

bool PluginManager::releasePlugin(const std::filesystem::path& path)
{
    std::lock_guard<std::mutex> librariesLock(mLibrariesMutex);

    auto libraryIt = mLibraries.find(path);
    if (libraryIt == mLibraries.end())
    {
        logWarning("Failed to release plugin library {}. The library isn't loaded.", path);
        return false;
    }

    // Get library handle.
    SharedLibraryHandle library = libraryIt->second;

    // Delete all the classes that were owned by the library.
    {
        std::lock_guard<std::mutex> classDescsLock(mClassDescsMutex);
        for (auto it = mClassDescs.begin(); it != mClassDescs.end();)
        {
            if (it->second->library == library)
                it = mClassDescs.erase(it);
            else
                ++it;
        }
    }

    releaseSharedLibrary(library);
    mLibraries.erase(libraryIt);

    return true;
}

void PluginManager::loadAllPlugins()
{
    CpuTimer timer;
    timer.update();

    std::ifstream ifs(getRuntimeDirectory() / "plugins" / "plugins.json");
    auto json = nlohmann::json::parse(ifs);
    size_t loadedCount = 0;
    for (const auto& name : json)
    {
        if (loadPluginByName(name.get<std::string>()))
            loadedCount++;
    }

    timer.update();
    if (loadedCount > 0)
        logInfo("Loaded {} plugin(s) in {:.3}s", loadedCount, timer.delta());
}

void PluginManager::releaseAllPlugins()
{
    // Disabled: releasing plugins in a loop hangs on certain DLLs (e.g., dlss.dll).
    while (0)
    {
        std::filesystem::path path;
        {
            std::lock_guard<std::mutex> lock(mLibrariesMutex);
            if (mLibraries.empty())
                return;
            path = mLibraries.begin()->first;
        }
        releasePlugin(path);
    }
}

} // namespace Falcor
