#include "LockFile.h"

#if FALCOR_WINDOWS
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#elif FALCOR_LINUX
#include <unistd.h>
#include <sys/types.h>
#include <sys/file.h>
#else
#error "Unknown OS"
#endif

namespace Falcor
{

LockFile::LockFile(const std::filesystem::path& path)
{
    open(path);
}

LockFile::~LockFile()
{
    close();
}

bool LockFile::open(const std::filesystem::path& path)
{
#if FALCOR_WINDOWS
    mFileHandle = ::CreateFileW(
        path.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        NULL
    );
    mIsOpen = mFileHandle != INVALID_HANDLE_VALUE;
#elif FALCOR_LINUX
    mFileHandle = ::open(path.c_str(), O_RDWR | O_CREAT, 0600);
    mIsOpen = mFileHandle != -1;
#endif
    return mIsOpen;
}

void LockFile::close()
{
    if (!mIsOpen)
        return;

#if FALCOR_WINDOWS
    ::CloseHandle(mFileHandle);
#elif FALCOR_LINUX
    ::close(mFileHandle);
#endif
    mIsOpen = false;
}

bool LockFile::tryLock(LockType lockType)
{
    if (!mIsOpen)
        return false;

    bool success = false;
#if FALCOR_WINDOWS
    OVERLAPPED overlapped = {0};
    DWORD flags = lockType == LockType::Shared ? LOCKFILE_FAIL_IMMEDIATELY : (LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY);
    success = ::LockFileEx(mFileHandle, flags, DWORD(0), ~DWORD(0), ~DWORD(0), &overlapped);
#elif FALCOR_LINUX
    int operation = lockType == LockType::Shared ? (LOCK_SH | LOCK_NB) : (LOCK_EX | LOCK_NB);
    success = ::flock(mFileHandle, operation) == 0;
#endif
    return success;
}

bool LockFile::lock(LockType lockType)
{
    if (!mIsOpen)
        return false;

    bool success = false;
#if FALCOR_WINDOWS
    OVERLAPPED overlapped = {0};
    overlapped.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    DWORD flags = lockType == LockType::Shared ? 0 : LOCKFILE_EXCLUSIVE_LOCK;
    success = ::LockFileEx(mFileHandle, flags, DWORD(0), ~DWORD(0), ~DWORD(0), &overlapped);
    if (!success)
    {
        auto err = ::GetLastError();
        if (err == ERROR_IO_PENDING)
        {
            DWORD bytes;
            if (::GetOverlappedResult(mFileHandle, &overlapped, &bytes, TRUE))
                success = true;
        }
    }
    ::CloseHandle(overlapped.hEvent);
#elif FALCOR_LINUX
    int operation = lockType == LockType::Shared ? LOCK_SH : LOCK_EX;
    success = ::flock(mFileHandle, operation) == 0;
#endif
    return success;
}

bool LockFile::unlock()
{
    if (!mIsOpen)
        return false;

    bool success = false;
#if FALCOR_WINDOWS
    ::OVERLAPPED overlapped = {0};
    success = ::UnlockFileEx(mFileHandle, DWORD(0), ~DWORD(0), ~DWORD(0), &overlapped);
#elif FALCOR_LINUX
    success = ::flock(mFileHandle, LOCK_UN) == 0;
#endif
    return success;
}

} // namespace Falcor
