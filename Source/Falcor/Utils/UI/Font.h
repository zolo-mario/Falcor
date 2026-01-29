#pragma once
#include "Core/Error.h"
#include "Core/API/fwd.h"
#include "Core/API/Texture.h"
#include "Utils/Math/Vector.h"
#include <filesystem>
#include <memory>
#include <string>

namespace Falcor
{
/**
 * This class holds data and texture used to render text.
 * It represents a mono-spaced font.
 */
class Font
{
public:
    /**
     * Constructor. Throws an exception if creation failed.
     * @param[in] path File path without extension.
     */
    Font(ref<Device> pDevice, const std::filesystem::path& path);

    ~Font();

    /**
     * The structs contains information on the location of the character in the texture
     */
    struct CharTexCrdDesc
    {
        float2 topLeft; ///< Non-normalized origin of the character in the texture
        float2 size;    ///< Size in pixels of the character. This should be used to initialize the texture-coordinate when rendering.
    };

    /**
     * Get the texture containing the characters
     */
    ref<Texture> getTexture() const { return mpTexture; }

    /**
     * Get the character descriptor
     */
    const CharTexCrdDesc& getCharDesc(char c) const
    {
        FALCOR_ASSERT(c >= mFirstChar && c <= mLastChar);
        return mCharDesc[c - mFirstChar];
    }

    /**
     * Get the height in pixels of the font
     */
    float getFontHeight() const { return mFontHeight; }

    /**
     * Get the width in pixels of the tab character
     */
    float getTabWidth() const { return mTabWidth; }

    /**
     * Get the spacing in pixels between 2 characters. This is measured as (start-of-char-2) - (start-of-char-1).
     */
    float getLettersSpacing() const { return mLetterSpacing; }

private:
    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;

    bool loadFromFile(ref<Device> pDevice, const std::filesystem::path& path);

    static const char mFirstChar = '!';
    static const char mLastChar = '~';
    static const uint32_t mCharCount = mLastChar - mFirstChar + 1;
    static const uint32_t mTexWidth = 1024;

    ref<Texture> mpTexture;
    CharTexCrdDesc mCharDesc[mCharCount];
    float mFontHeight;
    float mTabWidth;
    float mLetterSpacing;
};
} // namespace Falcor
