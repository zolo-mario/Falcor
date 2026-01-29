#pragma once
#include <vector>
#include <cstddef>

namespace Falcor
{

/**
 * UnionFind for any integral type.
 * @tparam T - The integral type for which UnionFind works.
 */
template<typename T>
class UnionFind
{
    static_assert(std::is_unsigned_v<T>, "UnionFind only works on unsigned integer types");

public:
    UnionFind() = default;
    UnionFind(size_t size) { reset(size); }

    void reset(size_t size)
    {
        mParent.resize(size);
        for (size_t i = 0; i < size; ++i)
            mParent[i] = T(i);
        mSetSize.assign(size, 1);
        mSetCount = size;
    }

    T findSet(T v)
    {
        // If root of the set, return it
        if (v == mParent[v])
            return v;
        // Otherwise find the root from the parent, and relink to the root (so the search chain shortens)
        mParent[v] = findSet(mParent[v]);
        return mParent[v];
    }

    bool connectedSets(T v0, T v1) { return findSet(v0) == findSet(v1); }

    void unionSet(T v0, T v1)
    {
        // Find the roots
        v0 = findSet(v0);
        v1 = findSet(v1);
        // If already in the same set, bail out
        if (v0 == v1)
            return;
        // Make v0 root of the larger set
        if (mSetSize[v0] < mSetSize[v1])
            std::swap(v0, v1);
        // The smaller set is parented under the larger set (balances the depth)
        mParent[v1] = v0;
        mSetSize[v0] += mSetSize[v1];
        --mSetCount;
    }

    size_t getSetCount() const { return mSetCount; }

private:
    std::vector<T> mParent;
    std::vector<size_t> mSetSize;
    size_t mSetCount{0};
};

} // namespace Falcor
