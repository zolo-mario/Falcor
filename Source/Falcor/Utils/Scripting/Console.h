#pragma once
#include "Core/Macros.h"
#include <optional>
#include <string>
#include <vector>

// Forward declaration.
struct ImGuiInputTextCallbackData;

namespace Falcor
{
class Gui;

class FALCOR_API Console
{
public:
    /**
     * Clears the console.
     */
    void clear();

    /**
     * Renders the console and handles important keyboard input events:
     * - The "`" key is used to open/close the console.
     * - The ESC key is used to close the console if currently open.
     * - The UP/DOWN keys are used to browse through the history.
     * @param[in] pGui GUI.
     * @param[in,out] show Flag to indicate if console is shown.
     */
    void render(Gui* pGui, bool& show);

    /**
     * Processes console input. Should be called once at the end of every frame.
     * @return Returns true if some processing occured.
     */
    bool flush();

private:
    void enterCommand();
    std::optional<std::string> browseHistory(bool upOrDown);
    static int inputTextCallback(ImGuiInputTextCallbackData* data);

    std::string mLog;
    char mCmdBuffer[2048] = {};
    std::string mCmdPending;
    std::vector<std::string> mHistory;
    int32_t mHistoryIndex = -1;
    bool mScrollToBottom = true;
};
} // namespace Falcor
