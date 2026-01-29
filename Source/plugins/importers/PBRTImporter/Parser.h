// This code is based on pbrt:
// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

#pragma once

#include "Types.h"
#include "Parameters.h"
#include <functional>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace Falcor::pbrt
{

class ParserTarget
{
public:
    virtual ~ParserTarget();

    virtual void onScale(Float sx, Float sy, Float sz, FileLoc loc) = 0;
    virtual void onShape(const std::string& name, ParsedParameterVector params, FileLoc loc) = 0;

    virtual void onOption(const std::string& name, const std::string& value, FileLoc loc) = 0;

    virtual void onIdentity(FileLoc loc) = 0;
    virtual void onTranslate(Float dx, Float dy, Float dz, FileLoc loc) = 0;
    virtual void onRotate(Float angle, Float ax, Float ay, Float az, FileLoc loc) = 0;
    virtual void onLookAt(Float ex, Float ey, Float ez, Float lx, Float ly, Float lz, Float ux, Float uy, Float uz, FileLoc loc) = 0;
    virtual void onConcatTransform(Float transform[16], FileLoc loc) = 0;
    virtual void onTransform(Float transform[16], FileLoc loc) = 0;
    virtual void onCoordinateSystem(const std::string&, FileLoc loc) = 0;
    virtual void onCoordSysTransform(const std::string&, FileLoc loc) = 0;
    virtual void onActiveTransformAll(FileLoc loc) = 0;
    virtual void onActiveTransformEndTime(FileLoc loc) = 0;
    virtual void onActiveTransformStartTime(FileLoc loc) = 0;
    virtual void onTransformTimes(Float start, Float end, FileLoc loc) = 0;

    virtual void onColorSpace(const std::string& n, FileLoc loc) = 0;
    virtual void onPixelFilter(const std::string& name, ParsedParameterVector params, FileLoc loc) = 0;
    virtual void onFilm(const std::string& type, ParsedParameterVector params, FileLoc loc) = 0;
    virtual void onAccelerator(const std::string& name, ParsedParameterVector params, FileLoc loc) = 0;
    virtual void onIntegrator(const std::string& name, ParsedParameterVector params, FileLoc loc) = 0;
    virtual void onCamera(const std::string&, ParsedParameterVector params, FileLoc loc) = 0;
    virtual void onMakeNamedMedium(const std::string& name, ParsedParameterVector params, FileLoc loc) = 0;
    virtual void onMediumInterface(const std::string& insideName, const std::string& outsideName, FileLoc loc) = 0;
    virtual void onSampler(const std::string& name, ParsedParameterVector params, FileLoc loc) = 0;

    virtual void onWorldBegin(FileLoc loc) = 0;
    virtual void onAttributeBegin(FileLoc loc) = 0;
    virtual void onAttributeEnd(FileLoc loc) = 0;
    virtual void onAttribute(const std::string& target, ParsedParameterVector params, FileLoc loc) = 0;
    virtual void onTexture(
        const std::string& name,
        const std::string& type,
        const std::string& texname,
        ParsedParameterVector params,
        FileLoc loc
    ) = 0;
    virtual void onMaterial(const std::string& name, ParsedParameterVector params, FileLoc loc) = 0;
    virtual void onMakeNamedMaterial(const std::string& name, ParsedParameterVector params, FileLoc loc) = 0;
    virtual void onNamedMaterial(const std::string& name, FileLoc loc) = 0;
    virtual void onLightSource(const std::string& name, ParsedParameterVector params, FileLoc loc) = 0;
    virtual void onAreaLightSource(const std::string& name, ParsedParameterVector params, FileLoc loc) = 0;
    virtual void onReverseOrientation(FileLoc loc) = 0;
    virtual void onObjectBegin(const std::string& name, FileLoc loc) = 0;
    virtual void onObjectEnd(FileLoc loc) = 0;
    virtual void onObjectInstance(const std::string& name, FileLoc loc) = 0;

    virtual void onEndOfFiles() = 0;
};

void parseFile(ParserTarget& target, const std::filesystem::path& path);
void parseString(ParserTarget& target, std::string str);

struct Token
{
    Token() = default;
    Token(const std::string_view token, FileLoc loc) : token(token), loc(loc) {}
    std::string toString() const;
    std::string_view token;
    FileLoc loc;
};

class Tokenizer
{
public:
    Tokenizer(std::string str, const std::filesystem::path& path);

    static std::unique_ptr<Tokenizer> createFromFile(const std::filesystem::path& path);
    static std::unique_ptr<Tokenizer> createFromString(std::string str);

    /**
     * Get the next token.
     * Note: The Token::token field is only valid until the next call to next().
     */
    std::optional<Token> next();

    const std::filesystem::path& getPath() const { return mPath; }

private:
    /**
     * Static list of filenames to allow file locations (FileLoc::filename) to be valid
     * even after the tokenizer is destroyed.
     */
    static std::vector<std::unique_ptr<std::string>>& getFilenames()
    {
        static std::vector<std::unique_ptr<std::string>> filenames;
        return filenames;
    }

    bool isUTF16(const void* ptr, size_t len) const;

    int getChar()
    {
        if (mPos == mEnd)
            return EOF;
        int ch = *mPos++;
        if (ch == '\n')
        {
            ++mLoc.line;
            mLoc.column = 0;
        }
        else
        {
            ++mLoc.column;
        }
        return ch;
    }

    void ungetChar()
    {
        --mPos;
        if (*mPos == '\n')
        {
            // Don't worry about the column; we'll be going to the start of
            // the next line again shortly...
            --mLoc.line;
        }
    }

    std::filesystem::path mPath; ///< File path we're reading from.
    FileLoc mLoc;                ///< File location.
    std::string mContents;       ///< File contents we're parsing.

    const char* mPos; ///< Current position in the file.
    const char* mEnd; ///< End of the file (one past).

    std::string mEscaped; ///< Temporary storage for escaped tokens.
};

} // namespace Falcor::pbrt
