#include "TinyRenderer.h"

#include "Core/AssetResolver.h"
#include "Scene/SceneBuilder.h"
#include "Utils/Math/FalcorMath.h"
#include "Utils/Math/MatrixMath.h"
#include "Utils/Math/VectorMath.h"

#include <algorithm>
#include <exception>

FALCOR_EXPORT_D3D12_AGILITY_SDK

namespace
{
const char kPresentShader[] = "Samples/GraphicsGems/TinyRenderer/TinyRendererPresent.slang";

struct PresentVertex
{
    float3 position;
    float2 uv;
};

/// Build a CPU `TriangleMesh` from a scene mesh (same vertex/index layout as `Scene::createMeshUVTiles`).
ref<TriangleMesh> triangleMeshFromScene(const ref<Scene>& scene, MeshID meshID)
{
    if (!scene || meshID.get() >= scene->getMeshCount())
        return nullptr;

    const MeshDesc& desc = scene->getMesh(meshID);
    const auto& staticData = scene->getMeshStaticData();
    const auto& indexData = scene->getMeshIndexData();

    if (!staticData.hasCpuData())
    {
        logWarning("TinyRenderer: scene vertex CPU data was dropped; cannot CPU-rasterize this mesh.");
        return nullptr;
    }
    if (desc.useVertexIndices() && indexData.empty())
    {
        logWarning("TinyRenderer: scene has indexed mesh but index buffer is empty.");
        return nullptr;
    }
    if (desc.useVertexIndices() && !indexData.hasCpuData())
    {
        logWarning("TinyRenderer: scene index CPU data was dropped; cannot CPU-rasterize.");
        return nullptr;
    }

    TriangleMesh::VertexList vertices;
    TriangleMesh::IndexList indices;

    vertices.reserve(desc.vertexCount);
    for (uint32_t i = 0; i < desc.vertexCount; ++i)
    {
        StaticVertexData v = staticData[(size_t)desc.vbOffset + i].unpack();
        vertices.push_back(TriangleMesh::Vertex{v.position, v.normal, v.texCrd});
    }

    const uint32_t triCount = desc.getTriangleCount();
    indices.reserve(triCount * 3);

    if (desc.useVertexIndices())
    {
        const uint8_t* meshIndexData8 = reinterpret_cast<const uint8_t*>(&indexData[desc.ibOffset]);
        for (uint32_t t = 0; t < triCount; ++t)
        {
            uint32_t vidx[3];
            if (desc.use16BitIndices())
            {
                uint byteOffset = t * 3 * sizeof(uint16_t);
                vidx[0] = reinterpret_cast<const uint16_t*>(meshIndexData8 + byteOffset)[0];
                vidx[1] = reinterpret_cast<const uint16_t*>(meshIndexData8 + byteOffset)[1];
                vidx[2] = reinterpret_cast<const uint16_t*>(meshIndexData8 + byteOffset)[2];
            }
            else
            {
                uint byteOffset = t * 3 * sizeof(uint32_t);
                vidx[0] = reinterpret_cast<const uint32_t*>(meshIndexData8 + byteOffset)[0];
                vidx[1] = reinterpret_cast<const uint32_t*>(meshIndexData8 + byteOffset)[1];
                vidx[2] = reinterpret_cast<const uint32_t*>(meshIndexData8 + byteOffset)[2];
            }
            indices.push_back(vidx[0]);
            indices.push_back(vidx[1]);
            indices.push_back(vidx[2]);
        }
    }
    else
    {
        for (uint32_t t = 0; t < triCount; ++t)
        {
            uint32_t base = t * 3;
            indices.push_back(base);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
        }
    }

    return TriangleMesh::create(vertices, indices, desc.isFrontFaceCW());
}
} // namespace

TinyRenderer::TinyRenderer(SampleApp* pHost) : SampleBase(pHost) {}

TinyRenderer::~TinyRenderer() = default;

void TinyRenderer::rebuildCpuFramebuffer()
{
    mCpuFb.resize(mCpuWidth, mCpuHeight);
    mpCpuResultTexture = getDevice()->createTexture2D(
        mCpuWidth, mCpuHeight, ResourceFormat::RGBA8Unorm, 1, 1, nullptr, ResourceBindFlags::ShaderResource
    );
    if (mpVars)
        mpVars->getRootVar()["g_texture"] = mpCpuResultTexture;
}

void TinyRenderer::loadSceneAndMesh()
{
    mpMesh.reset();
    mpScene.reset();

    try
    {
        mpScene = SceneBuilder(getDevice(), mScenePath, Settings(), SceneBuilder::Flags::Default).getScene();
    }
    catch (const std::exception& e)
    {
        logWarning("TinyRenderer: SceneBuilder failed ({}): {}. Using procedural cube.", mScenePath.string(), e.what());
        mpMesh = TriangleMesh::createCube(float3(1.f));
        computeModelMatrix();
        return;
    }

    if (!mpScene || mpScene->getMeshCount() == 0)
    {
        logWarning(
            "TinyRenderer: scene '{}' has no triangle meshes. Check FALCOR_MEDIA_FOLDERS. Using procedural cube.",
            mScenePath.string()
        );
        mpMesh = TriangleMesh::createCube(float3(1.f));
        computeModelMatrix();
        return;
    }

    // Prefer the densest mesh (e.g. bunny over floor quad in bunny.pyscene — mesh 0 is often a large ground quad).
    MeshID bestMesh(0);
    uint32_t bestTri = 0;
    for (uint32_t mi = 0; mi < mpScene->getMeshCount(); ++mi)
    {
        const MeshDesc& md = mpScene->getMesh(MeshID(mi));
        const uint32_t t = md.getTriangleCount();
        if (t > bestTri)
        {
            bestTri = t;
            bestMesh = MeshID(mi);
        }
    }

    mpMesh = triangleMeshFromScene(mpScene, bestMesh);
    if (!mpMesh)
    {
        logWarning("TinyRenderer: failed to extract mesh {} from scene. Using procedural cube.", bestMesh.get());
        mpMesh = TriangleMesh::createCube(float3(1.f));
    }
    else
        logInfo("TinyRenderer: CPU mesh from scene '{}' (mesh index {}, {} triangles).", mScenePath.string(), bestMesh.get(), bestTri);

    computeModelMatrix();
}

void TinyRenderer::computeModelMatrix()
{
    if (!mpMesh)
        return;
    const auto& verts = mpMesh->getVertices();
    if (verts.empty())
        return;
    float3 bmin(FLT_MAX);
    float3 bmax(-FLT_MAX);
    for (const auto& v : verts)
    {
        bmin = math::min(bmin, v.position);
        bmax = math::max(bmax, v.position);
    }
    float3 center = (bmin + bmax) * 0.5f;
    float radius = math::length(bmax - bmin) * 0.5f;
    float s = 1.f / std::max(radius, 1e-5f);
    float4x4 T = math::matrixFromTranslation(-center);
    float4x4 S = math::matrixFromScaling(float3(s, s, s));
    float4x4 R = math::matrixFromRotationY(mRotationRad);
    mShader.model = math::mul(R, math::mul(S, T));
    // Normalized mesh fits roughly in a unit ball around the origin (orbit target).
    mOrbitWorldRadius = 1.f;
}

void TinyRenderer::initOrbitCamera()
{
    mpRenderCamera = Camera::create();
    mpOrbitCam = std::make_unique<OrbiterCameraController>(mpRenderCamera);
    mpRenderCamera->setDepthRange(0.1f, 100.f);
    mpRenderCamera->setFocalLength(fovYToFocalLength(math::radians(45.f), Camera::kDefaultFrameHeight));
    syncOrbitCameraDistance();
}

void TinyRenderer::syncOrbitCameraDistance()
{
    if (mpOrbitCam)
        mpOrbitCam->setModelParams(float3(0.f), mOrbitWorldRadius, mCameraDistance);
}

void TinyRenderer::onLoad(RenderContext* /*pRenderContext*/)
{
    if (mScenePath.empty())
        mScenePath = std::filesystem::path("test_scenes/bunny.pyscene");
    loadSceneAndMesh();
    initOrbitCamera();

    mCpuFb.resize(mCpuWidth, mCpuHeight);
    mpCpuResultTexture = getDevice()->createTexture2D(
        mCpuWidth, mCpuHeight, ResourceFormat::RGBA8Unorm, 1, 1, nullptr, ResourceBindFlags::ShaderResource
    );

    const PresentVertex vertices[] = {
        {{-1.f, -1.f, 0.f}, {0.f, 1.f}},
        {{-1.f, 3.f, 0.f}, {0.f, -1.f}},
        {{3.f, -1.f, 0.f}, {2.f, 1.f}},
    };
    mpVertexBuffer = getDevice()->createBuffer(sizeof(vertices), ResourceBindFlags::Vertex, MemoryType::Upload, (void*)vertices);

    auto pBufLayout = VertexBufferLayout::create();
    pBufLayout->addElement("POSITION", offsetof(PresentVertex, position), ResourceFormat::RGB32Float, 1, 0);
    pBufLayout->addElement("TEXCOORD", offsetof(PresentVertex, uv), ResourceFormat::RG32Float, 1, 1);
    auto pLayout = VertexLayout::create();
    pLayout->addBufferLayout(0, pBufLayout);
    mpVao = Vao::create(Vao::Topology::TriangleList, pLayout, {mpVertexBuffer});

    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(TextureFilteringMode::Point, TextureFilteringMode::Point, TextureFilteringMode::Point)
        .setAddressingMode(TextureAddressingMode::Clamp, TextureAddressingMode::Clamp, TextureAddressingMode::Clamp);
    mpSampler = getDevice()->createSampler(samplerDesc);

    ProgramDesc desc;
    desc.addShaderLibrary(kPresentShader).vsEntry("VSMain").psEntry("PSMain");
    mpProgram = Program::create(getDevice(), desc);
    mpVars = ProgramVars::create(getDevice(), mpProgram.get());
    mpVars->getRootVar()["g_texture"] = mpCpuResultTexture;
    mpVars->getRootVar()["g_sampler"] = mpSampler;

    mpState = GraphicsState::create(getDevice());
    mpState->setVao(mpVao);
    mpState->setProgram(mpProgram);
    mpState->setDepthStencilState(DepthStencilState::create(DepthStencilState::Desc().setDepthEnabled(false)));
    mpState->setRasterizerState(RasterizerState::create(RasterizerState::Desc().setCullMode(RasterizerState::CullMode::None)));
}

void TinyRenderer::onShutdown()
{
    mpOrbitCam.reset();
    mpRenderCamera.reset();
    mpState.reset();
    mpVars.reset();
    mpProgram.reset();
    mpSampler.reset();
    mpCpuResultTexture.reset();
    mpVao.reset();
    mpVertexBuffer.reset();
    mpMesh.reset();
    mpScene.reset();
}

void TinyRenderer::onResize(uint32_t /*width*/, uint32_t /*height*/) {}

void TinyRenderer::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    computeModelMatrix();

    const float aspect = float(mCpuWidth) / float(std::max(1u, mCpuHeight));
    if (mpOrbitCam && mpRenderCamera)
    {
        mpRenderCamera->setAspectRatio(aspect);
        mpOrbitCam->update();
        mpRenderCamera->beginFrame();
        const float4x4 V = mpRenderCamera->getViewMatrix();
        const float4x4 P = mpRenderCamera->getProjMatrix();
        mShader.mvp = math::mul(P, math::mul(V, mShader.model));
    }
    else
    {
        float4x4 V = math::matrixFromLookAt(float3(0.f, 0.6f, mCameraDistance), float3(0.f, 0.2f, 0.f), float3(0.f, 1.f, 0.f));
        float4x4 P = math::perspective(math::radians(45.f), aspect, 0.1f, 100.f);
        mShader.mvp = math::mul(P, math::mul(V, mShader.model));
    }

    mCpuFb.clear(float3(0.15f, 0.18f, 0.22f));
    tiny::drawMesh(mCpuFb, mpMesh.get(), mShader);

    pRenderContext->updateTextureData(mpCpuResultTexture.get(), mCpuFb.color.data());

    const float4 kClear(0.05f, 0.05f, 0.08f, 1.f);
    pRenderContext->clearFbo(pTargetFbo.get(), kClear, 1.0f, 0, FboAttachmentType::All);
    mpState->setFbo(pTargetFbo, true);
    pRenderContext->draw(mpState.get(), mpVars.get(), 3, 0);
}

void TinyRenderer::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "CPU TinyRenderer (ssloy-style)", {420, 340}, {10, 10});
    w.text("Scene -> TriangleMesh (densest mesh) -> CPU raster -> updateTextureData -> fullscreen tri.");
    w.text("Camera: left-drag orbit, mouse wheel zoom (or Camera Z slider).");
    w.separator();
    w.slider("CPU width", mCpuWidth, 128u, 1024u);
    w.slider("CPU height", mCpuHeight, 128u, 1024u);
    if (w.button("Apply CPU framebuffer size"))
        rebuildCpuFramebuffer();
    if (w.slider("Camera Z", mCameraDistance, 0.8f, 8.f))
        syncOrbitCameraDistance();
    w.slider("Rotation Y (rad)", mRotationRad, -3.15f, 3.15f);
    w.text(fmt::format("Scene: {}", mScenePath.string()));
    renderGlobalUI(pGui);
}

bool TinyRenderer::onKeyEvent(const KeyboardEvent& /*keyEvent*/)
{
    return false;
}

bool TinyRenderer::onMouseEvent(const MouseEvent& mouseEvent)
{
    if (mpOrbitCam && mpOrbitCam->onMouseEvent(mouseEvent))
        return true;
    return false;
}

void TinyRenderer::onHotReload(HotReloadFlags /*reloaded*/)
{}

void TinyRenderer::setProperties(const Properties& props)
{
    const uint32_t prevW = mCpuWidth;
    const uint32_t prevH = mCpuHeight;

    if (props.has("scene"))
    {
        std::filesystem::path p(props.get<std::string>("scene"));
        if (p.is_absolute())
            mScenePath = p;
        else
        {
            std::filesystem::path r = AssetResolver::getDefaultResolver().resolvePath(p, AssetCategory::Scene);
            mScenePath = r.empty() ? p : r;
        }
        loadSceneAndMesh();
        syncOrbitCameraDistance();
    }
    if (props.has("cpu-width"))
        mCpuWidth = props.get<uint32_t>("cpu-width");
    if (props.has("cpu-height"))
        mCpuHeight = props.get<uint32_t>("cpu-height");
    if (props.has("camera-z"))
    {
        mCameraDistance = props.get<float>("camera-z");
        syncOrbitCameraDistance();
    }

    if (mpCpuResultTexture && (mCpuWidth != prevW || mCpuHeight != prevH))
        rebuildCpuFramebuffer();
}

Properties TinyRenderer::getProperties() const
{
    Properties p;
    p.set("scene", mScenePath.string());
    p.set("cpu-width", mCpuWidth);
    p.set("cpu-height", mCpuHeight);
    p.set("camera-z", mCameraDistance);
    return p;
}

SampleBase* TinyRenderer::create(SampleApp* pHost)
{
    return new TinyRenderer(pHost);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<SampleBase, TinyRenderer>();
}
