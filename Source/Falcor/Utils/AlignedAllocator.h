#pragma once
#include "Core/Error.h"
#include "Utils/Math/Common.h"
#include <new>
#include <utility>
#include <vector>

namespace Falcor
{

/**
 * Utility class for aligned memory allocations on the GPU.
 *
 * AlignedAllocator can enforce various alignment requirements,
 * including minimum byte alignment and (optionally) that
 * allocated objects don't span two cache lines if they can fit
 * into one.  Note that it's intended to be used to manage GPU
 * allocations and so it assumes that the base pointer starts at a
 * cache line.  As such, it doesn't provide any alignment
 * guarantees on the CPU side (where it doesn't matter anyway).
 */
class AlignedAllocator
{
public:
    /**
     * Sets the minimum alignment for allocated objects. If a value of
     * zero is provided, no additional alignment is performed.
     */
    void setMinimumAlignment(int minAlignment)
    {
        FALCOR_ASSERT(minAlignment == 0 || isPowerOf2(minAlignment));
        mMinAlignment = minAlignment;
    }

    /**
     * Sets the cache line size so that allocations can be aligned so
     * that they don't span multiple cache lines (if possible).  If a
     * value of zero is provided, then the allocator doesn't prevent
     * objects from spanning cache lines.
     */
    void setCacheLineSize(int cacheLineSize)
    {
        FALCOR_ASSERT(cacheLineSize == 0 || isPowerOf2(cacheLineSize));
        mCacheLineSize = cacheLineSize;
    }

    /**
     * Allocates an object of given type and executes its constructor.
     * @param[in] args Arguments to pass to the constructor.
     * @return pointer to allocated object.
     */
    template<typename T, typename... Args>
    T* allocate(Args&&... args)
    {
        const size_t size = sizeof(T);
        computeAndAllocatePadding(size);
        void* ptr = allocInternal(size);
        return new (ptr) T(std::forward<Args>(args)...);
    }

    /**
     * Allocates an object of given type, potentially including additional memory at
     * the end of it, and executes its constructor.
     * @param[in] size Amount of memory to allocate. Must be >= sizeof(T).
     * @param[in] args Arguments to pass to the constructor.
     * @return pointer to allocated object.
     */
    template<typename T, typename... Args>
    T* allocateSized(size_t size, Args&&... args)
    {
        FALCOR_ASSERT(size >= sizeof(T));
        computeAndAllocatePadding(size);
        void* ptr = allocInternal(size);
        return new (ptr) T(std::forward<Args>(args)...);
    }

    void reserve(size_t size) { mBuffer.reserve(size); }

    void resize(size_t size) { mBuffer.resize(size, 0); }

    /**
     * Returns the pointer to the start of the allocated buffer.
     */
    void* getStartPointer() { return mBuffer.data(); }
    const void* getStartPointer() const { return mBuffer.data(); }

    /**
     * Returns of the offset of the given pointer inside the allocation buffer.
     */
    size_t offsetOf(void* ptr) const
    {
        FALCOR_ASSERT(ptr >= mBuffer.data() && ptr < mBuffer.data() + mBuffer.size());
        return static_cast<uint8_t*>(ptr) - mBuffer.data();
    }

    void reset() { mBuffer.clear(); }

    size_t getSize() const { return mBuffer.size(); }
    size_t getCapacity() const { return mBuffer.capacity(); }

private:
    void computeAndAllocatePadding(size_t size)
    {
        size_t currentOffset = mBuffer.size();

        if (mMinAlignment > 0 && (currentOffset % mMinAlignment) != 0)
        {
            // We're not at the minimum alignment; get aligned.
            currentOffset += mMinAlignment - (currentOffset % mMinAlignment);
        }

        if (mCacheLineSize > 0)
        {
            const size_t cacheLineOffset = currentOffset % mCacheLineSize;
            if (size <= mCacheLineSize && cacheLineOffset + size > mCacheLineSize)
            {
                // The allocation is smaller than or equal to a cache line but
                // would span two cache lines; move to the start of the next cache line.
                currentOffset += mCacheLineSize - cacheLineOffset;
            }
        }

        size_t pad = currentOffset - mBuffer.size();
        if (pad > 0)
        {
            allocInternal(pad);
        }
        FALCOR_ASSERT(mMinAlignment == 0 || mBuffer.size() % mMinAlignment == 0);
    }

    void* allocInternal(size_t size)
    {
        auto iter = mBuffer.insert(mBuffer.end(), size, {});
        return &*iter;
    }

    size_t mMinAlignment = 16;
    size_t mCacheLineSize = 128;
    std::vector<uint8_t> mBuffer;
};
} // namespace Falcor
