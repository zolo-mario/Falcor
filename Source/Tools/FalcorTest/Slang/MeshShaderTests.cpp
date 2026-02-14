/** Mesh shader compilation tests for Falcor/Slang.
    Verifies that mesh shader code compiles successfully.
    Based on Slang project mesh shader tests (tests/pipeline/rasterization/mesh/).
*/

#include "Testing/UnitTest.h"

namespace Falcor
{
/** Compilation-only test: verify mesh shader compiles via Falcor/Slang. */
GPU_TEST(MeshShaderCompilation, DEVICE_TYPES(Device::Type::D3D12, Device::Type::Vulkan))
{
    ProgramDesc desc;
    desc.addShaderLibrary("Slang/mesh-simple.slang").meshEntry("meshMain");
    desc.setShaderModel(ShaderModel::SM6_5);
    ctx.createProgram(desc);
}

/** Compilation-only test: verify amplification + mesh shader compiles (task dispatches mesh with payload). */
GPU_TEST(MeshTaskShaderCompilation, DEVICE_TYPES(Device::Type::D3D12, Device::Type::Vulkan))
{
    ProgramDesc desc;
    desc.addShaderLibrary("Slang/mesh-task-simple.slang")
        .amplificationEntry("taskMain")
        .meshEntry("meshMain");
    desc.setShaderModel(ShaderModel::SM6_5);
    ctx.createProgram(desc);
}

/** Compilation-only test: verify mesh shader with OutputPrimitives compiles. */
GPU_TEST(MeshPrimitiveOutputCompilation, DEVICE_TYPES(Device::Type::D3D12, Device::Type::Vulkan))
{
    ProgramDesc desc;
    desc.addShaderLibrary("Slang/mesh-primitive-output.slang").meshEntry("meshMain");
    desc.setShaderModel(ShaderModel::SM6_5);
    ctx.createProgram(desc);
}
} // namespace Falcor
