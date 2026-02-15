#pragma once
#include "Falcor.h"
#include "Core/SampleApp.h"
#include "Utils/Math/MatrixMath.h"

using namespace Falcor;

// Match D3D12 Pipeline State Cache sample - effect indices
enum EffectPipelineType : uint32_t
{
    PostBlit = 2,
    PostInvert = 3,
    PostGrayScale = 4,
    PostEdgeDetect = 5,
    PostBlur = 6,
    PostWarp = 7,
    PostPixelate = 8,
    PostDistort = 9,
    PostWave = 10,
    EffectPipelineTypeCount = 11, // 9 effects + 2 padding for indices
};

class D3D12PipelineStateCache : public SampleApp
{
public:
    D3D12PipelineStateCache(const SampleAppConfig& config);
    ~D3D12PipelineStateCache();

    void onLoad(RenderContext* pRenderContext) override;
    void onShutdown() override;
    void onResize(uint32_t width, uint32_t height) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onHotReload(HotReloadFlags reloaded) override;

private:
    void toggleEffect(EffectPipelineType type);

    // Match D3D12 sample constants
    static constexpr float kIntermediateClearColor[4] = {0.0f, 0.2f, 0.3f, 1.0f};
    static constexpr uint32_t kQuadsX = 3;
    static constexpr uint32_t kQuadsY = 3;

    ref<GraphicsState> mpCubeState;
    ref<GraphicsState> mpEffectState;
    ref<Program> mpCubeProgram;
    ref<Program> mpEffectProgram;
    ref<ProgramVars> mpCubeVars;
    ref<ProgramVars> mpEffectVars;
    ref<Vao> mpCubeVao;
    ref<Vao> mpQuadVao;
    ref<Buffer> mpCubeVertexBuffer;
    ref<Buffer> mpQuadVertexBuffer;
    ref<Buffer> mpCubeIndexBuffer;
    ref<Buffer> mpConstantBuffer;
    ref<Fbo> mpIntermediateFbo;
    ref<Sampler> mpSampler;

    bool mEnabledEffects[EffectPipelineTypeCount];
    float mRotation = 0.0f;
    float4x4 mProjectionMatrix;
};
