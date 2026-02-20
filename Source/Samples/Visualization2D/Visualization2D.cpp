#include "Visualization2D.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

namespace
{
const char kMarkerShaderFile[] = "Samples/Visualization2D/Visualization2d.ps.slang";
const char kNormalsShaderFile[] = "Samples/Visualization2D/VoxelNormals.ps.slang";

const Gui::DropdownList kModeList = {
    {(uint32_t)Visualization2D::Scene::MarkerDemo, "Marker demo"},
    {(uint32_t)Visualization2D::Scene::VoxelNormals, "Voxel normals"},
};
} // namespace

Visualization2D::Visualization2D(SampleApp* pHost) : SampleBase(pHost) {}

Visualization2D::~Visualization2D() {}

void Visualization2D::onLoad(RenderContext* pRenderContext)
{
    createRenderPass();
}

void Visualization2D::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    float width = (float)pTargetFbo->getWidth();
    float height = (float)pTargetFbo->getHeight();
    auto var = mpMainPass->getRootVar();
    var["Visual2DCB"]["iResolution"] = float2(width, height);
    var["Visual2DCB"]["iGlobalTime"] = (float)getGlobalClock().getTime();
    var["Visual2DCB"]["iMousePosition"] = mMousePosition;

    switch (mSelectedScene)
    {
    case Scene::MarkerDemo:
        break;
    case Scene::VoxelNormals:
        var["VoxelNormalsCB"]["iShowNormalField"] = mVoxelNormalsGUI.showNormalField;
        var["VoxelNormalsCB"]["iShowBoxes"] = mVoxelNormalsGUI.showBoxes;
        var["VoxelNormalsCB"]["iShowBoxDiagonals"] = mVoxelNormalsGUI.showBoxDiagonals;
        var["VoxelNormalsCB"]["iShowBorderLines"] = mVoxelNormalsGUI.showBorderLines;
        var["VoxelNormalsCB"]["iShowBoxAroundPoint"] = mVoxelNormalsGUI.showBoxAroundPoint;
        break;
    default:
        break;
    }

    // Run main pass.
    mpMainPass->execute(pRenderContext, pTargetFbo);
}

void Visualization2D::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Visualization 2D", {700, 900}, {10, 10});
    bool changed = w.dropdown("Scene selection", kModeList, reinterpret_cast<uint32_t&>(mSelectedScene));
    if (changed)
    {
        createRenderPass();
    }
    bool paused = getGlobalClock().isPaused();
    changed = w.checkbox("Pause time", paused);
    if (changed)
    {
        if (paused)
        {
            getGlobalClock().pause();
        }
        else
        {
            getGlobalClock().play();
        }
    }

    renderGlobalUI(pGui);
    if (mSelectedScene == Scene::MarkerDemo)
    {
        w.text("Left-click and move mouse...");
    }
    else if (mSelectedScene == Scene::VoxelNormals)
    {
        w.text("Left-click and move mouse in the left boxes to display the normal there.");
        w.checkbox("Show normal field", mVoxelNormalsGUI.showNormalField, false);
        w.checkbox("Show boxes", mVoxelNormalsGUI.showBoxes, false);
        w.checkbox("Show box diagonals", mVoxelNormalsGUI.showBoxDiagonals, false);
        w.checkbox("Show border lines", mVoxelNormalsGUI.showBorderLines, false);
        w.checkbox("Show box around point", mVoxelNormalsGUI.showBoxAroundPoint, false);
    }
}

bool Visualization2D::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return false;
}

bool Visualization2D::onMouseEvent(const MouseEvent& mouseEvent)
{
    bool bHandled = false;
    switch (mouseEvent.type)
    {
    case MouseEvent::Type::ButtonDown:
    case MouseEvent::Type::ButtonUp:
        if (mouseEvent.button == Input::MouseButton::Left)
        {
            mLeftButtonDown = mouseEvent.type == MouseEvent::Type::ButtonDown;
            bHandled = true;
        }
        break;
    case MouseEvent::Type::Move:
        if (mLeftButtonDown)
        {
            mMousePosition = mouseEvent.screenPos;
            bHandled = true;
        }
        break;
    default:
        break;
    }
    return bHandled;
}

void Visualization2D::createRenderPass()
{
    switch (mSelectedScene)
    {
    case Scene::MarkerDemo:
        mpMainPass = FullScreenPass::create(getDevice(), kMarkerShaderFile);
        break;
    case Scene::VoxelNormals:
        mpMainPass = FullScreenPass::create(getDevice(), kNormalsShaderFile);
        break;
    default:
        FALCOR_UNREACHABLE();
        break;
    }
}

SampleBase* Visualization2D::create(SampleApp* pHost)
{
    return new Visualization2D(pHost);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<SampleBase, Visualization2D>();
}
