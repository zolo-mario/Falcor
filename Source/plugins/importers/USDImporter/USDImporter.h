#pragma once
#include "Scene/Importer.h"
#include <filesystem>
#include <memory>

namespace Falcor
{
    class USDImporter : public Importer
    {
    public:
        FALCOR_PLUGIN_CLASS(USDImporter, "USDImporter", PluginInfo({"Importer for USD assets", {"usd", "usda", "usdc", "usdz"}}));

        static std::unique_ptr<Importer> create();

        void importScene(const std::filesystem::path& path, SceneBuilder& builder, const std::map<std::string, std::string>& materialToShortName) override;
    };
}
