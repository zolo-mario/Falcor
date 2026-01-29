#include "Testing/UnitTest.h"
#include "Utils/Math/Rectangle.h"

namespace Falcor
{

CPU_TEST(Rectangle_Constructors)
{
    Rectangle tile0;
    EXPECT_FALSE(tile0.valid());

    Rectangle tile1(float2(0.5f));
    EXPECT_TRUE(tile1.valid());
    EXPECT_EQ(tile1.center(), float2(0.5f));
    EXPECT_EQ(tile1.extent(), float2(0.f));

    tile0.set(float2(0.5f));
    EXPECT(tile0 == tile1);
    EXPECT(tile0 == tile0.intersection(tile1));
}

CPU_TEST(Rectangle_Comparisons)
{
    Rectangle tile0;
    EXPECT_FALSE(tile0.valid());

    tile0.include(float2(-1.f));
    EXPECT_TRUE(tile0.valid());
    EXPECT_EQ(tile0.area(), 0.f);

    tile0.include(float2(1.f));
    EXPECT_TRUE(tile0.valid());
    EXPECT_EQ(tile0.area(), 4.f);
    EXPECT_EQ(tile0.extent(), float2(2.f));

    Rectangle tile1(float2(0.f), float2(2.f));

    Rectangle tile2 = tile0.intersection(tile1);
    Rectangle tile3 = tile1.intersection(tile2);

    EXPECT(tile2 == tile3);

    EXPECT_EQ(tile2.maxPoint, float2(1.f));
    EXPECT_EQ(tile2.minPoint, float2(0.f));
}

CPU_TEST(Rectangle_Contains)
{
    Rectangle invalid;
    ASSERT_FALSE(invalid.valid());

    Rectangle big(float2(-1.f), float2(1.f));
    Rectangle small0(float2(0.f), float2(1.f));
    Rectangle small1(float2(-1.f), float2(0.f));
    Rectangle small2(float2(-1.1f), float2(0.f));

    EXPECT(big.contains(big));
    EXPECT(big.contains(small0));
    EXPECT(big.contains(small1));
    EXPECT_FALSE(big.contains(small2));

    Rectangle invalid0;
    ASSERT_FALSE(invalid0.valid());
    EXPECT_FALSE(invalid0.overlaps(small0));
    EXPECT_FALSE(invalid0.overlaps(small1));
    EXPECT_FALSE(invalid0.overlaps(small2));
    EXPECT_FALSE(small0.overlaps(invalid0));
    EXPECT_FALSE(small1.overlaps(invalid0));
    EXPECT_FALSE(small2.overlaps(invalid0));

    Rectangle invalid1;
    ASSERT_FALSE(invalid1.valid());
    EXPECT_FALSE(invalid0.overlaps(invalid1));
}

CPU_TEST(Rectangle_Overlaps)
{
    Rectangle tile0(float2(-1.f), float2(1.f));
    Rectangle tile1(float2(0.f), float2(2.f));
    Rectangle tile2(float2(1.f), float2(2.f));

    EXPECT(tile0.overlaps(tile0));
    EXPECT(tile0.overlaps(tile1));
    EXPECT(tile1.overlaps(tile0));
    EXPECT_FALSE(tile0.overlaps(tile2));
    EXPECT_FALSE(tile2.overlaps(tile0));

    Rectangle invalid0;
    ASSERT_FALSE(invalid0.valid());
    EXPECT_FALSE(invalid0.overlaps(tile0));
    EXPECT_FALSE(invalid0.overlaps(tile1));
    EXPECT_FALSE(invalid0.overlaps(tile2));
    EXPECT_FALSE(tile0.overlaps(invalid0));
    EXPECT_FALSE(tile1.overlaps(invalid0));
    EXPECT_FALSE(tile2.overlaps(invalid0));

    Rectangle invalid1;
    ASSERT_FALSE(invalid1.valid());
    EXPECT_FALSE(invalid0.overlaps(invalid1));
}

} // namespace Falcor
