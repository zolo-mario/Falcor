#pragma once
#include "StateGraph.h"
#include "Core/Macros.h"
#include "Core/Object.h"
#include "Core/API/ComputeStateObject.h"
#include "Core/Program/Program.h"
#include <memory>

namespace Falcor
{
class ProgramVars;

/**
 * Compute state.
 * This class contains the entire state required by a single dispatch call. It's not an immutable object - you can change it dynamically
 * during rendering. The recommended way to use it is to create multiple ComputeState objects (ideally, a single object per program)
 */
class FALCOR_API ComputeState : public Object
{
    FALCOR_OBJECT(ComputeState)
public:
    ~ComputeState() = default;

    /**
     * Create a new state object.
     * @param pDevice GPU device.
     * @return A new object, or an exception is thrown if creation failed.
     */
    static ref<ComputeState> create(ref<Device> pDevice);

    /**
     * Bind a program to the pipeline
     */
    ComputeState& setProgram(ref<Program> pProgram)
    {
        mpProgram = pProgram;
        return *this;
    }

    /**
     * Get the currently bound program
     */
    ref<Program> getProgram() const { return mpProgram; }

    /**
     * Get the active compute state object
     */
    ref<ComputeStateObject> getCSO(const ProgramVars* pVars);

private:
    ComputeState(ref<Device> pDevice);

    ref<Device> mpDevice;
    ref<Program> mpProgram;
    ComputeStateObjectDesc mDesc;

    struct CachedData
    {
        const ProgramKernels* pProgramKernels = nullptr;
    };
    CachedData mCachedData;

    using ComputeStateGraph = StateGraph<ref<ComputeStateObject>, void*>;
    std::unique_ptr<ComputeStateGraph> mpCsoGraph;
};
} // namespace Falcor
