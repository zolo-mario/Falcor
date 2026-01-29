#pragma once
#include "Mogwai.h"

namespace Mogwai
{
    class Renderer;

    class MogwaiSettings : public Extension
    {
    public:
        using UniquePtr = std::unique_ptr<Extension>;
        static UniquePtr create(Renderer* pRenderer);

        void renderUI(Gui* pGui) override;
        bool mouseEvent(const MouseEvent& e) override;
        bool keyboardEvent(const KeyboardEvent& e) override;
        bool gamepadEvent(const GamepadEvent& e) override;

        void onOptionsChange(const Settings::Options& settings) override;
    private:
        MogwaiSettings(Renderer* pRenderer) : Extension(pRenderer, "Settings") {}

        void renderMainMenu(Gui* pGui);
        void renderOverlay(Gui* pGui);
        void renderGraphs(Gui* pGui);
        void renderTimeSettings(Gui* pGui);
        void renderWindowSettings(Gui* pGui);
        void selectNextGraph();
        void exitIfNeeded();

        bool mAutoHideMenu = false;
        bool mShowFps = true;
        bool mShowOverlayUI = true;
        bool mShowGraphUI = true;
        bool mShowConsole = false;
        bool mShowTime = false;
        bool mShowWinSize = false;
        uint2 mMousePosition;
    };
}
