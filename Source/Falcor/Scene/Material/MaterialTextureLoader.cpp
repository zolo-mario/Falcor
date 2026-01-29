#include "MaterialTextureLoader.h"
#include "Utils/Logger.h"

namespace Falcor
{
    MaterialTextureLoader::MaterialTextureLoader(TextureManager& textureManager, bool useSrgb)
        : mUseSrgb(useSrgb)
        , mTextureManager(textureManager)
    {
    }

    MaterialTextureLoader::~MaterialTextureLoader()
    {
        assignTextures();
    }

    void MaterialTextureLoader::loadTexture(const ref<Material>& pMaterial, Material::TextureSlot slot, const std::filesystem::path& path)
    {
        FALCOR_ASSERT(pMaterial);
        if (!pMaterial->hasTextureSlot(slot))
        {
            logWarning("MaterialTextureLoader::loadTexture() - Material '{}' does not have texture slot '{}'. Ignoring call.", pMaterial->getName(), to_string(slot));
            return;
        }

        bool srgb = mUseSrgb && pMaterial->getTextureSlotInfo(slot).srgb;

        // Request texture to be loaded.
        auto handle = mTextureManager.loadTexture(
            path,
            true /*mips*/,
            srgb,
            ResourceBindFlags::ShaderResource,
            true /*async*/,
            Bitmap::ImportFlags::None,
            nullptr /*search dirs*/,
            nullptr /*load count*/,
            pMaterial.get()
        );

        // Store assignment to material for later.
        mTextureAssignments.emplace_back(TextureAssignment{ pMaterial, slot, handle });
    }

    void MaterialTextureLoader::assignTextures()
    {
        mTextureManager.waitForAllTexturesLoading();

        // Assign textures to materials.
        for (const auto& assignment : mTextureAssignments)
        {
            auto pTexture = mTextureManager.getTexture(assignment.handle);
            assignment.pMaterial->setTexture(assignment.textureSlot, pTexture);
        }
        mTextureAssignments.clear();
    }
}
