#pragma once
#include "Core/Macros.h"

#if FALCOR_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <fstream>
#include <iostream>

namespace Falcor
{
/**
 * Opens a console window and redirects std::cout, std::cerr, and std::cin there.
 * Upon destruction of the object, the console is closed and the streams are restored to the previous state.
 */
class DebugConsole
{
public:
    /**
     * Opens a console window. The destructor closes it again.
     * @param[in] waitForKey If true, the console waits for a key press before closing.
     */
    DebugConsole(bool waitForKey = true) : mWaitForKey(waitForKey)
    {
        // Open console window
        AllocConsole();

        // Redirect cout/cerr/cin streams to our console window
        mPrevCout = std::cout.rdbuf();
        mCout.open("CONOUT$");
        std::cout.rdbuf(mCout.rdbuf());

        mPrevCerr = std::cerr.rdbuf();
        mCerr.open("CONERR$");
        std::cerr.rdbuf(mCerr.rdbuf());

        mPrevCin = std::cin.rdbuf();
        mCin.open("CONIN$");
        std::cin.rdbuf(mCin.rdbuf());

        // Redirect stdout for printf() to our console
        // freopen_s(&mFp, "CONOUT$", "w", stdout);
        // std::cout.clear();
    }

    virtual ~DebugConsole()
    {
        flush();
        if (mWaitForKey)
        {
            pause();
        }

        // Restore the streams
        std::cin.rdbuf(mPrevCin);
        std::cerr.rdbuf(mPrevCerr);
        std::cout.rdbuf(mPrevCout);

        // Restore stdout to default
        // freopen("OUT", "w", stdout);
        // fclose(mFp);

        // Close console window
        FreeConsole();
    }

    void pause() const
    {
        std::cout << "Press any key to continue..." << std::endl;
        flush();
        char c = std::cin.get();
    }

    void flush() const
    {
        std::cout.flush();
        std::cerr.flush();
    }

private:
    std::ofstream mCout;
    std::ofstream mCerr;
    std::ifstream mCin;
    std::streambuf* mPrevCout;
    std::streambuf* mPrevCerr;
    std::streambuf* mPrevCin;
    // FILE* mFp = nullptr;

    bool mWaitForKey = true;
};
} // namespace Falcor

#endif // FALCOR_WINDOWS
