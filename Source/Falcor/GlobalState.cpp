#include "GlobalState.h"
#include "Core/Error.h"

namespace Falcor
{

static SceneBuilder* spActivePythonSceneBuilder;    // TODO: REMOVEGLOBAL
static ref<Device> spActivePythonRenderGraphDevice; // TODO: REMOVEGLOBAL

void setActivePythonSceneBuilder(SceneBuilder* pSceneBuilder)
{
    spActivePythonSceneBuilder = pSceneBuilder;
}

SceneBuilder& accessActivePythonSceneBuilder()
{
    if (!spActivePythonSceneBuilder)
        FALCOR_THROW("This can only be called in a Python scene building context!");
    return *spActivePythonSceneBuilder;
}

AssetResolver& getActiveAssetResolver()
{
    return spActivePythonSceneBuilder ? spActivePythonSceneBuilder->getAssetResolver() : AssetResolver::getDefaultResolver();
}

void setActivePythonRenderGraphDevice(ref<Device> pDevice)
{
    spActivePythonRenderGraphDevice = pDevice;
}

ref<Device> getActivePythonRenderGraphDevice()
{
    return spActivePythonRenderGraphDevice;
}

ref<Device> accessActivePythonRenderGraphDevice()
{
    if (!spActivePythonRenderGraphDevice)
        FALCOR_THROW("This can only be called from a script executed in Mogwai or when loading a render graph file!");
    return spActivePythonRenderGraphDevice;
}

} // namespace Falcor
