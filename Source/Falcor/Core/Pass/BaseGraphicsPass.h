#pragma once
#include "Core/Macros.h"
#include "Core/Object.h"
#include "Core/State/GraphicsState.h"
#include "Core/Program/Program.h"
#include "Core/Program/ProgramVars.h"
#include "Core/Program/ShaderVar.h"
#include <string>

namespace Falcor
{
class FALCOR_API BaseGraphicsPass : public Object
{
    FALCOR_OBJECT(BaseGraphicsPass)
public:
    virtual ~BaseGraphicsPass() = default;

    /**
     * Add a define
     */
    void addDefine(const std::string& name, const std::string& value = "", bool updateVars = false);

    /**
     * Remove a define
     */
    void removeDefine(const std::string& name, bool updateVars = false);

    /**
     * Get the program
     */
    ref<Program> getProgram() const { return mpState->getProgram(); }

    /**
     * Get the state
     */
    const ref<GraphicsState>& getState() const { return mpState; }

    /**
     * Get the vars
     */
    const ref<ProgramVars>& getVars() const { return mpVars; }

    ShaderVar getRootVar() const { return mpVars->getRootVar(); }

    /**
     * Set a vars object. Allows the user to override the internal vars, for example when one wants to share a vars object between different
     * passes.
     * @param[in] pVars The new ProgramVars object. If this is nullptr, then the pass will automatically create a new ProgramVars object
     */
    void setVars(const ref<ProgramVars>& pVars);

    void breakStrongReferenceToDevice();

protected:
    /**
     * Create a new object.
     * @param[in] pDevice GPU device.
     * @param[in] progDesc The program description.
     * @param[in] programDefines List of macro definitions to set into the program. The macro definitions will be set on all shader stages.
     * @return A new object, or an exception is thrown if creation failed.
     */
    BaseGraphicsPass(ref<Device> pDevice, const ProgramDesc& progDesc, const DefineList& programDefines);

    BreakableReference<Device> mpDevice;
    ref<ProgramVars> mpVars;
    ref<GraphicsState> mpState;
};
} // namespace Falcor
