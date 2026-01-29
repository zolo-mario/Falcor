RWStructuredBuffer<uint> result;

#ifdef USE_INT
typedef int Type
#else
typedef uint Type
#endif

#ifdef TEST_A

    //
    // Test generic interface.
    //

    interface ITestA<let N : Type> {
    static const Type scale;
    static void eval(Type val[N], out Type res[N]);
};

struct TestA<let N : Type> : ITestA<N>
{
    static const Type scale = 3 * N;
    static void eval(Type val[N], out Type res[N])
    {
        for (uint i = 0; i < N; i++)
            res[i] = scale * val[i];
    }
};

[numthreads(32, 1, 1)]
void testGenericsInterface(uint3 threadID: SV_DispatchThreadID)
{
    const uint i = threadID.x;
    if (i >= 32)
        return;

    Type val[4] = { i, i + 1, i + 2, i + 3 };
    Type res[4];

    TestA<4> test;
    test.eval(val, res);

    result[i * 4 + 0] = res[0];
    result[i * 4 + 1] = res[1];
    result[i * 4 + 2] = res[2];
    result[i * 4 + 3] = res[3];
}

#elif defined(TEST_B)

    //
    // Test generic function in interface.
    //

    interface ITestB {
    static void eval<let N : Type>(Type val[N], out Type res[N]);
};

struct TestB : ITestB
{
    static void eval<let N : Type>(Type val[N], out Type res[N])
    {
        static const Type scale = 3 * N;
        for (uint i = 0; i < N; i++)
            res[i] = scale * val[i];
    }
};

[numthreads(32, 1, 1)]
void testGenericsFunction(uint3 threadID: SV_DispatchThreadID)
{
    const uint i = threadID.x;
    if (i >= 32)
        return;

    Type val[4] = { i, i + 1, i + 2, i + 3 };
    Type res[4];

    TestB test;
    test.eval<4>(val, res);

    result[i * 4 + 0] = res[0];
    result[i * 4 + 1] = res[1];
    result[i * 4 + 2] = res[2];
    result[i * 4 + 3] = res[3];
}

#endif
