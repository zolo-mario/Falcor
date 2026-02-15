#include "D3D12MeshletInstancing.h"
#include "Core/Platform/OS.h"

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

uint32_t mSampleGuiWidth = 250;
uint32_t mSampleGuiHeight = 200;
uint32_t mSampleGuiPositionX = 20;
uint32_t mSampleGuiPositionY = 40;

D3D12MeshletInstancing::D3D12MeshletInstancing(const SampleAppConfig& config) : SampleApp(config) {}

D3D12MeshletInstancing::~D3D12MeshletInstancing() {}

std::filesystem::path D3D12MeshletInstancing::findToyRobotBin()
{
    auto rt = getRuntimeDirectory();
    std::vector<std::filesystem::path> searchPaths = {
        rt / "ToyRobot.bin",
        rt / "data" / "ToyRobot.bin",
        rt.parent_path() / "DirectX-Graphics-Samples" / "Samples" / "Desktop" / "D3D12MeshShaders" / "Assets" /
            "ToyRobot.bin",
        rt.parent_path().parent_path() / "DirectX-Graphics-Samples" / "Samples" / "Desktop" / "D3D12MeshShaders" /
            "Assets" / "ToyRobot.bin",
    };

    for (const auto& p : searchPaths)
    {
        if (std::filesystem::exists(p))
            return p;
    }
    return rt / "ToyRobot.bin";
}

void D3D12MeshletInstancing::onLoad(RenderContext* pRenderContext)
{
    if (!getDevice()->isShaderModelSupported(ShaderModel::SM6_5))
    {
        logError("D3D12MeshletInstancing requires Shader Model 6.5 for mesh shader support.");
        return;
    }

    auto path = findToyRobotBin();
    if (!mModel.loadFromFile(getDevice().get(), path))
    {
        logError("Failed to load ToyRobot.bin. Place it in the runtime directory or build DirectX-Graphics-Samples MeshletGenerator.");
        return;
    }

    mModel.uploadGpuResources(pRenderContext);

    // Camera - match D3D12 sample: Init({0, 5, 35}), SetMoveSpeed(60)
    mpCamera = Camera::create("Main");
    mpCamera->setPosition(float3(0.f, 5.f, 35.f));
    mpCamera->setTarget(float3(0.f, 0.f, 0.f));
    mpCamera->setUpVector(float3(0.f, 1.f, 0.f));
    mpCameraController = std::make_unique<OrbiterCameraController>(mpCamera);
    mpCameraController->setCameraSpeed(60.f);
    const auto& bs = mModel.getBoundingSphere();
    mpCameraController->setModelParams(
        float3(bs.center[0], bs.center[1], bs.center[2]),
        bs.radius,
        3.5f);

    uint2 size = getWindow() ? getWindow()->getClientAreaSize() : uint2(1280, 720);
    mpCamera->setAspectRatio((float)size.x / std::max(size.y, 1u));

    // Constant buffers
    mpConstantBuffer = getDevice()->createBuffer(
        sizeof(SceneConstants),
        ResourceBindFlags::Constant,
        MemoryType::Upload,
        nullptr);

    // DrawParams and MeshInfo - small buffers updated per-dispatch
    struct DrawParamsData
    {
        uint32_t InstanceCount;
        uint32_t InstanceOffset;
    };
    struct MeshInfoData
    {
        uint32_t IndexBytes;
        uint32_t MeshletCount;
        uint32_t MeshletOffset;
    };
    mpDrawParamsBuffer = getDevice()->createBuffer(sizeof(DrawParamsData), ResourceBindFlags::Constant, MemoryType::Upload, nullptr);
    mpMeshInfoBuffer = getDevice()->createBuffer(sizeof(MeshInfoData), ResourceBindFlags::Constant, MemoryType::Upload, nullptr);

    // Program - mesh + pixel shader
    ProgramDesc desc;
    desc.addShaderLibrary("Samples/Desktop/D3D12MeshletInstancing/MeshletInstancing.ms.slang")
        .meshEntry("meshMain")
        .psEntry("psMain");
    desc.setShaderModel(ShaderModel::SM6_5);
    mpProgram = Program::create(getDevice(), desc);
    mpVars = ProgramVars::create(getDevice(), mpProgram.get());

    mpState = GraphicsState::create(getDevice());
    mpState->setProgram(mpProgram);
    mpState->setVao(nullptr); // Mesh pipeline has no VAO
    mpState->setDepthStencilState(DepthStencilState::create(DepthStencilState::Desc().setDepthEnabled(true).setDepthFunc(ComparisonFunc::LessEqual)));

    regenerateInstances();
}

void D3D12MeshletInstancing::regenerateInstances()
{
    mUpdateInstances = true;

    const float radius = mModel.getBoundingSphere().radius;
    const float padding = 0.0f;
    const float spacing = (1.0f + padding) * radius * 2.0f;

    uint32_t width = mInstanceLevel * 2 + 1;
    float extents = spacing * (float)mInstanceLevel;

    mInstanceCount = width * width * width;

    mInstanceData.resize(mInstanceCount);
    for (uint32_t i = 0; i < mInstanceCount; ++i)
    {
        float ix = (float)(i % width);
        float iy = (float)((i / width) % width);
        float iz = (float)(i / (width * width));
        float3 location = float3(ix, iy, iz) * spacing - float3(extents, extents, extents);

        float4x4 world = math::matrixFromTranslation(location);
        float4x4 worldInvTranspose = math::transpose(math::inverse(world));

        mInstanceData[i].World = math::transpose(world);
        mInstanceData[i].WorldInvTranspose = math::transpose(worldInvTranspose);
    }

    size_t instanceBufferSize = mInstanceCount * sizeof(InstanceData);
    instanceBufferSize = (instanceBufferSize + 255) & ~255; // CB alignment

    mpInstanceBuffer = getDevice()->createBuffer(
        (uint32_t)instanceBufferSize,
        ResourceBindFlags::ShaderResource,
        MemoryType::Upload,
        mInstanceData.data());
}

void D3D12MeshletInstancing::onShutdown() {}

void D3D12MeshletInstancing::onResize(uint32_t width, uint32_t height)
{
    if (mpCamera)
        mpCamera->setAspectRatio((float)width / std::max(height, 1u));
}

void D3D12MeshletInstancing::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    if (!mpProgram || !mpVars || mModel.getMeshCount() == 0)
    {
        const float4 clearColor(0.0f, 0.2f, 0.4f, 1.0f);
        pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
        return;
    }

    mpCameraController->update();

    // Update instance buffer if needed
    if (mUpdateInstances)
    {
        size_t instanceBufferSize = mInstanceCount * sizeof(InstanceData);
        instanceBufferSize = (instanceBufferSize + 255) & ~255;
        mpInstanceBuffer->setBlob(mInstanceData.data(), 0, (uint32_t)(mInstanceCount * sizeof(InstanceData)));
        mUpdateInstances = false;
    }

    // Update constant buffer
    SceneConstants cbData;
    cbData.View = math::transpose(mpCamera->getViewMatrix());
    cbData.ViewProj = math::transpose(mpCamera->getViewProjMatrix());
    cbData.DrawMeshlets = mDrawMeshlets ? 1u : 0u;
    mpConstantBuffer->setBlob(&cbData, 0, sizeof(cbData));

    // Match D3D12 Meshlet Instancing clear color (0.0f, 0.2f, 0.4f, 1.0f)
    const float4 clearColor(0.0f, 0.2f, 0.4f, 1.0f);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);

    mpState->setFbo(pTargetFbo, true);

    auto var = mpVars->getRootVar();
    var["Globals"] = mpConstantBuffer;
    var["Instances"] = mpInstanceBuffer;

    for (uint32_t meshIdx = 0; meshIdx < mModel.getMeshCount(); ++meshIdx)
    {
        auto& mesh = mModel.getMesh(meshIdx);

        var["Vertices"] = mesh.pVertexBuffer;
        var["Meshlets"] = mesh.pMeshletBuffer;
        var["UniqueVertexIndices"] = mesh.pUniqueVertexIndexBuffer;
        var["PrimitiveIndices"] = mesh.pPrimitiveIndexBuffer;

        for (uint32_t i = 0; i < (uint32_t)mesh.MeshletSubsets.size(); ++i)
        {
            const auto& subset = mesh.MeshletSubsets[i];
            uint32_t packCount = mesh.getLastMeshletPackCount(i, 64, 126);
            float groupsPerInstance = (float)(subset.Count - 1) + 1.0f / (float)std::max(packCount, 1u);
            uint32_t maxInstancePerBatch = (uint32_t)((float)c_maxGroupDispatchCount / groupsPerInstance);
            maxInstancePerBatch = std::max(maxInstancePerBatch, 1u);
            uint32_t dispatchCount = divRoundUp(mInstanceCount, maxInstancePerBatch);

            for (uint32_t j = 0; j < dispatchCount; ++j)
            {
                uint32_t instanceOffset = maxInstancePerBatch * j;
                uint32_t instanceCount = std::min(mInstanceCount - instanceOffset, maxInstancePerBatch);

                DrawParamsData drawParams = {instanceCount, instanceOffset};
                MeshInfoData meshInfo = {mesh.IndexSize, subset.Count, subset.Offset};
                mpDrawParamsBuffer->setBlob(&drawParams, 0, sizeof(drawParams));
                mpMeshInfoBuffer->setBlob(&meshInfo, 0, sizeof(meshInfo));

                var["DrawParamsCB"] = mpDrawParamsBuffer;
                var["MeshInfoCB"] = mpMeshInfoBuffer;

                uint32_t groupCount = (uint32_t)std::ceil(groupsPerInstance * (float)instanceCount);
                pRenderContext->drawMeshTasks(mpState.get(), mpVars.get(), groupCount, 1, 1);
            }
        }
    }
}

void D3D12MeshletInstancing::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "D3D12 Meshlet Instancing", {250, 200});
    renderGlobalUI(pGui);
    w.text("D3D12 Meshlet Instancing - migrated to Falcor");
    w.text("+/- : Instance level | Space : Toggle meshlet colors");
    w.checkbox("Draw Meshlets (color by meshlet)", mDrawMeshlets);
}

bool D3D12MeshletInstancing::onKeyEvent(const KeyboardEvent& keyEvent)
{
    if (keyEvent.type == KeyboardEvent::Type::KeyPressed)
    {
        switch (keyEvent.key)
        {
        case Input::Key::Plus:
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
    return mpCameraController->onKeyEvent(keyEvent);
}

bool D3D12MeshletInstancing::onMouseEvent(const MouseEvent& mouseEvent)
{
    return mpCameraController->onMouseEvent(mouseEvent);
}

void D3D12MeshletInstancing::onHotReload(HotReloadFlags reloaded) {}

int runMain(int argc, char** argv)
{
    SampleAppConfig config;
    config.windowDesc.title = "D3D12 Meshlet Instancing";
    config.windowDesc.resizableWindow = true;

    D3D12MeshletInstancing project(config);
    return project.run();
}

int main(int argc, char** argv)
{
    return catchAndReportAllExceptions([&]() { return runMain(argc, argv); });
}
