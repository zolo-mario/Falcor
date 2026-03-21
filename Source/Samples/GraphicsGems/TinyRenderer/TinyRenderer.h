#pragma once

#include "Falcor.h"
#include "Core/SampleBase.h"
#include "Scene/Camera/CameraController.h"
#include "Scene/Scene.h"
#include "Scene/TriangleMesh.h"
#include "src/CpuPipeline.h"

#include <filesystem>
#include <memory>

using namespace Falcor;

class TinyRenderer : public SampleBase
{
public:
    FALCOR_PLUGIN_CLASS(TinyRenderer, "TinyRenderer", SampleBase::PluginInfo{"Samples/GraphicsGems/TinyRenderer"});

    explicit TinyRenderer(SampleApp* pHost);
    ~TinyRenderer() override;

    static SampleBase* create(SampleApp* pHost);

    void onLoad(RenderContext* pRenderContext) override;
    void onShutdown() override;
    void onResize(uint32_t width, uint32_t height) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onHotReload(HotReloadFlags reloaded) override;

    void setProperties(const Properties& props) override;
    Properties getProperties() const override;

private:
    void rebuildCpuFramebuffer();
    void loadSceneAndMesh();
    void computeModelMatrix();
    void initOrbitCamera();
    void syncOrbitCameraDistance();

    ref<Scene> mpScene;
    std::filesystem::path mScenePath;
    ref<TriangleMesh> mpMesh;
    tiny::CpuFramebuffer mCpuFb;
    tiny::LambertCpuShader mShader;

    uint32_t mCpuWidth = 512;
    uint32_t mCpuHeight = 384;

    ref<Buffer> mpVertexBuffer;
    ref<Vao> mpVao;
    ref<Texture> mpCpuResultTexture;
    ref<Sampler> mpSampler;
    ref<Program> mpProgram;
    ref<ProgramVars> mpVars;
    ref<GraphicsState> mpState;

    ref<Camera> mpRenderCamera;
    std::unique_ptr<OrbiterCameraController> mpOrbitCam;
    /// World-space radius after `computeModelMatrix` normalization (mesh centered at origin, ~unit ball).
    float mOrbitWorldRadius = 1.f;

    float mRotationRad = 0.f;
    float mCameraDistance = 2.8f;
};
