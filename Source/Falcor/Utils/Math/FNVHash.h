#pragma once

#include "Core/Macros.h"
#include "Core/Error.h"

#include <cstdint>

namespace Falcor
{

namespace detail
{
template<typename TUInt>
struct FNVHashConstants
{};

template<>
struct FNVHashConstants<uint64_t>
{
    static constexpr uint64_t kOffsetBasis = UINT64_C(4695981039346656037);
    static constexpr uint64_t kPrime = UINT64_C(1099511628211);
};

template<>
struct FNVHashConstants<uint32_t>
{
    static constexpr uint32_t kOffsetBasis = UINT32_C(2166136261);
    static constexpr uint32_t kPrime = UINT32_C(16777619);
};
} // namespace detail

/**
 * Accumulates Fowler-Noll-Vo hash for inserted data.
 * To hash multiple items, create one Hash and insert all the items into it if at all possible.
 * This is superior to hashing the items individually and combining the hashes.
 *
 * @tparam TUInt - type of the storage for the hash, either 32 or 64 unsigned integer
 */
template<typename TUInt>
class FNVHash
{
public:
    static constexpr TUInt kOffsetBasis = detail::FNVHashConstants<TUInt>::kOffsetBasis;
    static constexpr TUInt kPrime = detail::FNVHashConstants<TUInt>::kPrime;

    /**
     * Inserts all data between [begin,end) into the hash.
     * @param[in] begin
     * @param[in] end
     */
    void insert(const void* begin, const void* end)
    {
        FALCOR_ASSERT(begin <= end);
        const uint8_t* srcData8 = reinterpret_cast<const uint8_t*>(begin);

        for (; srcData8 != end; ++srcData8)
        {
            mHash *= kPrime;
            mHash ^= *srcData8;
        }
    }

    /**
     * Inserts all data starting at data and going for size bytes into the hash
     * @param[in] data
     * @param[in] size
     */
    void insert(const void* data, size_t size)
    {
        const uint8_t* srcData8 = reinterpret_cast<const uint8_t*>(data);
        insert(srcData8, srcData8 + size);
    }

    template<typename T>
    void insert(const T& data)
    {
        insert(&data, sizeof(T));
    }

    TUInt get() const { return mHash; }

    constexpr bool operator==(const FNVHash& rhs) { return get() == rhs.get(); }

    constexpr bool operator!=(const FNVHash& rhs) { return get() != rhs.get(); }

    constexpr bool operator<=(const FNVHash& rhs) { return get() <= rhs.get(); }

    constexpr bool operator>=(const FNVHash& rhs) { return get() >= rhs.get(); }

    constexpr bool operator<(const FNVHash& rhs) { return get() < rhs.get(); }

    constexpr bool operator>(const FNVHash& rhs) { return get() > rhs.get(); }

private:
    TUInt mHash = kOffsetBasis;
};

using FNVHash64 = FNVHash<uint64_t>;
using FNVHash32 = FNVHash<uint32_t>;

inline uint64_t fnvHashArray64(const void* data, size_t size)
{
    FNVHash64 hash;
    hash.insert(data, size);
    return hash.get();
}

inline uint32_t fnvHashArray32(const void* data, size_t size)
{
    FNVHash32 hash;
    hash.insert(data, size);
    return hash.get();
}

} // namespace Falcor
