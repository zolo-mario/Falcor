#pragma once
#include "Core/Macros.h"
#include <memory>
#include <string>
#include <vector>

namespace Falcor
{
/**
 * Creates a progress bar visual and manages a new thread for it.
 */
class FALCOR_API ProgressBar
{
public:
    struct Window;

    ProgressBar();
    ~ProgressBar();

    /**
     * Show the progress bar.
     * @param[in] msg Message to display on the progress bar.
     */
    void show(const std::string& msg);

    /**
     * Close the progress bar.
     */
    void close();

    /**
     * Check if the progress bar is currently active.
     * @return Returns true if progress bar is active.
     */
    bool isActive() const { return mpWindow != nullptr; }

private:
    std::unique_ptr<Window> mpWindow;
};
} // namespace Falcor
