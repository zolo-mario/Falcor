#pragma once
#include "Falcor.h"
#include "Utils/Timing/CpuTimer.h"
#include "../ComparisonPass.h"

using namespace Falcor;

class SplitScreenPass : public ComparisonPass
{
public:
    FALCOR_PLUGIN_CLASS(SplitScreenPass, "SplitScreenPass", "Allows the user to split the screen between two inputs.");

    static ref<SplitScreenPass> create(ref<Device> pDevice, const Properties& props) { return make_ref<SplitScreenPass>(pDevice, props); }

    SplitScreenPass(ref<Device> pDevice, const Properties& props);

    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) override;
    virtual void renderUI(Gui::Widgets& widget) override;

private:
    virtual void createProgram() override;

    /// A texture storing a 16x16 grayscale arrow
    ref<Texture> mpArrowTex;

    // Mouse parameters

    /// Is the mouse over the divider?
    bool mMouseOverDivider = false;
    /// Where was mouse in last mouse event processed
    int2 mMousePos = int2(0, 0);
    /// Are we grabbing the divider?
    bool mDividerGrabbed = false;

    /// When hovering over divider, show arrows?
    bool mDrawArrows = false;

    /// Time of last mouse click (double-click detection)
    CpuTimer::TimePoint mTimeOfLastClick{};
};
