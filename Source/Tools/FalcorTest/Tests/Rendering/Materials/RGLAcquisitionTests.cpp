#include "Testing/UnitTest.h"
#include "Scene/Scene.h"
#include "Scene/Material/StandardMaterial.h"
#include "Rendering/Materials/RGLAcquisition.h"

namespace Falcor
{
GPU_TEST(RGLAcquisition)
{
    // Create material.
    ref<StandardMaterial> pMaterial = StandardMaterial::create(ctx.getDevice(), "testMaterial");
    pMaterial->setBaseColor(float4(0.3f, 0.8f, 0.9f, 1.f));
    pMaterial->setMetallic(0.f);
    pMaterial->setRoughness(1.f);
    pMaterial->setSpecularTransmission(0.f);

    // Create and update scene containing the material.
    Scene::SceneData sceneData;
    sceneData.pMaterials = std::make_unique<MaterialSystem>(ctx.getDevice());
    MaterialID materialID = sceneData.pMaterials->addMaterial(pMaterial);

    ref<Scene> pScene = Scene::create(ctx.getDevice(), std::move(sceneData));
    auto updateFlags = pScene->update(ctx.getRenderContext(), 0.0);

    // Create acquisition class.
    RGLAcquisition acquisition(ctx.getDevice(), pScene);

    // Acquire BSDF.
    acquisition.acquireIsotropic(ctx.getRenderContext(), materialID);

    auto file = acquisition.toRGLFile();

    // There is no good way to test correctness of the output data, but
    // if we get here, we at least know the acquisition can complete without
    // crashing.
}
} // namespace Falcor
