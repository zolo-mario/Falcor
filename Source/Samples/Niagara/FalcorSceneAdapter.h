#pragma once

#include "SceneFormat.h"

namespace Falcor
{

class Scene;

bool convertFalcorSceneToNiagaraScene(Scene* pScene, NiagaraFormat::NiagaraSceneFormat& outScene);

}
