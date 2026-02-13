#pragma once

#include "Core/Macros.h"

#if FALCOR_WINDOWS

#include <windows.h>
#include <dbghelp.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

// Auto-link DbgHelp library
#pragma comment(lib, "dbghelp.lib")

namespace Falcor
{

/// Windows crash handler that prints stack traces and generates full memory dumps on unhandled exceptions.
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
        SymInitialize(GetCurrentProcess(), NULL, TRUE);
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

    static void printExceptionInfo(EXCEPTION_RECORD* exceptionRecord)
    {
        wprintf(L"Exception Code: 0x%08X\n", exceptionRecord->ExceptionCode);

        switch (exceptionRecord->ExceptionCode)
        {
            case EXCEPTION_ACCESS_VIOLATION:
                wprintf(L"Access Violation Exception\n");
                if (exceptionRecord->NumberParameters >= 2)
                {
                    wprintf(
                        L"  %s at address 0x%p\n",
                        exceptionRecord->ExceptionInformation[0] ? L"Write" : L"Read",
                        (void*)exceptionRecord->ExceptionInformation[1]
                    );
                }
                break;
            case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
                wprintf(L"Array Bounds Exceeded Exception\n");
                break;
            case EXCEPTION_BREAKPOINT:
                wprintf(L"Breakpoint Exception\n");
                break;
            case EXCEPTION_DATATYPE_MISALIGNMENT:
                wprintf(L"Datatype Misalignment Exception\n");
                break;
            case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            case EXCEPTION_INT_DIVIDE_BY_ZERO:
                wprintf(L"Divide By Zero Exception\n");
                break;
            case EXCEPTION_ILLEGAL_INSTRUCTION:
                wprintf(L"Illegal Instruction Exception\n");
                break;
            case EXCEPTION_IN_PAGE_ERROR:
                wprintf(L"In-Page Error Exception\n");
                break;
            case EXCEPTION_STACK_OVERFLOW:
                wprintf(L"Stack Overflow Exception\n");
                break;
            default:
                wprintf(L"Unknown Exception\n");
                break;
        }
    }

    static void printStackTrace(PEXCEPTION_POINTERS pExceptionPointers)
    {
        CONTEXT* context = pExceptionPointers->ContextRecord;

        STACKFRAME64 stackFrame;
        ZeroMemory(&stackFrame, sizeof(STACKFRAME64));

#if defined(_M_X64) || defined(_M_AMD64)
        stackFrame.AddrPC.Offset = context->Rip;
        stackFrame.AddrPC.Mode = AddrModeFlat;
        stackFrame.AddrFrame.Offset = context->Rsp;
        stackFrame.AddrFrame.Mode = AddrModeFlat;
        stackFrame.AddrStack.Offset = context->Rsp;
        stackFrame.AddrStack.Mode = AddrModeFlat;
        DWORD machineType = IMAGE_FILE_MACHINE_AMD64;
#else
        stackFrame.AddrPC.Offset = context->Eip;
        stackFrame.AddrPC.Mode = AddrModeFlat;
        stackFrame.AddrFrame.Offset = context->Ebp;
        stackFrame.AddrFrame.Mode = AddrModeFlat;
        stackFrame.AddrStack.Offset = context->Esp;
        stackFrame.AddrStack.Mode = AddrModeFlat;
        DWORD machineType = IMAGE_FILE_MACHINE_I386;
#endif

        while (StackWalk64(
            machineType,
            GetCurrentProcess(),
            GetCurrentThread(),
            &stackFrame,
            context,
            nullptr,
            SymFunctionTableAccess64,
            SymGetModuleBase64,
            nullptr))
        {
            DWORD64 address = stackFrame.AddrPC.Offset;
            DWORD64 displacement = 0;
            IMAGEHLP_SYMBOL64* symbol =
                (IMAGEHLP_SYMBOL64*)malloc(sizeof(IMAGEHLP_SYMBOL64) + 256);
            symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
            symbol->MaxNameLength = 255;

            if (SymGetSymFromAddr64(GetCurrentProcess(), address, &displacement, symbol))
            {
                std::wcout << L"  " << symbol->Name << L" (0x" << std::hex << address << std::dec << L")\n";
            }
            else
            {
                std::wcout << L"  0x" << std::hex << address << std::dec << L" (unresolved symbol)\n";
            }
            free(symbol);
        }
    }

    static LONG WINAPI GenerateDump(PEXCEPTION_POINTERS pExceptionPointers)
    {
        wprintf(L"\n\n[CRASH DETECTED] An exception occurred:\n");
        printExceptionInfo(pExceptionPointers->ExceptionRecord);
        wprintf(L"\nStack trace:\n");
        printStackTrace(pExceptionPointers);

        std::wstring fileName = getDumpFileName();
        wprintf(L"\n[CRASH DETECTED] Generating full memory dump: %s\n", fileName.c_str());

#if 0 // disable minidump generation for now, it's too time consuming
        HANDLE hDumpFile =
            CreateFileW(fileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hDumpFile != INVALID_HANDLE_VALUE)
        {
            MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
            dumpInfo.ThreadId = GetCurrentThreadId();
            dumpInfo.ExceptionPointers = pExceptionPointers;
            dumpInfo.ClientPointers = FALSE;

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
#endif
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
