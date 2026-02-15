#pragma once
#include "Falcor.h"
#include "Core/SampleApp.h"

using namespace Falcor;

class D3D12ExecuteIndirect : public SampleApp
{
public:
    D3D12ExecuteIndirect(const SampleAppConfig& config);
    ~D3D12ExecuteIndirect();

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
        float padding[36]; // 256-byte aligned
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

    ref<Buffer> mpConstantBuffer;
    ref<Buffer> mpCommandBuffer;           // Full indirect commands (all triangles)
    ref<Buffer> mpProcessedCommandBuffer;  // AppendStructuredBuffer output (visible only)

    std::vector<SceneConstantBuffer> mConstantBufferData;
    uint32_t mFrameIndex = 0;
    bool mEnableCulling = true;
};
