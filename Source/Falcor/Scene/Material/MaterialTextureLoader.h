#pragma once
#include "Core/Macros.h"
#include "Scene/Material/Material.h"
#include "Utils/Image/TextureManager.h"
#include <filesystem>
#include <vector>

namespace Falcor
{
    /** Helper class to load material textures using the texture manager.

        Calling `loadTexture` does not assign the texture to the material right away.
        Instead, an asynchronous texture load request is issued and a reference for the
        material assignment is stored. When the client destroys the instance of the
        `MaterialTextureLoader`, it blocks until all textures are loaded and assigns
        them to the materials.
    */
    class FALCOR_API MaterialTextureLoader
    {
    public:
        MaterialTextureLoader(TextureManager& textureManager, bool useSrgb);
        ~MaterialTextureLoader();

        /** Request loading a material texture.
            \param[in] pMaterial Material to load texture into.
            \param[in] slot Slot to load texture into.
            \param[in] path Texture file path.
        */
        void loadTexture(const ref<Material>& pMaterial, Material::TextureSlot slot, const std::filesystem::path& path);

        void finishLoading()
        {
            assignTextures();
        }
    private:
        void assignTextures();

        struct TextureAssignment
        {
            ref<Material> pMaterial;
            Material::TextureSlot textureSlot;
            TextureManager::CpuTextureHandle handle;
        };

        bool mUseSrgb;
        std::vector<TextureAssignment> mTextureAssignments;
        TextureManager& mTextureManager;
    };
}
