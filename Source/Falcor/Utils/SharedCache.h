#pragma once

#include <functional>
#include <map>
#include <memory>
#include <mutex>

namespace Falcor
{

/**
 * Helper class for managing a shared cache.
 *
 * This is used in a few places where Falcor used global statics in the past.
 * Because Falcor now supports multiple devices, global statics don't work anymore.
 * The shared cache is used locally in a file where some resource is shared
 * among all instances of a class. The first instance creates the shared
 * resource, all subsequent instances can reuse the cached data. If all instances
 * are destroyed, the shared resource is automatically released as only the
 * instances hold a shared_ptr to the cached item, the cache itself only
 * holds a weak_ptr. Using a Key type, we can cache multiple versions of the same
 * data, typically used to cache one set for every GPU device instance.
 */
template<typename T, typename Key>
struct SharedCache
{
    std::shared_ptr<T> acquire(Key key, const std::function<std::shared_ptr<T>()>& init)
    {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = cache.find(key);
        if (it != cache.end())
        {
            if (auto data = it->second.lock())
                return data;
            else
                cache.erase(it);
        }

        std::shared_ptr<T> data = init();
        cache.emplace(key, data);
        return data;
    }

    std::mutex mutex;
    std::map<Key, std::weak_ptr<T>> cache;
};

} // namespace Falcor
