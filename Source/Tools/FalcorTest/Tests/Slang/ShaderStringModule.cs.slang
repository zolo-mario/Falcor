import GeneratedModule;

RWStructuredBuffer<uint> result;

[numthreads(32, 1, 1)]
void main(uint3 threadID: SV_DispatchThreadID)
{
    // The function f() is defined in the generated module.
    result[threadID.x] = f(threadID.x);
}
