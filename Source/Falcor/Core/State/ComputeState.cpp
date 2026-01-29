#include "ComputeState.h"
#include "Core/ObjectPython.h"
#include "Core/Program/ProgramVars.h"
#include "Utils/Scripting/ScriptBindings.h"

namespace Falcor
{

ref<ComputeState> ComputeState::create(ref<Device> pDevice)
{
    return ref<ComputeState>(new ComputeState(pDevice));
}

ComputeState::ComputeState(ref<Device> pDevice) : mpDevice(pDevice)
{
    mpCsoGraph = std::make_unique<ComputeStateGraph>();
}

ref<ComputeStateObject> ComputeState::getCSO(const ProgramVars* pVars)
{
    auto pProgramKernels = mpProgram ? mpProgram->getActiveVersion()->getKernels(mpDevice.get(), pVars) : nullptr;
    bool newProgram = (pProgramKernels.get() != mCachedData.pProgramKernels);
    if (newProgram)
    {
        mCachedData.pProgramKernels = pProgramKernels.get();
        mpCsoGraph->walk((void*)mCachedData.pProgramKernels);
    }

    ref<ComputeStateObject> pCso = mpCsoGraph->getCurrentNode();

    if (pCso == nullptr)
    {
        mDesc.pProgramKernels = pProgramKernels;

        ComputeStateGraph::CompareFunc cmpFunc = [&desc = mDesc](ref<ComputeStateObject> pCso) -> bool
        { return pCso && (desc == pCso->getDesc()); };

        if (mpCsoGraph->scanForMatchingNode(cmpFunc))
        {
            pCso = mpCsoGraph->getCurrentNode();
        }
        else
        {
            pCso = mpDevice->createComputeStateObject(mDesc);
            mpCsoGraph->setCurrentNodeData(pCso);
        }
    }

    return pCso;
}

FALCOR_SCRIPT_BINDING(ComputeState)
{
    pybind11::class_<ComputeState, ref<ComputeState>>(m, "ComputeState");
}
} // namespace Falcor
