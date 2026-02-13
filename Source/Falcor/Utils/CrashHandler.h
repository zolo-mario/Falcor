#pragma once

#include "Core/Macros.h"

#if FALCOR_WINDOWS

#include <windows.h>
#include <dbghelp.h>
#include <cstdlib>
#include <ctime>
#include <string>

// Auto-link DbgHelp library
#pragma comment(lib, "dbghelp.lib")

namespace Falcor
{

/// Windows crash handler that generates full memory dumps on unhandled exceptions.
/// Call Install() early in main() before any crashes can occur.
class CrashHandler
{
public:
    static void Install()
    {
        SetUnhandledExceptionFilter(GenerateDump);
        // Prevent CRT abort/retry/ignore popups
        _set_abort_behavior(0, _WRITE_ABORT_MSG);
        _set_error_mode(_OUT_TO_STDERR);
    }

private:
    static std::wstring getDumpFileName()
    {
        time_t now = time(0);
        tm tstruct;
        localtime_s(&tstruct, &now);
        wchar_t buf[80];
        wcsftime(buf, sizeof(buf) / sizeof(wchar_t), L"Falcor_Crash_%Y-%m-%d_%H-%M-%S.dmp", &tstruct);
        return std::wstring(buf);
    }

    static LONG WINAPI GenerateDump(PEXCEPTION_POINTERS pExceptionPointers)
    {
        std::wstring fileName = getDumpFileName();
        wprintf(L"\n\n[CRASH DETECTED] Generating Full Memory Dump: %s\n", fileName.c_str());

        HANDLE hDumpFile =
            CreateFileW(fileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hDumpFile != INVALID_HANDLE_VALUE)
        {
            MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
            dumpInfo.ThreadId = GetCurrentThreadId();
            dumpInfo.ExceptionPointers = pExceptionPointers;
            dumpInfo.ClientPointers = FALSE;

            // MiniDumpWithFullMemory ensures we can see all heap variables in VS
            DWORD flags = MiniDumpWithFullMemory | MiniDumpWithHandleData | MiniDumpWithUnloadedModules;

            BOOL result = MiniDumpWriteDump(
                GetCurrentProcess(),
                GetCurrentProcessId(),
                hDumpFile,
                (MINIDUMP_TYPE)flags,
                &dumpInfo,
                NULL,
                NULL
            );
            CloseHandle(hDumpFile);

            if (result)
                wprintf(L"[SUCCESS] Dump saved. Open .dmp file in Visual Studio to debug.\n");
            else
                wprintf(L"[FAILED] Failed to write dump. Error: %u\n", GetLastError());
        }
        return EXCEPTION_EXECUTE_HANDLER;
    }
};

} // namespace Falcor

#else

namespace Falcor
{

/// Crash handler (Windows only). No-op on other platforms.
class CrashHandler
{
public:
    static void Install() {}
};

} // namespace Falcor

#endif // FALCOR_WINDOWS
