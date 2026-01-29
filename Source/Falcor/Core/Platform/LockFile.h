#pragma once

#include "Core/Macros.h"

#include <filesystem>

namespace Falcor
{

/**
 * Helper class abstracting lock files.
 * Uses LockFileEx() on Windows systems and flock() on POSIX systems.
 */
class FALCOR_API LockFile
{
public:
    enum class LockType
    {
        Exclusive,
        Shared,
    };

    LockFile() = default;

    /**
     * Construct and open the loc file. This will create the file if it doesn't exist yet.
     * @note Use isOpen() to check if the file was successfully opened.
     * @param path File path.
     */
    LockFile(const std::filesystem::path& path);

    ~LockFile();

    /**
     * Open the lock file. This will create the file if it doesn't exist yet.
     * @param path File path.
     * @return True if successful.
     */
    bool open(const std::filesystem::path& path);

    /// Closes the lock file.
    void close();

    /// Returns true if the lock file is open.
    bool isOpen() const { return mIsOpen; }

    /**
     * Acquire the lock in non-blocking mode.
     * @param lockType Lock type (Exclusive or Shared).
     * @return True if successful.
     */
    bool tryLock(LockType lockType = LockType::Exclusive);

    /**
     * Acquire the lock in blocking mode.
     * @param lockType Lock type (Exclusive or Shared).
     * @return True if successful.
     */
    bool lock(LockType lockType = LockType::Exclusive);

    /**
     * Release the lock.
     * @return True if successful.
     */
    bool unlock();

private:
    LockFile(const LockFile&) = delete;
    LockFile(LockFile&) = delete;
    LockFile& operator=(const LockFile&) = delete;
    LockFile& operator=(const LockFile&&) = delete;

#if FALCOR_WINDOWS
    using FileHandle = void*;
#elif FALCOR_LINUX
    using FileHandle = int;
#else
#error "Unknown OS"
#endif

    FileHandle mFileHandle;
    bool mIsOpen = false;
};

} // namespace Falcor
