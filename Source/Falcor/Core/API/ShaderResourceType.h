#pragma once

namespace Falcor
{
enum class ShaderResourceType
{
    TextureSrv,
    TextureUav,
    RawBufferSrv,
    RawBufferUav,
    TypedBufferSrv,
    TypedBufferUav,
    Cbv,
    StructuredBufferUav,
    StructuredBufferSrv,
    AccelerationStructureSrv,
    Dsv,
    Rtv,
    Sampler,

    Count
};
}
