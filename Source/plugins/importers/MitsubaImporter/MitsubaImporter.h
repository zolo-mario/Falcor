#pragma once
#include "Scene/Importer.h"
#include "Scene/SceneBuilder.h"
#include <filesystem>
#include <memory>

namespace Falcor
{

/**
 * Scene importer for Mitsuba scenes.
 */
class MitsubaImporter : public Importer
{
public:
    FALCOR_PLUGIN_CLASS(MitsubaImporter, "MitsubaImporter", PluginInfo({"Importer for mitsuba assets", {"xml"}}));

    static std::unique_ptr<Importer> create();

    void importScene(
        const std::filesystem::path& path,
        SceneBuilder& builder,
        const std::map<std::string, std::string>& materialToShortName
    ) override;
};

} // namespace Falcor
