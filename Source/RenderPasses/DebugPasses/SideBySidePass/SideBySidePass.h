#pragma once
#include "Falcor.h"
#include "../ComparisonPass.h"

using namespace Falcor;

class SideBySidePass : public ComparisonPass
{
public:
    FALCOR_PLUGIN_CLASS(SideBySidePass, "SideBySidePass", "Allows the user to compare two inputs side-by-side.");

    static ref<SideBySidePass> create(ref<Device> pDevice, const Properties& props) { return make_ref<SideBySidePass>(pDevice, props); }

    SideBySidePass(ref<Device> pDevice, const Properties& props);

    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;

private:
    virtual void createProgram() override;
    uint32_t mImageLeftBound = 0; ///< Location of output left side in original input image in pixels
};
