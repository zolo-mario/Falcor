#include "Testing/UnitTest.h"
#include "Utils/AlignedAllocator.h"

namespace Falcor
{
template<int N>
struct SizedStruct
{
    char buf[N];
};

CPU_TEST(AlignedAllocator)
{
    AlignedAllocator alloc;
    alloc.setMinimumAlignment(16);
    alloc.setCacheLineSize(128);
    alloc.reserve(1024);
    EXPECT_EQ(1024, alloc.getCapacity());
    EXPECT_EQ(0, alloc.getSize());

    // Do an initial 15 byte allocation. Make sure that everything
    // makes sense.
    EXPECT_EQ(15, sizeof(SizedStruct<15>));
    void* ptr = alloc.allocate<SizedStruct<15>>();
    EXPECT_EQ(15, alloc.getSize());
    EXPECT_EQ(0, alloc.offsetOf(ptr));
    EXPECT_EQ(0, reinterpret_cast<char*>(ptr) - reinterpret_cast<char*>(alloc.getStartPointer()));

    // Allocate another 8 bytes. Make sure it starts 16-byte aligned.
    ptr = alloc.allocate<SizedStruct<8>>();
    EXPECT_EQ(24, alloc.getSize());
    EXPECT_EQ(16, alloc.offsetOf(ptr));

    // Do a one byte allocation and make sure it also starts aligned.
    ptr = alloc.allocate<char>();
    EXPECT_EQ(33, alloc.getSize());
    EXPECT_EQ(32, alloc.offsetOf(ptr));

    // A 100 byte allocation should start at a new cache line now.
    ptr = alloc.allocate<SizedStruct<100>>();
    EXPECT_EQ(128, alloc.offsetOf(ptr));
    EXPECT_EQ(228, alloc.getSize());

    // The next aligned offset is 240. A 17 byte allocation should start at a new cache line now.
    ptr = alloc.allocate<SizedStruct<17>>();
    EXPECT_EQ(256, alloc.offsetOf(ptr));
    EXPECT_EQ(273, alloc.getSize());
}

CPU_TEST(AlignedAllocatorNoCacheLine)
{
    AlignedAllocator alloc;
    alloc.setMinimumAlignment(16);
    alloc.setCacheLineSize(0); // Don't worry about allocations that span cache lines.
    alloc.reserve(1024);
    EXPECT_EQ(1024, alloc.getCapacity());
    EXPECT_EQ(0, alloc.getSize());

    void* ptr = alloc.allocate<SizedStruct<64>>();
    EXPECT_EQ(64, alloc.getSize());
    EXPECT_EQ(0, alloc.offsetOf(ptr));

    // Now allocate 72 bytes. It should be immediately after the
    // initial allocation since we're already aligned.
    ptr = alloc.allocate<SizedStruct<72>>();
    EXPECT_EQ(64 + 72, alloc.getSize());
    EXPECT_EQ(64, alloc.offsetOf(ptr));
}
} // namespace Falcor
