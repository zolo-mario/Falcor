#pragma once

#include "Core/Macros.h"

#include <cstdint>

#include <filesystem>
#include <limits>

namespace Falcor
{

/**
 * Utility class for reading memory-mapped files.
 */
class FALCOR_API MemoryMappedFile
{
public:
    enum class AccessHint
    {
        Normal,         ///< Good overall performance.
        SequentialScan, ///< Read file once with few seeks.
        RandomAccess    ///< Good for random access.
    };

    static constexpr size_t kWholeFile = std::numeric_limits<size_t>::max();

    /**
     * Default constructor. Use open() for opening a file.
     */
    MemoryMappedFile() = default;

    /**
     * Constructor opening a file. Use isOpen() to check if successful.
     * @param path Path to open.
     * @param mappedSize Number of bytes to map into memory (automatically clamped to the file size).
     * @param accessHint Hint on how memory is accessed.
     */
    MemoryMappedFile(const std::filesystem::path& path, size_t mappedSize = kWholeFile, AccessHint accessHint = AccessHint::Normal);

    /// Destructor. Closes the file.
    ~MemoryMappedFile();

    /**
     * Open a file.
     * @param path Path to open.
     * @param mappedSize Number of bytes to map into memory (automatically clamped to the file size).
     * @param accessHint Hint on how memory is accessed.
     * @return True if file was successfully opened.
     */
    bool open(const std::filesystem::path& path, size_t mappedSize = kWholeFile, AccessHint accessHint = AccessHint::Normal);

    /// Close the file.
    void close();

    /// True, if file successfully opened.
    bool isOpen() const { return mMappedData != nullptr; }

    /// Get the file size in bytes.
    size_t getSize() const { return mSize; }

    /// Get the mapped data.
    const void* getData() const { return mMappedData; };

    /// Get the mapped memory size in bytes.
    size_t getMappedSize() const { return mMappedSize; };

    /// Get the OS page size (for remap).
    static size_t getPageSize();

private:
    MemoryMappedFile(const MemoryMappedFile&) = delete;
    MemoryMappedFile(MemoryMappedFile&) = delete;
    MemoryMappedFile& operator=(const MemoryMappedFile&) = delete;
    MemoryMappedFile& operator=(const MemoryMappedFile&&) = delete;

    /**
     * Replace mapping by a new one of the same file.
     * @param offset Offset from start of the file in bytes (must be multiple of page size).
     * @param mappedSize Size of mapping in bytes.
     * @return True if successful.
     */
    bool remap(uint64_t offset, size_t mappedSize);

    std::filesystem::path mPath;
    AccessHint mAccessHint = AccessHint::Normal;
    size_t mSize = 0;

#if FALCOR_WINDOWS
    using FileHandle = void*;
    void* mMappedFile{nullptr};
#elif FALCOR_LINUX
    using FileHandle = int;
#else
#error "Unknown OS"
#endif

    FileHandle mFile = 0;
    void* mMappedData = 0;
    size_t mMappedSize = 0;
};

} // namespace Falcor
