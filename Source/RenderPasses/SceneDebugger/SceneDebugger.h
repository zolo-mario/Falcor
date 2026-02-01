#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"
#include "Utils/Debug/PixelDebug.h"
#include "Scene/HitInfoType.slang"
#include "SharedTypes.slang"
#include <meshoptimizer.h>


using namespace Falcor;

/**
 * Scene debugger render pass.
 *
 * This pass helps identify asset issues such as incorrect normals.
 */
class SceneDebugger : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(SceneDebugger, "SceneDebugger", "Scene debugger for identifying asset issues.");

    static ref<SceneDebugger> create(ref<Device> pDevice, const Properties& props) { return make_ref<SceneDebugger>(pDevice, props); }

    SceneDebugger(ref<Device> pDevice, const Properties& props);

    Properties getProperties() const override;
    RenderPassReflection reflect(const CompileData& compileData) override;
    void compile(RenderContext* pRenderContext, const CompileData& compileData) override;
    void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    void renderUI(Gui::Widgets& widget) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override { return false; }

    // Scripting functions
    SceneDebuggerMode getMode() const { return (SceneDebuggerMode)mParams.mode; }
    void setMode(SceneDebuggerMode mode) { mParams.mode = (uint32_t)mode; }

private:
    void renderPixelDataUI(Gui::Widgets& widget);
    void initInstanceInfo();
    void buildMeshlets();
    void createMeshletBuffers(const std::vector<uint32_t>& triangleToMeshlet, uint32_t totalTriangles);

    // Meshlet build result from meshoptimizer
    struct MeshletBuildResult
    {
        std::vector<meshopt_Meshlet> meshlets;
        std::vector<uint32_t> meshletVertices;
        std::vector<uint8_t> meshletTriangles;
        std::vector<float4> meshletBounds;
        std::vector<float3> remappedPositions;
        size_t remappedVertexCount = 0;
    };

    // Internal state

    std::unique_ptr<PixelDebug> mpPixelDebug; ///< Utility class for pixel debugging (print in shaders).
    ref<SampleGenerator> mpSampleGenerator;
    ref<Scene> mpScene;
    sigs::Connection mUpdateFlagsConnection; ///< Connection to the UpdateFlags signal.
    /// IScene::UpdateFlags accumulated since last `beginFrame()`
    IScene::UpdateFlags mUpdateFlags = IScene::UpdateFlags::None;

    SceneDebuggerParams mParams;
    ref<ComputePass> mpDebugPass;
    ref<Fence> mpFence;
    /// Buffer for recording pixel data at the selected pixel.
    ref<Buffer> mpPixelData;
    /// Readback buffer.
    ref<Buffer> mpPixelDataStaging;
    ref<Buffer> mpMeshToBlasID;
    ref<Buffer> mpInstanceInfo;
    bool mPixelDataAvailable = false;
    bool mVBufferAvailable = false;

    // Meshlet data
    MeshletBuildResult mMeshletBuildResult;
    ref<Buffer> mpMeshletBuffer;
    ref<Buffer> mpMeshletVertices;
    ref<Buffer> mpMeshletTriangles;
    ref<Buffer> mpMeshletGlobalPositions;
    ref<Buffer> mpMeshletData;
    ref<Buffer> mpTriangleToMeshlet; // Maps triangle ID to meshlet ID
};
