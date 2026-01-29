#pragma once

#include "Device.h"
#include <slang-gfx.h>

namespace Falcor
{
class Scene;
class Program;
class RtStateObject;
class RtProgramVars;
class RenderContext;

// clang-format off
/**
 * This class represents the GPU shader table for raytracing programs.
 * We are using the following layout for the shader table:
 *
 * +------------+--------+--------+-----+--------+---------+--------+-----+--------+--------+-----+--------+-----+---------+---------+-----+---------+
 * |            |        |        | ... |        |         |        | ... |        |        | ... |        | ... |         |         | ... |         |
 * |   RayGen   |  Miss  |  Miss  | ... |  Miss  |  Hit    |  Hit   | ... |  Hit   |  Hit   | ... |  Hit   | ... |  Hit    |  Hit    | ... |  Hit    |
 * |   Entry    |  Idx0  |  Idx1  | ... | IdxM-1 |  Ray0   |  Ray1  | ... | RayK-1 |  Ray0  | ... | RayK-1 | ... |  Ray0   |  Ray1   | ... | RayK-1  |
 * |            |        |        | ... |        |  Geom0  |  Geom0 | ... |  Geom0 |  Geom1 | ... |  Geom1 | ... | GeomN-1 | GeomN-1 | ... | GeomN-1 |
 * +------------+--------+--------+-----+--------+---------+--------+-----+--------+--------+-----+--------+-----+---------+---------+-----+---------+
 *
 * The first record is the ray gen record, followed by the M miss records, followed by the geometry hit group records.
 * For each of the N geometries in the scene we have K hit group records, where K is the number of ray types (the same for all geometries).
 * The size of each record is based on the requirements of the local root signatures. By default, raygen, miss, and hit group records
 * contain only the program identifier (32B).
 *
 * User provided local root signatures are currently not supported for performance reasons. Managing and updating data for custom root
 * signatures results in significant overhead. To get the root signature that matches this table, call the static function
 * getRootSignature().
 */
// clang-format on

// In GFX, we use gfx::IShaderTable directly. We wrap
// the ComPtr with `ShaderTablePtr` class so it will be freed
// with the deferred release mechanism.
class ShaderTablePtr
{
public:
    ShaderTablePtr(ref<Device> pDevice) : mpDevice(pDevice) {}

    gfx::IShaderTable& operator*() { return *mTable; }

    gfx::IShaderTable* operator->() { return mTable; }

    gfx::IShaderTable* get() { return mTable.get(); }

    gfx::IShaderTable** writeRef() { return mTable.writeRef(); }

    operator gfx::IShaderTable*() { return mTable.get(); }

    ~ShaderTablePtr() { mpDevice->releaseResource(mTable); }

private:
    ref<Device> mpDevice;
    Slang::ComPtr<gfx::IShaderTable> mTable;
};
} // namespace Falcor
