/** Test basic compute shader support for the given shader model.
 */
RWStructuredBuffer<uint> result;

[numthreads(256, 1, 1)]
void main(uint3 threadID: SV_DispatchThreadID)
{
    uint i = threadID.x;
    result[i] = 3 * i;
}
