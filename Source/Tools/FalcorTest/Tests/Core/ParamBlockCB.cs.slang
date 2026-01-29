RWStructuredBuffer<float> result;

struct S
{
    float a;
};

ParameterBlock<S> gParamBlock;

[numthreads(1, 1, 1)]
void main()
{
    result[0] = gParamBlock.a;
}
