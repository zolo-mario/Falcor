#pragma once
#include "Falcor.h"
#include <filesystem>

namespace Mogwai
{
    /** Holds a set of persistent application data stored in the user directory.
    */
    class AppData
    {
    public:
        AppData(const std::filesystem::path& path);

        const std::vector<std::filesystem::path>& getRecentScripts() const { return mRecentScripts; }
        const std::vector<std::filesystem::path>& getRecentScenes() const { return mRecentScenes; }

        void addRecentScript(const std::filesystem::path& path);
        void addRecentScene(const std::filesystem::path& path);

    private:
        void addRecentPath(std::vector<std::filesystem::path>& paths, const std::filesystem::path& path);
        void removeNonExistingPaths(std::vector<std::filesystem::path>& paths);

        void save();

        void loadFromFile(const std::filesystem::path& path);
        void saveToFile(const std::filesystem::path& path);

        std::filesystem::path mPath;

        std::vector<std::filesystem::path> mRecentScripts;
        std::vector<std::filesystem::path> mRecentScenes;
    };
}
