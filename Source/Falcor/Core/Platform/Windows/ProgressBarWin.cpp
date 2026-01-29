#include "../ProgressBar.h"
#include "../OS.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <CommCtrl.h>
#include <random>
#include <thread>

namespace Falcor
{
struct ProgressBar::Window
{
    bool running;
    std::thread thread;

    Window(const std::string& msg)
    {
        running = true;
        thread = std::thread(threadFunc, this, msg);
    }

    ~Window()
    {
        running = false;
        thread.join();
    }

    static void threadFunc(ProgressBar::Window* pThis, std::string msgText)
    {
        // Create the window
        int w = 200;
        int h = 60;
        int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
        int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
        HWND hwnd = CreateWindowEx(
            0, PROGRESS_CLASS, nullptr, WS_VISIBLE | PBS_MARQUEE, x, y, w, h, nullptr, nullptr, GetModuleHandle(nullptr), nullptr
        );

        SetWindowTextA(hwnd, msgText.c_str());
        SetForegroundWindow(hwnd);
        setWindowIcon(getRuntimeDirectory() / "data/framework/nvidia.ico", hwnd);

        // Execute
        while (pThis->running)
        {
            SendMessage(hwnd, PBM_STEPIT, 0, 0);
            SendMessage(hwnd, WM_PAINT, 0, 0);
            Sleep(50);
            MSG msg;
            while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        DestroyWindow(hwnd);
    }
};

ProgressBar::ProgressBar()
{
    // Initialize the common controls
    INITCOMMONCONTROLSEX init;
    init.dwSize = sizeof(INITCOMMONCONTROLSEX);
    init.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&init);
}

ProgressBar::~ProgressBar()
{
    close();
}

void ProgressBar::show(const std::string& msg)
{
    close();
    mpWindow = std::make_unique<Window>(msg);
}

void ProgressBar::close()
{
    mpWindow.reset();
}
} // namespace Falcor
