#pragma once
#include "Falcor.h"
#include "Core/SampleApp.h"
#include "Core/Pass/RasterPass.h"

using namespace Falcor;

class HelloDXR : public SampleApp
{
public:
    HelloDXR(const SampleAppConfig& config);
    ~HelloDXR();

    void onLoad(RenderContext* pRenderContext) override;
    void onResize(uint32_t width, uint32_t height) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;

private:
    void loadScene(const std::filesystem::path& path, const Fbo* pTargetFbo);
    void setPerFrameVars(const Fbo* pTargetFbo);
    void renderRaster(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo);
    void renderRT(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo);

    ref<Scene> mpScene;
    ref<Camera> mpCamera;

    ref<RasterPass> mpRasterPass;

    ref<Program> mpRaytraceProgram;
    ref<RtProgramVars> mpRtVars;
    ref<Texture> mpRtOut;

    bool mRayTrace = true;
    bool mUseDOF = false;

    uint32_t mSampleIndex = 0xdeadbeef;
};
