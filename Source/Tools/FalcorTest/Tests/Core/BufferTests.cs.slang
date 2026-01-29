/** Unit tests for RMW operations on different buffer types.

    The updateBuffer() kernel is run multiple times followed by readBuffer()
    that copies the result to the output buffer.
*/

#if TYPE == 0
RWByteAddressBuffer buffer;
#elif TYPE == 1
RWBuffer<uint> buffer;
#elif TYPE == 2
RWStructuredBuffer<uint> buffer;
#endif

RWStructuredBuffer<uint> result;

// Clear kernel.
// This is necessary for RWStructuredBuffer as we can't use ID3D12CommandList::ClearUnorderedAccessViewUint for structured buffers.
[numthreads(256, 1, 1)]
void clearBuffer(uint3 threadId: SV_DispatchThreadID)
{
    uint i = threadId.x;
#if TYPE == 0
    buffer.Store(i * 4, 0);
#else
    buffer[i] = 0;
#endif
}

[numthreads(256, 1, 1)]
void updateBuffer(uint3 threadId: SV_DispatchThreadID)
{
    uint i = threadId.x;
#if TYPE == 0
    uint val = buffer.Load(i * 4);
    val += i + 1;
    buffer.Store(i * 4, val);
#else
    buffer[i] += i + 1;
#endif
}

[numthreads(256, 1, 1)]
void readBuffer(uint3 threadId: SV_DispatchThreadID)
{
    uint i = threadId.x;
#if TYPE == 0
    result[i] = buffer.Load(i * 4);
#else
    result[i] = buffer[i];
#endif
}

// Structures for verifying various buffer sizes when allocated on CPU via reflection
struct BufferSizeTest1_12B_T
{
    int16_t a;
    float b;
    int16_t c;
};
struct BufferSizeTest2_2B_T
{
    int16_t a;
};
struct BufferSizeTest3_24B_T
{
    int16_t a;
    float64_t b;
    int16_t c;
};
struct BufferSizeTest4_24B_T
{
    int16_t a;
    float4 b;
    int16_t c;
};
RWStructuredBuffer<BufferSizeTest1_12B_T> bufferSizeTest1_12B_buffer;
RWStructuredBuffer<BufferSizeTest2_2B_T> bufferSizeTest2_2B_buffer;
RWStructuredBuffer<BufferSizeTest3_24B_T> bufferSizeTest3_24B_buffer;
RWStructuredBuffer<BufferSizeTest4_24B_T> bufferSizeTest4_24B_buffer;

// Dummy computer shader to write to buffers, as this allows DXIL to be generated that references them
[numthreads(1, 1, 1)]
void writeSizeTest(uint3 threadId: SV_DispatchThreadID)
{
    bufferSizeTest1_12B_buffer[0] = {};
    bufferSizeTest2_2B_buffer[0] = {};
    bufferSizeTest3_24B_buffer[0] = {};
    bufferSizeTest4_24B_buffer[0] = {};
}
