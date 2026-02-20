#pragma once
#include "Falcor.h"
#include "Core/Pass/FullScreenPass.h"
#include "Core/SampleBase.h"

using namespace Falcor;

struct VoxelNormalsGUI
{
    bool showNormalField = false;
    bool showBoxes = true;
    bool showBoxDiagonals = true;
    bool showBorderLines = false;
    bool showBoxAroundPoint = false;
};

class Visualization2D : public SampleBase
{
public:
    FALCOR_PLUGIN_CLASS(Visualization2D, "Visualization2D", SampleBase::PluginInfo{"Samples/Visualization2D"});

    explicit Visualization2D(SampleApp* pHost);
    ~Visualization2D();

    static SampleBase* create(SampleApp* pHost);

    void onLoad(RenderContext* pRenderContext) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;

public:
    enum Scene
    {
        MarkerDemo,
        VoxelNormals
    };

private:
    void createRenderPass();

private:
    ref<FullScreenPass> mpMainPass;

    bool mLeftButtonDown = false;
    float2 mMousePosition = float2(0.2f, 0.1f);

    VoxelNormalsGUI mVoxelNormalsGUI;
    Scene mSelectedScene = Scene::MarkerDemo;
};
