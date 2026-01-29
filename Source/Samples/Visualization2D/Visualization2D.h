#pragma once
#include "Falcor.h"
#include "Core/SampleApp.h"
#include "Core/Pass/FullScreenPass.h"

using namespace Falcor;

struct VoxelNormalsGUI
{
    bool showNormalField = false;
    bool showBoxes = true;
    bool showBoxDiagonals = true;
    bool showBorderLines = false;
    bool showBoxAroundPoint = false;
};

class Visualization2D : public SampleApp
{
public:
    Visualization2D(const SampleAppConfig& config);
    ~Visualization2D();

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
