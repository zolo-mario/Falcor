#pragma once

#include <pxr/base/vt/array.h>
#include <unordered_map>

namespace Falcor
{

/**
 * @brief Class to convert vector of possibly-duplicate items to a vector of indices into a set of unique data items.
 *
 * @tparam T Underlying type
 * @tparam I Index value type
 * @tparam H Hash object on type T, used to determine data item equivalence
 */
template<typename T, typename I, typename H, typename E = std::equal_to<T>>
class UsdIndexedVector
{
public:
    /**
     * @brief Append data item.
     * @param[in] v Data item to append
     */
    void append(const T& v)
    {
        uint32_t idx;
        append(v, idx);
    }

    /**
     * @brief Append data item.
     *
     * @param[in] v Data item to append
     * @param[out] idx Index of the unique item corresponding to v
     * @return True if @p v was newly inserted into the set of unique data item
     */
    bool append(const T& v, uint32_t& outIdx)
    {
        bool insertedNew = false;
        auto iter = mIndexMap.find(v);
        if (iter == mIndexMap.end())
        {
            iter = mIndexMap.insert(std::make_pair(v, I(mValues.size()))).first;
            outIdx = mValues.size();
            mValues.push_back(v);
            insertedNew = true;
        }
        else
        {
            outIdx = iter->second;
        }
        mIndices.push_back(iter->second);
        return insertedNew;
    }
    /**
     * @brief Get the set of unique data items.
     */
    const pxr::VtArray<T>& getValues() const { return mValues; }

    /**
     * @brief Get the ordered list of item indices.
     */
    const pxr::VtArray<I>& getIndices() const { return mIndices; }

private:
    std::unordered_map<T, I, H, E> mIndexMap;
    pxr::VtArray<T> mValues;
    pxr::VtArray<I> mIndices;
};
} // namespace Falcor
