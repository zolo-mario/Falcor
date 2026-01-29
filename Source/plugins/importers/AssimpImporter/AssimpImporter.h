#pragma once
#include "Scene/Importer.h"
#include <filesystem>
#include <memory>

namespace Falcor
{

class AssimpImporter : public Importer
{
public:
    FALCOR_PLUGIN_CLASS(
        AssimpImporter,
        "AssimpImporter",
        PluginInfo(
            {"Importer for Assimp supported assets",
             {
                 "fbx", "gltf", "obj", "dae",  "x",   "md5mesh", "ply", "3ds", "blend", "ase", "ifc", "xgl", "zgl", "dxf", "lwo", "lws",
                 "lxo", "stl",  "ac",  "ms3d", "cob", "scn",     "3d",  "mdl", "mdl2",  "pk3", "smd", "vta", "raw", "ter", "glb",
             }}
        )
    );

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
};

} // namespace Falcor
