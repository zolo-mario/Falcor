#include "Error.h"
#include "Platform/OS.h"
#include "Utils/Logger.h"
#include "Utils/Scripting/ScriptBindings.h"
#include <atomic>

namespace Falcor
{

/// Global error diagnostic flags.
const ErrorDiagnosticFlags gErrorDiagnosticFlags = ErrorDiagnosticFlags::BreakOnThrow | ErrorDiagnosticFlags::BreakOnAssert | ErrorDiagnosticFlags::AppendStackTrace;

void throwException(const fstd::source_location& loc, std::string_view msg)
{
    std::string fullMsg = fmt::format("{}\n\n{}:{} ({})", msg, loc.file_name(), loc.line(), loc.function_name());

    if (is_set(gErrorDiagnosticFlags, ErrorDiagnosticFlags::AppendStackTrace))
        fullMsg += fmt::format("\n\nStacktrace:\n{}", getStackTrace(1));

    if (is_set(gErrorDiagnosticFlags, ErrorDiagnosticFlags::BreakOnThrow) && isDebuggerPresent())
        debugBreak();

    logErrorOnce(fullMsg);
}

void reportAssertion(const fstd::source_location& loc, std::string_view cond, std::string_view msg)
{
    std::string fullMsg = fmt::format(
        "Assertion failed: {}\n{}{}\n{}:{} ({})", cond, msg, msg.empty() ? "" : "\n", loc.file_name(), loc.line(), loc.function_name()
    );

    if (is_set(gErrorDiagnosticFlags, ErrorDiagnosticFlags::AppendStackTrace))
        fullMsg += fmt::format("\n\nStacktrace:\n{}", getStackTrace(1));

    if (is_set(gErrorDiagnosticFlags, ErrorDiagnosticFlags::BreakOnAssert) && isDebuggerPresent())
        debugBreak();

    logErrorOnce(fullMsg);
}

//
// Error handling.
//

void setErrorDiagnosticFlags(ErrorDiagnosticFlags flags)
{
    //gErrorDiagnosticFlags = flags;
}

ErrorDiagnosticFlags getErrorDiagnosticFlags()
{
    return gErrorDiagnosticFlags;
}

void reportErrorAndContinue(std::string_view msg)
{
    logError(msg);

    if (is_set(gErrorDiagnosticFlags, ErrorDiagnosticFlags::ShowMessageBoxOnError))
    {
        // Show message box
        msgBox("Error", std::string(msg), MsgBoxType::Ok, MsgBoxIcon::Error);
    }
}

bool reportErrorAndAllowRetry(std::string_view msg)
{
    logError(msg);

    if (is_set(gErrorDiagnosticFlags, ErrorDiagnosticFlags::ShowMessageBoxOnError))
    {
        enum ButtonId
        {
            Retry,
            Abort
        };

        // Setup message box buttons
        std::vector<MsgBoxCustomButton> buttons;
        buttons.push_back({Retry, "Retry"});
        buttons.push_back({Abort, "Abort"});

        // Show message box
        auto result = msgBox("Error", std::string(msg), buttons, MsgBoxIcon::Error);
        return result == Retry;
    }

    return false;
}

[[noreturn]] void reportFatalErrorAndTerminate(std::string_view msg)
{
    // Immediately terminate on re-entry.
    static std::atomic<bool> entered;
    if (entered.exchange(true) == true)
        std::quick_exit(1);

    std::string fullMsg = fmt::format("{}\n\nStacktrace:\n{}", msg, getStackTrace(3));

    logFatal(fullMsg);

    if (is_set(gErrorDiagnosticFlags, ErrorDiagnosticFlags::ShowMessageBoxOnError))
    {
        enum ButtonId
        {
            Debug,
            Abort
        };

        // Setup message box buttons
        std::vector<MsgBoxCustomButton> buttons;
        if (isDebuggerPresent())
            buttons.push_back({Debug, "Debug"});
        buttons.push_back({Abort, "Abort"});

        // Show message box
        auto result = msgBox("Fatal Error", fullMsg, buttons, MsgBoxIcon::Error);
        if (result == Debug)
            debugBreak();
    }
    else
    {
        if (isDebuggerPresent())
            debugBreak();
    }

    std::quick_exit(1);
}

FALCOR_SCRIPT_BINDING(Error)
{
    pybind11::register_exception<RuntimeError>(m, "RuntimeError");
    pybind11::register_exception<AssertionError>(m, "AssertionError");
}

} // namespace Falcor
