#include "BaseGraphicsPass.h"

namespace Falcor
{
BaseGraphicsPass::BaseGraphicsPass(ref<Device> pDevice, const ProgramDesc& progDesc, const DefineList& programDefines) : mpDevice(pDevice)
{
    auto pProg = Program::create(mpDevice, progDesc, programDefines);
    pProg->breakStrongReferenceToDevice();

    mpState = GraphicsState::create(mpDevice);
    mpState->breakStrongReferenceToDevice();
    mpState->setProgram(pProg);

    mpVars = ProgramVars::create(mpDevice, pProg.get());
}

void BaseGraphicsPass::addDefine(const std::string& name, const std::string& value, bool updateVars)
{
    mpState->getProgram()->addDefine(name, value);
    if (updateVars)
        mpVars = ProgramVars::create(mpDevice, mpState->getProgram().get());
}

void BaseGraphicsPass::removeDefine(const std::string& name, bool updateVars)
{
    mpState->getProgram()->removeDefine(name);
    if (updateVars)
        mpVars = ProgramVars::create(mpDevice, mpState->getProgram().get());
}

void BaseGraphicsPass::setVars(const ref<ProgramVars>& pVars)
{
    mpVars = pVars ? pVars : ProgramVars::create(mpDevice, mpState->getProgram().get());
}

void BaseGraphicsPass::breakStrongReferenceToDevice()
{
    mpDevice.breakStrongReference();
}

} // namespace Falcor
