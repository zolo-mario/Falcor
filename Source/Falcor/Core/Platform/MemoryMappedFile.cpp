#include "MemoryMappedFile.h"

#include <stdexcept>
#include <cstdio>

#if FALCOR_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#elif FALCOR_LINUX
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#else
#error "Unknown OS"
#endif

namespace Falcor
{

MemoryMappedFile::MemoryMappedFile(const std::filesystem::path& path, size_t mappedSize, AccessHint accessHint)
{
    open(path, mappedSize, accessHint);
}

MemoryMappedFile::~MemoryMappedFile()
{
    close();
}

bool MemoryMappedFile::open(const std::filesystem::path& path, size_t mappedSize, AccessHint accessHint)
{
    if (isOpen())
        return false;

    mPath = path;
    mAccessHint = accessHint;

#if FALCOR_WINDOWS
    // Handle access hint.
    DWORD flags = 0;
    switch (mAccessHint)
    {
    case AccessHint::Normal:
        flags = FILE_ATTRIBUTE_NORMAL;
        break;
    case AccessHint::SequentialScan:
        flags = FILE_FLAG_SEQUENTIAL_SCAN;
        break;
    case AccessHint::RandomAccess:
        flags = FILE_FLAG_RANDOM_ACCESS;
        break;
    default:
        break;
    }

    // Open file.
    mFile = ::CreateFile(mPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, flags, NULL);
    if (!mFile)
        return false;

    // Get file size.
    LARGE_INTEGER size;
    if (!::GetFileSizeEx(mFile, &size))
    {
        close();
        return false;
    }
    mSize = static_cast<size_t>(size.QuadPart);

    // Create file mapping.
    mMappedFile = ::CreateFileMapping(mFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!mMappedFile)
    {
        close();
        return false;
    }

#elif FALCOR_LINUX

    // Open file.
    mFile = ::open(path.c_str(), O_RDONLY | O_LARGEFILE);
    if (mFile == -1)
    {
        mFile = 0;
        return false;
    }

    // Get file size.
    struct stat64 statInfo;
    if (fstat64(mFile, &statInfo) < 0)
    {
        close();
        return false;
    }
    mSize = statInfo.st_size;

#endif

    // Initial mapping.
    if (!remap(0, mappedSize))
    {
        close();
        return false;
    }

    return true;
}

void MemoryMappedFile::close()
{
    // Unmap memory.
    if (mMappedData)
    {
#if FALCOR_WINDOWS
        ::UnmapViewOfFile(mMappedData);
#elif FALCOR_LINUX
        ::munmap(mMappedData, mMappedSize);
#endif
        mMappedData = nullptr;
    }

#if FALCOR_WINDOWS
    if (mMappedFile)
    {
        ::CloseHandle(mMappedFile);
        mMappedFile = nullptr;
    }
#endif

    // Close file.
    if (mFile)
    {
#if FALCOR_WINDOWS
        ::CloseHandle(mFile);
#elif FALCOR_LINUX
        ::close(mFile);
#endif
        mFile = 0;
    }

    mSize = 0;
}

size_t MemoryMappedFile::getPageSize()
{
#if FALCOR_WINDOWS
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwAllocationGranularity;
#elif FALCOR_LINUX
    return sysconf(_SC_PAGESIZE);
#endif
}

bool MemoryMappedFile::remap(uint64_t offset, size_t mappedSize)
{
    if (!mFile)
        return false;
    if (offset >= mSize)
        return false;

    // Close previous mapping.
    if (mMappedData)
    {
#if FALCOR_WINDOWS
        ::UnmapViewOfFile(mMappedData);
#elif FALCOR_LINUX
        ::munmap(mMappedData, mMappedSize);
#endif
        mMappedData = nullptr;
        mMappedSize = 0;
    }

    // Clamp mapped range.
    if (offset + mappedSize > mSize)
        mappedSize = size_t(mSize - offset);

#if FALCOR_WINDOWS
    DWORD offsetLow = DWORD(offset & 0xFFFFFFFF);
    DWORD offsetHigh = DWORD(offset >> 32);

    // Create new mapping.
    mMappedData = ::MapViewOfFile(mMappedFile, FILE_MAP_READ, offsetHigh, offsetLow, mappedSize);
    if (!mMappedData)
        mMappedSize = 0;
    mMappedSize = mappedSize;
#else
    // Create new mapping.
    mMappedData = ::mmap64(NULL, mappedSize, PROT_READ, MAP_SHARED, mFile, offset);
    if (mMappedData == MAP_FAILED)
    {
        mMappedData = nullptr;
        return false;
    }
    mMappedSize = mappedSize;

    // Handle access hint.
    int advice = 0;
    switch (mAccessHint)
    {
    case AccessHint::Normal:
        advice = MADV_NORMAL;
        break;
    case AccessHint::SequentialScan:
        advice = MADV_SEQUENTIAL;
        break;
    case AccessHint::RandomAccess:
        advice = MADV_RANDOM;
        break;
    default:
        break;
    }
    ::madvise(mMappedData, mMappedSize, advice);
#endif

    return true;
}

} // namespace Falcor
