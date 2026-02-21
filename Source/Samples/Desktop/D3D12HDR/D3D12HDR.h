#pragma once
#include "Falcor.h"
#include "Core/SampleBase.h"

using namespace Falcor;

class D3D12HDR : public SampleBase
{
public:
    FALCOR_PLUGIN_CLASS(D3D12HDR, "D3D12HDR", SampleBase::PluginInfo{"Samples/Desktop/D3D12HDR"});

    explicit D3D12HDR(SampleApp* pHost);
    ~D3D12HDR();

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
    void updatePaletteVertices();

    enum DisplayCurve
    {
        sRGB = 0,
        ST2084,
        Linear,
        DisplayCurveCount
    };

    struct GradientVertex
    {
        float3 position;
        float3 color;
    };

    struct TrianglesVertex
    {
        float3 position;
        float2 uv;
    };

    struct PresentVertex
    {
        float3 position;
        float2 uv;
    };

    ref<Fbo> mpIntermediateFbo;
    ref<Buffer> mpGradientVB;
    ref<Buffer> mpPaletteVB;
    ref<Buffer> mpPresentVB;
    ref<Vao> mpGradientVao;
    ref<Vao> mpPaletteVao;
    ref<Vao> mpPresentVao;

    ref<Program> mpGradientProgram;
    ref<Program> mpPalette709Program;
    ref<Program> mpPalette2020Program;
    ref<Program> mpPresentProgram;
    ref<ProgramVars> mpGradientVars;
    ref<ProgramVars> mpPalette709Vars;
    ref<ProgramVars> mpPalette2020Vars;
    ref<ProgramVars> mpPresentVars;
    ref<Sampler> mpPresentSampler;
    ref<GraphicsState> mpGraphicsState;
    ref<DepthStencilState> mpDepthStencilState;

    std::vector<TrianglesVertex> mPaletteVertices;
    uint32_t mWidth = 0;
    uint32_t mHeight = 0;
    float mAspectRatio = 1.0f;

    DisplayCurve mDisplayCurve = sRGB;
    float mReferenceWhiteNits = 80.0f;
};
