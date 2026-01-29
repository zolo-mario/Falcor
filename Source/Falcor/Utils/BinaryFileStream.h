#pragma once
#include <filesystem>
#include <fstream>

namespace Falcor
{
/**
 * Helper class to manage file I/O with binary files
 */
class BinaryFileStream
{
public:
    /**
     * Mode to open file as
     */
    enum class Mode
    {
        Read = 0x1,     ///< Open file for reading
        Write = 0x2,    ///< Open file for writing
        ReadWrite = 0x3 ///< Open file for both reading and writing
    };

    /**
     * Default constructor.
     */
    BinaryFileStream(){};

    /**
     * Constructor that opens a file
     * @param[in] path Path of file to open or create
     * @param[in] mode Mode to open file as
     */
    BinaryFileStream(const std::filesystem::path& path, Mode mode = Mode::ReadWrite) { open(path, mode); }

    /**
     * Destructor
     */
    ~BinaryFileStream() { close(); }

    /**
     * Opens a file stream. Fails if a file is already open.
     * @param[in] path Path of file to open or create
     * @param[in] mode Mode to open file as
     */
    void open(const std::filesystem::path& path, Mode mode = Mode::ReadWrite)
    {
        std::ios::openmode iosMode = std::ios::binary;
        iosMode |= ((mode == Mode::Read) || (mode == Mode::ReadWrite)) ? std::ios::in : (std::ios::openmode)0;
        iosMode |= ((mode == Mode::Write) || (mode == Mode::ReadWrite)) ? std::ios::out : (std::ios::openmode)0;
        mStream.open(path, iosMode);
        mPath = path;
    }

    /**
     * Close the file stream.
     */
    void close() { mStream.close(); }

    /**
     * Skip data in an input stream. Advances file stream without reading.
     * @param[in] count Bytes to skip
     */
    void skip(uint32_t count) { mStream.ignore(count); }

    /**
     * Deletes the managed file.
     */
    void remove()
    {
        if (mStream.is_open())
        {
            close();
        }
        std::filesystem::remove(mPath);
    }

    /**
     * Calculates amount of remaining data in the file.
     * @return Number of bytes remaining in the stream
     */
    uint32_t getRemainingStreamSize()
    {
        std::streamoff currentPos = mStream.tellg();
        mStream.seekg(0, mStream.end);
        std::streamoff length = mStream.tellg();
        mStream.seekg(currentPos);
        return (uint32_t)(length - currentPos);
    }

    /**
     * Checks for validity of the stream
     * @return Returns true if no errors have been encountered and the end of the stream has not been reached
     */
    bool isGood() { return mStream.good(); }

    /**
     * Checks for stream errors.
     * @return Returns true if an error has occurred while reading or writing data.
     */
    bool isBad() { return mStream.bad(); }

    /**
     * Checks for stream errors.
     * @return Returns true if any error has occurred while reading the file.
     */
    bool isFail() { return mStream.fail(); }

    /**
     * Checks if the end of file has been reached.
     * @return Returns true if stream has reached the end of the file
     */
    bool isEof() { return mStream.eof(); }

    /**
     * Reads data from the file stream
     * @param[out] pData Pointer to a buffer to copy/read data into
     * @param[in] count Number of bytes to read
     */
    BinaryFileStream& read(void* pData, size_t count)
    {
        mStream.read((char*)pData, count);
        return *this;
    }

    /**
     * Writes data to the file stream
     * @param[in] pData Pointer to buffer containing data to write to the stream
     * @param[in] count Number of bytes to write
     */
    BinaryFileStream& write(const void* pData, size_t count)
    {
        mStream.write((char*)pData, count);
        return *this;
    }

    // Operator overloads

    /**
     * Extracts a single value from the stream
     * @param[out] val Reference of value to extract into
     */
    template<typename T>
    BinaryFileStream& operator>>(T& val)
    {
        return read(&val, sizeof(T));
    }

    /**
     * Writes a value into the file stream
     * @param[in] val Value to write
     */
    template<typename T>
    BinaryFileStream& operator<<(const T& val)
    {
        return write(&val, sizeof(T));
    }

private:
    std::fstream mStream;
    std::filesystem::path mPath;
};
} // namespace Falcor
