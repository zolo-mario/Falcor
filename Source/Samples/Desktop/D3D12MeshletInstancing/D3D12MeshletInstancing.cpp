#include "D3D12MeshletInstancing.h"
#include "Scene/SceneBuilder.h"
#include "Scene/SceneMeshletData.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

namespace
{
    const uint32_t c_maxGroupDispatchCount = 65536u;

    template <typename T, typename U>
    T divRoundUp(T value, U divisor)
    {
        return (value + divisor - 1) / divisor;
    }
}

static const char kMeshShaderFile[] = "Samples/Desktop/D3D12MeshletInstancing/MeshletInstancingBunny.ms.slang";
static const float4 kClearColor(0.0f, 0.2f, 0.4f, 1.0f);

D3D12MeshletInstancing::D3D12MeshletInstancing(SampleApp* pHost) : SampleBase(pHost) {}

D3D12MeshletInstancing::~D3D12MeshletInstancing() {}

void D3D12MeshletInstancing::onLoad(RenderContext* pRenderContext)
{
    if (!getDevice()->isShaderModelSupported(ShaderModel::SM6_5))
    {
        logError("D3D12MeshletInstancing requires Shader Model 6.5 for mesh shader support.");
        return;
    }

    // Load bunny scene (Falcor bunny.pyscene)
    mpScene = SceneBuilder(getDevice(), "test_scenes/bunny.pyscene", Settings(), SceneBuilder::Flags::Default).getScene();
    if (!mpScene || mpScene->getGeometryInstanceCount() == 0)
    {
        logError("D3D12MeshletInstancing: Failed to load test_scenes/bunny.pyscene. Check FALCOR_MEDIA_FOLDERS.");
        return;
    }

    // Use Falcor camera system: Orbiter around scene center
    mpScene->setCameraController(Scene::CameraControllerType::Orbiter);
    mpScene->setCameraSpeed(25.f);
    mpScene->setCameraAspectRatio(16.f / 9.f);

    // Get meshlet count from scene
    SceneMeshletData* pMeshletData = mpScene->getMeshletData(pRenderContext);
    if (!pMeshletData || !pMeshletData->isValid())
    {
        logError("D3D12MeshletInstancing: Failed to build meshlet data.");
        return;
    }
    mMeshletCount = pMeshletData->getMeshletCount();

    // Program - mesh + pixel shader with Scene integration
    ProgramDesc desc;
    desc.addShaderModules(mpScene->getShaderModules());
    desc.addShaderLibrary(kMeshShaderFile).meshEntry("meshMain").psEntry("psMain");
    desc.addTypeConformances(mpScene->getTypeConformances());
    desc.setShaderModel(ShaderModel::SM6_5);

    DefineList defines;
    defines.add(mpScene->getSceneDefines());

    mpProgram = Program::create(getDevice(), desc, defines);
    mpVars = ProgramVars::create(getDevice(), mpProgram.get());

    mpState = GraphicsState::create(getDevice());
    mpState->setProgram(mpProgram);
    mpState->setVao(nullptr);
    mpState->setDepthStencilState(
        DepthStencilState::create(DepthStencilState::Desc().setDepthEnabled(true).setDepthFunc(ComparisonFunc::LessEqual)));

    regenerateInstances();
}

void D3D12MeshletInstancing::regenerateInstances()
{
    mUpdateInstances = true;

    float radius = 1.0f;
    if (mpScene && mpScene->getSceneBounds().valid())
        radius = mpScene->getSceneBounds().radius();

    const float padding = 0.0f;
    const float spacing = (1.0f + padding) * radius * 2.0f;

    uint32_t width = mInstanceLevel * 2 + 1;
    float extents = spacing * (float)mInstanceLevel;

    // Special case: mInstanceLevel=0 -> 2 instances (for debugging instance>1)
    mInstanceCount = (mInstanceLevel == 0) ? 2u : (width * width * width);

    mInstanceData.resize(mInstanceCount);
    for (uint32_t i = 0; i < mInstanceCount; ++i)
    {
        float3 location;
        if (mInstanceLevel == 0)
        {
            location = float3((float)i * spacing - spacing * 0.5f, 0.f, 0.f);
        }
        else
        {
            float ix = (float)(i % width);
            float iy = (float)((i / width) % width);
            float iz = (float)(i / (width * width));
            location = float3(ix, iy, iz) * spacing - float3(extents, extents, extents);
        }

        float4x4 world = math::matrixFromTranslation(location);
        float4x4 worldInvTranspose = math::transpose(math::inverse(world));

        // Store transpose for mul(position, World) row*matrix (match original DX sample)
        mInstanceData[i].World = math::transpose(world);
        mInstanceData[i].WorldInvTranspose = math::transpose(worldInvTranspose);
    }

    size_t instanceBufferSize = mInstanceCount * sizeof(InstanceData);
    instanceBufferSize = (instanceBufferSize + 255) & ~255;

    mpInstanceBuffer = getDevice()->createBuffer(
        (uint32_t)instanceBufferSize,
        ResourceBindFlags::ShaderResource,
        MemoryType::Upload,
        mInstanceData.data());
}

void D3D12MeshletInstancing::onShutdown()
{
    mpScene.reset();
}

void D3D12MeshletInstancing::onResize(uint32_t width, uint32_t height)
{
    if (mpScene)
        mpScene->setCameraAspectRatio(width / (float)height);
}

void D3D12MeshletInstancing::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    pRenderContext->clearFbo(pTargetFbo.get(), kClearColor, 1.0f, 0, FboAttachmentType::All);

    if (!mpProgram || !mpVars || !mpScene || mMeshletCount == 0)
        return;

    // Update scene (camera, animations)
    mpScene->update(pRenderContext, (float)getFrameRate().getLastFrameTime());

    // Update instance buffer if needed
    if (mUpdateInstances)
    {
        size_t instanceBufferSize = mInstanceCount * sizeof(InstanceData);
        instanceBufferSize = (instanceBufferSize + 255) & ~255;
        mpInstanceBuffer->setBlob(mInstanceData.data(), 0, (uint32_t)(mInstanceCount * sizeof(InstanceData)));
        mUpdateInstances = false;
    }

    SceneMeshletData* pMeshletData = mpScene->getMeshletData(pRenderContext);
    if (!pMeshletData || !pMeshletData->isValid())
        return;

    mpState->setFbo(pTargetFbo, true);

    auto var = mpVars->getRootVar();
    var["CB"]["gDrawMeshlets"] = mDrawMeshlets ? 1u : 0u;
    var["CB"]["gDebugInstanceColor"] = mDebugInstanceColor ? 1u : 0u;
    var["gMeshlets"] = pMeshletData->getMeshletBuffer();
    var["gMeshletVertices"] = pMeshletData->getMeshletVerticesBuffer();
    var["gMeshletTriangles"] = pMeshletData->getMeshletTrianglesBuffer();
    mpScene->bindShaderDataForRaytracing(pRenderContext, var["gScene"]);
    var["Instances"] = mpInstanceBuffer;

    struct DrawParamsData
    {
        uint32_t InstanceCount;
        uint32_t InstanceOffset;
    };
    struct MeshInfoData
    {
        uint32_t MeshletCount;
        uint32_t MeshletOffset;
    };

    // Batch by meshlet (not instance) to avoid InstanceOffset>0 which may cause deformation
    uint32_t maxMeshletsPerBatch = std::max(1u, c_maxGroupDispatchCount / mInstanceCount);
    uint32_t dispatchCount = divRoundUp(mMeshletCount, maxMeshletsPerBatch);

    for (uint32_t j = 0; j < dispatchCount; ++j)
    {
        uint32_t meshletOffset = maxMeshletsPerBatch * j;
        uint32_t meshletCount = std::min(mMeshletCount - meshletOffset, maxMeshletsPerBatch);

        var["DrawParamsCB"]["DrawParams_data"]["InstanceCount"] = mInstanceCount;
        var["DrawParamsCB"]["DrawParams_data"]["InstanceOffset"] = 0u;
        var["MeshInfoCB"]["MeshInfo_data"]["MeshletCount"] = meshletCount;
        var["MeshInfoCB"]["MeshInfo_data"]["MeshletOffset"] = meshletOffset;

        uint32_t groupCount = meshletCount * mInstanceCount;
        pRenderContext->drawMeshTasks(mpState.get(), mpVars.get(), groupCount, 1, 1);
    }
}

void D3D12MeshletInstancing::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "D3D12 Meshlet Instancing", {250, 200});
    renderGlobalUI(pGui);
    w.text("D3D12 Meshlet Instancing - Falcor bunny + SceneMeshletData");
    w.text("+/- : Instance level | Space : Toggle meshlet colors");
    w.checkbox("Draw Meshlets (color by meshlet)", mDrawMeshlets);
    w.checkbox("Debug: Color by instance (instance0=red,1=green)", mDebugInstanceColor);
    w.text(fmt::format("Instances: {} | Meshlets: {}", mInstanceCount, mMeshletCount));
}

bool D3D12MeshletInstancing::onKeyEvent(const KeyboardEvent& keyEvent)
{
    if (keyEvent.type == KeyboardEvent::Type::KeyPressed)
    {
        switch (keyEvent.key)
        {
        case Input::Key::Equal:   // + key on main keyboard (Shift+=)
        case Input::Key::KeypadAdd:
            ++mInstanceLevel;
            regenerateInstances();
            return true;
        case Input::Key::Minus:
        case Input::Key::KeypadSubtract:
            if (mInstanceLevel != 0)
            {
                --mInstanceLevel;
                regenerateInstances();
            }
            return true;
        case Input::Key::Space:
            mDrawMeshlets = !mDrawMeshlets;
            return true;
        default:
            break;
        }
    }
    return mpScene && mpScene->onKeyEvent(keyEvent);
}

bool D3D12MeshletInstancing::onMouseEvent(const MouseEvent& mouseEvent)
{
    return mpScene && mpScene->onMouseEvent(mouseEvent);
}

void D3D12MeshletInstancing::onHotReload(HotReloadFlags reloaded) {}

SampleBase* D3D12MeshletInstancing::create(SampleApp* pHost)
{
    return new D3D12MeshletInstancing(pHost);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<SampleBase, D3D12MeshletInstancing>();
}
