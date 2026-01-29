#pragma once
#include "Scene/Importer.h"
#include <filesystem>
#include <memory>

namespace Falcor
{

class PythonImporter : public Importer
{
public:
    FALCOR_PLUGIN_CLASS(PythonImporter, "PythonImporter", PluginInfo({"Importer for Python scene files", {"pyscene"}}));

    static std::unique_ptr<Importer> create();

    void importScene(
        const std::filesystem::path& path,
        SceneBuilder& builder,
        const std::map<std::string, std::string>& materialToShortName
    ) override;
    void importSceneFromMemory(
        const void* buffer,
        size_t byteSize,
        std::string_view extension,
        SceneBuilder& builder,
        const std::map<std::string, std::string>& materialToShortName
    ) override;

private:
    void importInternal(const std::string& script, const std::filesystem::path& path, SceneBuilder& builder);
};

} // namespace Falcor
