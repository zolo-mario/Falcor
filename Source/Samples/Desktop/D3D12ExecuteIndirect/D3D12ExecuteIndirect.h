#pragma once
#include "Falcor.h"
#include "Core/SampleBase.h"

using namespace Falcor;

class D3D12ExecuteIndirect : public SampleBase
{
public:
    FALCOR_PLUGIN_CLASS(D3D12ExecuteIndirect, "D3D12ExecuteIndirect", SampleBase::PluginInfo{"Samples/Desktop/D3D12ExecuteIndirect"});

    explicit D3D12ExecuteIndirect(SampleApp* pHost);
    ~D3D12ExecuteIndirect();

    static SampleBase* create(SampleApp* pHost);

    void onLoad(RenderContext* pRenderContext) override;
    void onShutdown() override;
    void onResize(uint32_t width, uint32_t height) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onHotReload(HotReloadFlags reloaded) override;

private:
    static constexpr uint32_t kTriangleCount = 1024;
    static constexpr uint32_t kFrameCount = 3;
    static constexpr float kTriangleHalfWidth = 0.05f;
    static constexpr float kTriangleDepth = 1.0f;
    static constexpr float kCullingCutoff = 0.5f;
    static constexpr uint32_t kComputeThreadBlockSize = 128;

    struct Vertex
    {
        float3 position;
    };

    struct SceneConstantBuffer
    {
        float4 velocity;
        float4 offset;
        float4 color;
        float4x4 projection;
    };

    struct DrawIndirectCommand
    {
        uint32_t vertexCount;
        uint32_t instanceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;
    };

    float getRandomFloat(float min, float max);

    ref<Buffer> mpVertexBuffer;
    ref<Vao> mpVao;
    ref<Program> mpGraphicsProgram;
    ref<ProgramVars> mpGraphicsVars;
    ref<GraphicsState> mpGraphicsState;
    ref<DepthStencilState> mpDepthStencilState;

    ref<ComputePass> mpCullPass;
    ref<ProgramVars> mpCullVars;
    ref<ComputePass> mpBuildArgsPass;
    ref<ProgramVars> mpBuildArgsVars;

    ref<Buffer> mpConstantBuffers[kFrameCount];
    ref<Buffer> mpCommandBuffer;           // Full indirect commands (all triangles) - non-culling path
    ref<Buffer> mpVisibleIndicesBuffer;   // AppendStructuredBuffer<uint> output (visible triangle indices)
    ref<Buffer> mpDrawArgsBuffer;          // (3, count, 0, 0) for single drawIndirect

    ref<Program> mpGraphicsProgramCulling;
    ref<ProgramVars> mpGraphicsVarsCulling;

    std::vector<SceneConstantBuffer> mConstantBufferData;
    uint32_t mFrameIndex = 0;
    bool mEnableCulling = true;
};
