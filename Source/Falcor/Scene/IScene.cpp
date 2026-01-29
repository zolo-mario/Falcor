#pragma once
#include "IScene.h"
#include "Core/API/RenderContext.h"
#include "Core/Program/ShaderVar.h"
#include "Core/Program/ProgramVars.h"

namespace Falcor
{

/// Convenience function when the Scene wants to do something besides just calling raytrace.
/// TODO: Remove when no longer useful
void IScene::raytrace(RenderContext* renderContext, Program* pProgram, const ref<RtProgramVars>& pVars, uint3 dispatchDims)
{
    bindShaderDataForRaytracing(renderContext, pVars->getRootVar()["gScene"], pVars->getRayTypeCount());
    renderContext->raytrace(pProgram, pVars.get(), dispatchDims.x, dispatchDims.y, dispatchDims.z);
}

} // namespace Falcor
