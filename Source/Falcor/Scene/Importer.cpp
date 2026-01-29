#include "Importer.h"
#include "Utils/Scripting/ScriptBindings.h"

namespace Falcor
{
    std::unique_ptr<Importer> Importer::create(std::string_view extension, const PluginManager& pm)
    {
        for (const auto& [type, info] : pm.getInfos<Importer>())
            if (std::find(info.extensions.begin(), info.extensions.end(), extension) != info.extensions.end())
                return pm.createClass<Importer>(type);
        return nullptr;
    }

    std::vector<std::string> Importer::getSupportedExtensions(const PluginManager& pm)
    {
        std::vector<std::string> extensions;
        for (const auto& [type, info] : pm.getInfos<Importer>())
            extensions.insert(extensions.end(), info.extensions.begin(), info.extensions.end());
        return extensions;
    }

    void Importer::importSceneFromMemory(const void* buffer, size_t byteSize, std::string_view extension, SceneBuilder& builder, const std::map<std::string, std::string>& materialToShortName)
    {
        FALCOR_THROW("Not implemented.");
    }

    FALCOR_SCRIPT_BINDING(Importer)
    {
        pybind11::register_exception<ImporterError>(m, "ImporterError");
    }
}
