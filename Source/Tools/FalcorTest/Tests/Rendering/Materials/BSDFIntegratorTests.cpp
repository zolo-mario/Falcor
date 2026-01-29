#include "Testing/UnitTest.h"
#include "Scene/Scene.h"
#include "Scene/Material/StandardMaterial.h"
#include "Rendering/Materials/BSDFIntegrator.h"

namespace Falcor
{
namespace
{
const float3 kExpectedResults[] = {
    {0.271488f, 0.666471f, 0.745583f},
    {0.230911f, 0.580707f, 0.650769f},
    {0.220602f, 0.562734f, 0.631260f},
    {0.218110f, 0.560894f, 0.629551f},
};

const float kMaxL2 = 1e-6f;
} // namespace

GPU_TEST(BSDFIntegrator)
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

    // Create BSDF integrator utility.
    BSDFIntegrator integrator(ctx.getDevice(), pScene);

    // Integrate BSDF.
    std::vector<float> cosThetas = {0.25f, 0.5f, 0.75f, 1.f};
    auto results = integrator.integrateIsotropic(ctx.getRenderContext(), materialID, cosThetas);

    // Validate results.
    for (size_t i = 0; i < cosThetas.size(); i++)
    {
        float3 e = results[i] - kExpectedResults[i];
        float l2 = std::sqrt(dot(e, e));
        EXPECT_LE(l2, kMaxL2) << " result=" << to_string(results[i]) << " expected=" << to_string(kExpectedResults[i])
                              << " cosTheta=" << cosThetas[i];
    }
}
} // namespace Falcor
