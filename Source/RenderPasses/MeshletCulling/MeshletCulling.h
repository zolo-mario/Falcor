#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"
#include "MeshletTypes.slang"
#include <meshoptimizer.h>

using namespace Falcor;

enum class VisualizeMode : uint32_t
{
    MeshletID = 0,
    TriangleID = 1,
    Combined = 2
};

FALCOR_ENUM_INFO(
    VisualizeMode,
    {
        { VisualizeMode::MeshletID, "MeshletID" },
        { VisualizeMode::TriangleID, "TriangleID" },
        { VisualizeMode::Combined, "Combined" },
    }
);
FALCOR_ENUM_REGISTER(VisualizeMode);

class MeshletCulling : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(MeshletCulling, "MeshletCulling", "GPU Meshlet Culling and Rendering Pass");

    static ref<MeshletCulling> create(ref<Device> pDevice, const Properties& props);

    MeshletCulling(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;

private:
    struct MeshletBuildResult
    {
        std::vector<meshopt_Meshlet> meshlets;
        std::vector<uint32_t> meshletVertices;
        std::vector<uint8_t> meshletTriangles;
        std::vector<float4> meshletBounds;
        std::vector<float3> remappedPositions;
        std::vector<uint32_t> flattenedIndices;
        size_t remappedVertexCount = 0;

        void clear()
        {
            meshlets.clear();
            meshletVertices.clear();
            meshletTriangles.clear();
            meshletBounds.clear();
            remappedPositions.clear();
            flattenedIndices.clear();
            remappedVertexCount = 0;
        }
    };

    void buildMeshlets();
    void createGpuBuffers();
    void updateFrustumData(const Camera* pCamera);
    void runCullingPass(RenderContext* pRenderContext);
    void initRasterPass();
    void runRasterPass(RenderContext* pContext, const ref<Texture>& pVBuffer, const ref<Texture>& pDepth);
    void initVisualizePass();
    void runVisualizePass(RenderContext* pContext, const ref<Texture>& pVBuffer, const ref<Texture>& pOutput);

    ref<Scene> mpScene;
    MeshletBuildResult mBuildResult;

    ref<Buffer> mpMeshletBuffer;
    ref<Buffer> mpMeshletVertices;
    ref<Buffer> mpMeshletTriangles;
    ref<Buffer> mpMeshletPositions;
    ref<Buffer> mpSceneDataBuffer;

    ref<Buffer> mpFlattenedIndexBuffer;
    ref<Buffer> mpVertexBuffer;

    ref<Buffer> mpVisibleMeshletIDs;
    ref<Buffer> mpIndirectArgsBuffer;
    ref<Buffer> mpFrustumBuffer;

    ref<ComputePass> mpCullingPass;
    ref<ComputePass> mpVisualizePass;

    ref<GraphicsState> mpGraphicsState;
    ref<ProgramVars> mpGraphicsVars;
    ref<RasterizerState> mpRasterState;
    ref<DepthStencilState> mpDepthStencilState;
    ref<Fbo> mpFbo;

    // --- Internal Textures ---
    ref<Texture> mpInternalVBuffer;   // R32_UINT
    ref<Texture> mpInternalDepth;     // D32_FLOAT

    struct Stats
    {
        uint32_t totalMeshlets = 0;
        uint32_t visibleMeshlets = 0;
        uint32_t totalTriangles = 0;
        uint32_t visibleTriangles = 0;
    } mStats;

    bool mEnableCulling = true;
    bool mFreezeCulling = false;
    bool mShowStats = true;
    bool mReadbackStats = false;
    bool mOutputVBuffer = true;        // true = VBuffer, false = Color
    VisualizeMode mVisualizeMode = VisualizeMode::MeshletID;

    FrustumData mCachedFrustum;
    uint32_t mFrameCount = 0;
};
