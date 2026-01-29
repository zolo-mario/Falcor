#include "Font.h"
#include "Core/Error.h"
#include "Core/API/Texture.h"
#include <fstream>

namespace Falcor
{
static const uint32_t kFontMagicNumber = 0xDEAD0001;

#pragma pack(1)
struct FontFileHeader
{
    uint32_t structSize;
    uint32_t charDataSize;
    uint32_t magicNumber;
    uint32_t charCount;
    float fontHeight;
    float tabWidth;
    float letterSpacing;
};

#pragma pack(1)
struct FontCharData
{
    char character;
    float topLeftX;
    float topLeftY;
    float width;
    float height;
};

Font::Font(ref<Device> pDevice, const std::filesystem::path& path)
{
    if (!loadFromFile(pDevice, path))
        FALCOR_THROW("Failed to create font resource");
}

Font::~Font() = default;

bool Font::loadFromFile(ref<Device> pDevice, const std::filesystem::path& path)
{
    std::filesystem::path texturePath = path;
    texturePath.replace_extension(".dds");
    std::filesystem::path dataPath = path;
    dataPath.replace_extension(".bin");

    if (!std::filesystem::exists(texturePath) || !std::filesystem::exists(dataPath))
        return false;

    // Load the data
    std::ifstream data(dataPath, std::ios::binary);
    FontFileHeader header;
    // Read the header
    data.read((char*)&header, sizeof(header));
    bool valid = (header.structSize == sizeof(header));
    valid = valid && (header.magicNumber == kFontMagicNumber);
    valid = valid && (header.charDataSize == sizeof(FontCharData));
    valid = valid && (header.charCount == mCharCount);

    if (!valid)
        return false;

    mTabWidth = header.tabWidth;
    mFontHeight = header.fontHeight;

    mLetterSpacing = 0;
    // Load the char data
    for (uint32_t i = 0; i < mCharCount; i++)
    {
        FontCharData charData;
        data.read((char*)&charData, sizeof(FontCharData));
        if (charData.character != i + mFirstChar)
        {
            data.close();
            return false;
        }

        mCharDesc[i].topLeft.x = charData.topLeftX;
        mCharDesc[i].topLeft.y = charData.topLeftY;
        mCharDesc[i].size.x = charData.width;
        mCharDesc[i].size.y = charData.height;
        mLetterSpacing = std::max(mLetterSpacing, charData.width);
    }

    // Load the texture
    mpTexture = Texture::createFromFile(pDevice, texturePath, false, false);
    return mpTexture != nullptr;
}
} // namespace Falcor
