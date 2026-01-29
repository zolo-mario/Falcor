#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"

using namespace Falcor;

class TestRtProgram : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(TestRtProgram, "TestRtProgram", "Test pass for RtProgram.");

    static ref<TestRtProgram> create(ref<Device> pDevice, const Properties& props) { return make_ref<TestRtProgram>(pDevice, props); }

    TestRtProgram(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override {}
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) override { return false; }
    virtual bool onKeyEvent(const KeyboardEvent& keyEvent) override { return false; }

    static void registerScriptBindings(pybind11::module& m);

private:
    void sceneChanged();
    void addCustomPrimitive();
    void removeCustomPrimitive(uint32_t index);
    void moveCustomPrimitive();

    // Internal state
    ref<Scene> mpScene;

    uint32_t mMode = 0;
    uint32_t mSelectedIdx = 0;
    uint32_t mPrevSelectedIdx = -1;
    uint32_t mUserID = 0;
    AABB mSelectedAABB;

    struct
    {
        ref<Program> pProgram;
        ref<RtProgramVars> pVars;
    } mRT;
};
