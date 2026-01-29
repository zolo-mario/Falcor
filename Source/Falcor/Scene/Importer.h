#pragma once
#include "SceneBuilder.h"
#include "ImporterError.h"
#include "Core/Macros.h"
#include "Core/Error.h"
#include "Core/Plugin.h"
#include "Core/Platform/OS.h"
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Falcor
{
    /** Base class for importers.
        Importers are bound to a set of file extensions. This allows the right importer to
        be called when importing an asset file.
    */
    class FALCOR_API Importer
    {
    public:
        using PluginCreate = std::function<std::unique_ptr<Importer>()>;
        struct PluginInfo
        {
            std::string desc; ///< Importer description.
            std::vector<std::string> extensions; ///< List of handled file extensions.
        };

        FALCOR_PLUGIN_BASE_CLASS(Importer);

        virtual ~Importer() {}

        /** Import a scene.
            \param[in] path File path.
            \param[in] builder Scene builder.
            \param[in] dict Optional dictionary.
            Throws an ImporterError if something went wrong.
        */
        virtual void importScene(const std::filesystem::path& path, SceneBuilder& builder, const std::map<std::string, std::string>& materialToShortName) = 0;

        /** Import a scene from memory.
            \param[in] buffer Memory buffer.
            \param[in] byteSize Size in bytes of memory buffer.
            \param[in] extension File extension for the format the scene is stored in.
            \param[in] builder Scene builder.
            \param[in] dict Optional dictionary.
            Throws an ImporterError if something went wrong.
        */
        virtual void importSceneFromMemory(const void* buffer, size_t byteSize, std::string_view extension, SceneBuilder& builder, const std::map<std::string, std::string>& materialToShortName);

        // Importer factory

        /** Create an importer for a file of an asset with the given file extension.
            \param extension File extension.
            \param pm Plugin manager.
            \return Returns an instance of the importer or nullptr if no compatible importer was found.
         */
        static std::unique_ptr<Importer> create(std::string_view extension, const PluginManager& pm = PluginManager::instance());

        /** Return a list of supported file extensions by the current set of loaded importer plugins.
        */
        static std::vector<std::string> getSupportedExtensions(const PluginManager& pm = PluginManager::instance());
    };
}
