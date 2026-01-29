#pragma once
#include "Core/Macros.h"
#include <array>
#include <string>
#include <cstdint>
#include <cstdlib>

namespace Falcor
{
/**
 * Helper to compute SHA-1 hash.
 */
class FALCOR_API SHA1
{
public:
    using MD = std::array<uint8_t, 20>; ///< Message digest.

    SHA1();

    /**
     * Update hash by adding one byte.
     * @param[in] value Value to hash.
     */
    void update(uint8_t value);

    /**
     * Update hash by adding the given data.
     * @param[in] data Data to hash.
     * @param[in] len Length of data in bytes.
     */
    void update(const void* data, size_t len);

    /**
     * Update hash by adding one value of fundamental type T.
     * @param[in] Value to hash.
     */
    template<typename T, std::enable_if_t<std::is_fundamental<T>::value, bool> = true>
    void update(const T& value)
    {
        update(&value, sizeof(value));
    }

    /**
     * Update hash by adding the given string view.
     */
    void update(const std::string_view str) { update(str.data(), str.size()); }

    /**
     * Return final message digest.
     * @return Returns the SHA-1 message digest.
     */
    MD finalize();

    /**
     * Compute SHA-1 hash over the given data.
     * @param[in] data Data to hash.
     * @param[in] len Length of data in bytes.
     * @return Returns the SHA-1 message digest.
     */
    static MD compute(const void* data, size_t len);

    /**
     * Convert SHA-1 hash to 40-character string in hexadecimal notation.
     */
    static std::string toString(const MD& sha1);

private:
    void addByte(uint8_t x);
    void processBlock(const uint8_t* ptr);

    uint32_t mIndex;
    uint64_t mBits;
    uint32_t mState[5];
    uint8_t mBuf[64];
};
}; // namespace Falcor
