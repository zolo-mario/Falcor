#pragma once

#include "Falcor.h"
#include "Core/SampleBase.h"
#include "Scene/Camera/CameraController.h"
#include "src/CpuColorBuffer.h"
#include "src/RayTracer.h"

#include <memory>

using namespace Falcor;

class TinyRaytracer : public SampleBase
{
public:
    FALCOR_PLUGIN_CLASS(TinyRaytracer, "TinyRaytracer", SampleBase::PluginInfo{"Samples/GraphicsGems/TinyRaytracer"});

    explicit TinyRaytracer(SampleApp* pHost);
    ~TinyRaytracer() override;

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
    void initOrbitCamera();
    void syncOrbitCameraDistance();

    tinyrt::CpuColorBuffer mCpuFb;
    tinyrt::RayTracerScene mScene;

    uint32_t mCpuWidth = 512;
    uint32_t mCpuHeight = 384;
    int mScenePreset = 0;
    int mMaxDepth = 4;

    ref<Buffer> mpVertexBuffer;
    ref<Vao> mpVao;
    ref<Texture> mpCpuResultTexture;
    ref<Sampler> mpSampler;
    ref<Program> mpProgram;
    ref<ProgramVars> mpVars;
    ref<GraphicsState> mpState;

    ref<Camera> mpRenderCamera;
    std::unique_ptr<OrbiterCameraController> mpOrbitCam;
    float mOrbitWorldRadius = 1.f;
    float mCameraDistance = 3.2f;
};
