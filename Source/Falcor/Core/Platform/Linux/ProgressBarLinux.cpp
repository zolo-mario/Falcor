#include "Core/Platform/ProgressBar.h"
#include "Core/Error.h"

#include <gtk/gtk.h>

#include <chrono>
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

    static void threadFunc(ProgressBar::Window* pThis, std::string msg)
    {
        // Create window
        GtkWidget* pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_position(GTK_WINDOW(pWindow), GTK_WIN_POS_CENTER_ALWAYS);
        gtk_window_set_decorated(GTK_WINDOW(pWindow), FALSE);

        GtkWidget* pVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_container_add(GTK_CONTAINER(pWindow), pVBox);

        // Create label for message
        GtkWidget* pLabel = gtk_label_new(msg.c_str());
        gtk_label_set_justify(GTK_LABEL(pLabel), GTK_JUSTIFY_CENTER);
        gtk_label_set_lines(GTK_LABEL(pLabel), 1);
        gtk_box_pack_start(GTK_BOX(pVBox), pLabel, TRUE, FALSE, 0);

        // Create and attach progress bar
        GtkWidget* pBar = gtk_progress_bar_new();
        gtk_box_pack_start(GTK_BOX(pVBox), pBar, TRUE, FALSE, 0);

        guint pulseTimerId = g_timeout_add(100, pulseTimer, pBar);

        gtk_widget_show_all(pWindow);

        while (pThis->running || gtk_events_pending())
        {
            gtk_main_iteration_do(FALSE);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        gtk_window_close(GTK_WINDOW(pWindow));

        g_source_remove(pulseTimerId);
        gtk_widget_destroy(pWindow);
    }

    // At regular intervals, pulse the progress bar
    static gboolean pulseTimer(gpointer pGtkData)
    {
        GtkWidget* pBar = (GtkWidget*)pGtkData;
        gtk_progress_bar_pulse(GTK_PROGRESS_BAR(pBar));
        return TRUE;
    }
};

ProgressBar::ProgressBar() {}

ProgressBar::~ProgressBar()
{
    close();
}

void ProgressBar::show(const std::string& msg)
{
    close();

    if (!gtk_init_check(0, nullptr))
        FALCOR_THROW("Failed to initialize GTK.");

    mpWindow = std::make_unique<Window>(msg);
}

void ProgressBar::close()
{
    mpWindow.reset();
}
} // namespace Falcor
