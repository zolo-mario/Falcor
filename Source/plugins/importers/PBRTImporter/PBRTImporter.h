#pragma once
#include "Scene/Importer.h"
#include <filesystem>
#include <memory>

namespace Falcor
{

/**
 * Scene importer for pbrt-v4 scenes.
 */
class PBRTImporter : public Importer
{
public:
    FALCOR_PLUGIN_CLASS(PBRTImporter, "PBRTImporter", PluginInfo({"Importer for pbrt-v4 assets", {"pbrt"}}));

    static std::unique_ptr<Importer> create();

    void importScene(
        const std::filesystem::path& path,
        SceneBuilder& builder,
        const std::map<std::string, std::string>& materialToShortName
    ) override;
};

} // namespace Falcor
