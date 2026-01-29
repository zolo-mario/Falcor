#include "Testing/UnitTest.h"
#include "Utils/Algorithm/UnionFind.h"

#include <random>
#include <vector>
#include <set>

namespace Falcor
{

namespace
{

template<typename T>
class TrivialUnionFind
{
public:
    TrivialUnionFind() = default;
    TrivialUnionFind(size_t size) { reset(size); }

    void reset(size_t size)
    {
        mSets.clear();
        mSets.resize(size);
        for (size_t i = 0; i < size; ++i)
            mSets[i].insert(i);
    }

    T findSet(T v)
    {
        for (size_t i = 0; i < mSets.size(); ++i)
            if (mSets[i].count(v) > 0)
                return i;
        return 0;
    }

    bool connectedSets(T v0, T v1) { return findSet(v0) == findSet(v1); }

    void unionSet(T v0, T v1)
    {
        v0 = findSet(v0);
        v1 = findSet(v1);
        if (v0 == v1)
            return;
        mSets[v0].insert(mSets[v1].begin(), mSets[v1].end());
        mSets.erase(mSets.begin() + v1);
    }

    size_t getSetCount() const { return mSets.size(); }

private:
    std::vector<std::set<T>> mSets;
};

} // namespace

CPU_TEST(UnionFind_randomized)
{
    size_t count = 10;

    for (int run = 0; run < 20; ++run)
    {
        std::mt19937 r(1234 + run);
        UnionFind<size_t> uf;
        TrivialUnionFind<size_t> reference;
        uf.reset(count);
        reference.reset(count);

        size_t iter = 0;
        while (reference.getSetCount() > 1 && iter < 1000)
        {
            size_t v0 = r() % count;
            size_t v1 = r() % count;

            EXPECT_EQ(uf.connectedSets(v0, v1), reference.connectedSets(v0, v1)) << fmt::format("Iter: {}/{}", iter, run);
            uf.unionSet(v0, v1);
            reference.unionSet(v0, v1);
            ASSERT_EQ(uf.getSetCount(), reference.getSetCount()) << fmt::format("Iter: {}/{}", iter, run);

            for (size_t i = 0; i < count; ++i)
            {
                for (size_t j = i + 1; j < count; ++j)
                {
                    EXPECT_EQ(uf.connectedSets(i, j), reference.connectedSets(i, j))
                        << fmt::format("Iter: {}/{}; i = {}; j = {}", iter, run, i, j);
                }
            }

            ++iter;
        }
    }
}

} // namespace Falcor
