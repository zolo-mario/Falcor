#pragma once
#include "Falcor.h"
#include "Core/SampleApp.h"
#include "Camera/Camera.h"
#include "Camera/OrbiterCameraController.h"

using namespace Falcor;

/** GPU structures - must match MeshletCull.slang */
struct InstanceData
{
    float4x4 World;
    float4x4 WorldInvTrans;
    float Scale;
    uint32_t Flags;
};

struct ConstantsData
{
    float4x4 View;
    float4x4 ViewProj;
    float4 Planes[6];
    float3 ViewPosition;
    uint32_t HighlightedIndex;
    float3 CullViewPosition;
    uint32_t SelectedIndex;
    uint32_t DrawMeshlets;
};

struct MeshInfoData
{
    uint32_t IndexSize;
    uint32_t MeshletCount;
    uint32_t LastMeshletVertCount;
    uint32_t LastMeshletPrimCount;
};

struct MeshletData
{
    uint32_t VertCount;
    uint32_t VertOffset;
    uint32_t PrimCount;
    uint32_t PrimOffset;
};

struct CullDataGpu
{
    float4 BoundingSphere;
    uint32_t NormalCone;
    float ApexOffset;
};

struct VertexData
{
    float3 Position;
    float3 Normal;
};

class MeshletCull : public SampleApp
{
public:
    MeshletCull(const SampleAppConfig& config);
    ~MeshletCull();

    void onLoad(RenderContext* pRenderContext) override;
    void onShutdown() override;
    void onResize(uint32_t width, uint32_t height) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onHotReload(HotReloadFlags reloaded) override;

private:
    void createProceduralMeshlet();
    void updateConstants(RenderContext* pRenderContext);

    static const uint32_t kASGroupSize = 32;

    ref<Program> mpMeshletProgram;
    ref<ProgramVars> mpMeshletVars;
    ref<GraphicsState> mpMeshletState;
    ref<DepthStencilState> mpDepthStencilState;
    ref<RasterizerState> mpRasterizerState;

    ref<Buffer> mpConstantsBuffer;
    ref<Buffer> mpInstanceBuffer;
    ref<Buffer> mpMeshInfoBuffer;
    ref<Buffer> mpVertexBuffer;
    ref<Buffer> mpMeshletBuffer;
    ref<Buffer> mpUniqueVertexIndexBuffer;
    ref<Buffer> mpPrimitiveIndexBuffer;
    ref<Buffer> mpCullDataBuffer;

    uint32_t mMeshletCount = 0;
    float mFovy = 3.14159265f / 3.f;

    uint32_t mHighlightedIndex = 0xFFFFFFFF;
    uint32_t mSelectedIndex = 0xFFFFFFFF;
    bool mDrawMeshlets = true;

    ref<Camera> mpCamera;
    std::unique_ptr<OrbiterCameraController> mpCameraController;
};
