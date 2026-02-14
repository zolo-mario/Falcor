#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"
#include "../GBufferBase.h"

using namespace Falcor;

class VBufferMeshletRaster : public GBufferBase
{
public:
    FALCOR_PLUGIN_CLASS(VBufferMeshletRaster, "VBufferMeshletRaster", "Mesh shader rasterizer for VBuffer using meshlets (supports multiple instances).");

    static ref<VBufferMeshletRaster> create(ref<Device> pDevice, const Properties& props)
    {
        return make_ref<VBufferMeshletRaster>(pDevice, props);
    }

    VBufferMeshletRaster(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override {}
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) override { return false; }
    virtual bool onKeyEvent(const KeyboardEvent& keyEvent) override { return false; }

private:
    // Mesh shader pipeline
    ref<Fbo> mpFbo;
    struct
    {
        ref<GraphicsState> pState;
        ref<Program> pProgram;
        ref<ProgramVars> pVars;
    } mRaster;

    uint32_t mMeshletCount = 0;
};
