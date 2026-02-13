/** Test for SlangUserGuide 02-conventional-features.

    验证 Slang 常规语言特性：标量、向量、数组、结构体、枚举、函数等。
    参考：https://shader-slang.org/slang/user-guide/conventional-features.html
*/

#include "Testing/UnitTest.h"
#include "Utils/Math/ScalarMath.h"

namespace Falcor
{
GPU_TEST(ConventionalFeaturesTypes)
{
    const uint32_t nElements = 20;

    ctx.createProgram("SlangUserGuide/02-conventional-features/conventional-features.slang", "computeMain");
    ctx.allocateStructuredBuffer("result", nElements);
    ctx.runProgram(1, 1, 1);

    std::vector<uint32_t> result = ctx.readBuffer<uint32_t>("result");
    EXPECT_EQ(result.size(), nElements);

    uint32_t idx = 0;

    // 整数: a8+128=0, a16=32767, a32=2147483647
    EXPECT_EQ(result[idx++], 0u);
    EXPECT_EQ(result[idx++], 32767u);
    EXPECT_EQ(result[idx++], 2147483647u);

    // 无符号: u8=255, u16=65535, u32=0xFFFFFFFF
    EXPECT_EQ(result[idx++], 255u);
    EXPECT_EQ(result[idx++], 65535u);
    EXPECT_EQ(result[idx++], 0xFFFFFFFFu);

    // 浮点 3.14f
    EXPECT_EQ(result[idx++], 0x4048f5c3u); // asuint(3.14f)

    // 布尔
    EXPECT_EQ(result[idx++], 1u);
    EXPECT_EQ(result[idx++], 0u);

    // 向量 v2(1,2), v3.z=3, v4.w=4
    EXPECT_EQ(result[idx++], asuint(1.0f));
    EXPECT_EQ(result[idx++], asuint(2.0f));
    EXPECT_EQ(result[idx++], asuint(3.0f));
    EXPECT_EQ(result[idx++], asuint(4.0f));

    // 数组 {1,2,3}
    EXPECT_EQ(result[idx++], 1u);
    EXPECT_EQ(result[idx++], 2u);
    EXPECT_EQ(result[idx++], 3u);

    // 结构体 data.id=1, data.value=10.0f
    EXPECT_EQ(result[idx++], 1u);
    EXPECT_EQ(result[idx++], asuint(10.0f));

    // 枚举 Color.Red = 0
    EXPECT_EQ(result[idx++], 0u);

    // add(5,10) = 15.0f
    EXPECT_EQ(result[idx++], asuint(15.0f));
}
} // namespace Falcor
