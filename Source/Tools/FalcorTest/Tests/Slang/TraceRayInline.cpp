#include "Testing/UnitTest.h"

namespace Falcor
{
GPU_TEST(testTraceRayInlineAPI)
{
    // We don't actually run the program, just make sure it compiles.
    ctx.createProgram(
        "Tests/Slang/TraceRayInline.cs.slang", "testTraceRayInlineAPI", DefineList(), SlangCompilerFlags::None, ShaderModel::SM6_5
    );
}
} // namespace Falcor
