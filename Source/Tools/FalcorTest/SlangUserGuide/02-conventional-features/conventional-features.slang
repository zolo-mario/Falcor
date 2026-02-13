/** Slang 常规语言特性示例 - 基于 https://shader-slang.org/slang/user-guide/conventional-features.html

    测试标量、向量、矩阵、数组、结构体、枚举、函数等类型。
    结果写入 result 缓冲区供 CPU 验证。
*/

RWStructuredBuffer<uint> result;

// 结构体类型示例
struct MyData
{
    int id;
    float value;
    float3 position;
};

// 枚举类型示例
enum Color
{
    Red,
    Green,
    Blue
};

// 函数示例
float add(float x, float y)
{
    return x + y;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    uint idx = 0;

    // 整数类型 -> 存为 uint (取低 32 位或验证值)
    int8_t a8 = -128;
    int16_t a16 = 32767;
    int a32 = 2147483647;
    result[idx++] = (uint)(a8 + 128);   // 0
    result[idx++] = (uint)a16;          // 32767
    result[idx++] = (uint)a32;          // 2147483647

    uint8_t u8 = 255;
    uint16_t u16 = 65535;
    uint u32 = 4294967295u;
    result[idx++] = (uint)u8;           // 255
    result[idx++] = (uint)u16;          // 65535
    result[idx++] = u32;                // 需特殊处理 64 位，存 0xFFFFFFFF

    // 浮点数 -> 存 asuint
    float f = 3.14f;
    result[idx++] = asuint(f);

    // 布尔 -> 存 0/1
    bool isTrue = true;
    bool isFalse = false;
    result[idx++] = isTrue ? 1u : 0u;
    result[idx++] = isFalse ? 1u : 0u;

    // 向量
    float2 v2 = float2(1.0, 2.0);
    float3 v3 = float3(1.0, 2.0, 3.0);
    float4 v4 = float4(1.0, 2.0, 3.0, 4.0);
    result[idx++] = asuint(v2.x);
    result[idx++] = asuint(v2.y);
    result[idx++] = asuint(v3.z);
    result[idx++] = asuint(v4.w);

    // 数组
    int array1[3] = {1, 2, 3};
    result[idx++] = (uint)array1[0];
    result[idx++] = (uint)array1[1];
    result[idx++] = (uint)array1[2];

    // 结构体与枚举
    MyData data;
    data.id = 1;
    data.value = 10.0f;
    data.position = float3(1.0, 2.0, 3.0);

    Color myColor = Color.Red;

    result[idx++] = (uint)data.id;
    result[idx++] = asuint(data.value);
    result[idx++] = (uint)myColor;

    // 函数调用
    float sum = add(5.0f, 10.0f);
    result[idx++] = asuint(sum);
}
