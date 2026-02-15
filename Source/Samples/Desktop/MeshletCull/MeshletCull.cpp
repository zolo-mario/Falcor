#include "MeshletCull.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

MeshletCull::MeshletCull(const SampleAppConfig& config) : SampleApp(config) {}

MeshletCull::~MeshletCull() {}

void MeshletCull::onLoad(RenderContext* pRenderContext)
{
    if (!getDevice()->isShaderModelSupported(ShaderModel::SM6_5))
    {
        throw RuntimeError("MeshletCull requires Shader Model 6.5 for mesh shader support.");
    }

    // Match D3D12 Meshlet Cull camera: position (0, 15, 40), look at origin
    mpCamera = Camera::create("MainCamera");
    mpCamera->setPosition(float3(0.f, 15.f, 40.f));
    mpCamera->setTarget(float3(0.f, 0.f, 0.f));
    mpCamera->setUpVector(float3(0.f, 1.f, 0.f));
    mpCameraController = std::make_unique<OrbiterCameraController>(mpCamera);
    mpCameraController->setModelParams(float3(0.f, 0.f, 0.f), 50.f, 2.f);
    mpCameraController->setCameraSpeed(25.f);

    createProceduralMeshlet();
}

void MeshletCull::createProceduralMeshlet()
{
    // Single triangle meshlet - match D3D12 Hello Triangle aspect ratio
    const float aspectRatio = 1280.f / 720.f;
    const VertexData vertices[] = {
        {{0.0f, 0.25f * aspectRatio, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{0.25f, -0.25f * aspectRatio, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{-0.25f, -0.25f * aspectRatio, 0.0f}, {0.0f, 0.0f, 1.0f}},
    };

    const MeshletData meshlet = {3, 0, 1, 0};

    // Unique vertex indices (32-bit)
    const uint32_t uniqueIndices[] = {0, 1, 2};

    // Packed triangle: i0=0, i1=1, i2=2 -> (0) | (1<<10) | (2<<20)
    const uint32_t packedPrim = 0 | (1 << 10) | (2 << 20);

    // Cull data: bounding sphere (center at centroid, radius to cover triangle)
    float3 center = (vertices[0].Position + vertices[1].Position + vertices[2].Position) / 3.f;
    float r0 = length(vertices[0].Position - center);
    float r1 = length(vertices[1].Position - center);
    float r2 = length(vertices[2].Position - center);
    float radius = std::max({r0, r1, r2});
    // Normal cone: axis +Z, full hemisphere = degenerate (0xff in top byte)
    CullDataGpu cullData;
    cullData.BoundingSphere = float4(center, radius);
    cullData.NormalCone = 0x80 | (0x80 << 8) | (0xff << 16) | (0xff << 24); // degenerate cone
    cullData.ApexOffset = 0.f;

    MeshInfoData meshInfo;
    meshInfo.IndexSize = 4;
    meshInfo.MeshletCount = 1;
    meshInfo.LastMeshletVertCount = 3;
    meshInfo.LastMeshletPrimCount = 1;

    mpVertexBuffer = getDevice()->createBuffer(
        sizeof(vertices), ResourceBindFlags::ShaderResource, MemoryType::Upload, vertices);
    mpMeshletBuffer = getDevice()->createBuffer(
        sizeof(meshlet), ResourceBindFlags::ShaderResource, MemoryType::Upload, &meshlet);
    mpUniqueVertexIndexBuffer = getDevice()->createBuffer(
        sizeof(uniqueIndices), ResourceBindFlags::ShaderResource, MemoryType::Upload, uniqueIndices);
    mpPrimitiveIndexBuffer = getDevice()->createBuffer(
        sizeof(packedPrim), ResourceBindFlags::ShaderResource, MemoryType::Upload, &packedPrim);
    mpCullDataBuffer = getDevice()->createBuffer(
        sizeof(cullData), ResourceBindFlags::ShaderResource, MemoryType::Upload, &cullData);
    mpMeshInfoBuffer = getDevice()->createBuffer(
        sizeof(meshInfo), ResourceBindFlags::ShaderResource, MemoryType::Upload, &meshInfo);

    mMeshletCount = 1;
}

    ProgramDesc desc;
    desc.addShaderLibrary("Samples/Desktop/MeshletCull/MeshletCull.slang")
        .amplificationEntry("ampMain")
        .meshEntry("meshMain")
        .psEntry("psMain");
    desc.setShaderModel(ShaderModel::SM6_5);

    mpMeshletProgram = Program::create(getDevice(), desc);
    mpMeshletVars = ProgramVars::create(getDevice(), mpMeshletProgram.get());

    mpMeshletState = GraphicsState::create(getDevice());
    mpMeshletState->setProgram(mpMeshletProgram);
    mpMeshletState->setVao(nullptr); // Mesh pipeline has no VAO

    mpDepthStencilState = DepthStencilState::create(
        DepthStencilState::Desc().setDepthFunc(ComparisonFunc::LessEqual).setDepthWriteMask(true));
    mpMeshletState->setDepthStencilState(mpDepthStencilState);

    mpRasterizerState = RasterizerState::create(RasterizerState::Desc().setCullMode(RasterizerState::CullMode::None));
    mpMeshletState->setRasterizerState(mpRasterizerState);

    // Constant buffers
    mpConstantsBuffer = getDevice()->createBuffer(sizeof(ConstantsData), ResourceBindFlags::Constant, MemoryType::Upload);
    mpInstanceBuffer = getDevice()->createBuffer(sizeof(InstanceData), ResourceBindFlags::Constant, MemoryType::Upload);
}

void MeshletCull::updateConstants(RenderContext* pRenderContext)
{
    uint2 size = getWindow() ? getWindow()->getClientAreaSize() : uint2(1280, 720);
    const float aspectRatio = size.y > 0 ? (float)size.x / size.y : (1280.f / 720.f);

    const Camera* pCamera = mpCamera.get();
    float4x4 view = pCamera->getViewMatrix();
    float4x4 proj = pCamera->getProjMatrix();
    float4x4 viewProj = proj * view;

    ConstantsData constants;
    constants.View = view;
    constants.ViewProj = viewProj;
    constants.ViewPosition = pCamera->getPosition();
    constants.CullViewPosition = pCamera->getPosition();
    constants.HighlightedIndex = mHighlightedIndex;
    constants.SelectedIndex = mSelectedIndex;
    constants.DrawMeshlets = mDrawMeshlets ? 1u : 0u;

    // Extract frustum planes from view-proj (transposed for plane extraction)
    float4x4 vp = transpose(viewProj);
    auto normalizePlane = [](float4 p) {
        float len = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
        return float4(p.x / len, p.y / len, p.z / len, p.w / len);
    };
    constants.Planes[0] = normalizePlane(vp[3] + vp[0]); // Left
    constants.Planes[1] = normalizePlane(vp[3] - vp[0]); // Right
    constants.Planes[2] = normalizePlane(vp[3] + vp[1]); // Bottom
    constants.Planes[3] = normalizePlane(vp[3] - vp[1]); // Top
    constants.Planes[4] = normalizePlane(vp[2]);         // Near
    constants.Planes[5] = normalizePlane(vp[3] - vp[2]); // Far

    InstanceData instance;
    instance.World = float4x4::identity();
    instance.WorldInvTrans = float4x4::identity();
    instance.Scale = 1.f;
    instance.Flags = 0x1 | 0x2; // CULL_FLAG | MESHLET_FLAG

    mpConstantsBuffer->setBlob(&constants, 0, sizeof(constants));
    mpInstanceBuffer->setBlob(&instance, 0, sizeof(instance));
}

void MeshletCull::onShutdown() {}

void MeshletCull::onResize(uint32_t width, uint32_t height)
{
    if (mpCamera)
        mpCamera->setAspectRatio((float)width / (float)height);
}

void MeshletCull::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    // Match D3D12 Meshlet Cull clear color (0.0f, 0.2f, 0.4f, 1.0f)
    const float4 clearColor(0.0f, 0.2f, 0.4f, 1.0f);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);

    if (!mpMeshletProgram || !mpMeshletVars || !mpCamera)
        return;

    if (mpCameraController && mpCameraController->update())
        mpCamera->beginFrame(false);

    updateConstants(pRenderContext);

    auto var = mpMeshletVars->getRootVar();
    var["gConstants"] = mpConstantsBuffer;
    var["gMeshInfo"] = mpMeshInfoBuffer;
    var["gInstance"] = mpInstanceBuffer;
    var["gVertices"] = mpVertexBuffer;
    var["gMeshlets"] = mpMeshletBuffer;
    var["gUniqueVertexIndices"] = mpUniqueVertexIndexBuffer;
    var["gPrimitiveIndices"] = mpPrimitiveIndexBuffer;
    var["gMeshletCullData"] = mpCullDataBuffer;

    mpMeshletState->setFbo(pTargetFbo, true);

    // Dispatch AS groups: 1 group per 32 meshlets
    const uint32_t asGroupCount = (mMeshletCount + kASGroupSize - 1) / kASGroupSize;
    pRenderContext->drawMeshTasks(mpMeshletState.get(), mpMeshletVars.get(), asGroupCount, 1, 1);

}

void MeshletCull::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "D3D12 Meshlet Cull", {250, 200});
    renderGlobalUI(pGui);
    w.text("D3D12 Meshlet Cull - migrated to Falcor");
    w.text("AS + MS + PS pipeline with procedural triangle");
    w.checkbox("Draw Meshlets", mDrawMeshlets);
}

bool MeshletCull::onKeyEvent(const KeyboardEvent& keyEvent)
{
    if (keyEvent.type == KeyboardEvent::Type::KeyPressed)
    {
        if (keyEvent.key == Input::Key::Space)
        {
            mDrawMeshlets = !mDrawMeshlets;
            mSelectedIndex = 0xFFFFFFFF;
            mHighlightedIndex = 0xFFFFFFFF;
            return true;
        }
    }
    return mpCameraController && mpCameraController->onKeyEvent(keyEvent);
}

bool MeshletCull::onMouseEvent(const MouseEvent& mouseEvent)
{
    return mpCameraController && mpCameraController->onMouseEvent(mouseEvent);
}

void MeshletCull::onHotReload(HotReloadFlags reloaded)
{
    if (is_set(reloaded, HotReloadFlags::Shader))
    {
        ProgramDesc desc;
        desc.addShaderLibrary("Samples/Desktop/MeshletCull/MeshletCull.slang")
            .amplificationEntry("ampMain")
            .meshEntry("meshMain")
            .psEntry("psMain");
        desc.setShaderModel(ShaderModel::SM6_5);
        mpMeshletProgram = Program::create(getDevice(), desc);
        mpMeshletVars = ProgramVars::create(getDevice(), mpMeshletProgram.get());
        mpMeshletState->setProgram(mpMeshletProgram);
    }
}

int runMain(int argc, char** argv)
{
    SampleAppConfig config;
    config.windowDesc.title = "D3D12 Meshlet Cull";
    config.windowDesc.resizableWindow = true;

    MeshletCull project(config);
    return project.run();
}

int main(int argc, char** argv)
{
    return catchAndReportAllExceptions([&]() { return runMain(argc, argv); });
}
