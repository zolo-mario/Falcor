#pragma once

#include <cstdint>

namespace Falcor
{
struct DispatchArguments
{
    uint32_t ThreadGroupCountX;
    uint32_t ThreadGroupCountY;
    uint32_t ThreadGroupCountZ;
};

struct DrawArguments
{
    uint32_t VertexCountPerInstance;
    uint32_t InstanceCount;
    uint32_t StartVertexLocation;
    uint32_t StartInstanceLocation;
};

struct DrawIndexedArguments
{
    uint32_t IndexCountPerInstance;
    uint32_t InstanceCount;
    uint32_t StartIndexLocation;
    int32_t BaseVertexLocation;
    uint32_t StartInstanceLocation;
};
} // namespace Falcor
